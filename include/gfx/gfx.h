#pragma once
#include <gfx/image.h>
#include <iostream>
#include <core/hio.h>
#include <core/math.h>
#include <functional>

#define FX_KEY_SPACE 32
#define FX_KEY_APOSTROPHE 39 /* ' */
#define FX_KEY_COMMA 44      /* , */
#define FX_KEY_MINUS 45      /* - */
#define FX_KEY_PERIOD 46     /* . */
#define FX_KEY_SLASH 47      /* / */
#define FX_KEY_0 48
#define FX_KEY_1 49
#define FX_KEY_2 50
#define FX_KEY_3 51
#define FX_KEY_4 52
#define FX_KEY_5 53
#define FX_KEY_6 54
#define FX_KEY_7 55
#define FX_KEY_8 56
#define FX_KEY_9 57
#define FX_KEY_SEMICOLON 59 /* ; */
#define FX_KEY_EQUAL 61     /* = */
#define FX_KEY_A 65
#define FX_KEY_B 66
#define FX_KEY_C 67
#define FX_KEY_D 68
#define FX_KEY_E 69
#define FX_KEY_F 70
#define FX_KEY_G 71
#define FX_KEY_H 72
#define FX_KEY_I 73
#define FX_KEY_J 74
#define FX_KEY_K 75
#define FX_KEY_L 76
#define FX_KEY_M 77
#define FX_KEY_N 78
#define FX_KEY_O 79
#define FX_KEY_P 80
#define FX_KEY_Q 81
#define FX_KEY_R 82
#define FX_KEY_S 83
#define FX_KEY_T 84
#define FX_KEY_U 85
#define FX_KEY_V 86
#define FX_KEY_W 87
#define FX_KEY_X 88
#define FX_KEY_Y 89
#define FX_KEY_Z 90
#define FX_KEY_LEFT_BRACKET 91  /* [ */
#define FX_KEY_BACKSLASH 92     /* \ */
#define FX_KEY_RIGHT_BRACKET 93 /* ] */
#define FX_KEY_GRAVE_ACCENT 96  /* ` */
#define FX_KEY_WORLD_1 161      /* non-US #1 */
#define FX_KEY_WORLD_2 162      /* non-US #2 */
#define FX_KEY_ESCAPE 256
#define FX_KEY_ENTER 257
#define FX_KEY_TAB 258
#define FX_KEY_BACKSPACE 259
#define FX_KEY_INSERT 260
#define FX_KEY_DELETE 261
#define FX_KEY_RIGHT 262
#define FX_KEY_LEFT 263
#define FX_KEY_DOWN 264
#define FX_KEY_UP 265
#define FX_KEY_PAGE_UP 266
#define FX_KEY_PAGE_DOWN 267
#define FX_KEY_HOME 268
#define FX_KEY_END 269
#define FX_KEY_CAPS_LOCK 280
#define FX_KEY_SCROLL_LOCK 281
#define FX_KEY_NUM_LOCK 282
#define FX_KEY_PRINT_SCREEN 283
#define FX_KEY_PAUSE 284
#define FX_KEY_F1 290
#define FX_KEY_F2 291
#define FX_KEY_F3 292
#define FX_KEY_F4 293
#define FX_KEY_F5 294
#define FX_KEY_F6 295
#define FX_KEY_F7 296
#define FX_KEY_F8 297
#define FX_KEY_F9 298
#define FX_KEY_F10 299
#define FX_KEY_F11 300
#define FX_KEY_F12 301
#define FX_KEY_F13 302
#define FX_KEY_F14 303
#define FX_KEY_F15 304
#define FX_KEY_F16 305
#define FX_KEY_F17 306
#define FX_KEY_F18 307
#define FX_KEY_F19 308
#define FX_KEY_F20 309
#define FX_KEY_F21 310
#define FX_KEY_F22 311
#define FX_KEY_F23 312
#define FX_KEY_F24 313
#define FX_KEY_F25 314
#define FX_KEY_KP_0 320
#define FX_KEY_KP_1 321
#define FX_KEY_KP_2 322
#define FX_KEY_KP_3 323
#define FX_KEY_KP_4 324
#define FX_KEY_KP_5 325
#define FX_KEY_KP_6 326
#define FX_KEY_KP_7 327
#define FX_KEY_KP_8 328
#define FX_KEY_KP_9 329
#define FX_KEY_KP_DECIMAL 330
#define FX_KEY_KP_DIVIDE 331
#define FX_KEY_KP_MULTIPLY 332
#define FX_KEY_KP_SUBTRACT 333
#define FX_KEY_KP_ADD 334
#define FX_KEY_KP_ENTER 335
#define FX_KEY_KP_EQUAL 336
#define FX_KEY_LEFT_SHIFT 340
#define FX_KEY_LEFT_CONTROL 341
#define FX_KEY_LEFT_ALT 342
#define FX_KEY_LEFT_SUPER 343
#define FX_KEY_RIGHT_SHIFT 344
#define FX_KEY_RIGHT_CONTROL 345
#define FX_KEY_RIGHT_ALT 346
#define FX_KEY_RIGHT_SUPER 347
#define FX_KEY_MENU 348

#define _TK_MOUSE_OFFSET 480
#define FX_MOUSE_BUTTON_1 (0 + _TK_MOUSE_OFFSET)
#define FX_MOUSE_BUTTON_2 (1 + _TK_MOUSE_OFFSET)
#define FX_MOUSE_BUTTON_3 (2 + _TK_MOUSE_OFFSET)
#define FX_MOUSE_BUTTON_4 (3 + _TK_MOUSE_OFFSET)
#define FX_MOUSE_BUTTON_5 (4 + _TK_MOUSE_OFFSET)
#define FX_MOUSE_BUTTON_6 (5 + _TK_MOUSE_OFFSET)
#define FX_MOUSE_BUTTON_7 (6 + _TK_MOUSE_OFFSET)
#define FX_MOUSE_BUTTON_8 (7 + _TK_MOUSE_OFFSET)
#define FX_MOUSE_BUTTON_LEFT FX_MOUSE_BUTTON_1
#define FX_MOUSE_BUTTON_RIGHT FX_MOUSE_BUTTON_2
#define FX_MOUSE_BUTTON_MIDDLE FX_MOUSE_BUTTON_3

#define FX_MOD_ANY 0x0000
#define FX_MOD_SHIFT 0x0001
#define FX_MOD_CONTROL 0x0002
#define FX_MOD_ALT 0x0004
#define FX_MOD_SUPER 0x0008
#define FX_MOD_CAPS_LOCK 0x0010
#define FX_MOD_NUM_LOCK 0x0020

#define FX_SCROLL_NO 0
#define FX_SCROLL_UP 1
#define FX_SCROLL_DOWN 2

namespace flux::gfx
{

struct brush;

double tk_nanos();
double tk_seconds();
int tk_ticks();
double tk_partial();
double tk_delta();
double tk_lerp(double old, double now);
int tk_real_fps();
int tk_real_tps();

void tk_make_handle();
void tk_title(const std::string &title);
void tk_size(vec2 size);
void tk_pos(vec2 pos);
void tk_visible(bool visible);
void tk_maximize();
void tk_icon(shared<image> img);
void tk_cursor(shared<image> img, vec2 hotspot);
void tk_end_make_handle();
std::string tk_get_title();
vec2 tk_get_size();
vec2 tk_get_pos();
vec2 tk_get_device_size();

// begins the main loop (blocks the current thread).
// fps, if negative, means uncapped.
void tk_lifecycle(int fps, int tps, bool vsync);

void tk_swap_buffers();
bool tk_poll_events();

void tk_hook_event_tick(std::function<void()> callback);
void tk_hook_event_render(std::function<void(brush *brush)> callback);
void tk_hook_event_dispose(std::function<void()> callback);
void tk_hook_event_resize(std::function<void(int w, int h)> callback);
void tk_hook_mouse_state(std::function<void(int button, int action, int mods)> callback);
void tk_hook_cursor_pos(std::function<void(double x, double y)> callback);
void tk_hook_key_state(std::function<void(int button, int scancode, int action, int mods)> callback);

bool tk_key_held(int key, int mod = FX_MOD_ANY);
bool tk_key_press(int key, int mod = FX_MOD_ANY);
vec2 tk_get_cursor();
// consumes an abs value of scroll.
double tk_consume_scroll();
// returns FX_SCROLL_UP, FX_SCROLL_DOWN or FX_SCROLL_NO
int tk_get_scroll_towards();
// consume the typed characters since last call.
std::string tk_consume_chars();

} // namespace flux::gfx
