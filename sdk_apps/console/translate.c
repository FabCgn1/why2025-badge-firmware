#include <badgevms/event.h>
#include <stdio.h>

#define SCANMAP									\
	E(ESCAPE,		'\033',	-1,		-1)			\
	E(BACKSPACE,	'\b',	-1,		-1)			\
												\
	E(GRAVE,		'`',	'~',	-1)			\
	E(1,			'1',	'!',	-1)			\
	E(2,			'2',	'@',	-1)			\
	E(3,			'3',	'#',	-1)			\
	E(4,			'4',	'$',	-1)			\
	E(5,			'5',	'%',	-1)			\
	E(6,			'6',	'^',	-1)			\
	E(7,			'7',	'&',	-1)			\
	E(8,			'8',	'*',	-1)			\
	E(9,			'9',	'(',	-1)			\
	E(0,			'0',	')',	-1)			\
	E(MINUS,		'-',	'_',	-1)			\
	E(EQUALS,		'=',	'+',	-1)			\
												\
	E(TAB,			'\t',	-1,		-1)			\
	E(Q,			'q',	'Q',	'Q' - 0x40)	\
	E(W,			'w',	'W',	'W' - 0x40)	\
	E(E,			'e',	'E',	'E' - 0x40)	\
	E(R,			'r',	'R',	'R' - 0x40)	\
	E(T,			't',	'T',	'T' - 0x40)	\
	E(Y,			'y',	'Y',	'Y' - 0x40)	\
	E(U,			'u',	'U',	'U' - 0x40)	\
	E(I,			'i',	'I',	'I' - 0x40)	\
	E(O,			'o',	'O',	'O' - 0x40)	\
	E(P,			'p',	'P',	'P' - 0x40)	\
	E(LEFTBRACKET,	'[',	'{',	']' - 0x40)	\
	E(RIGHTBRACKET,']',	'}',	']' - 0x40)		\
	E(BACKSLASH,	'\\',	'|',	'\\' - 0x40)\
												\
	E(CAPSLOCK,	-1,		-1,		-1)				\
	E(A,			'a',	'A',	'A' - 0x40)	\
	E(S,			's',	'S',	'S' - 0x40)	\
	E(D,			'd',	'D',	'D' - 0x40)	\
	E(F,			'f',	'F',	'F' - 0x40)	\
	E(G,			'g',	'G',	'G' - 0x40)	\
	E(H,			'h',	'H',	'H' - 0x40)	\
	E(J,			'j',	'J',	'J' - 0x40)	\
	E(K,			'k',	'K',	'K' - 0x40)	\
	E(L,			'l',	'L',	'L' - 0x40)	\
	E(SEMICOLON,	';',	':',	-1)			\
	E(APOSTROPHE,	'\'',	'"',	-1)			\
	E(RETURN,		'\n',	-1,		-1)			\
												\
	E(Z,			'z',	'Z',	'Z' - 0x40)	\
	E(X,			'x',	'X',	'X' - 0x40)	\
	E(C,			'c',	'C',	'C' - 0x40)	\
	E(V,			'v',	'V',	'V' - 0x40)	\
	E(B,			'b',	'B',	'B' - 0x40)	\
	E(N,			'n',	'N',	'N' - 0x40)	\
	E(M,			'm',	'M',	'M' - 0x40)	\
	E(COMMA,		',',	'<',	-1)			\
	E(PERIOD,		'.',	'>',	-1)			\
	E(SLASH,		'/',	'?',	-1)			\
												\
	E(NONUSHASH,	'\\',	'|',	-1)			\
	E(NONUSBACKSLASH,'\\',	'|',	-1)			\
	E(SPACE,		' ',	' ',	-1)			\

static int select_key (key_mod_t mod, int normal, int shift, int ctrl)
{
	if ((mod & BADGEVMS_KMOD_CTRL) != 0)
		return ctrl;

	if ((mod & BADGEVMS_KMOD_SHIFT) != 0)
		return shift;

	return normal;
}

int translate_scancode (keyboard_event_t *e)
{
	switch (e->scancode) {
#define E(code, normal, shift, ctrl) case KEY_SCANCODE_##code: return select_key (e->mod, normal, shift, ctrl);
SCANMAP
#undef E
	default:
		printf ("translate_scancode(): unhandled scancode: 0x%02x\n", e->scancode);
		return -1;
	}
}

/* vim: set ts=4 sw=4 noet: */
