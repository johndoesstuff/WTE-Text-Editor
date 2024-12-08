#ifndef TAB_UTILS_H
#define TAB_UTILS_H
#include "constants.h"
#include <stddef.h>

size_t tab_strlen(const char *str, int tabOffset);
size_t tab_strlenTo(const char *str, int tabOffset, int col);
size_t tab_strlenFrom(const char *str, int tabOffset, int col);
void tab_mvprintw(int y, int x, int tabOffset, const char *format, ...);

#endif
