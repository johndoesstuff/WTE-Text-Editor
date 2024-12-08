#include "tabUtil.h"
#include <string.h>
#include <stdio.h>
#include <ncurses.h>

size_t tab_strlen(const char *str, int tabOffset) { //number of displayed columns for a string accounting for tabs
	size_t len = 0;
	for (int i = 0; i < strlen(str); i++) { //iterate each character in string
		char ch = str[i];
		if (ch == '\t') {
			int nextTabStop = (((len - tabOffset) / TAB_WIDTH) + 1) * TAB_WIDTH + tabOffset;
			len = nextTabStop;
		} else {
			len++;
		}
	}
	return len;
}

size_t tab_strlenTo(const char *str, int tabOffset, int col) { //number of actual characters until a displayed column
	size_t len = 0;
	int i;
	for (i = 0; str[i] != '\0'; i++) { //iterate each character in string
		if (len >= col) break;
		char ch = str[i];
		if (ch == '\t') {
			int nextTabStop = (((len - tabOffset) / TAB_WIDTH) + 1) * TAB_WIDTH + tabOffset;
			len = nextTabStop;
		} else {
			len++;
		}
	}
	return i;
}

size_t tab_strlenFrom(const char *str, int tabOffset, int col) { //number of displayed columns for a string accounting for tabs
	size_t len = 0;
	for (int i = 0; i < col; i++) { //iterate each character in string
		char ch = str[i];
		if (ch == '\t') {
			int nextTabStop = (((len - tabOffset) / TAB_WIDTH) + 1) * TAB_WIDTH + tabOffset;
			len = nextTabStop;
		} else {
			len++;
		}
	}
	return len;
}

void tab_mvprintw(int y, int x, int tabOffset, const char *format, ...) {
	va_list args;
	va_start(args, format);
	char formattedStr[1024];
	vsnprintf(formattedStr, sizeof(formattedStr), format, args);
	va_end(args);

	int currentX = x;
	for (int i = 0; formattedStr[i] != '\0'; i++) {
		if (formattedStr[i] == '\t') {
			int nextTabStop = (((currentX - tabOffset) / TAB_WIDTH) + 1) * TAB_WIDTH + tabOffset;
			currentX = nextTabStop;
		} else {
			mvaddch(y, currentX++, formattedStr[i]);
		}
	}
}
