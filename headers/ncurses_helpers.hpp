#ifndef NCURSES_HELPERS_H
#define NCURSES_HELPERS_H

#include <ncurses.h>
#include <menu.h>
#include <string>
#include <SFML/Audio.hpp>

void ncursesSetup();
void ncursesWinControl(WINDOW* artist_menu_win, WINDOW* song_menu_win, WINDOW* status_win, WINDOW* title_win, const std::string& choice);
void displayHelpWindow(WINDOW* menu_win);
void updateStatusBar(WINDOW* status_win, const std::string& songName, const std::string& artistName, const std::string& songGenre, const sf::Music& music, bool firstEnterPressed, bool showingLyrics);
bool showExitConfirmation(WINDOW* parent_win);
void highlightFocusedWindow(MENU* menu, bool focused);
void printMultiLine(WINDOW* win, const std::vector<std::string>& lines, int start_line, std::string& currentSong, std::string& currentArtist);
void ncursesMenuSetup(MENU* Menu, WINDOW* win, int menu_height, int menu_width);
void move_menu_down(MENU* artistMenu, MENU* songMenu, bool showingArtists);
void move_menu_up(MENU* artistMenu, MENU* songMenu, bool showingArtists);
ITEM** createItems(const std::string& name, std::vector<std::string>& allArtists, std::vector<std::string>& songTitles);

#endif
