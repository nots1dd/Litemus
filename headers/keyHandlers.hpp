#ifndef KEY_HANDLERS_HPP
#define KEY_HANDLERS_HPP

#include <ncurses.h>
#include <menu.h>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <SFML/Audio.hpp>

void handleKeyEvent_1(MENU* artistMenu, MENU* songMenu, WINDOW* artist_menu_win, bool showingArtists, int menu_height, int menu_width);
void handleKeyEvent_tab(MENU* artistMenu, MENU* songMenu, WINDOW* artist_menu_win, WINDOW* song_menu_win, bool showingArtists, int menu_height, int menu_width);
void handleKeyEvent_slash(MENU* artistMenu, MENU* songMenu, bool showingArtists);
void displayLyricsWindow(WINDOW *artist_menu_win, std::string& currentLyrics, std::string& currentSong, std::string& currentArtist, int menu_height, int menu_width, sf::Music &music, WINDOW *status_win, bool firstEnterPressed, bool showingLyrics, WINDOW *song_menu_win, MENU *songMenu, std::string& currentGenre, bool showingArtists);
void quitFunc(sf::Music& music, std::vector<std::string>& allArtists, std::vector<std::string>& songTitles, ITEM** artistItems, ITEM** songItems, MENU* artistMenu, MENU* songMenu);
void printSessionDetails(WINDOW* menu_win, const std::string& songsDirectory, int artistsSize, int songsSize);

#endif
