#ifndef KEYPRESS_HEADER
#define KEYPRESS_HEADER

#include <bits/stdc++.h>
#include <termios.h>
#include <unistd.h>

namespace keypress
{
struct termios saved_termios;

void
save_termios()
{
	tcgetattr(STDIN_FILENO, &saved_termios);
}

void
start_getch_mode()
{
	struct termios new_termios = saved_termios;
	new_termios.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void
stop_getch_mode()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
}

int
getch()
{
	char ch = getchar();
	return ch;
}

enum SpecialKeys : uint8_t
{
	CTRL_BACKSPACE = 8,
	TAB            = 9,
	ENTER          = 10,
	DELETE         = 21,
	SPACE          = 32,
	BACKSPACE      = 127,

	// Extension keys

	ARROW_UP = 128,
	ARROW_DOWN,
	ARROW_RIGHT,
	ARROW_LEFT,

	// Compound arrows

	SHIFT_ARROW_UP,
	SHIFT_ARROW_DOWN,
	SHIFT_ARROW_RIGHT,
	SHIFT_ARROW_LEFT,
	ALT_ARROW_UP,
	ALT_ARROW_DOWN,
	ALT_ARROW_RIGHT,
	ALT_ARROW_LEFT,
	SHIFT_ALT_ARROW_UP,
	SHIFT_ALT_ARROW_DOWN,
	SHIFT_ALT_ARROW_RIGHT,
	SHIFT_ALT_ARROW_LEFT,
	CTRL_ARROW_UP,
	CTRL_ARROW_DOWN,
	CTRL_ARROW_RIGHT,
	CTRL_ARROW_LEFT,
	CTRL_SHIFT_ARROW_UP,
	CTRL_SHIFT_ARROW_DOWN,
	CTRL_SHIFT_ARROW_RIGHT,
	CTRL_SHIFT_ARROW_LEFT,
	CTRL_ALT_ARROW_UP,
	CTRL_ALT_ARROW_DOWN,
	CTRL_ALT_ARROW_RIGHT,
	CTRL_ALT_ARROW_LEFT,

	// Other compound keys

	SHIFT_TAB,
	ALT_BACKSPACE,
	CTRL_ALT_BACKSPACE,
	ALT_ENTER,

	// Compound delete keys

	SHIFT_DELETE,
	SHIFT_ALT_DELETE,
	CTRL_DELETE,
	CTRL_SHIFT_DELETE,
	CTRL_ALT_SHIFT_DELETE,

	UNKNOWN = 255
};

uint8_t
scan_compound_arrow_key()
{
	int fourth = getch();
	if (fourth != ';')
		return UNKNOWN;

	int fifth = getch();
	uint8_t kind;

	switch (fifth)
	{
	case '2':
		kind = 1;
		break;

	case '3':
		kind = 2;
		break;

	case '4':
		kind = 3;
		break;

	case '5':
		kind = 4;
		break;

	case '6':
		kind = 5;
		break;
	case '7':

		kind = 6;
		break;
	}

	int sixth = getch();

	switch (sixth)
	{
	case 'A':
		return ARROW_UP + kind * 4;
	case 'B':
		return ARROW_DOWN + kind * 4;
	case 'C':
		return ARROW_RIGHT + kind * 4;
	case 'D':
		return ARROW_LEFT + kind * 4;
	default:
		return UNKNOWN;
	}
}

uint8_t
scan_delete()
{
	int fourth = getch();

	if (fourth == '~')
		return DELETE;
	else if (fourth == ';')
		goto compound_delete;
	else
		return UNKNOWN;

compound_delete:

	int fifth = getch();
	int kind;

	switch (fifth)
	{
	case '2':
		kind = 0;
		break;

	case '3':
		kind = 1;
		break;

	case '4':
		kind = 2;
		break;

	case '5':
		kind = 3;
		break;

	case '6':
		kind = 4;
		break;

	case '8':
		kind = 5;
		break;
	}

	uint8_t sixth = getch();

	if (sixth == '~')
		return SHIFT_DELETE + kind;
	return UNKNOWN;
}

uint8_t
get_key()
{
	int first = getch();
	if (first != '\e')
		return first;

	// Escape sequence

	int second = getch();

	switch (second)
	{
	case '[':
		goto CSI;
	case BACKSPACE:
		return ALT_BACKSPACE;
	case CTRL_BACKSPACE:
		return CTRL_ALT_BACKSPACE;
	case ENTER:
		return ALT_ENTER;
	default:
		return UNKNOWN;
	}

CSI:

	int third = getch();

	switch (third)
	{
	case 'A':
		return ARROW_UP;
	case 'B':
		return ARROW_DOWN;
	case 'C':
		return ARROW_RIGHT;
	case 'D':
		return ARROW_LEFT;
	case 'Z':
		return SHIFT_TAB;
	case '1':
		return scan_compound_arrow_key();
	case '3':
		return scan_delete();
	default:
		return UNKNOWN;
	}
}
}; // namespace keypress

#endif