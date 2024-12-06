#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

int max(int a, int b) {
	return a > b ? a : b;
}

int main(int argc, char *argv[]) {

	//find file
	char *targetFileName = argv[1];
	FILE *targetFile = fopen(targetFileName, "r+"); //open file for reading/writing
	if (targetFile == NULL) {
		printf("Unable to locate file %s", targetFileName);
		return 0;
	}
	
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

	//render file banner
	attron(COLOR_PAIR(COLOR_BW));
	int fileNameLen = strlen(targetFileName);
	for (int i = 0; i < screenWidth; i++) {
		char fillChar = colorMode ? ' ' : 183; //if color not supported use middle dot
		mvaddch(screenHeight - 1, i, i < fileNameLen ? targetFileName[i] : fillChar);
	}
	attroff(COLOR_PAIR(COLOR_BW));

	refresh();
	getch();
	endwin();
	return 0;
}
