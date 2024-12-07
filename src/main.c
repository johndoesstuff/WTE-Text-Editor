#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <math.h>

#define LINE_BUFFER_MIN 2
#define LINES_BUFFER_MIN 2
#define SCROLL_PADDING 5

int max(int a, int b) {
	return a > b ? a : b;
}

int min(int a, int b) {
	return a < b ? a : b;
}

size_t tab_strlen(const char *str) {
	size_t len = 0;
	for (int i = 0; i < strlen(str); i++) {
		len += str[i] == '\t' ? 4 : 1;
	}
	return len;
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
	int COMMAND_MODE = 0;
	int EDIT_MODE = 1;
	int REPLACE_MODE = 2;
	curs_set(0);

	//initialize colors
	int COLOR_BW = 1;
	int COLOR_GRAY = 2;
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
	int col = 0;
	getmaxyx(stdscr, screenHeight, screenWidth);
	wchar_t input = 0;
	int scroll = 0;

	do { 
		if (input == KEY_LEFT) {
			col--;
			goalCol = col;
		} else if (input == KEY_RIGHT) {
			if (col < tab_strlen(fileContents[line]) - 1) {
				col++;
				goalCol = max(col, goalCol);
			}
		} else if (input == KEY_UP) {
			line--;
		} else if (input == KEY_DOWN) {
			line++;
		} else if (input == 'q') {
			break;
		}

		//bound cursor
		int displayRowStart = 1; //make room for file banner
		line = min(line, linesCount);
		line = max(0, line);
		col = min(goalCol, tab_strlen(fileContents[line]) - 1);
		col = max(0, col);

		if (line - scroll >= screenHeight - displayRowStart - SCROLL_PADDING) {
			scroll = min(line - (screenHeight - displayRowStart - SCROLL_PADDING), linesCount - 1);
		}
		if (line - scroll < SCROLL_PADDING) {
			scroll = max(line - SCROLL_PADDING, 0);
		}

		//render file text
		int lineW = (int) log10((double) linesCount) + 2; //make room for line numbers
		int linesUntil = min(linesCount, screenHeight) + displayRowStart;
		for (int i = 0; i < linesUntil; i++) {
			mvprintw(i + displayRowStart, lineW, "%.*s", screenWidth - lineW, fileContents[i + scroll]);
		}
		attron(COLOR_PAIR(COLOR_GRAY));
		for (int i = 0; i < linesUntil; i++) {
			mvprintw(i + displayRowStart, 0, "%*d ", lineW - 1, i + 1 + scroll);
		}
		attroff(COLOR_PAIR(COLOR_GRAY));

		//render file banner
		int bannerRow = 0;
		attron(COLOR_PAIR(COLOR_BW));
		for (int i = 0; i < screenWidth; i++) {
			char fillChar = colorMode ? ' ' : 183; //if color not supported use middle dot
			mvaddch(bannerRow, i, fillChar);
		}
		mvprintw(bannerRow, 0, "WTE %s %dL %dC", targetFileName, linesCount, charsCount);
		attroff(COLOR_PAIR(COLOR_BW));

		refresh();
		//display cursor
		curs_set(1);
		move(line + displayRowStart - scroll, col + lineW);

		input = getch();
	} while (1);
	endwin();
	return 0;
}
