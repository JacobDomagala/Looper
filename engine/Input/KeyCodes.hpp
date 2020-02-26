// #pragma once

// #include <SDL_events.h>
// #include <string>

// namespace noc::input
// {

// enum class NOC_KEY : uint32_t
// {
// 	UNKNOWN      = SDLK_UNKNOWN,
// 	RETURN       = SDLK_RETURN,
// 	ESCAPE       = SDLK_ESCAPE,
// 	BACKSPACE    = SDLK_BACKSPACE,
// 	TAB          = SDLK_TAB,
// 	SPACE        = SDLK_SPACE,
// 	EXCLAIM      = SDLK_EXCLAIM,
// 	QUOTEDBL     = SDLK_QUOTEDBL,
// 	HASH         = SDLK_HASH,
// 	PERCENT      = SDLK_PERCENT,
// 	DOLLAR       = SDLK_DOLLAR,
// 	AMPERSAND    = SDLK_AMPERSAND,
// 	QUOTE        = SDLK_QUOTE,
// 	LEFTPAREN    = SDLK_LEFTPAREN,
// 	RIGHTPAREN   = SDLK_RIGHTPAREN,
// 	ASTERISK     = SDLK_ASTERISK,
// 	PLUS         = SDLK_PLUS,
// 	COMMA        = SDLK_COMMA,
// 	MINUS        = SDLK_MINUS,
// 	PERIOD       = SDLK_PERIOD,
// 	SLASH        = SDLK_SLASH,

// 	NUM_0        = SDLK_0,
// 	NUM_1        = SDLK_1,
// 	NUM_2        = SDLK_2,
// 	NUM_3        = SDLK_3,
// 	NUM_4        = SDLK_4,
// 	NUM_5        = SDLK_5,
// 	NUM_6        = SDLK_6,
// 	NUM_7        = SDLK_7,
// 	NUM_8        = SDLK_8,
// 	NUM_9        = SDLK_9,

// 	COLON        = SDLK_COLON,
// 	SEMICOLON    = SDLK_SEMICOLON,
// 	LESS         = SDLK_LESS,
// 	EQUALS       = SDLK_EQUALS,
// 	GREATER      = SDLK_GREATER,
// 	QUESTION     = SDLK_QUESTION,
// 	AT           = SDLK_AT,
// 	LEFTBRACKET  = SDLK_LEFTBRACKET,
// 	BACKSLASH    = SDLK_BACKSLASH,
// 	RIGHTBRACKET = SDLK_RIGHTBRACKET,
// 	CARET        = SDLK_CARET,
// 	UNDERSCORE   = SDLK_UNDERSCORE,
// 	BACKQUOTE    = SDLK_BACKQUOTE,

// 	A            = SDLK_a,
// 	B            = SDLK_b,
// 	C            = SDLK_c,
// 	D            = SDLK_d,
// 	E            = SDLK_e,
// 	F            = SDLK_f,
// 	G            = SDLK_g,
// 	H            = SDLK_h,
// 	I            = SDLK_i,
// 	J            = SDLK_j,
// 	K            = SDLK_k,
// 	L            = SDLK_l,
// 	M            = SDLK_m,
// 	N            = SDLK_n,
// 	O            = SDLK_o,
// 	P            = SDLK_p,
// 	Q            = SDLK_q,
// 	R            = SDLK_r,
// 	S            = SDLK_s,
// 	T            = SDLK_t,
// 	U            = SDLK_u,
// 	V            = SDLK_v,
// 	W            = SDLK_w,
// 	X            = SDLK_x,
// 	Y            = SDLK_y,
// 	Z            = SDLK_z,

// 	CAPSLOCK = SDLK_CAPSLOCK,

// 	F1  = SDLK_F1,
// 	F2  = SDLK_F2,
// 	F3  = SDLK_F3,
// 	F4  = SDLK_F4,
// 	F5  = SDLK_F5,
// 	F6  = SDLK_F6,
// 	F7  = SDLK_F7,
// 	F8  = SDLK_F8,
// 	F9  = SDLK_F9,
// 	F10 = SDLK_F10,
// 	F11 = SDLK_F11,
// 	F12 = SDLK_F12,

// 	PRINTSCREEN = SDLK_PRINTSCREEN,
// 	SCROLLLOCK  = SDLK_SCROLLLOCK,
// 	PAUSE       = SDLK_PAUSE,
// 	INSERT      = SDLK_INSERT,
// 	HOME        = SDLK_HOME,
// 	PAGEUP      = SDLK_PAGEUP,
// 	DELETE      = SDLK_DELETE,
// 	END         = SDLK_END,
// 	PAGEDOWN    = SDLK_PAGEDOWN,
// 	RIGHT       = SDLK_RIGHT,
// 	LEFT        = SDLK_LEFT,
// 	DOWN        = SDLK_DOWN,
// 	UP          = SDLK_UP,

// 	NUMLOCKCLEAR = SDLK_NUMLOCKCLEAR,

// 	LCTRL  = SDLK_LCTRL,
// 	LSHIFT = SDLK_LSHIFT,
// 	LALT   = SDLK_LALT,
// 	RCTRL  = SDLK_RCTRL,
// 	RSHIFT = SDLK_RSHIFT,
// 	RALT   = SDLK_RALT
// };

// enum class NOC_Keymod
// {
// 	NONE     = KMOD_NONE,
// 	LSHIFT   = KMOD_LSHIFT,
// 	RSHIFT   = KMOD_RSHIFT,
// 	LCTRL    = KMOD_LCTRL,
// 	RCTRL    = KMOD_RCTRL,
// 	LALT     = KMOD_LALT,
// 	RALT     = KMOD_RALT,
// 	LGUI     = KMOD_LGUI,
// 	RGUI     = KMOD_RGUI,
// 	NUM      = KMOD_NUM,
// 	CAPS     = KMOD_CAPS,
// 	MODE     = KMOD_MODE,
// 	RESERVED = KMOD_RESERVED,

// 	CTRL = KMOD_CTRL,
// 	SHIFT = KMOD_SHIFT,
// 	ALT = KMOD_ALT,
// 	GUI = KMOD_GUI
// };

// enum class NOC_KEY_STATE
// {
// 	PRESSED = 0,
// 	RELEASED
// };

// }   // namespace noc::input
