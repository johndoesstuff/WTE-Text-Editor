#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <math.h>
#include "constants.h"

int max(int a, int b) {
	return a > b ? a : b;
}

int min(int a, int b) {
	return a < b ? a : b;
}

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

char** readFileToCharArray(FILE* file, int* numLines, int* numChars) {
	int charCount = 0;
	int linesCount = 0;
	int lineCount = 0;
	
	size_t linesSize = LINES_BUFFER_MIN;
	char** lines = malloc(linesSize * sizeof(char*));
	size_t lineSize = LINE_BUFFER_MIN;
	char* line = malloc(lineSize * sizeof(char));

	int ch;
	while ((ch = fgetc(file)) || 1) { //read file character by character (avoid max line length)
		int newLine = ch == '\n';
		int eof = ch == EOF;
		if (lineSize - 1 <= lineCount) { //resize line
			lineSize *= 2;
			line = realloc(line, lineSize * sizeof(char));
		}

		line[lineCount] = ch; //add char
		lineCount++;

		if (newLine || eof) { //increment lines buffers
			line[lineCount] = '\0'; //terminate line
			if (linesSize - 1 <= linesCount) { //resize lines
				linesSize *= 2;
				lines = realloc(lines, linesSize * sizeof(char*));
			}
			
			lines[linesCount] = line;
			linesCount++;
			
			if (eof) break;

			lineSize = LINE_BUFFER_MIN;
			line = malloc(lineSize * sizeof(char)); //start new line
			charCount += lineCount;
			lineCount = 0;
		}
	}
	*numLines = linesCount - 1;
	*numChars = charCount - 1;
	return lines;
}

int main(int argc, char *argv[]) {

	//find file
	char *targetFileName = argv[1];
	FILE *targetFile = fopen(targetFileName, "r+"); //open file for reading/writing
	if (targetFile == NULL) {
		printf("Unable to locate file %s", targetFileName);
		return 0;
	}
	
	//read file
	int linesCount;
	int charsCount;
	char **fileContents = readFileToCharArray(targetFile, &linesCount, &charsCount);

	//initialize ncurses
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, 1);
	curs_set(0);

	//initialize colors
	int colorMode = has_colors();
	if (colorMode) {
		start_color();
		init_pair(COLOR_BW, COLOR_BLACK, COLOR_WHITE);
		init_pair(COLOR_GRAY, 8, COLOR_BLACK);
	}
	
	//initialize screen info
	int screenWidth, screenHeight;
	int line = 0;
	int goalCol = 0;
	int colDisp = 0;
	int colActual = 0;
	getmaxyx(stdscr, screenHeight, screenWidth);
	wchar_t input = 0;
	int scroll = 0;

	//main loop
	do {
		erase();
		
		int lineW = (int) log10((double) linesCount) + 2; //make room for line numbers
		
		switch (input) {
			case KEY_LEFT:
				colActual--;
				if (fileContents[line][colActual] != '\t') {
					colDisp--;
				} else {
					colDisp = tab_strlenFrom(fileContents[line], 0, colActual);
				}
				goalCol = colDisp;
				break;
			case KEY_RIGHT:
				if (fileContents[line][colActual] != '\t') {
					colDisp++;
				} else {
					colDisp = tab_strlenFrom(fileContents[line], 0, colActual + 1);
				}
				colActual++;
				goalCol = max(colDisp, goalCol);
				break;
			case KEY_UP:
				if (line > 0) {
					line--;
					colActual = tab_strlenTo(fileContents[line], 0, goalCol);
				} else {
					colActual = 0;
				}
				break;
			case KEY_DOWN:
				if (line < linesCount - 1) {
					line++;
					colActual = tab_strlenTo(fileContents[line], 0, goalCol);
				} else {
					colActual = strlen(fileContents[line]);
				}
				break;
			case KEY_PPAGE:
				line -= screenHeight;
				break;
			case KEY_NPAGE:
				line += screenHeight;
				break;
		}

		if (input == 'q') break;

		//bound cursor
		int displayRowStart = 1; //make room for file banner
				line = max(0, line);
				line = min(line, linesCount - 1);
		colActual = min(colActual, strlen(fileContents[line]) - 1);
		colActual = max(0, colActual);
		colDisp = tab_strlenFrom(fileContents[line], 0, colActual);
		colActual = tab_strlenTo(fileContents[line], 0, colDisp);

		if (line - scroll >= screenHeight - displayRowStart - SCROLL_PADDING) {
			scroll = min(line - (screenHeight - displayRowStart - SCROLL_PADDING), linesCount - 1);
		}
		if (line - scroll < SCROLL_PADDING) {
			scroll = max(line - SCROLL_PADDING, 0);
		}

		//render file text
		int linesUntil = screenHeight - displayRowStart;
		for (int i = 0; i < linesUntil; i++) {
			int currentLine = i + scroll;
			int displayCol = lineW;
			int currentRow = i + displayRowStart;
			if (currentLine < linesCount) {
				tab_mvprintw(currentRow, displayCol, lineW, "%.*s", screenWidth - lineW, fileContents[currentLine]);	
				attron(COLOR_PAIR(COLOR_GRAY));
				mvprintw(currentRow, 0, "%*d ", lineW - 1, currentLine + 1);
				attroff(COLOR_PAIR(COLOR_GRAY));
			} else {
				attron(COLOR_PAIR(COLOR_GRAY));
				mvprintw(currentRow, displayCol, "~");
				attroff(COLOR_PAIR(COLOR_GRAY));
			}
		}

		//render file banner
		int bannerRow = 0;
		attron(COLOR_PAIR(COLOR_BW));
		for (int i = 0; i < screenWidth; i++) {
			char fillChar = colorMode ? ' ' : 183; //if color not supported use middle dot
			mvaddch(bannerRow, i, fillChar);
		}
		mvprintw(bannerRow, 0, "WTE %s %dL %dC", targetFileName, linesCount, charsCount);
		mvprintw(bannerRow, screenWidth-16, "%4d:%4d-%4d", line, colDisp, colActual);
		attroff(COLOR_PAIR(COLOR_BW));

		refresh();

		//display cursor
		curs_set(1);
		move(line + displayRowStart - scroll, colDisp + lineW);

		input = getch();
	} while (1);
	endwin();
	return 0;
}
