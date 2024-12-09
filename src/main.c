#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <math.h>
#include "constants.h"
#include <ctype.h>
#include "tabUtil.h"

const char *MODES[] = {"COMMAND", "EDIT", "REPLACE"};
char *EXTS[] = {
	"js",
	"c",
	"html",
	"txt",
	"cpp",
	"ua",
	"py",
	"h",
	"css",
	"svg",
	"md",
	"csv",
	"log",
	"xml",
	"json",
	"yaml",
	"yml",
	"ini",
	"java",
	"ts",
	"sh",
	"bat",
	NULL, //terminate list or some shit idk im lazy
};
char *FORMATS[] = {
	"Javascript",
	"C",
	"Hypertext Markup Language",
	"Text",
	"C++",
	"Uiua",
	"Python",
	"C Header",
	"Cascading Style Sheets",
	"Scalable Vector Graphics",
	"Markdown",
	"Comma-Separated Values",
	"Log",
	"XML",
	"JSON",
	"YAML",
	"YAML",
	"INI Configuration",
	"Java Source",
	"Typescript",
	"Shell Script",
	"Batch",
	NULL,
};

int max(int a, int b) {
	return a > b ? a : b;
}

int min(int a, int b) {
	return a < b ? a : b;
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

void writeLinesToFile(char* filename, char* lines[], int lineCount) {
	FILE* file = fopen(filename, "w");
	for (int i = 0; i < lineCount; i++) {
		if (fputs(lines[i], file) == EOF) {
			fclose(file);
			return;
		}
	}
	fclose(file);
}

void insertChar(char** str, char ch, int index) {
	int len = strlen(*str);
	if (index > len) {
		index = len;
	}

	char *newStr = realloc(*str, len + 2); // +1 for ch +1 for \0
	*str = newStr;

	memmove(&(*str)[index + 1], &(*str)[index], len - index + 1);
	(*str)[index] = ch;
}

void insertLine(char*** lines, int* linesCount, int line, int col) {
	if (line > *linesCount) {
		line = *linesCount;
	}

	*lines = realloc(*lines, (*linesCount + 1) * sizeof(char*));
	int lineCount = strlen((*lines)[line]);

	for (int i = *linesCount; i > line; i--) { 
		(*lines)[i] = (*lines)[i - 1];
	}

	char* newLine = malloc(lineCount - col + 1);
	strcpy(newLine, &(*lines)[line][col]);

	(*lines)[line] = realloc((*lines)[line], col + 2);
	(*lines)[line][col] = '\n';
	(*lines)[line][col + 1] = '\0';

	(*lines)[line+1] = newLine;

	(*linesCount)++;
}

void removeChar(char** str, int index) {
	int len = strlen(*str);
	if (index >= len) {
		index = len - 1;
	}
	if (index < 0) return;

	memmove(&(*str)[index], &(*str)[index + 1], len - index);
}

void removeNewline(char*** lines, int* linesCount, int line) {
	if (line == 0) return; //cannot move line to -1

	size_t prevLen = strlen((*lines)[line - 1]);
	size_t currLen = strlen((*lines)[line]);

	(*lines)[line - 1][prevLen - 1] = '\0'; //remove newline
	prevLen--;
	
	(*lines)[line - 1] = realloc((*lines)[line - 1], prevLen + currLen + 1);
	strcat((*lines)[line - 1], (*lines)[line]);

	free((*lines)[line]);

	for (int i = line; i < *linesCount - 1; i++) {
		(*lines)[i] = (*lines)[i + 1];
	}


	(*linesCount)--;
}

int indexOf(char** strings, int numStrings, char* str) {
	for (int i = 0; i < numStrings && strings[i] != NULL; i++) {
		if (strcmp(strings[i], str) == 0) {
			return i;
		}
	}
	return -1;
}

char* getExtensionFromFilename(char* filename) {
	int len = strlen(filename);
	for (int i = len - 1; i >= 0; i--) {
		if (filename[i] == '.') {
			if (i < len - 1) {
				return &filename[i + 1];
			} else {
				return NULL; //case of "file."
			}
		}
	}
	return NULL; //no extension
}

void quit(FILE* file) {
	fclose(file);
	endwin();
	exit(0);
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Please provide a filename to edit\n");
		return 0;
	}

	//find file
	char *targetFileName = argv[1];
	char *targetFileExt = getExtensionFromFilename(targetFileName);
	char *targetFileType;
	targetFileType = NULL;
	if (targetFileExt != NULL) {
		int index = indexOf(EXTS, 1e4, targetFileExt);
		targetFileType = FORMATS[index];
	}
	FILE *targetFile = fopen(targetFileName, "r+"); //open file for reading/writing
	if (targetFile == NULL) {
		printf("Unable to locate file %s\n", targetFileName);
		return 0;
	}
	
	//read file
	int linesCount;
	int charsCount;
	char **fileContents = readFileToCharArray(targetFile, &linesCount, &charsCount);
	if (linesCount == 0 && charsCount == -1) { //completely empty file case
		strcpy(fileContents[0], "\n");
		charsCount++;
		linesCount++;
	}

	//initialize ncurses
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, 1);
	curs_set(0);
	set_escdelay(25);

	//initialize colors
	int colorMode = has_colors();
	if (colorMode) {
		start_color();
		init_pair(COLOR_BW, COLOR_BLACK, COLOR_WHITE);
		init_pair(COLOR_GRAY, 8, COLOR_BLACK);
		init_pair(COLOR_COM, COLOR_GREEN, COLOR_BLACK);
		init_pair(COLOR_REP, COLOR_WHITE, COLOR_WHITE);
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
	int MODE = 0;
	int SUCCESSFUL_WRITE = 0;

	//main loop
	do {
		erase();
		getmaxyx(stdscr, screenHeight, screenWidth);
		
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
			case 27: //esc
				MODE = COMMAND_MODE;
		}

		switch (MODE) {
			case COMMAND_MODE:
				switch (input) {
					case 'e':
						MODE = EDIT_MODE;
						break;
					case 'r':
						MODE = REPLACE_MODE;
						break;
					case 'q':
						quit(targetFile);
						break;
					case 'w':
						writeLinesToFile(targetFileName, fileContents, linesCount);
						SUCCESSFUL_WRITE = 1;
						break;
				}
				break;
			case EDIT_MODE:
				switch (input) {
					case KEY_BACKSPACE:
						if (colActual != 0) {
							removeChar(&fileContents[line], colActual - 1);
							colActual--;
							charsCount--;
						} else {
							if (line > 0) {
								colActual = strlen(fileContents[line - 1]) - 1;
								removeNewline(&fileContents, &linesCount, line);
								line--;
								charsCount--;
							}
						}
						break;
					case KEY_DC:
						if (colActual < strlen(fileContents[line]) - 1) {
							removeChar(&fileContents[line], colActual);
							charsCount--;
						}
						break;
					case '\n': //TODO: auto add tabs to newlines based on previous
						insertLine(&fileContents, &linesCount, line, colActual);
						line++;
						charsCount++;
						colActual = 0;
						break;
				}
				if (isprint(input) || input == '\t') {
					insertChar(&fileContents[line], input, colActual);
					colActual++;
					charsCount++;
				}
				break;
			case REPLACE_MODE: //no idea why you would use this an stackoverflow doesnt really know either but oh well
				switch (input) {
					case KEY_BACKSPACE:
						colActual--;
						colActual = max(colActual, 0);
						break;
				}
				if (isprint(input)) {
					if (colActual < strlen(fileContents[line]) - 1) {
						removeChar(&fileContents[line], colActual);
						charsCount--;
					}
					insertChar(&fileContents[line], input, colActual);
					colActual++;
					charsCount++;
				}
				break;
		}

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
		if (targetFileType != NULL) {
			mvprintw(bannerRow, 0, "WTE\t%s\t%s File\t%dL\t%dC", targetFileName, targetFileType, linesCount, charsCount);
		} else if (targetFileExt != NULL) {
			mvprintw(bannerRow, 0, "WTE\t%s\t%s File\t%dL\t%dC", targetFileName, targetFileExt, linesCount, charsCount);
		} else {
			mvprintw(bannerRow, 0, "WTE\t%s\t\t%dL\t%dC", targetFileName, linesCount, charsCount);
		}
		mvprintw(bannerRow, screenWidth-24, "\t%d\t%d-%d", line, colDisp, colActual);
		attroff(COLOR_PAIR(COLOR_BW));

		//render command banner
		int commandRow = screenHeight - 1;
		attron(COLOR_PAIR(COLOR_COM));
		for (int i = 0; i < screenWidth; i++) {
			char fillChar = colorMode ? ' ' : 183; //if color not supported use middle dot
			mvaddch(commandRow, i, fillChar);
		}
		if (SUCCESSFUL_WRITE) {
			mvprintw(commandRow, 0, "-- WRITE SUCCESSFUL --");
			SUCCESSFUL_WRITE = 0;	
		} else {
			mvprintw(commandRow, 0, "== %s MODE ==", MODES[MODE]);	
		}
		attroff(COLOR_PAIR(COLOR_COM));

		refresh();

		//display cursor
		int cursorRow = line + displayRowStart - scroll;
		int cursorCol = colDisp + lineW;
		if (MODE != REPLACE_MODE) {
			curs_set(1);
			move(cursorRow, cursorCol);
		} else {
			curs_set(0);
			attron(COLOR_PAIR(COLOR_REP));
			mvaddch(cursorRow, cursorCol, ' ');
			attroff(COLOR_PAIR(COLOR_REP));
		}

		input = getch();
	} while (1);
	quit(targetFile);
	return 0;
}
