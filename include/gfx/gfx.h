#pragma once
#include <gfx/image.h>
#include <iostream>
#include <core/hio.h>
#include <core/math.h>
#include <functional>

namespace flux::gfx
{

#define KEY_SPACE 32
#define KEY_APOSTROPHE 39 /* ' */
#define KEY_COMMA 44      /* , */
#define KEY_MINUS 45      /* - */
#define KEY_PERIOD 46     /* . */
#define KEY_SLASH 47      /* / */
#define KEY_0 48
#define KEY_1 49
#define KEY_2 50
#define KEY_3 51
#define KEY_4 52
#define KEY_5 53
#define KEY_6 54
#define KEY_7 55
#define KEY_8 56
#define KEY_9 57
#define KEY_SEMICOLON 59 /* ; */
#define KEY_EQUAL 61     /* = */
#define KEY_A 65
#define KEY_B 66
#define KEY_C 67
#define KEY_D 68
#define KEY_E 69
#define KEY_F 70
#define KEY_G 71
#define KEY_H 72
#define KEY_I 73
#define KEY_J 74
#define KEY_K 75
#define KEY_L 76
#define KEY_M 77
#define KEY_N 78
#define KEY_O 79
#define KEY_P 80
#define KEY_Q 81
#define KEY_R 82
#define KEY_S 83
#define KEY_T 84
#define KEY_U 85
#define KEY_V 86
#define KEY_W 87
#define KEY_X 88
#define KEY_Y 89
#define KEY_Z 90
#define KEY_LEFT_BRACKET 91  /* [ */
#define KEY_BACKSLASH 92     /* \ */
#define KEY_RIGHT_BRACKET 93 /* ] */
#define KEY_GRAVE_ACCENT 96  /* ` */
#define KEY_WORLD_1 161      /* non-US #1 */
#define KEY_WORLD_2 162      /* non-US #2 */
#define KEY_ESCAPE 256
#define KEY_ENTER 257
#define KEY_TAB 258
#define KEY_BACKSPACE 259
#define KEY_INSERT 260
#define KEY_DELETE 261
#define KEY_RIGHT 262
#define KEY_LEFT 263
#define KEY_DOWN 264
#define KEY_UP 265
#define KEY_PAGE_UP 266
#define KEY_PAGE_DOWN 267
#define KEY_HOME 268
#define KEY_END 269
#define KEY_CAPS_LOCK 280
#define KEY_SCROLL_LOCK 281
#define KEY_NUM_LOCK 282
#define KEY_PRINT_SCREEN 283
#define KEY_PAUSE 284
#define KEY_F1 290
#define KEY_F2 291
#define KEY_F3 292
#define KEY_F4 293
#define KEY_F5 294
#define KEY_F6 295
#define KEY_F7 296
#define KEY_F8 297
#define KEY_F9 298
#define KEY_F10 299
#define KEY_F11 300
#define KEY_F12 301
#define KEY_F13 302
#define KEY_F14 303
#define KEY_F15 304
#define KEY_F16 305
#define KEY_F17 306
#define KEY_F18 307
#define KEY_F19 308
#define KEY_F20 309
#define KEY_F21 310
#define KEY_F22 311
#define KEY_F23 312
#define KEY_F24 313
#define KEY_F25 314
#define KEY_KP_0 320
#define KEY_KP_1 321
#define KEY_KP_2 322
#define KEY_KP_3 323
#define KEY_KP_4 324
#define KEY_KP_5 325
#define KEY_KP_6 326
#define KEY_KP_7 327
#define KEY_KP_8 328
#define KEY_KP_9 329
#define KEY_KP_DECIMAL 330
#define KEY_KP_DIVIDE 331
#define KEY_KP_MULTIPLY 332
#define KEY_KP_SUBTRACT 333
#define KEY_KP_ADD 334
#define KEY_KP_ENTER 335
#define KEY_KP_EQUAL 336
#define KEY_LEFT_SHIFT 340
#define KEY_LEFT_CONTROL 341
#define KEY_LEFT_ALT 342
#define KEY_LEFT_SUPER 343
#define KEY_RIGHT_SHIFT 344
#define KEY_RIGHT_CONTROL 345
#define KEY_RIGHT_ALT 346
#define KEY_RIGHT_SUPER 347
#define KEY_MENU 348

#define _TK_MOUSE_OFFSET = 480;
#define MOUSE_BUTTON_1 0 + _TK_MOUSE_OFFSET
#define MOUSE_BUTTON_2 1 + _TK_MOUSE_OFFSET
#define MOUSE_BUTTON_3 2 + _TK_MOUSE_OFFSET
#define MOUSE_BUTTON_4 3 + _TK_MOUSE_OFFSET
#define MOUSE_BUTTON_5 4 + _TK_MOUSE_OFFSET
#define MOUSE_BUTTON_6 5 + _TK_MOUSE_OFFSET
#define MOUSE_BUTTON_7 6 + _TK_MOUSE_OFFSET
#define MOUSE_BUTTON_8 7 + _TK_MOUSE_OFFSET
#define MOUSE_BUTTON_LEFT MOUSE_BUTTON_1
#define MOUSE_BUTTON_RIGHT MOUSE_BUTTON_2
#define MOUSE_BUTTON_MIDDLE MOUSE_BUTTON_3

#define MOD_ANY 0x0000
#define MOD_SHIFT 0x0001
#define MOD_CONTROL 0x0002
#define MOD_ALT 0x0004
#define MOD_SUPER 0x0008
#define MOD_CAPS_LOCK 0x0010
#define MOD_NUM_LOCK 0x0020

#define SCROLL_NO 0
#define SCROLL_UP 1
#define SCROLL_DOWN 2

// pre-declaration
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
void tk_end_make_handle();
std::string tk_get_title();
vec2 tk_get_size();
vec2 tk_get_pos();
vec2 tk_get_device_size();

void tk_lifecycle(int fps, int tps, bool vsync);

void tk_swap_buffers();
bool tk_poll_events();

void tk_hook_event_tick(std::function<void(double delta)> callback);
void tk_hook_event_render(std::function<void(brush *brush, double partial)> callback);
void tk_hook_event_dispose(std::function<void()> callback);
void tk_hook_event_resize(std::function<void(int w, int h)> callback);
void tk_hook_mouse_state(std::function<void(int button, int action, int mods)> callback);
void tk_hook_cursor_pos(std::function<void(double x, double y)> callback);
void tk_hook_key_state(std::function<void(int button, int scancode, int action, int mods)> callback);

bool tk_key_held(int key, int mod = MOD_ANY);
bool tk_key_press(int key, int mod = MOD_ANY);
vec2 tk_get_cursor();
double tk_get_scroll();
int tk_get_scroll_towards();
std::string tk_consume_chars();

} // namespace flux::gfx
