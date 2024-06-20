#include <SFML/Audio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <ncurses.h>
#include <menu.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <cstring>

// Function to list MP3 files in the directory
std::vector<std::string> listSongs(const std::string& directory) {
    std::vector<std::string> songs;
    DIR* dir = opendir(directory.c_str());
    if (dir) {
        struct dirent* ent;
        while ((ent = readdir(dir)) != nullptr) {
            std::string file = ent->d_name;
            if (file.size() > 4 && file.substr(file.size() - 4) == ".mp3") {
                songs.push_back(file);
            }
        }
        closedir(dir);
    } else {
        std::cerr << "Could not open directory: " << directory << std::endl;
    }
    return songs;
}

void printControls(WINDOW* win) {
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 2, "Controls:");
    wattroff(win, A_BOLD);
    mvwprintw(win, 3, 2, "p - Pause/Play");
    mvwprintw(win, 4, 2, "f - Seek forward 5 seconds");
    mvwprintw(win, 5, 2, "g - Seek backward 5 seconds");
    mvwprintw(win, 6, 2, "r - Replay song");
    mvwprintw(win, 7, 2, "j - Move up");
    mvwprintw(win, 8, 2, "k - Move down");
    mvwprintw(win, 9, 2, "9 - Volume up");
    mvwprintw(win, 10, 2, "0 - Volume down");
    mvwprintw(win, 11, 2, "n - Next song");
    mvwprintw(win, 12, 2, "b - Previous song");
    mvwprintw(win, 13, 2, "q - Quit");
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
}

void updateStatusBar(WINDOW* status_win, const std::string& songName, const sf::Music& music) {
    const int maxSongNameLength = 40;  // Maximum characters to show
    std::string displayName = songName.length() > maxSongNameLength ? songName.substr(0, maxSongNameLength) + "..." : songName;

    wmove(status_win, 1, 1);
    wclrtoeol(status_win);

    wattron(status_win, COLOR_PAIR(5));  // Set color for the status bar background
    wbkgd(status_win, COLOR_PAIR(5) | A_BOLD); // Set background color

    // Set text color to black
    wattron(status_win, COLOR_PAIR(6));  // Assuming COLOR_PAIR(6) is set to black text
    mvwprintw(status_win, 1, 1, " Now Playing: %s", displayName.c_str());

    // Get current position and duration
    sf::Time currentTime = music.getPlayingOffset();
    sf::Time duration = music.getDuration();
    int posMinutes = static_cast<int>(currentTime.asSeconds()) / 60;
    int posSeconds = static_cast<int>(currentTime.asSeconds()) % 60;
    int durMinutes = static_cast<int>(duration.asSeconds()) / 60;
    int durSeconds = static_cast<int>(duration.asSeconds()) % 60;

    char timeStr[50];
    snprintf(timeStr, sizeof(timeStr), " | (%02d:%02d / %02d:%02d) ", posMinutes, posSeconds, durMinutes, durSeconds);
    mvwprintw(status_win, 1, strlen(displayName.c_str()) + 14, "%s", timeStr);
    wattroff(status_win, COLOR_PAIR(5));  // Reset background color
    wattroff(status_win, COLOR_PAIR(6));  // Reset text color
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
    // Initialize ncurses
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    init_pair(1, COLOR_CYAN, COLOR_BLACK);   // For the menu items
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // For the controls
    init_pair(3, COLOR_GREEN, COLOR_BLACK);  // For the playing song
    init_pair(4, COLOR_BLUE, COLOR_BLACK);   // For the status bar
    init_pair(5, COLOR_BLACK, COLOR_BLACK);  // For the greyish status bar background
    init_pair(6, COLOR_BLACK, COLOR_WHITE);  // For text color in the status bar

    const std::string songsDirectory = "/home/s1dd/Downloads/Songs/";
    std::vector<std::string> songs = listSongs(songsDirectory);

    if (songs.empty()) {
        printw("No songs found in directory.\n");
        refresh();
        endwin();
        return -1;
    }

    // Create menu items
    ITEM** items = new ITEM*[songs.size() + 1];
    for (size_t i = 0; i < songs.size(); ++i) {
        items[i] = new_item(songs[i].c_str(), "");
    }
    items[songs.size()] = nullptr;

    // Create the menu 

    WINDOW* title_win = newwin(3, 100, 0, 0);  // Title bar at the top 
                                                  
    wbkgd(title_win, COLOR_PAIR(1));
    wattron(title_win, A_BOLD | COLOR_PAIR(1));
    mvwprintw(title_win, 1, (getmaxx(title_win) - strlen("litemus")) / 2, "litemus");
    wattroff(title_win, A_BOLD | COLOR_PAIR(1));
    wrefresh(title_win);

    MENU* menu = new_menu(items);

    // Calculate the dimensions of the windows
    int menu_height = songs.size() + 4 > 25 ? 25 : songs.size() + 4;
    int menu_width = 100;
    int controls_height = 15;
    int controls_width = 50; 

    WINDOW* menu_win = newwin(menu_height, menu_width, 2, 2);
    WINDOW* controls_win = newwin(controls_height, controls_width, menu_height + 3, 2);
    WINDOW* status_win = newwin(10, 200, LINES - 3, 0); // Status bar at the bottom

    // Set the menu window and sub-window
    set_menu_win(menu, menu_win);
    set_menu_sub(menu, derwin(menu_win, menu_height - 4, menu_width - 2, 3, 1));
    set_menu_mark(menu, " > ");

    // Box the windows
    box(menu_win, 0, 0);
    box(controls_win, 0, 0);
    box(status_win, 0, 0);
    box(title_win, 0, 0);

    // Print title and controls
    printTitle(menu_win, "Select a song to play");
    printControls(controls_win);
    mvwprintw(status_win, 1, 1, "Now Playing: ");
    wrefresh(title_win); // Refresh the title window
    wrefresh(menu_win);
    wrefresh(controls_win);
    wrefresh(status_win);

    sf::Music music;
    int currentSongIndex = 0;
    std::string currentSong = songs.empty() ? "" : songs[0];
    // playMusic(music, songsDirectory + currentSong);

    // Set ncurses timeout
    timeout(100); // Set timeout to 100ms

    while (true) {
        int ch = getch();
        if (ch != ERR) {  // Handle user input
            switch (ch) {
                case KEY_DOWN:
                case 'k':
                    menu_driver(menu, REQ_DOWN_ITEM);
                    break;
                case KEY_UP:
                case 'j':
                    menu_driver(menu, REQ_UP_ITEM);
                    break;
                case 10: // Enter key
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
                case 'p': // Pause/Resume
                    if (music.getStatus() == sf::Music::Paused) {
                        music.play();
                        mvwprintw(controls_win, 12, 2, "Music resumed.  ");
                    } else {
                        music.pause();
                        mvwprintw(controls_win, 12, 2, "Music paused.   ");
                    }
                    break;
                case 'f': // Seek 5 seconds forward
                    music.setPlayingOffset(music.getPlayingOffset() + sf::seconds(5));
                    mvwprintw(controls_win, 12, 2, "Seeking 5 seconds forward.   ");
                    break;
                case 'g': // Seek 5 seconds backward
                    if (music.getPlayingOffset() > sf::seconds(5)) {
                        music.setPlayingOffset(music.getPlayingOffset() - sf::seconds(5));
                        mvwprintw(controls_win, 12, 2, "Seeking 5 seconds backward.   ");
                    } else {
                        music.setPlayingOffset(sf::seconds(0));
                        mvwprintw(controls_win, 12, 2, "Seeking to the start of the song.   ");
                    }
                    break;
                case 'r': // Replay the song
                    music.stop();
                    music.play();
                    mvwprintw(controls_win, 12, 2, "Replaying the song.   ");
                    break;
                case '9': // Volume Up
                    adjustVolume(music, 10.f);
                    mvwprintw(controls_win, 12, 2, "Volume increased.   ");
                    break;
                case '0': // Volume Down
                    adjustVolume(music, -10.f);
                    mvwprintw(controls_win, 12, 2, "Volume decreased.   ");
                    break;
                case 'n': // Next song
                    nextSong(music, songs, currentSongIndex, songsDirectory);
                    currentSong = songs[currentSongIndex];
                    updateStatusBar(status_win, currentSong, music);
                    mvwprintw(controls_win, 12, 2, "Next song selected.   ");
                    break;
                case 'b': // Previous song
                    previousSong(music, songs, currentSongIndex, songsDirectory);
                    currentSong = songs[currentSongIndex];
                    updateStatusBar(status_win, currentSong, music);
                    mvwprintw(controls_win, 12, 2, "Previous song selected.   ");
                    break;
                case 'q': // Quit
                    music.stop();
                    endwin();
                    return 0;
                default:
                    mvwprintw(controls_win, 12, 2, "Invalid input.                ");
            }
        }

        // Always update status bar
        updateStatusBar(status_win, currentSong, music);

        // Redraw windows and refresh
        wrefresh(menu_win);
        box(menu_win, 0, 0);
        printTitle(menu_win, "Select a song to play");
        mvwprintw(controls_win, 12, 2, "                                  "); // Clear message line
        printControls(controls_win);
        post_menu(menu);

        wrefresh(menu_win);
        wrefresh(controls_win);
        wrefresh(status_win);
        wrefresh(title_win); // Refresh the title window

        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Periodic update
    }

    // Cleanup menu
    unpost_menu(menu);
    for (size_t i = 0; i < songs.size(); ++i) {
        free_item(items[i]);
    }
    free_menu(menu);
    delwin(menu_win);
    delwin(controls_win);
    delwin(status_win);
    delwin(title_win); // Delete the title window
    endwin();

    return 0;
}
