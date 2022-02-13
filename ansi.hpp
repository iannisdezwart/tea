#ifndef TEA_ANSI_HEADER
#define TEA_ANSI_HEADER

#include <bits/stdc++.h>

/*
	Terminal output sequences
*/

#define ANSI_CURSOR_UP(n)        "\x1b[" #n "A"
#define ANSI_CURSOR_DOWN(n)      "\x1b[" #n "B"
#define ANSI_CURSOR_FORWARD(n)   "\x1b[" #n "C"
#define ANSI_CURSOR_BACK(n)      "\x1b[" #n "D"
#define ANSI_CURSOR_NEXT_LINE(n) "\x1b[" #n "E"
#define ANSI_CURSOR_PREV_LINE(n) "\x1b[" #n "F"
#define ANSI_CURSOR_TO_COL(n)    "\x1b[" #n "G"
#define ANSI_CURSOR_TO(x, y)     "\x1b[" #x ";" #y "H"
#define ANSI_ERASE_DISPLAY(n)    "\x1b[" #n "J"
#define ANSI_ERASE_LINE(n)       "\x1b[" #n "K"
#define ANSI_SCROLL_UP(n)        "\x1b[" #n "S"
#define ANSI_SCROLL_DOWN(n)      "\x1b[" #n "T"

void
ansi_cursor_up(size_t n)
{
	printf("\x1b[%luA", n);
}
void
ansi_cursor_down(size_t n)
{
	printf("\x1b[%luB", n);
}
void
ansi_cursor_forward(size_t n)
{
	printf("\x1b[%luC", n);
}
void
ansi_cursor_back(size_t n)
{
	printf("\x1b[%luD", n);
}
void
ansi_cursor_next_line(size_t n)
{
	printf("\x1b[%luE", n);
}
void
ansi_cursor_prev_line(size_t n)
{
	printf("\x1b[%luF", n);
}
void
ansi_cursor_to_col(size_t n)
{
	printf("\x1b[%luG", n);
}
void
ansi_cursor_to(size_t x, size_t y)
{
	printf("\x1b[%lu;%luH", x, y);
}
void
ansi_erase_display(size_t n)
{
	printf("\x1b[J");
}
void
ansi_erase_line(size_t n)
{
	printf("\x1b[K");
}
void
ansi_scroll_up(size_t n)
{
	printf("\x1b[%luS", n);
}
void
ansi_scroll_down(size_t n)
{
	printf("\x1b[%luT", n);
}

/*
	Rendition
*/

// Basic SGR

#define ANSI_SGR(n) "\x1b[" #n "m"

// SGR effects

#define ANSI_RESET     ANSI_SGR(0)
#define ANSI_BOLD      ANSI_SGR(1)
#define ANSI_FAINT     ANSI_SGR(2)
#define ANSI_ITALIC    ANSI_SGR(3)
#define ANSI_UNDERLINE ANSI_SGR(4)
#define ANSI_BLINK     ANSI_SGR(5)
#define ANSI_REVERSE   ANSI_SGR(7)
#define ANSI_CROSSED   ANSI_SGR(9)

// SGR disable effects

#define ANSI_DOUBLE_UNDERLINE  ANSI_SGR(21)
#define ANSI_DISABLE_BOLD      ANSI_SGR(22)
#define ANSI_DISABLE_FAINT     ANSI_DISABLE_BOLD
#define ANSI_DISABLE_ITALIC    ANSI_SGR(23)
#define ANSI_DISABLE_UNDERLINE ANSI_SGR(24)
#define ANSI_DISABLE_BLINK     ANSI_SGR(25)
#define ANSI_DISABLE_REVERSE   ANSI_SGR(27)
#define ANSI_DISABLE_CROSSED   ANSI_SGR(29)

#define ANSI_DEFAULT_FG ANSI_SGR(39)
#define ANSI_DEFAULT_BG ANSI_SGR(49)

// Default foreground colours

#define ANSI_BLACK   ANSI_SGR(30)
#define ANSI_RED     ANSI_SGR(31)
#define ANSI_GREEN   ANSI_SGR(32)
#define ANSI_YELLOW  ANSI_SGR(33)
#define ANSI_BLUE    ANSI_SGR(34)
#define ANSI_MAGENTA ANSI_SGR(35)
#define ANSI_CYAN    ANSI_SGR(36)
#define ANSI_WHITE   ANSI_SGR(37)

// Default background colours

#define ANSI_BLACK_BG   ANSI_SGR(40)
#define ANSI_RED_BG     ANSI_SGR(41)
#define ANSI_GREEN_BG   ANSI_SGR(42)
#define ANSI_YELLOW_BG  ANSI_SGR(43)
#define ANSI_BLUE_BG    ANSI_SGR(44)
#define ANSI_MAGENTA_BG ANSI_SGR(45)
#define ANSI_CYAN_BG    ANSI_SGR(46)
#define ANSI_WHITE_BG   ANSI_SGR(47)

// Bright foreground colours

#define ANSI_BRIGHT_BLACK   ANSI_SGR(90)
#define ANSI_BRIGHT_RED     ANSI_SGR(91)
#define ANSI_BRIGHT_GREEN   ANSI_SGR(92)
#define ANSI_BRIGHT_YELLOW  ANSI_SGR(93)
#define ANSI_BRIGHT_BLUE    ANSI_SGR(94)
#define ANSI_BRIGHT_MAGENTA ANSI_SGR(95)
#define ANSI_BRIGHT_CYAN    ANSI_SGR(96)
#define ANSI_BRIGHT_WHITE   ANSI_SGR(97)

// Bright background colours

#define ANSI_BRIGHT_BLACK_BG   ANSI_SGR(100)
#define ANSI_BRIGHT_RED_BG     ANSI_SGR(101)
#define ANSI_BRIGHT_GREEN_BG   ANSI_SGR(102)
#define ANSI_BRIGHT_YELLOW_BG  ANSI_SGR(103)
#define ANSI_BRIGHT_BLUE_BG    ANSI_SGR(104)
#define ANSI_BRIGHT_MAGENTA_BG ANSI_SGR(105)
#define ANSI_BRIGHT_CYAN_BG    ANSI_SGR(106)
#define ANSI_BRIGHT_WHITE_BG   ANSI_SGR(107)

// 24-bit colour

#define ANSI_SET_COLOUR(r, g, b)    "\x1b[38;2;" #r ";" #g ";" #b "m"
#define ANSI_SET_BG_COLOUR(r, g, b) "\x1b[48;2;" #r ";" #g ";" #b "m"

#endif