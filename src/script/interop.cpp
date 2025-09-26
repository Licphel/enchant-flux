#include <script/interop.h>
#include <memory>
#include <quickjs.h>
#include <quickjs-libc.h>
#include <vector>
#include <gfx/toolkit.h>
#include <kernel/log.h>
#include <kernel/hio.h>
#include <map>

namespace flux
{

JSRuntime *rt;
JSContext *ctx;

struct dynvalue::_impl
{
    JSValue rawv;
};

void __dump_jsval(JSValueConst val)
{
    prtlog(DEBUG, "====== value print ======");

    if (JS_IsException(val) || JS_IsUndefined(val))
    {
        prtlog(WARN, "value is undefined or an exception");
        return;
    }

    JSPropertyEnum *tab = nullptr;
    uint32_t len = 0;

    if (JS_GetOwnPropertyNames(ctx, &tab, &len, val, JS_GPN_STRING_MASK) < 0)
    {
        prtlog(WARN, "failed to get own property names.");
        return;
    }

    prtlog(DEBUG, "detected {} properties.", len);
    for (uint32_t i = 0; i < len; ++i)
    {
        JSAtom atom = tab[i].atom;
        const char *key = JS_AtomToCString(ctx, atom);
        if (!key)
            continue;

        JSValue v = JS_GetProperty(ctx, val, atom);
        const char *valStr = JS_ToCString(ctx, v);
        prtlog(DEBUG, "| - {} = {}", key, valStr ? valStr : "?");
        JS_FreeCString(ctx, valStr);
        JS_FreeValue(ctx, v);
        JS_FreeCString(ctx, key);
    }
    js_free(ctx, tab);
}

void __prterr()
{
    JSValue exception = JS_GetException(ctx);

    JSValue stack = JS_GetPropertyStr(ctx, exception, "stack");
    if (!JS_IsUndefined(stack))
    {
        const char *stack_str = JS_ToCString(ctx, stack);
        if (stack_str)
        {
            prtlog(FATAL, "[Interop] error stack: {}", stack_str);
            JS_FreeCString(ctx, stack_str);
        }
        JS_FreeValue(ctx, stack);
    }

    JSValue strVal = JS_ToString(ctx, exception);
    const char *utf8 = JS_ToCString(ctx, strVal);
    if (utf8)
    {
        prtlog_throw(FATAL, "[Interop] error: {}", utf8);
        JS_FreeCString(ctx, utf8);
    }

    JS_FreeValue(ctx, exception);
    JS_FreeValue(ctx, strVal);
}

dynvalue __genv(const JSValue &v, __dvdbg_type type)
{
    dynvalue d = dynvalue(std::make_shared<dynvalue::_impl>());
    d.__p->rawv = v;
    d.__type = type;
    return d;
}

dynvalue::dynvalue(shared<_impl> __p) : __p(__p)
{
    JS_FreeValue(ctx, __p->rawv);
}

dynvalue::~dynvalue()
{
    JS_FreeValue(ctx, __p->rawv);
}

dynvalue dynvalue::vint(int v)
{
    return __genv(JS_NewInt32(ctx, v), __IP_INT);
}

dynvalue dynvalue::vdouble(double v)
{
    return __genv(JS_NewFloat64(ctx, v), __IP_DOUBLE);
}

dynvalue dynvalue::vstr(const std::string &v)
{
    return __genv(JS_NewString(ctx, v.c_str()), __IP_STR);
}

dynvalue dynvalue::vbool(bool v)
{
    return __genv(JS_NewBool(ctx, v), __IP_BOOL);
}

int dynvalue::tint()
{
    int i;
    JS_ToInt32(ctx, &i, __p->rawv);
    return i;
}

double dynvalue::tdouble()
{
    double i;
    JS_ToFloat64(ctx, &i, __p->rawv);
    return i;
}

std::string dynvalue::tstr()
{
    const char *cstr = JS_ToCString(ctx, __p->rawv);
    if (!cstr)
        return "";
    std::string s(cstr);
    JS_FreeCString(ctx, cstr);
    return s;
}

bool dynvalue::tbool()
{
    return JS_ToBool(ctx, __p->rawv);
}

dynvalue dynvalue::veval(const std::string &name, const std::string &code)
{
#ifdef MODULIZED
    JSValue func_val =
        JS_Eval(ctx, code.c_str(), code.length(), name.c_str(), JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
#else
    JSValue func_val =
        JS_Eval(ctx, code.c_str(), code.length(), name.c_str(), JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_COMPILE_ONLY);
#endif
    if (JS_IsException(func_val))
    {
        __prterr();
        JS_FreeValue(ctx, func_val);
        return __genv(JS_UNDEFINED, __IP_UNDEF);
    }
#ifdef MODULIZED
    js_module_set_import_meta(ctx, func_val, true, true);
#endif
    JSValue ret = JS_EvalFunction(ctx, func_val);
    if (JS_IsException(ret))
    {
        __prterr();
        JS_FreeValue(ctx, ret);
        return __genv(JS_UNDEFINED, __IP_UNDEF);
    }

    return __genv(ret, __IP_UNKNOWN);
}

dynvalue dynvalue::vgval(const std::string &key)
{
    JSValue obj = JS_GetGlobalObject(ctx);
    JSValue ret = JS_GetPropertyStr(ctx, obj, key.c_str());

    if (JS_IsException(ret))
    {
        __prterr();
        JS_FreeValue(ctx, ret);
        return __genv(JS_UNDEFINED, __IP_UNDEF);
    }
    return __genv(ret, __IP_UNKNOWN);
}

dynvalue dynvalue::vsub(const std::string &key)
{
    JSValue obj = __p->rawv;
    JSValue ret = JS_GetPropertyStr(ctx, obj, key.c_str());

    if (JS_IsException(ret))
    {
        __prterr();
        JS_FreeValue(ctx, ret);
        return __genv(JS_UNDEFINED, __IP_UNDEF);
    }
    return __genv(ret, __IP_UNKNOWN);
}

dynvalue dynvalue::vcall(int argc, ...)
{
    if (!JS_IsFunction(ctx, __p->rawv))
    {
        prtlog_throw(FATAL, "[Interop] calling a non-function dynvalue.");
        return __genv(JS_UNDEFINED, __IP_UNDEF);
    }

    std::vector<JSValue> argv;
    argv.reserve(argc);
    va_list ap;
    va_start(ap, argc);
    for (int i = 0; i < argc; ++i)
    {
        dynvalue *dv = va_arg(ap, dynvalue *);
        if (!dv || !dv->__p)
        {
            argv.push_back(JS_UNDEFINED);
        }
        else
        {
            argv.push_back(JS_DupValue(ctx, dv->__p->rawv));
        }
    }
    va_end(ap);

    JSValue ret = JS_Call(ctx, __p->rawv, JS_UNDEFINED, argc, argv.data());

    for (auto &v : argv)
        JS_FreeValue(ctx, v);

    if (JS_IsException(ret))
    {
        __prterr();
        JS_FreeValue(ctx, ret);
        return __genv(JS_UNDEFINED, __IP_UNDEF);
    }
    return __genv(ret, __IP_UNKNOWN);
}

#ifdef MODULIZED
JSModuleDef *__gen_module_loader(JSContext *ctx, const char *name, void *opaque)
{
    std::string code = read_str(open_local(name));
    if (code.empty())
    {
        prtlog(WARN, "[Interop] cannot load module: {}", name);
        return nullptr;
    }

    JSValue func_val = JS_Eval(ctx, code.c_str(), code.length(), name, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(func_val))
    {
        __prterr();
        JS_FreeValue(ctx, func_val);
        return nullptr;
    }

    js_module_set_import_meta(ctx, func_val, true, 1);

    JSValue ret = JS_EvalFunction(ctx, func_val);
    if (JS_IsException(ret))
    {
        __prterr();
        JS_FreeValue(ctx, ret);
        return nullptr;
    }

    JSModuleDef *m = JS_VALUE_GET_PTR(ret) ? (JSModuleDef *)JS_VALUE_GET_PTR(ret) : nullptr;

    if (!m)
    {
        prtlog_throw(FATAL, "[Interop] failed to get module definition for: {}", name);
        JS_FreeValue(ctx, ret);
        return nullptr;
    }

    return m;
}
#endif

static JSValue js_print(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    for (int i = 0; i < argc; ++i)
    {
        const char *str = JS_ToCString(ctx, argv[i]);
        if (str)
        {
            prtlog(INFO, "{}", str);
            JS_FreeCString(ctx, str);
        }
    }
    return JS_UNDEFINED;
}

void ip_init()
{
    rt = JS_NewRuntime();
    if (!rt)
        return;
    ctx = JS_NewContext(rt);
    if (!ctx)
    {
        JS_FreeRuntime(rt);
        return;
    }

#ifdef MODULIZED
    JS_SetModuleLoaderFunc(rt, nullptr, __gen_module_loader, nullptr);
#endif
    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "print", JS_NewCFunction(ctx, js_print, "print", 1));
    JS_FreeValue(ctx, global);

    tk_hook_event_dispose([]() {
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
    });
}

} // namespace flux
