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

#define GREY_BACKGROUND_COLOR 7

const char* title_content = "  LITEMUS - Light Music player                                                                                                                                                                       ";

using json = nlohmann::json;

void changeDirectory(const std::string& path) {
    if (chdir(path.c_str()) != 0) {
        std::cerr << "Directory not found" << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::vector<std::string> listSongs(const std::string& directory, const std::string& cacheFile) {
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

    if (!j.is_array()) {
        std::cerr << "Invalid JSON format: Expected an array" << std::endl;
        return {};
    }

    std::vector<std::string> songs;
    for (const auto& songName : j) {
        if (songName.is_string()) {
            songs.push_back(songName.get<std::string>());
        } else {
            std::cerr << "Invalid JSON format: Array element is not a string" << std::endl;
            return {};
        }
    }

    return songs;
}

void displayHelpWindow(WINDOW* menu_win) {
    werase(menu_win);
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 1, 2, "Help Controls");
    mvwprintw(menu_win, 3, 2, "p - Pause/Play");
    mvwprintw(menu_win, 4, 2, "f - Seek forward 5 seconds");
    mvwprintw(menu_win, 5, 2, "g - Seek backward 5 seconds");
    mvwprintw(menu_win, 6, 2, "r - Replay song");
    mvwprintw(menu_win, 7, 2, "j - Move up");
    mvwprintw(menu_win, 8, 2, "k - Move down");
    mvwprintw(menu_win, 9, 2, "q - Quit");
    mvwprintw(menu_win, 10, 2, "n - Next Song");
    mvwprintw(menu_win, 11, 2, "b - Previous Song");
    mvwprintw(menu_win, 12, 2, "9 - Increase Volume");
    mvwprintw(menu_win, 13, 2, "0 - Decrease Volume");
    mvwprintw(menu_win, 15, 2, "2 - To show help menu");
init_pair(GREY_BACKGROUND_COLOR, COLOR_BLACK, COLOR_WHITE);  // Grey background and black text for title
    mvwprintw(menu_win, 17, 2, "Press '1' to go back to the menu");
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

void updateStatusBar(WINDOW* status_win, const std::string& songName, const sf::Music& music) {
    const int maxSongNameLength = 45;
    std::string displayName = songName.length() > maxSongNameLength ? songName.substr(0, maxSongNameLength) + "..." : songName;

    wmove(status_win, 1, 1);
    wclrtoeol(status_win);

    wattron(status_win, COLOR_PAIR(5));
    wbkgd(status_win, COLOR_PAIR(5) | A_BOLD);
    wattron(status_win, COLOR_PAIR(6));

    const char* playPauseSymbol = (music.getStatus() == sf::Music::Playing) ? "<>" : "!!";
    sf::Time currentTime = music.getPlayingOffset();
    sf::Time duration = music.getDuration();
    int posMinutes = static_cast<int>(currentTime.asSeconds()) / 60;
    int posSeconds = static_cast<int>(currentTime.asSeconds()) % 60;
    int durMinutes = static_cast<int>(duration.asSeconds()) / 60;
    int durSeconds = static_cast<int>(duration.asSeconds()) % 60;

    float volume = music.getVolume();

    mvwprintw(status_win, 1, 1, " Status:  %s   |   %s   |   %02d:%02d / %02d:%02d   |  Vol. %.0f%%                                                                                        Litemus ", playPauseSymbol, displayName.c_str(), posMinutes, posSeconds, durMinutes, durSeconds, volume);

    wattroff(status_win, COLOR_PAIR(5));
    wattroff(status_win, COLOR_PAIR(6));
    wrefresh(status_win);
}

void nextSong(sf::Music& music, const std::vector<std::string>& songs, int& currentSongIndex, const std::string& directory) {
    music.stop();
    currentSongIndex = (currentSongIndex + 1) % songs.size();
    std::string nextSongPath = directory + songs[currentSongIndex];
    playMusic(music, nextSongPath);    
}

void previousSong(sf::Music& music, const std::vector<std::string>& songs, int& currentSongIndex, const std::string& directory) {
    music.stop();
    currentSongIndex = (currentSongIndex - 1 + songs.size()) % songs.size();
    std::string prevSongPath = directory + songs[currentSongIndex];
    playMusic(music, prevSongPath);
}

void adjustVolume(sf::Music& music, float volumeChange) {
    float currentVolume = music.getVolume();
    currentVolume += volumeChange;
    currentVolume = std::max(0.f, std::min(100.f, currentVolume));
    music.setVolume(currentVolume);
}

int main() {
    const std::string songsDirectory = "/home/s1dd/Downloads/Songs/";
    changeDirectory(songsDirectory);

    setlocale(LC_ALL, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_BLACK, COLOR_BLACK);
    init_pair(6, COLOR_BLACK, COLOR_WHITE);
    init_pair(GREY_BACKGROUND_COLOR, COLOR_WHITE, GREY_BACKGROUND_COLOR);

    const std::string cacheDirectory = "/home/s1dd/Downloads/Songs/.cache/litemus/info/song_names.json";
    std::vector<std::string> songs = listSongs(songsDirectory, cacheDirectory);

    if (songs.empty()) {
        printw("No songs found in directory.\n");
        refresh();
        endwin();
        return -1;
    }

    ITEM** items = new ITEM*[songs.size() + 1];
    for (size_t i = 0; i < songs.size(); ++i) {
        items[i] = new_item(songs[i].c_str(), "");
    }
    items[songs.size()] = nullptr;

    MENU* menu = new_menu(items);

    int title_height = 2;
    int menu_height = songs.size() + 4 > 42 ? 42 : songs.size() + 4;
    int menu_width = 100;
    int title_width = 206;
    int controls_height = 15;
    int controls_width = 50;

    WINDOW* title_win = newwin(title_height, title_width, 0, 2);
    wbkgd(title_win, COLOR_PAIR(GREY_BACKGROUND_COLOR));
    box(title_win, 0, 0);
    wrefresh(title_win);

    WINDOW* menu_win = newwin(menu_height, menu_width, title_height, 2);
    WINDOW* controls_win = newwin(controls_height, controls_width, title_height + menu_height + 1, 2);
    WINDOW* status_win = newwin(10, 200, LINES - 3, 0);

    set_menu_win(menu, menu_win);
    set_menu_sub(menu, derwin(menu_win, menu_height - 3, menu_width - 2, 3, 1));
    set_menu_mark(menu, " > ");

    box(menu_win, 0, 0);
    box(controls_win, 0, 0);
    box(status_win, 0, 0);
    box(title_win, 0, 0);

    printTitle(menu_win, "Select a song to play: ");
    wrefresh(menu_win);
    wrefresh(controls_win);
    wrefresh(status_win);

    sf::Music music;
    int currentSongIndex = 0;
    std::string currentSong = songs.empty() ? "" : songs[0];
  
    timeout(100);

    while (true) {
        int ch = getch();
        if (ch != ERR) {
            switch (ch) {
                case '1':
                    werase(menu_win);
                    menu_driver(menu, REQ_UP_ITEM);
                    menu_driver(menu, REQ_DOWN_ITEM);
                    break;
                case KEY_DOWN:
                case 'k':
                    werase(menu_win);
                    menu_driver(menu, REQ_DOWN_ITEM);
                    break;
                case KEY_UP:
                case 'j':
                    werase(menu_win);
                    menu_driver(menu, REQ_UP_ITEM);
                    break;
                case KEY_RIGHT:
                    werase(menu_win);
                    menu_driver(menu, REQ_SCR_DPAGE);
                    break;
                case KEY_LEFT:
                    werase(menu_win);
                    menu_driver(menu, REQ_SCR_UPAGE);
                    break;
                case 10:
                {
                    ITEM* curItem = current_item(menu);
                    std::string selectedSong = item_name(curItem);
                    if (selectedSong != currentSong) {
                        currentSong = selectedSong;
                        currentSongIndex = std::distance(songs.begin(), std::find(songs.begin(), songs.end(), selectedSong));
                        playMusic(music, songsDirectory + currentSong);
                        updateStatusBar(status_win, currentSong, music);
                    }
                }
                break;
                case 'p':
                    if (music.getStatus() == sf::Music::Paused) {
                        music.play();
                    } else {
                        music.pause();
                    }
                    break;
                case 'f':
                    music.setPlayingOffset(music.getPlayingOffset() + sf::seconds(5));
                    break;
                case 'g':
                    if (music.getPlayingOffset() > sf::seconds(5)) {
                        music.setPlayingOffset(music.getPlayingOffset() - sf::seconds(5));
                    } else {
                        music.setPlayingOffset(sf::seconds(0));
                    }
                    break;
                case 'r':
                    music.stop();
                    music.play();
                    break;
                case '9':
                    adjustVolume(music, 10.f);
                    break;
                case '0':
                    adjustVolume(music, -10.f);
                    break;
                case 'n':
                    nextSong(music, songs, currentSongIndex, songsDirectory);
                    currentSong = songs[currentSongIndex];
                    updateStatusBar(status_win, currentSong, music);
                    break;
                case 'b':
                    previousSong(music, songs, currentSongIndex, songsDirectory);
                    currentSong = songs[currentSongIndex];
                    updateStatusBar(status_win, currentSong, music);
                    break;
                case '2':
                    displayHelpWindow(menu_win);
                    break;
                case 'q':
                    music.stop();
                    endwin();
                    return 0;
                default:
                    mvwprintw(controls_win, 12, 2, "Invalid input.");
            }
        }

        updateStatusBar(status_win, currentSong, music);

        wrefresh(menu_win);
        box(menu_win, 0, 0);
        wmove(title_win, 0, 0);
        wclrtoeol(title_win);
        wattron(title_win, COLOR_PAIR(5));
        wbkgd(title_win, COLOR_PAIR(5) | A_BOLD);
        wattron(title_win, COLOR_PAIR(6));
        mvwprintw(title_win, 1, 1, title_content);
        wattroff(title_win, COLOR_PAIR(5));
        wattroff(title_win, COLOR_PAIR(6));
        wrefresh(title_win);
        printTitle(menu_win, "Select a song to play: ");
        mvwprintw(controls_win, 12, 2, "                                  ");
        post_menu(menu);

        wrefresh(menu_win);
        wrefresh(controls_win);
        wrefresh(status_win);
        wrefresh(title_win);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    unpost_menu(menu);
    for (size_t i = 0; i < songs.size(); ++i) {
        free_item(items[i]);
    }
    free_menu(menu);
    delwin(menu_win);
    delwin(controls_win);
    delwin(status_win);
    delwin(title_win);
    endwin();

    return 0;
}
