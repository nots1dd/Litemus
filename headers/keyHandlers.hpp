#ifndef KEY_HANDLERS_HPP
#define KEY_HANDLERS_HPP

#include <ncurses.h>
#include <menu.h>
#include <cstring>

void handleKeyEvent_1(MENU* artistMenu, MENU* songMenu, WINDOW* artist_menu_win, bool showingArtists, int menu_height, int menu_width);
void handleKeyEvent_tab(MENU* artistMenu, MENU* songMenu, WINDOW* artist_menu_win, WINDOW* song_menu_win, bool showingArtists, int menu_height, int menu_width);
void handleKeyEvent_slash(MENU* artistMenu, MENU* songMenu, bool showingArtists);

#endif
