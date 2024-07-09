#include "../keyHandlers.hpp"
#include "../ncurses_helpers.hpp"
#include "../parsers.hpp"
#include "../sfml_helpers.hpp"
#include "../directoryUtils.hpp"

using json = nlohmann::json;

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}

int translateKey(const std::string& key) {
    std::string lowerkey = toLower(key);
    if (lowerkey == "tab") return 9;
    if (lowerkey == "enter") return 10;
    if (lowerkey == "escape") return 27;
    if (lowerkey == "right") return KEY_RIGHT;
    if (lowerkey == "left") return KEY_LEFT;
    if (key.length() == 1) return key[0]; // ASCII character
    return -1; // Invalid key
}

void loadKeybinds(const std::string& filepath, std::unordered_map<std::string, int>& keybinds) {
    std::ifstream keybindsFile(filepath);
    if (!keybindsFile.is_open()) {
        std::cerr << "Failed to open keybinds file: " << filepath << std::endl;
        return;
    }

    std::cout << YELLOW << BOLD << "---------------------- KEYBINDS -- SETUP ----------------------" << RESET << std::endl;
    std::cout << "Parsing keybinds from " << filepath << std::endl;

    try {
        json json;
        keybindsFile >> json;

        for (const auto& item : json.items()) {
            std::string key = item.key();
            std::string value = item.value().get<std::string>();
            // Check if key is valid 
            if (item.key().empty() || !item.value().is_string()) {
                std::cerr << "Invalid key or value in keybinds file: " << filepath << std::endl;
                std::cout << YELLOW << BOLD <<  "------------------ KEYBINDS -- SETUP -- END -------------------" << RESET << std::endl;
                exit(EXIT_FAILURE);
            }
            int translatedKey = translateKey(item.value().get<std::string>());
            if (translatedKey == -1) {
                std::cerr << "Invalid key value in keybinds file: " << filepath << std::endl;
                std::cout << YELLOW << BOLD <<  "------------------ KEYBINDS -- SETUP -- END -------------------" << RESET << std::endl;
                exit(EXIT_FAILURE);
            }
            keybinds[item.key()] = translatedKey;
        }
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error in keybinds file " << filepath << ": " << e.what() << std::endl;
        std::cout << YELLOW << BOLD <<  "------------------ KEYBINDS -- SETUP -- END -------------------" << RESET << std::endl;
        exit(EXIT_FAILURE);
    } catch (const std::exception& e) {
        std::cerr << "Error loading keybinds file " << filepath << ": " << e.what() << std::endl;
        std::cout << YELLOW << BOLD <<  "------------------ KEYBINDS -- SETUP -- END -------------------" << RESET << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << GREEN << "[SUCCESS] All keybinds parsed without an issue." << RESET << std::endl;
    std::cout << YELLOW << BOLD <<  "------------------ KEYBINDS -- SETUP -- END -------------------" << RESET << std::endl;
}

void handleKeyEvent_1(MENU* artistMenu, MENU* songMenu, WINDOW* artist_menu_win, bool showingArtists, int menu_height, int menu_width) {
  werase(artist_menu_win);
  ncursesMenuSetup(artistMenu, artist_menu_win, menu_height, menu_width, "artist");
  post_menu(artistMenu);
  post_menu(songMenu);
  box(artist_menu_win, 0, 0);
  highlightFocusedWindow(artistMenu, showingArtists);
  highlightFocusedWindow(songMenu, !showingArtists);
  wrefresh(artist_menu_win);
}

void handleKeyEvent_tab(MENU* artistMenu, MENU* songMenu, WINDOW* artist_menu_win, WINDOW* song_menu_win, bool showingArtists, int menu_height, int menu_width) { 
  if (showingArtists) {
      // Switch focus to artist menu
      werase(artist_menu_win);
      ncursesMenuSetup(artistMenu, artist_menu_win, menu_height, menu_width, "artist");
      post_menu(artistMenu);
      post_menu(songMenu);
      box(artist_menu_win, 0, 0);
      highlightFocusedWindow(artistMenu, true);
      highlightFocusedWindow(songMenu, false);
      wrefresh(artist_menu_win);
      set_menu_fore(songMenu, COLOR_PAIR(COLOR_YELLOW));
  } else {
      // Switch focus to song menu
      ncursesMenuSetup(songMenu, song_menu_win, menu_height, menu_width, "song");
      post_menu(songMenu);
      post_menu(artistMenu);
      box(song_menu_win, 0, 0);
      set_menu_format(songMenu, menu_height, 0);
      highlightFocusedWindow(artistMenu, false);
      highlightFocusedWindow(songMenu, true);
      wrefresh(song_menu_win);
      set_menu_fore(artistMenu, COLOR_PAIR(COLOR_YELLOW));
  }
}

void handleKeyEvent_slash(MENU* artistMenu, MENU* songMenu, bool showingArtists) {
  char search_str[256];
  int x, y;
  getmaxyx(stdscr, y, x); // get the screen dimensions
  int input_width = 40; // adjust this to your liking
  int input_height = 3;
  int input_y = (LINES - input_height) / 2;
  int input_x = (COLS - input_width) / 2;

  // create a window for the input field
  WINDOW *input_win = newwin(input_height, input_width, input_y, input_x);
  wattron(input_win, COLOR_PAIR(COLOR_RED));
  box(input_win, 0, 0); // add a border around the input field
  mvwprintw(input_win, 0, 2, " Search: "); // prompt the user
  wrefresh(input_win); // refresh the input window

  // get the user input
  int i = 0;
  int count = 0;

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
              count--;
          }
      } else if (c == 27) { // escape
          break;
      } else if (isprint(c) && count <= 10) { // only add printable characters to the input string
          search_str[i++] = c;
          search_str[i] = '\0'; // null-terminate the string
          mvwprintw(input_win, 1, 1, "%*s", input_width - 25, search_str); // print the input string
          wrefresh(input_win);
          count++;
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
      int artTopRow = top_row(artistMenu);
      int artRows, artCols;
      scale_menu(artistMenu, &artRows, &artCols);
      int artLastVisibleItem = artTopRow + artRows - 4;
      for (i = 0; i < item_count(artistMenu); i++) {
          const char *itemName = item_name(items[i]);
          if (strcasestr(itemName, search_str) != NULL) {
              if (i >= artLastVisibleItem) {
                menu_driver(artistMenu, REQ_SCR_DPAGE);
              }
              set_current_item(artistMenu, items[i]);
              break;
          }
      }
  } else {
      ITEM **items = menu_items(songMenu);
      int i;
      int songTopRow = top_row(songMenu);
      int songRows, songCols;
      scale_menu(songMenu, &songRows, &songCols);
      int songLastVisibleItem = songTopRow + songRows - 4;
      for (i = 0; i < item_count(songMenu); i++) {
          const char *itemName = item_name(items[i]);
          if (strcasestr(itemName, search_str) != NULL) {
             if (i >= songLastVisibleItem) {
               menu_driver(songMenu, REQ_SCR_DPAGE);
             }
              set_current_item(songMenu, items[i]);
              break;
          }
      }
  }

}


void displayLyricsWindow(WINDOW *artist_menu_win, std::string& currentLyrics, std::string& currentSong, std::string& currentArtist, int menu_height, int menu_width, sf::Music &music, WINDOW *status_win, bool firstEnterPressed, bool showingLyrics, WINDOW *song_menu_win, MENU *songMenu, std::string& currentGenre, bool showingArtists, std::unordered_map<std::string, int>& keybinds) {
    mvwprintw(artist_menu_win, 0, 2, "  Lyrics: "); 
    wrefresh(artist_menu_win);
    werase(artist_menu_win); 
    int x, y;
    getmaxyx(stdscr, y, x); // get the screen dimensions
    int warning_width = x * 0.34; // adjust this to your liking
    int warning_height = y * 0.38;
    int warning_y = (LINES - warning_height) / 2;
    int warning_x = (COLS - warning_width) / 2 + 10;

    WINDOW *warning_win = newwin(warning_height, warning_width, warning_y, warning_x);
    displayWindow(warning_win, "warning", keybinds);

    WINDOW *lyrics_win = derwin(artist_menu_win, menu_height - 2, menu_width - 2, 1, 1);
    mvwprintw(artist_menu_win, 0, 2, " Lyrics: ");

    int start_line = 0; // To keep track of the starting line for scrolling
    std::vector<std::string> lines = splitStringByNewlines(currentLyrics);

    // Initial display of lyrics
    printMultiLine(lyrics_win, lines, start_line, currentSong, currentArtist);
    wrefresh(lyrics_win);

    int ch;
    while ((ch = getch()) != '1') { // Press '1' to exit
        switch (ch) {
            case KEY_UP:
            case 'j':
                if (start_line > 0) {
                    start_line--;
                }
                break;
            case KEY_DOWN:
            case 'k':
                if (start_line + menu_height - 2 < lines.size()) {
                    start_line++;
                }
                break;
            case 'p':
                if (music.getStatus() == sf::Music::Paused) {
                    music.play();
                } else {
                    music.pause();
                }
                break;
            case '9':
                adjustVolume(music, 10.f);
                break;
            case '0':
                adjustVolume(music, -10.f);
                break;
        }
        werase(lyrics_win); // Clear the sub-window
        printMultiLine(lyrics_win, lines, start_line, currentSong, currentArtist);
        showingLyrics = true;
        box(song_menu_win, 0, 0);
        post_menu(songMenu);
        updateStatusBar(status_win, currentSong, currentArtist, currentGenre, music, firstEnterPressed, showingLyrics);
        wrefresh(lyrics_win);
        mvwprintw(song_menu_win, 0, 2, " Songs: ");
        wrefresh(song_menu_win);
        displayWindow(warning_win, "warning", keybinds);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void quitFunc(sf::Music& music, std::vector<std::string>& allArtists, std::vector<std::string>& songTitles, ITEM** artistItems, ITEM** songItems, MENU* artistMenu, MENU* songMenu) {
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
}

void printSessionDetails(WINDOW* menu_win, const std::string& songsDirectory, const std::string& cacheDir, const std::string& cacheDebugFile, const std::string& keybindsFilePath, int artistsSize, int songsSize) {
  werase(menu_win);
  mvwprintw(menu_win, 2, 10, "LiteMus Session Details");
  std::stringstream artStr;
  artStr << "Directory: " << songsDirectory << std::endl << std::endl << "    Cache Directory: " << cacheDir << std::endl << std::endl << "    Debug File: " << cacheDebugFile << std::endl << std::endl << "    keybinds.json Path: " << keybindsFilePath << std::endl
 << std::endl << std::endl << "    No of artists: " << artistsSize << std::endl << std::endl << "    No of songs: " << songsSize;
  std::string newartStr = artStr.str();
  mvwprintw(menu_win, 4, 4, newartStr.c_str());
  box(menu_win, 0, 0);
  wrefresh(menu_win);
}
