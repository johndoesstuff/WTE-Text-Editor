#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

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
	refresh();
	getch();
	endwin();
	return 0;
}
