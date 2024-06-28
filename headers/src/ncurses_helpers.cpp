#include "../ncurses_helpers.hpp"

#define GREY_BACKGROUND_COLOR 7
#define LIGHT_GREEN_COLOR 8

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

void ncursesWinControl(WINDOW* artist_menu_win, WINDOW* song_menu_win, WINDOW* status_win, WINDOW* title_win, const std::string& choice) {
  // Refresh all windows
  if (choice == "refresh") {
    wrefresh(artist_menu_win);
    wrefresh(song_menu_win);
    wrefresh(status_win);
    wrefresh(title_win);
  }
  else if (choice == "box") {
    box(artist_menu_win, 0, 0);
    box(song_menu_win, 0, 0);
    box(status_win, 0, 0);
    box(title_win, 0, 0);
  }
  else if (choice == "delete") {
    delwin(artist_menu_win);
    delwin(song_menu_win);
    delwin(status_win);
    delwin(title_win);
  }
}

void displayHelpWindow(WINDOW* menu_win) {
    werase(menu_win);
    // set_menu_fore(menu_win, A_NORMAL);
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 2, 2, "HELP WINDOW");
    mvwprintw(menu_win, 4, 2, "p - Pause/Play");
    mvwprintw(menu_win, 5, 2, "Enter - Play selected song");
    mvwprintw(menu_win, 6, 2, "Right Arrow - Seek forward 5 seconds");
    mvwprintw(menu_win, 7, 2, "Left Arrow - Seek backward 5 seconds");
    mvwprintw(menu_win, 8, 2, "f - Seek forward 60 seconds");
    mvwprintw(menu_win, 9, 2, "g - Seek backward 60 seconds");
    mvwprintw(menu_win, 10, 2, "r - Replay current song");
    mvwprintw(menu_win, 11, 2, "j - Move up");
    mvwprintw(menu_win, 12, 2, "k - Move down");
    mvwprintw(menu_win, 13, 2, "q - Quit");
    mvwprintw(menu_win, 14, 2, "n - Next Song");
    mvwprintw(menu_win, 15, 2, "b - Previous Song");
    mvwprintw(menu_win, 16, 2, "9 - Increase Volume");
    mvwprintw(menu_win, 17, 2, "0 - Decrease Volume");
    mvwprintw(menu_win, 18, 2, "/ - String search in focused window");
    mvwprintw(menu_win, 19, 2, "Tab - Toggle Focused Window");
    mvwprintw(menu_win, 21, 2, "2 - To show help menu");
init_pair(GREY_BACKGROUND_COLOR, COLOR_BLACK, COLOR_WHITE);  // Grey background and black text for title
    mvwprintw(menu_win, 23, 2, "Press '1' to go back to the menu");
    wrefresh(menu_win);
}

void updateStatusBar(WINDOW* status_win, const std::string& songName, const std::string& artistName, const std::string& songGenre, const sf::Music& music, bool firstEnterPressed, bool showingLyrics) {
    const int maxSongNameLength = 50;
    const std::string overallSongName = songName + " by " + artistName;
    std::string displayName = overallSongName.length() > maxSongNameLength ? overallSongName.substr(0, maxSongNameLength) + "..." : overallSongName;

    wmove(status_win, 1, 1);
    wclrtoeol(status_win);

    wattron(status_win, COLOR_PAIR(5));
    wbkgd(status_win, COLOR_PAIR(5) | A_BOLD);
    wattron(status_win, COLOR_PAIR(6));

    const char* playPauseSymbol;
    if (showingLyrics) {
      playPauseSymbol = "&&";
    } else {
      if (music.getStatus() == sf::Music::Playing) {
        playPauseSymbol = "<>";
      }
      else { 
        playPauseSymbol = "!!";
      }
    }
    const char* launchSymbol = "--";
    sf::Time currentTime = music.getPlayingOffset();
    sf::Time duration = music.getDuration();
    int posMinutes = static_cast<int>(currentTime.asSeconds()) / 60;
    int posSeconds = static_cast<int>(currentTime.asSeconds()) % 60;
    int durMinutes = static_cast<int>(duration.asSeconds()) / 60;
    int durSeconds = static_cast<int>(duration.asSeconds()) % 60;

    float volume = music.getVolume();

    firstEnterPressed ? mvwprintw(status_win, 1, 1, "   %s  |  %s  |   %02d:%02d / %02d:%02d   |  Vol. %.0f%%                                                                                     %s  | LITEMUS ", playPauseSymbol, displayName.c_str(), posMinutes, posSeconds, durMinutes, durSeconds, volume, songGenre.c_str()) : mvwprintw(status_win, 1, 1, "  %s   |  Unknown Song   |   00:00 / 00:00   |  Vol. %.0f%%                                                                                        LITEMUS ", launchSymbol, volume);
    wattroff(status_win, COLOR_PAIR(5));
    wattroff(status_win, COLOR_PAIR(6));
    wrefresh(status_win);
}

void highlightFocusedWindow(MENU* menu, bool focused) {
    if (focused) {
        set_menu_fore(menu, COLOR_PAIR(LIGHT_GREEN_COLOR));
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

void printMultiLine(WINDOW* win, const std::vector<std::string>& lines, int start_line) {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    for (int i = 1; i < max_y && (start_line + i) < lines.size(); ++i) {
        mvwprintw(win, i, 0, "%s", lines[start_line + i].c_str());
    }
}
