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
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include "headers/lmus_cache.hpp"
#include "headers/sflw_helpers.hpp"
#include "headers/ncurses_helpers.hpp"

#define COLOR_PAIR_FOCUSED 1 
#define COLOR_PAIR_SELECTED 3
#define GREY_BACKGROUND_COLOR 7
#define LIGHT_GREEN_COLOR 8

// Directory vars
const std::string songsDirectory = "/home/s1dd/Downloads/Songs/";
const std::string cacheDirectory = songsDirectory + ".cache/litemus/info/song_names.json";
const std::string cacheArtistDirectory = songsDirectory + ".cache/litemus/info/artists.json";

const char* title_content = "  LITEMUS - Light Music player                                                                                                                                                                                   ";

using json = nlohmann::json;

char* strdup(const char* s) {
    size_t len = strlen(s);
    char* dup = new char[len + 1];
    strcpy(dup, s);
    return dup;
}

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

std::pair<std::vector<std::string>, std::vector<std::string>> listSongs(const std::string& cacheFile, const std::string& artistName) {
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
        if (it.key() == artistName) {
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
    }

    return {songTitles, songPaths};
}

std::pair<std::string, std::string> findCurrentGenreArtist(const std::string& cacheFile, const std::string& currentSong, std::string& currentLyrics) {
    std::ifstream file(cacheFile);
    if (!file.is_open()) {
        std::cerr << "Could not open cache file: " << cacheFile << std::endl;
        return {};
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return {};
    }

    for (auto it = j.begin(); it != j.end(); ++it) {
        for (auto albumIt = it.value().begin(); albumIt != it.value().end(); ++albumIt) {
            for (auto discIt = albumIt.value().begin(); discIt != albumIt.value().end(); ++discIt) {
                for (auto trackIt = discIt.value().begin(); trackIt != discIt.value().end(); ++trackIt) {
                    if (trackIt.value()["title"] == currentSong) {
                        std::string genre = trackIt.value()["genre"];
                        std::string artist = it.key();
                        currentLyrics = trackIt.value()["lyrics"];
                        return {genre, artist}; // Return the genre and artist as a pair
                    }
                }
            }
        }
    }

    return {}; // Return empty pair if genre and artist not found
}

std::vector<std::string> splitStringByNewlines(const std::string& str) {
    std::vector<std::string> lines;
    std::istringstream stream(str);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

int main() {
    // Initialize ncurses
    lmus_cache_main(songsDirectory);
    ncursesSetup();

    // Cache directory and song information 
    std::vector<std::string> allArtists = parseArtists(cacheArtistDirectory);
    auto [songTitles, songPaths] = listSongs(cacheDirectory, allArtists[0]); // default to the first artist
 

    // Check if songs are found
    if (songTitles.empty() || songPaths.empty() || allArtists.empty()) {
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
    int menu_height = 44;
    int menu_width = 90;
    int title_width = 206;

    WINDOW* title_win = newwin(title_height, title_width, 0, 0);
    wbkgd(title_win, COLOR_PAIR(GREY_BACKGROUND_COLOR));
    box(title_win, 0, 0);
    wrefresh(title_win);

    WINDOW* artist_menu_win = newwin(menu_height, menu_width, 1, 0);
    WINDOW* song_menu_win = newwin(menu_height, menu_width + 29, 1, menu_width);
    WINDOW* status_win = newwin(10, 230, LINES - 2, 0);

    // Set menus to their respective windows
    set_menu_win(artistMenu, artist_menu_win);
    set_menu_sub(artistMenu, derwin(artist_menu_win, menu_height - 3, menu_width - 2, 2, 1));
    set_menu_mark(artistMenu, " > ");

    set_menu_win(songMenu, song_menu_win);
    set_menu_sub(songMenu, derwin(song_menu_win, menu_height - 3, menu_width - 2, 2, 1));
    set_menu_mark(songMenu, " > ");

    set_menu_format(artistMenu, menu_height, 0);
    set_menu_format(songMenu, menu_height, 0);

    ncursesWinControl(artist_menu_win, song_menu_win, status_win, title_win, "box");

   ncursesWinControl(artist_menu_win, song_menu_win, status_win, title_win, "refresh"); 

    // Initialize SFML Music
    sf::Music music;
    int currentSongIndex = -1;
    std::string currentSong = songTitles.empty() ? "" : songTitles[0];
    std::string currentArtist = allArtists.empty() ? "" : allArtists[0];
    std::string currentGenre = "";
    std::string currentLyrics = "";

    // Timeout for getch() to avoid blocking indefinitely
    timeout(100);

    // Flag to track first enter press
    bool firstEnterPressed = false;
    bool showingArtists = true;  // Start with artist menu in focus
    bool showingLyrics = false;

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
                        set_menu_format(songMenu, menu_height, 0);
                        highlightFocusedWindow(artistMenu, false);
                        highlightFocusedWindow(songMenu, true);
                        wrefresh(song_menu_win);
                    }
                    break;
                case KEY_DOWN:
                case 'k':
                    if (showingArtists) {
                        int itemCount = item_count(artistMenu);
                        ITEM* curItem = current_item(artistMenu);
                        int currentIndex = item_index(curItem);
                        if (currentIndex < itemCount - 1) {
                            werase(artist_menu_win);
                            menu_driver(artistMenu, REQ_DOWN_ITEM);
                        }
                    } else {
                        int itemCount = item_count(songMenu);
                        ITEM* curItem = current_item(songMenu);
                        int currentIndex = item_index(curItem);
                        if (currentIndex < itemCount - 1) {
                            werase(song_menu_win);
                            menu_driver(songMenu, REQ_DOWN_ITEM);
                        }
                    }
                    break;
                case KEY_UP:
                case 'j':
                    if (showingArtists) {
                        ITEM* curItem = current_item(artistMenu);
                        int currentIndex = item_index(curItem);
                        if (currentIndex > 0) {
                            werase(artist_menu_win);
                            menu_driver(artistMenu, REQ_UP_ITEM);
                        }
                    } else {
                        ITEM* curItem = current_item(songMenu);
                        int currentIndex = item_index(curItem);
                        if (currentIndex > 0) {
                            werase(song_menu_win);
                            menu_driver(songMenu, REQ_UP_ITEM);
                        }
                    }
                    break;
                case KEY_RIGHT:
                  music.setPlayingOffset(music.getPlayingOffset() + sf::seconds(5));
                  break;
                case KEY_LEFT:
                    if (music.getPlayingOffset() > sf::seconds(5)) {
                          music.setPlayingOffset(music.getPlayingOffset() - sf::seconds(5));
                    } else {
                          music.setPlayingOffset(sf::seconds(0));
                    }
                    break;
            case '/': // or case CTRL_F: for Ctrl+F
    {
        char search_str[256];
        int x, y;
        getmaxyx(stdscr, y, x); // get the screen dimensions
        int input_width = 40; // adjust this to your liking
        int input_height = 3;
        int input_x = x / 3; // center the input field
        int input_y = y / 2; // center the input field

        // create a window for the input field
        WINDOW *input_win = newwin(input_height, input_width, input_y, input_x);
        wattron(input_win, COLOR_PAIR(COLOR_RED));
        box(input_win, 0, 0); // add a border around the input field
        mvwprintw(input_win, 0, 2, " Search: "); // prompt the user
        wrefresh(input_win); // refresh the input window

        // get the user input
        int i = 0;

        while (true) {
            int c = wgetch(input_win);
            if (c == '\n') {
                break;
            } else if (c == KEY_BACKSPACE || c == 127) { // backspace
                if (i > 0) {
                    search_str[--i] = '\0'; // remove the last character from the array
                    mvwprintw(input_win, 1, 1, "%*s", input_width - 25, " "); // erase the last character
                    mvwprintw(input_win, 1, 1, "%*s", input_width - 25, search_str); // reprint the input string
                    wrefresh(input_win);
                }
            } else if (c == 27) { // escape
                break;
            } else if (isprint(c)) { // only add printable characters to the input string
                search_str[i++] = c;
                search_str[i] = '\0'; // null-terminate the string
                mvwprintw(input_win, 1, 1, "%*s", input_width - 25, search_str); // print the input string
                wrefresh(input_win);
            }
            if (i >= 255) {
                break;
            }
        }
        search_str[i] = '\0'; // null-terminate the string
        // destroy the input window
        delwin(input_win);

        if (showingArtists) {
            ITEM **items = menu_items(artistMenu);
            int i;
            for (i = 0; i < item_count(artistMenu); i++) {
                const char *itemName = item_name(items[i]);
                if (strcasestr(itemName, search_str) != NULL) {
                    set_current_item(artistMenu, items[i]);
                    break;
                }
            }
        } else {
            ITEM **items = menu_items(songMenu);
            int i;
            for (i = 0; i < item_count(songMenu); i++) {
                const char *itemName = item_name(items[i]);
                if (strcasestr(itemName, search_str) != NULL) {
                    set_current_item(songMenu, items[i]);
                    break;
                }
            }
        }
        break;
    }
            case 10:  // Enter key
              if (showingArtists) {
                  // Show details of selected artist (if needed)
                  ITEM* artItem = current_item(artistMenu);
                  int artselectedIndex = item_index(artItem); 
                  post_menu(artistMenu);
                  post_menu(songMenu);
                  box(artist_menu_win, 0, 0);
                  const char* selectedArtist = allArtists[artselectedIndex].c_str();

                  // Update song menu with songs of the selected artist
                      auto [newSongTitles, newSongPaths] = listSongs(cacheDirectory, selectedArtist);
                      for (size_t i = 0; i < songTitles.size(); ++i) {
                          free_item(songItems[i]);
                      }
                      free_menu(songMenu);
                      werase(song_menu_win);
                      songItems = new ITEM*[newSongTitles.size() + 1];
                      for (size_t i = 0; i < newSongTitles.size(); ++i) {
                          songItems[i] = new_item(strdup(newSongTitles[i].c_str()), "");
                      }
                      songItems[newSongTitles.size()] = nullptr;
                      songMenu = new_menu(songItems);
                      set_menu_win(songMenu, song_menu_win);
                      set_menu_sub(songMenu, derwin(song_menu_win, menu_height - 3, menu_width - 2, 2, 1));
                      set_menu_mark(songMenu, " > ");
                      set_menu_format(songMenu, menu_height, 0);
                      post_menu(songMenu);

                      songTitles = newSongTitles;
                      songPaths = newSongPaths;

                      showingArtists =!showingArtists;
                      highlightFocusedWindow(artistMenu, showingArtists);
                      highlightFocusedWindow(songMenu,!showingArtists);  // Highlight the song menu when switching focus
                      wrefresh(artist_menu_win);
                      wrefresh(song_menu_win);
                  } else {
                      // Play selected song from song menu
                      ITEM* curItem = current_item(songMenu);
                      int selectedIndex = item_index(curItem); 
                      post_menu(songMenu);
                      post_menu(artistMenu);
                      box(song_menu_win, 0, 0);
                      highlightFocusedWindow(artistMenu, showingArtists);  // Highlight the artist menu when switching focus
                      mvwprintw(song_menu_win, 0, 2, " Songs: ");
                      wrefresh(song_menu_win);
                      if (selectedIndex >= 0 && selectedIndex < songPaths.size()) {
                          currentSongIndex = selectedIndex;
                          playMusic(music, songPaths[currentSongIndex]);
                          currentSong = songTitles[currentSongIndex];
                          auto resultGA = findCurrentGenreArtist(cacheDirectory, currentSong, currentLyrics);
                          currentGenre = resultGA.first;
                          currentArtist = resultGA.second;
                          updateStatusBar(status_win, currentSong, currentArtist, currentGenre,  music, firstEnterPressed, showingLyrics);                          
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
                        music.setPlayingOffset(music.getPlayingOffset() + sf::seconds(60));
                        break;
                    case 'g':  // Rewind (go back 5 seconds)
                        if (music.getPlayingOffset() > sf::seconds(60)) {
                            music.setPlayingOffset(music.getPlayingOffset() - sf::seconds(60));
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
                        if (firstEnterPressed) {
                        nextSong(music, songPaths, currentSongIndex);
                        currentSong = songTitles[currentSongIndex];
                        updateStatusBar(status_win, currentSong, currentArtist, currentGenre,  music, firstEnterPressed, showingLyrics);
                      }
                    break;
                case 'b':  // Previous song
                    if (firstEnterPressed) {
                      previousSong(music, songPaths, currentSongIndex);
                      currentSong = songTitles[currentSongIndex];
                      updateStatusBar(status_win, currentSong, currentArtist, currentGenre, music, firstEnterPressed, showingLyrics);
                    }
                    break;
                case '2':  // Display help window
                    displayHelpWindow(artist_menu_win);
                    break;
                case '3':
                    if (currentLyrics.size() != 0) { 
                      werase(artist_menu_win);
                      int x,y;
                      getmaxyx(stdscr, y, x); // get the screen dimensions
                      int warning_width = 55; // adjust this to your liking
                      int warning_height = 5;
                      int warning_x = x / 2; // center the input field
                      int warning_y = y / 2; // center the input field
                      std::vector<std::string> lines = splitStringByNewlines(currentLyrics);
                      WINDOW *warning_win = newwin(warning_height, warning_width, warning_y, warning_x);
                      wattron(warning_win, COLOR_PAIR(COLOR_RED));
                      box(warning_win, 0, 0); // add a border around the input field
                      mvwprintw(warning_win, 0, 2, " Warning: ");
                      mvwprintw(warning_win, 1, 2, " You are in Lyrics view (&&), press 1 to exit! "); // prompt the user
                      mvwprintw(warning_win, 3, 2, " NO KEYBINDS EXCEPT KEY_UP and KEY_DOWN WILL WORK!!");
                      wrefresh(warning_win);
                      
                      WINDOW* lyrics_win = derwin(artist_menu_win, menu_height - 2, menu_width - 2, 1, 1);
                      mvwprintw(artist_menu_win, 0, 2, " Lyrics: ");

                      int start_line = 0; // To keep track of the starting line for scrolling

                      // Initial display of lyrics
                      printMultiLine(lyrics_win, lines, start_line);
                      wrefresh(lyrics_win);

                      int ch;
                      while ((ch = getch()) != '1') { // Press '1' to exit
                          switch (ch) {
                              case KEY_UP:
                                  if (start_line > 0) {
                                      start_line--;
                                  }
                                  break;
                              case KEY_DOWN:
                                  if (start_line + menu_height - 2 < lines.size()) {
                                      start_line++;
                                  }
                                  break; 
                          }
                          werase(lyrics_win); // Clear the sub-window
                          printMultiLine(lyrics_win, lines, start_line);
                          showingLyrics = true;
                          updateStatusBar(status_win, currentSong, currentArtist, currentGenre, music, firstEnterPressed, showingLyrics);
                          wrefresh(lyrics_win);
                          
                      }

                      delwin(lyrics_win);
                      delwin(warning_win);
                    
                    } else {
                      mvwprintw(artist_menu_win, 2, 20, "NO LYRICS FOR THIS SONG!");
                      mvwprintw(artist_menu_win, 4, 20, "Redirecting to main window...");
                      std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                    werase(artist_menu_win);
                    // werase(song_menu_win);
                    set_menu_win(artistMenu, artist_menu_win);
                    set_menu_sub(artistMenu, derwin(artist_menu_win, menu_height - 3, menu_width - 2, 2, 1));
                    set_menu_mark(artistMenu, " > ");
                    set_menu_format(artistMenu, menu_height, 0);
                    post_menu(artistMenu);
                    showingLyrics = false;
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
            updateStatusBar(status_win, currentSong, currentArtist, currentGenre, music, firstEnterPressed, showingLyrics);
        }

        // Update status bar and refresh windows
        updateStatusBar(status_win, currentSong, currentArtist, currentGenre, music, firstEnterPressed, showingLyrics);

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
        mvwprintw(artist_menu_win, 0, 2, " Artists: ");
        mvwprintw(song_menu_win, 0, 2, " Songs: ");
        wattroff(title_win, COLOR_PAIR(5));
        wattroff(title_win, COLOR_PAIR(6));
        wrefresh(title_win);
        post_menu(artistMenu);  // Post the active menu
        post_menu(songMenu);
        ncursesWinControl(artist_menu_win, song_menu_win, status_win, title_win, "refresh");

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
    ncursesWinControl(artist_menu_win, song_menu_win, status_win, title_win, "delete");
    endwin();

    return 0;
}
