#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <ncurses.h>
#include <menu.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include "headers/lmus_cache.hpp"

#define COLOR_PAIR_FOCUSED 1 
#define COLOR_PAIR_SELECTED 3
#define GREY_BACKGROUND_COLOR 7
#define LIGHT_GREEN_COLOR 8

// Directory vars
const std::string songsDirectory = "/home/s1dd/Downloads/Songs/";
const std::string cacheDirectory = songsDirectory + ".cache/litemus/info/song_names.json";
const std::string cacheArtistDirectory = songsDirectory + ".cache/litemus/info/artists.json";

const char* title_content = "  LITEMUS - Light Music player                                                                                                                                                                                 ";

using json = nlohmann::json;

std::vector<std::string> parseArtists(const std::string& artistsFile) {
    std::ifstream file(artistsFile);
    if (!file.is_open()) {
        std::cerr << "Could not open artists file: " << artistsFile << std::endl;
        return {};
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return {};
    }

    std::vector<std::string> artists;
    for (const auto& artist : j) {
        artists.push_back(artist.get<std::string>());
    }

    return artists;
}

std::pair<std::vector<std::string>, std::vector<std::string>> listSongs(const std::string& cacheFile) {
    std::ifstream file(cacheFile);
    if (!file.is_open()) {
        std::cerr << "Could not open cache file: " << cacheFile << std::endl;
        return {{}, {}};
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return {{}, {}};
    }

    std::vector<std::string> songTitles;
    std::vector<std::string> songPaths;

    // Iterate over each artist
    for (auto it = j.begin(); it != j.end(); ++it) {
        std::string artistName = it.key();
        auto& albums = it.value();

        // Iterate over each album of the artist
        for (auto albumIt = albums.begin(); albumIt != albums.end(); ++albumIt) {
            std::string albumName = albumIt.key();
            auto& discs = albumIt.value();

            // Iterate over each disc in the album
            for (auto discIt = discs.begin(); discIt != discs.end(); ++discIt) {
                auto& tracks = *discIt;

                // Iterate over each track in the disc
                for (auto trackIt = tracks.begin(); trackIt != tracks.end(); ++trackIt) {
                    auto& songInfo = *trackIt;

                    // Check if the track contains a valid song object
                    if (!songInfo.empty()) {
                        std::string songTitle = songInfo["title"].get<std::string>();
                        std::string songFilename = songInfo["filename"].get<std::string>();
                        songTitles.push_back(songTitle);
                        songPaths.push_back(songsDirectory + songFilename); // Adjust the path as per your directory structure
                    }
                }
            }
        }
    }

    return {songTitles, songPaths};
}

void highlightFocusedWindow(MENU* menu, bool focused) {
    if (focused) {
        set_menu_fore(menu, COLOR_PAIR(COLOR_PAIR_SELECTED));
        set_menu_back(menu, COLOR_PAIR(A_NORMAL));
        wattron(menu_win(menu), COLOR_PAIR(LIGHT_GREEN_COLOR));
        box(menu_win(menu), 0, 0);
    } else {
        set_menu_fore(menu, A_NORMAL);
        set_menu_back(menu, A_NORMAL);
        wattroff(menu_win(menu), COLOR_PAIR(LIGHT_GREEN_COLOR));
    }
    wrefresh(menu_win(menu));
}

void displayHelpWindow(WINDOW* menu_win) {
    werase(menu_win);
    // set_menu_fore(menu(menu_win), A_NORMAL);
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 1, 2, "Help Controls");
    mvwprintw(menu_win, 3, 2, "p - Pause/Play");
    mvwprintw(menu_win, 4, 2, "Enter - Play selected song");
    mvwprintw(menu_win, 5, 2, "f - Seek forward 5 seconds");
    mvwprintw(menu_win, 6, 2, "g - Seek backward 5 seconds");
    mvwprintw(menu_win, 7, 2, "r - Replay current song");
    mvwprintw(menu_win, 8, 2, "j - Move up");
    mvwprintw(menu_win, 9, 2, "k - Move down");
    mvwprintw(menu_win, 10, 2, "q - Quit");
    mvwprintw(menu_win, 11, 2, "n - Next Song");
    mvwprintw(menu_win, 12, 2, "b - Previous Song");
    mvwprintw(menu_win, 13, 2, "9 - Increase Volume");
    mvwprintw(menu_win, 14, 2, "0 - Decrease Volume");
    mvwprintw(menu_win, 15, 2, "Tab - Toggle Focused Window");
    mvwprintw(menu_win, 17, 2, "2 - To show help menu");
init_pair(GREY_BACKGROUND_COLOR, COLOR_BLACK, COLOR_WHITE);  // Grey background and black text for title
    mvwprintw(menu_win, 20, 2, "Press '1' to go back to the menu");
    wrefresh(menu_win);
}

void playMusic(sf::Music& music, const std::string& songPath) {
    if (music.getStatus() == sf::Music::Playing) {
        music.stop();
    }
    if (!music.openFromFile(songPath)) {
        std::cerr << "Error loading file" << std::endl;
    } else {
        music.play();
    }
}

void printTitle(WINDOW* win, const char* title) {
    wattron(win, A_BOLD | COLOR_PAIR(1));
    mvwprintw(win, 1, (getmaxx(win) - strlen(title)) / 2, "%s", title);
    wattroff(win, A_BOLD | COLOR_PAIR(1));
    wrefresh(win);
}

void updateStatusBar(WINDOW* status_win, const std::string& songName, const sf::Music& music, bool firstEnterPressed) {
    const int maxSongNameLength = 45;
    std::string displayName = songName.length() > maxSongNameLength ? songName.substr(0, maxSongNameLength) + "..." : songName;

    wmove(status_win, 1, 1);
    wclrtoeol(status_win);

    wattron(status_win, COLOR_PAIR(5));
    wbkgd(status_win, COLOR_PAIR(5) | A_BOLD);
    wattron(status_win, COLOR_PAIR(6));

    const char* playPauseSymbol = (music.getStatus() == sf::Music::Playing) ? "<>" : "!!";
    const char* launchSymbol = "--";
    sf::Time currentTime = music.getPlayingOffset();
    sf::Time duration = music.getDuration();
    int posMinutes = static_cast<int>(currentTime.asSeconds()) / 60;
    int posSeconds = static_cast<int>(currentTime.asSeconds()) % 60;
    int durMinutes = static_cast<int>(duration.asSeconds()) / 60;
    int durSeconds = static_cast<int>(duration.asSeconds()) % 60;

    float volume = music.getVolume();

    firstEnterPressed ? mvwprintw(status_win, 1, 1, " Status:  %s   |   %s   |   %02d:%02d / %02d:%02d   |  Vol. %.0f%%                                                                                        LITEMUS ", playPauseSymbol, displayName.c_str(), posMinutes, posSeconds, durMinutes, durSeconds, volume) : mvwprintw(status_win, 1, 1, " Status:  %s   |   Unknown Song   |   00:00 / 00:00   |  Vol. %.0f%%                                                                                        LITEMUS ", launchSymbol, volume);
    wattroff(status_win, COLOR_PAIR(5));
    wattroff(status_win, COLOR_PAIR(6));
    wrefresh(status_win);
}


void nextSong(sf::Music& music, const std::vector<std::string>& songs, int& currentSongIndex) {
    music.stop();
    currentSongIndex = (currentSongIndex + 1) % songs.size();
    std::string nextSongPath = songs[currentSongIndex];
    playMusic(music, nextSongPath);    
}

void previousSong(sf::Music& music, const std::vector<std::string>& songs, int& currentSongIndex) {
    music.stop();
    currentSongIndex = (currentSongIndex - 1 + songs.size()) % songs.size();
    std::string prevSongPath = songs[currentSongIndex];
    playMusic(music, prevSongPath);
}

void adjustVolume(sf::Music& music, float volumeChange) {
    float currentVolume = music.getVolume();
    currentVolume += volumeChange;
    currentVolume = std::max(0.f, std::min(100.f, currentVolume));
    music.setVolume(currentVolume);
}

bool showExitConfirmation(WINDOW* parent_win) {
    int height = 7;
    int width = 55;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2;

    WINDOW* confirm_win = newwin(height, width, start_y, start_x);
    wattron(confirm_win, COLOR_PAIR(COLOR_GREEN));
    box(confirm_win, 0, 0);
    mvwprintw(confirm_win, 1, 9, "Do you really want to exit LITEMUS?");
    mvwprintw(confirm_win, 4, 8, "     Yes (Y)               No (N)");

    wrefresh(confirm_win);

    int ch;
    bool exitConfirmed = false;
    while ((ch = wgetch(confirm_win)) != 'y' && ch != 'Y' && ch != 'n' && ch != 'N' && ch != 27 && ch != 10) {
        // Wait for valid input
    }
    if (ch == 'y' || ch == 'Y' || ch == 10) {
        exitConfirmed = true;
    }

    delwin(confirm_win);
    wrefresh(parent_win); // Refresh the parent window

    return exitConfirmed;
}

void ncursesSetup() {
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Initialize color pairs
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_BLACK, COLOR_BLACK);
    init_pair(6, COLOR_BLACK, COLOR_WHITE);
    init_pair(GREY_BACKGROUND_COLOR, COLOR_WHITE, GREY_BACKGROUND_COLOR);
    init_pair(LIGHT_GREEN_COLOR, COLOR_GREEN, COLOR_BLACK);
}

int main() {
    // Initialize ncurses
    lmus_cache_main(songsDirectory);
    ncursesSetup();

    // Cache directory and song information 
    std::vector<std::string> allArtists = parseArtists(cacheArtistDirectory);
    auto [songTitles, songPaths] = listSongs(cacheDirectory);

    // Check if songs are found
    if (songTitles.empty() || songPaths.empty()) {
        printw("No songs found in directory.\n");
        refresh();
        endwin();
        return -1;
    }

    // Initialize artist menu
    ITEM** artistItems = new ITEM*[allArtists.size() + 1];
    for (size_t i = 0; i < allArtists.size(); ++i) {
        artistItems[i] = new_item(allArtists[i].c_str(), "");
    }
    artistItems[allArtists.size()] = nullptr;
    MENU* artistMenu = new_menu(artistItems);

    // Initialize song menu
    ITEM** songItems = new ITEM*[songTitles.size() + 1];
    for (size_t i = 0; i < songTitles.size(); ++i) {
        songItems[i] = new_item(songTitles[i].c_str(), "");
    }
    songItems[songTitles.size()] = nullptr;
    MENU* songMenu = new_menu(songItems);

    // Window dimensions and initialization
    int title_height = 2;
    int menu_height = std::min(static_cast<int>(songTitles.size()) + 4, 44);
    int menu_width = 90;
    int title_width = 206;

    WINDOW* title_win = newwin(title_height, title_width, 0, 0);
    wbkgd(title_win, COLOR_PAIR(GREY_BACKGROUND_COLOR));
    box(title_win, 0, 0);
    wrefresh(title_win);

    WINDOW* artist_menu_win = newwin(menu_height, menu_width, 1, 0);
    WINDOW* song_menu_win = newwin(menu_height, menu_width + 29, 1, menu_width);
    WINDOW* status_win = newwin(10, 200, LINES - 2, 0);

    // Set menus to their respective windows
    set_menu_win(artistMenu, artist_menu_win);
    set_menu_sub(artistMenu, derwin(artist_menu_win, menu_height - 3, menu_width - 2, 2, 1));
    set_menu_mark(artistMenu, " > ");

    set_menu_win(songMenu, song_menu_win);
    set_menu_sub(songMenu, derwin(song_menu_win, menu_height - 3, menu_width - 2, 2, 1));
    set_menu_mark(songMenu, " > ");

    set_menu_format(artistMenu, menu_height, 0);
    set_menu_format(songMenu, menu_height, 0);

    box(artist_menu_win, 0, 0);
    box(song_menu_win, 0, 0);
    box(status_win, 0, 0);
    box(title_win, 0, 0);

    // Refresh all windows
    wrefresh(artist_menu_win);
    wrefresh(song_menu_win);
    wrefresh(status_win);
    wrefresh(title_win);

    // Initialize SFML Music
    sf::Music music;
    int currentSongIndex = -1;
    std::string currentSong = songTitles.empty() ? "" : songTitles[0];

    // Timeout for getch() to avoid blocking indefinitely
    timeout(100);

    // Flag to track first enter press
    bool firstEnterPressed = false;
    bool showingArtists = true;  // Start with artist menu in focus

    highlightFocusedWindow(artistMenu, true);
    highlightFocusedWindow(songMenu, false);
    while (true) {
        int ch = getch();
        if (ch != ERR) {
            switch (ch) {
                case '1':
                  werase(artist_menu_win);
                  set_menu_win(artistMenu, artist_menu_win);
                  set_menu_sub(artistMenu, derwin(artist_menu_win, menu_height - 3, menu_width - 2, 2, 1));
                  set_menu_mark(artistMenu, " > ");
                  post_menu(artistMenu);
                  post_menu(songMenu);
                  box(artist_menu_win, 0, 0);
                  highlightFocusedWindow(artistMenu, showingArtists);
                  wrefresh(artist_menu_win);
                  break;
                case 9:  // Tab to switch between menus
                    showingArtists = !showingArtists;
                    if (showingArtists) {
                        // Switch focus to artist menu
                        werase(artist_menu_win);
                        set_menu_win(artistMenu, artist_menu_win);
                        set_menu_sub(artistMenu, derwin(artist_menu_win, menu_height - 3, menu_width - 2, 2, 1));
                        set_menu_mark(artistMenu, " > ");
                        post_menu(artistMenu);
                        post_menu(songMenu);
                        box(artist_menu_win, 0, 0);
                        highlightFocusedWindow(artistMenu, true);
                        highlightFocusedWindow(songMenu, false);
                        wrefresh(artist_menu_win);
                    } else {
                        // Switch focus to song menu
                        set_menu_win(songMenu, song_menu_win);
                        set_menu_sub(songMenu, derwin(song_menu_win, menu_height - 3, menu_width - 2, 2, 1));
                        set_menu_mark(songMenu, " > ");
                        post_menu(songMenu);
                        post_menu(artistMenu);
                        box(song_menu_win, 0, 0);
                        highlightFocusedWindow(artistMenu, false);
                        highlightFocusedWindow(songMenu, true);
                        wrefresh(song_menu_win);
                    }
                    break;
                case KEY_DOWN:
                case 'k':
                    if (showingArtists) {
                        // Move down in artist menu
                        werase(artist_menu_win);
                        menu_driver(artistMenu, REQ_DOWN_ITEM);
                    } else {
                        // Move down in song menu
                        werase(song_menu_win);
                        menu_driver(songMenu, REQ_DOWN_ITEM);
                    }
                    break;
                case KEY_UP:
                case 'j':
                    if (showingArtists) {
                        // Move up in artist menu
                        werase(artist_menu_win);
                        menu_driver(artistMenu, REQ_UP_ITEM);
                    } else {
                        // Move up in song menu
                        werase(song_menu_win);
                        menu_driver(songMenu, REQ_UP_ITEM);
                    }
                    break;
                case KEY_RIGHT:
                    if (showingArtists) {
                        // Scroll down in artist menu
                        werase(artist_menu_win);
                        menu_driver(artistMenu, REQ_SCR_DPAGE);
                    } else {
                        // Scroll down in song menu
                        werase(song_menu_win);
                        menu_driver(songMenu, REQ_SCR_DPAGE);
                    }
                    break;
                case KEY_LEFT:
                    if (showingArtists) {
                        // Scroll up in artist menu
                        werase(artist_menu_win);
                        menu_driver(artistMenu, REQ_SCR_UPAGE);
                    } else {
                        // Scroll up in song menu
                        werase(song_menu_win);
                        menu_driver(songMenu, REQ_SCR_UPAGE);
                    }
                    break;
                
            case 10:  // Enter key
                if (showingArtists) {
                    // Show details of selected artist (if needed)
                    ITEM* artItem = current_item(artistMenu);
                    int artselectedIndex = item_index(artItem);
                    set_menu_win(artistMenu, artist_menu_win);
                    set_menu_sub(artistMenu, derwin(artist_menu_win, menu_height - 3, menu_width - 2, 2, 1));
                    set_menu_mark(artistMenu, " > ");
                    post_menu(artistMenu);
                    post_menu(songMenu);
                    box(artist_menu_win, 0, 0);
                    const char* selectedArtist = allArtists[artselectedIndex].c_str();
                    mvwprintw(song_menu_win, 1, 2, selectedArtist);
                    showingArtists = !showingArtists;
                    highlightFocusedWindow(artistMenu, showingArtists);
                    highlightFocusedWindow(songMenu, !showingArtists);  // Highlight the song menu when switching focus
                    wrefresh(artist_menu_win);
                } else {
                    // Play selected song from song menu
                    ITEM* curItem = current_item(songMenu);
                    int selectedIndex = item_index(curItem);
                    set_menu_win(songMenu, song_menu_win);
                    set_menu_sub(songMenu, derwin(song_menu_win, menu_height - 3, menu_width - 2, 2, 1));
                    set_menu_mark(songMenu, " > ");
                    post_menu(songMenu);
                    post_menu(artistMenu);
                    box(song_menu_win, 0, 0);
                    highlightFocusedWindow(artistMenu, showingArtists);  // Highlight the artist menu when switching focus
                    wrefresh(song_menu_win);
                    if (selectedIndex >= 0 && selectedIndex < songPaths.size()) {
                        currentSongIndex = selectedIndex;
                        playMusic(music, songPaths[currentSongIndex]);
                        currentSong = songTitles[currentSongIndex];
                        updateStatusBar(status_win, currentSong, music, firstEnterPressed);
                    }

                    firstEnterPressed = true;
                  }
                  break;
                case 'p':  // Pause/play music
                    if (music.getStatus() == sf::Music::Paused) {
                        music.play();
                    } else {
                        music.pause();
                    }
                    break;
                case 'f':  // Fast-forward (skip 5 seconds)
                    music.setPlayingOffset(music.getPlayingOffset() + sf::seconds(5));
                    break;
                case 'g':  // Rewind (go back 5 seconds)
                    if (music.getPlayingOffset() > sf::seconds(5)) {
                        music.setPlayingOffset(music.getPlayingOffset() - sf::seconds(5));
                    } else {
                        music.setPlayingOffset(sf::seconds(0));
                    }
                    break;
                case 'r':  // Restart current song
                    music.stop();
                    music.play();
                    break;
                case '9':  // Volume up
                    adjustVolume(music, 10.f);
                    break;
                case '0':  // Volume down
                    adjustVolume(music, -10.f);
                    break;
                case 'n':  // Next song
                    nextSong(music, songPaths, currentSongIndex);
                    currentSong = songTitles[currentSongIndex];
                    updateStatusBar(status_win, currentSong, music, firstEnterPressed);
                    break;
                case 'b':  // Previous song
                    previousSong(music, songPaths, currentSongIndex);
                    currentSong = songTitles[currentSongIndex];
                    updateStatusBar(status_win, currentSong, music, firstEnterPressed);
                    break;
                case '2':  // Display help window
                    displayHelpWindow(artist_menu_win);
                    break;
                case 'q':  // Quit
                if (showExitConfirmation(song_menu_win)) {
                    music.stop();
                    // Free resources and clean up
                    for (size_t i = 0; i < songTitles.size(); ++i) {
                        free_item(songItems[i]);
                    }
                    free_menu(songMenu);
                    for (size_t i = 0; i < allArtists.size(); ++i) {
                        free_item(artistItems[i]);
                    }
                    free_menu(artistMenu);
                    delwin(song_menu_win);
                    delwin(status_win);
                    endwin();
                    return 0;
                }
                break;
                case 'x': // force exit
                    music.stop();
                    // Free resources and clean up
                    for (size_t i = 0; i < songTitles.size(); ++i) {
                        free_item(songItems[i]);
                    }
                    free_menu(songMenu);
                    for (size_t i = 0; i < allArtists.size(); ++i) {
                        free_item(artistItems[i]);
                    }
                    free_menu(artistMenu);
                    delwin(song_menu_win);
                    delwin(status_win);
                    endwin();
                    return 0;
                break;
                default:
                    mvwprintw(status_win, 12, 2, "Invalid input.");
            }
        }

        // Handle song transition when current song ends
        if (music.getStatus() == sf::Music::Stopped && firstEnterPressed) {
            nextSong(music, songPaths, currentSongIndex);
            currentSong = songTitles[currentSongIndex];
            updateStatusBar(status_win, currentSong, music, firstEnterPressed);
        }

        // Update status bar and refresh windows
        updateStatusBar(status_win, currentSong, music, firstEnterPressed);

        wrefresh(artist_menu_win);
        wrefresh(song_menu_win);
        box(artist_menu_win, 0, 0);
        box(song_menu_win, 0, 0);
        wmove(title_win, 0, 0);
        wclrtoeol(title_win);
        wattron(title_win, COLOR_PAIR(5));
        wbkgd(title_win, COLOR_PAIR(5) | A_BOLD);
        wattron(title_win, COLOR_PAIR(6));
        mvwprintw(title_win, 0, 1, title_content);  // Replace with your title
        wattroff(title_win, COLOR_PAIR(5));
        wattroff(title_win, COLOR_PAIR(6));
        wrefresh(title_win);
        post_menu(artistMenu);  // Post the active menu
        post_menu(songMenu);
        wrefresh(artist_menu_win);
        wrefresh(song_menu_win);
        wrefresh(status_win);
        wrefresh(title_win);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Optional delay
    }

    // Clean up and exit
    unpost_menu(artistMenu);
    unpost_menu(songMenu);
    for (size_t i = 0; i < songTitles.size(); ++i) {
        free_item(songItems[i]);
    }
    free_menu(songMenu);
    for (size_t i = 0; i < allArtists.size(); ++i) {
        free_item(artistItems[i]);
    }
    free_menu(artistMenu);
    delwin(artist_menu_win);
    delwin(song_menu_win);
    delwin(status_win);
    delwin(title_win);
    endwin();

    return 0;
}
