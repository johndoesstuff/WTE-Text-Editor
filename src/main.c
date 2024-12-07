#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

#define LINE_BUFFER_MIN 64
#define LINES_BUFFER_MIN 64

int max(int a, int b) {
	return a > b ? a : b;
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
	while ((ch = fgetc(file)) != EOF) { //read file character by character (avoid max line length)
		int newLine = ch == '\n';
		if (newLine) { //if newline add \0 to line
			ch = '\0';
		}

		if (lineSize - 1 <= lineCount) { //resize line
			lineSize *= 2;
			line = realloc(line, lineSize * sizeof(char));
		}

		line[lineCount] = ch; //add char
		lineCount++;

		if (newLine) { //increment lines buffers
			if (linesSize - 1 <= linesCount) { //resize lines
				linesSize *= 2;
				lines = realloc(lines, linesSize * sizeof(char*));
			}
			
			lines[linesCount] = line;
			linesCount++;
			
			line = malloc(lineSize * sizeof(char)); //start new line
			charCount += lineCount;
			lineCount = 0;
		}
	}
	*numLines = linesCount;
	*numChars = charCount;
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
	int COMMAND_MODE = 0;
	int EDIT_MODE = 1;
	int REPLACE_MODE = 2;
	curs_set(0);
	int COLOR_BW = 1;
	int colorMode = has_colors();
	if (colorMode) {
		start_color();
		init_pair(COLOR_BW, COLOR_BLACK, COLOR_WHITE);
	}

	int screenWidth, screenHeight;
	getmaxyx(stdscr, screenHeight, screenWidth);

	//render file text
	

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
	getch();
	endwin();
	return 0;
}
