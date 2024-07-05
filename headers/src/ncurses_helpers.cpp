#include "../ncurses_helpers.hpp"
#include "../parsers.hpp"

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
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
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

void displayWindow(WINDOW* menu_win, const std::string window) {
  if (window == "help") {
    werase(menu_win);
    // set_menu_fore(menu_win, A_NORMAL);
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 0, 2, " HELP WIN: ");
    mvwprintw(menu_win, 2, 2, "p - Toggle playback");
    mvwprintw(menu_win, 3, 2, "Enter - Play selected song");
    mvwprintw(menu_win, 4, 2, "Right Arrow - Seek forward 5s");
    mvwprintw(menu_win, 5, 2, "Left Arrow - Seek backward 5s");
    mvwprintw(menu_win, 6, 2, "f - Seek forward 60s");
    mvwprintw(menu_win, 7, 2, "g - Seek backward 60s");
    mvwprintw(menu_win, 8, 2, "r - Replay current song");
    mvwprintw(menu_win, 9, 2, "j - Move up");
    mvwprintw(menu_win, 10, 2, "k - Move down");
    mvwprintw(menu_win, 11, 2, "q - Quit");
    mvwprintw(menu_win, 12, 2, "n - Next Song");
    mvwprintw(menu_win, 13, 2, "b - Previous Song");
    mvwprintw(menu_win, 14, 2, "9 - Increase Volume");
    mvwprintw(menu_win, 15, 2, "0 - Decrease Volume");
    mvwprintw(menu_win, 16, 2, "m - Toggle mute");
    mvwprintw(menu_win, 17, 2, "/ - String search in focused window");
    mvwprintw(menu_win, 18, 2, "Tab - Toggle Focused Window");
    mvwprintw(menu_win, 20, 2, "2 - To show help menu");
    mvwprintw(menu_win, 21, 2, "3 - Lyrics View");
    mvwprintw(menu_win, 22, 2, "4 - Session Details");
init_pair(GREY_BACKGROUND_COLOR, COLOR_BLACK, COLOR_WHITE);  // Grey background and black text for title
    mvwprintw(menu_win, 24, 2, "Press '1' to go back to the menu");
    wrefresh(menu_win);
  }
  else if (window == "warning") {
      wattron(menu_win, COLOR_PAIR(COLOR_RED));
      box(menu_win, 0, 0); // add a border around the input field
      mvwprintw(menu_win, 0, 2, " Warning: ");
      mvwprintw(menu_win, 1, 2, " You are in Lyrics view (&&), press 1 to exit! "); // prompt the user
      mvwprintw(menu_win, 3, 2, " ONLY THE FOLLOWING KEYBINDS WORK in LYRICS VIEW:");
      mvwprintw(menu_win, 5, 2, "1. Up Arrow/j - scroll up");
      mvwprintw(menu_win, 6, 2, "2. Down Arrow/k - scroll down");
      mvwprintw(menu_win, 7, 2, "3. p - Toggle playback");
      mvwprintw(menu_win, 8, 2, "4. 9 - Increase vol");
      mvwprintw(menu_win, 9, 2, "5. 0 - Decrease vol");
      mvwprintw(menu_win, 12, 2, "Current Song will play in a LOOP if you DO NOT exit lyrics view!!");
      wrefresh(menu_win);
  }
  else if (window == "LyricErr") {
    werase(menu_win);
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 0, 2, " Artists: ");
    mvwprintw(menu_win, 2, 20, "NO LYRICS FOR THIS SONG!");
    mvwprintw(menu_win, 4, 20, "Redirecting to main window...");
    wrefresh(menu_win);
  }
}

void ncursesWinLoop(MENU* artistMenu, MENU* songMenu, WINDOW* artist_menu_win, WINDOW* song_menu_win, WINDOW* status_win, WINDOW* title_win, const char* title_content) {
  ncursesWinControl(artist_menu_win, song_menu_win, status_win, title_win, "refresh");
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
}

void updateStatusBar(WINDOW* status_win, const std::string& songName, const std::string& artistName, const std::string& songGenre, const sf::Music& music, bool firstEnterPressed, bool showingLyrics) {
    const int maxTotalWidth = 201;  // Maximum width of the status bar
    const std::string separator = "  |  ";
    const int separatorLength = separator.length();

    // Fixed elements lengths
    const int playPauseLength = 2;
    const int timeLength = 12;  // "MM:SS / MM:SS"
    const int volumeLength = 10;  // "Vol. 100%"

    // Calculate the remaining space for song and artist names
    const int remainingWidth = maxTotalWidth - (playPauseLength + separatorLength * 3 + timeLength + volumeLength);

    const std::string overallSongName = songName + " by " + artistName;
    const std::string launchName = "Unknown Song";

    std::string displayName = overallSongName.length() > 50 ? overallSongName.substr(0, 47) + "..." : overallSongName;

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
        } else {
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

    std::ostringstream statusStream;
    statusStream << "   " << (firstEnterPressed ? playPauseSymbol : launchSymbol) << separator
                 << (firstEnterPressed ? displayName : launchName) << separator;
    
    if (firstEnterPressed) {
        statusStream << (posMinutes < 10 ? "0" : "") << posMinutes << ":"
                     << (posSeconds < 10 ? "0" : "") << posSeconds << " / "
                     << (durMinutes < 10 ? "0" : "") << durMinutes << ":"
                     << (durSeconds < 10 ? "0" : "") << durSeconds;
    } else {
        statusStream << "00:00 / 00:00";
    }

    statusStream << separator << "Vol. " << volume << "%";

    // Get the current status string
    std::string statusBar = statusStream.str();

    // Add whitespace padding to ensure the status bar is 230 characters wide
    if (statusBar.length() < maxTotalWidth) {
        statusBar.append(maxTotalWidth - statusBar.length() - songGenre.length(), ' ');
    }

    // Append the genre at the end
    statusBar += separator + songGenre + " ";

    mvwprintw(status_win, 1, 1, "%s", statusBar.c_str());

    wattroff(status_win, COLOR_PAIR(5));
    wattroff(status_win, COLOR_PAIR(6));
    wrefresh(status_win);
}

void highlightFocusedWindow(MENU* menu, bool focused) {
    if (focused) { 
        set_menu_fore(menu, COLOR_PAIR(COLOR_RED));  // Set text color to black
        set_menu_back(menu, COLOR_PAIR(A_NORMAL));  // Set background to grey
        wattron(menu_win(menu), COLOR_PAIR(LIGHT_GREEN_COLOR));  // Highlight the item
        box(menu_win(menu), 0, 0);
    } else {
        set_menu_fore(menu, A_NORMAL);
        set_menu_back(menu, COLOR_PAIR(A_NORMAL));
        wattroff(menu_win(menu), COLOR_PAIR(GREY_BACKGROUND_COLOR));
    }
    wrefresh(menu_win(menu));
}


bool showExitConfirmation(WINDOW* parent_win) {
    int height = 6;
    int width = 55;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2;

    WINDOW* confirm_win = newwin(height, width, start_y, start_x);
    wattron(confirm_win, COLOR_PAIR(COLOR_BLUE));
    box(confirm_win, 0, 0);
    mvwprintw(confirm_win, 0, 2, " !!! ");
    mvwprintw(confirm_win, 1, 20, "Exit LITEMUS?");
    mvwprintw(confirm_win, 4, 10, "Yes (Y/Q)                No (N/esc)");

    wrefresh(confirm_win);

    int ch;
    bool exitConfirmed = false;
    while ((ch = wgetch(confirm_win)) != 'y' && ch != 'Y' && ch != 'n' && ch != 'N' && ch != 27 && ch != 10 && ch != 'q' && ch != 'Q') {
        // Wait for valid input
    }
    if (ch == 'y' || ch == 'Y' || ch == 10 || ch == 'q' || ch == 'Q') {
        exitConfirmed = true;
    }

    delwin(confirm_win);
    wrefresh(parent_win); // Refresh the parent window

    return exitConfirmed;
}

void printMultiLine(WINDOW* win, const std::vector<std::string>& lines, int start_line, std::string& currentSong, std::string& currentArtist) {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    mvwprintw(win, 1, 2, " %s by %s", currentSong.c_str(), currentArtist.c_str());
    for (int i = 3; i < max_y && (start_line + i) < lines.size(); ++i) {
        mvwprintw(win, i, 0, " %s", lines[start_line + i - 2].c_str());
    }
}

void ncursesMenuSetup(MENU* Menu, WINDOW* win, int menu_height, int menu_width, const char* type) {
  set_menu_win(Menu, win);
  set_menu_sub(Menu, derwin(win, menu_height - 3, menu_width - 2, 2, 1)); 
  set_menu_mark(Menu, " ->");
}

void move_menu_down(MENU* artistMenu, MENU* songMenu, bool showingArtists) {
    if (showingArtists) {
        int itemCount = item_count(artistMenu);
        ITEM* curItem = current_item(artistMenu);
        int currentIndex = item_index(curItem);
        int topRow = top_row(artistMenu);
        int rows, cols;
        scale_menu(artistMenu, &rows, &cols);
        int lastVisibleItem = topRow + rows - 4;

        if (currentIndex == lastVisibleItem && itemCount >= 41) {
            // Move to the next page of items 
            menu_driver(artistMenu, REQ_SCR_DPAGE);
            set_current_item(artistMenu, curItem);
            topRow = top_row(artistMenu);
            scale_menu(artistMenu, &rows, &cols);
            lastVisibleItem = topRow + rows - 4;
        } else {
            werase(menu_win(artistMenu));
            menu_driver(artistMenu, REQ_DOWN_ITEM);
        }
    } else {
        int itemCount = item_count(songMenu);
        ITEM* curItem = current_item(songMenu);
        int currentIndex = item_index(curItem);
        int topRow = top_row(songMenu);
        int rows, cols;
        scale_menu(songMenu, &rows, &cols);
        int lastVisibleItem = topRow + rows - 4;

        if (currentIndex == lastVisibleItem && currentIndex != 0 && itemCount >= 41) {
            // Move to the next page of items
            menu_driver(songMenu, REQ_SCR_DPAGE);
            set_current_item(songMenu, curItem);
            topRow = top_row(songMenu);
            scale_menu(songMenu, &rows, &cols);
            lastVisibleItem = topRow + rows - 4;
        } else {
          if (currentIndex < itemCount - 1) {
            werase(menu_win(songMenu));
            menu_driver(songMenu, REQ_DOWN_ITEM);
          }
        }
    }
}

void move_menu_up(MENU* artistMenu, MENU* songMenu, bool showingArtists) {
  if (showingArtists) {
      ITEM* curItem = current_item(artistMenu);
      int currentIndex = item_index(curItem);
      if (currentIndex > 0) {
          werase(menu_win(artistMenu));
          menu_driver(artistMenu, REQ_UP_ITEM);
      }
  } else {
      ITEM* curItem = current_item(songMenu);
      int currentIndex = item_index(curItem);
      if (currentIndex > 0) {
          werase(menu_win(songMenu));
          menu_driver(songMenu, REQ_UP_ITEM);
      }
  }
}

ITEM** createItems(const std::string& name, std::vector<std::string>& allArtists, std::vector<std::string>& songTitles, std::vector<std::string>& songDurations) {
  if (name == "artist") {
    ITEM** artistItems = new ITEM*[allArtists.size() + 1];
    for (size_t i = 0; i < allArtists.size(); ++i) {
        artistItems[i] = new_item(allArtists[i].c_str(), "");
    }
    artistItems[allArtists.size()] = nullptr;
    return artistItems;
  }
  else if (name == "song") {
    ITEM** songItems = new ITEM*[songTitles.size() + 1];
    for (size_t i = 0; i < songTitles.size(); ++i) {
        songItems[i] = new_item(strdup(songTitles[i].c_str()), "");
  }
    songItems[songTitles.size()] = nullptr;
    return songItems;
  }
}
