#include <cstdio>
#include "headers/lmus_cache.hpp"
#include "headers/sfml_helpers.hpp"
#include "headers/ncurses_helpers.hpp"
#include "headers/parsers.hpp"
#include "headers/checkSongDir.hpp"
#include "headers/keyHandlers.hpp"

#define COLOR_PAIR_FOCUSED 1 
#define COLOR_PAIR_SELECTED 3
#define GREY_BACKGROUND_COLOR 7
#define LIGHT_GREEN_COLOR 8

// Directory vars
const std::string homeDir = get_home_directory();
const std::string cacheLitemusDir = homeDir + "/.cache/litemus/";
const std::string cacheInfoDir = cacheLitemusDir + "info/";
const std::string songDirCache = cacheLitemusDir + "songDirectory.txt";
const std::string cacheDirectory = cacheInfoDir + "song_names.json";
const std::string cacheInfoFile = cacheInfoDir + "song_cache_info.json";
const std::string cacheArtistDirectory = cacheInfoDir + "artists.json";
const std::string cacheDebugFile = cacheLitemusDir + "debug.log";

// ansi escape vals (colors)
const string ERROR = "\033[31m";
const string NC = "\033[0m";
const string BLD = "\033[1m";


const char* title_content = "  LITEMUS - Light Music player                                                                                                                                                                               ";
int menu_height = 44;
int menu_width = 90;
int title_height = 2; 
int title_width = 208;

char* strdup(const char* s) {
    size_t len = strlen(s);
    char* dup = new char[len + 1];
    strcpy(dup, s);
    return dup;
}

int main(int argc, char* argv[]) {
    // Initialize ncurses
    if (argc == 1) {
      litemusHelper(NC);
      return 0;
    }
    if (argc <= 2 && std::string(argv[1]) == "--remote-cache") {
        songDirMain(songDirCache, cacheLitemusDir);
        std::string songsDirectory = read_file_to_string(songDirCache);
        lmus_cache_main(songsDirectory, homeDir, cacheLitemusDir, cacheInfoDir, cacheInfoFile, cacheArtistDirectory, songDirCache, cacheDebugFile);
        cout << endl << "Successfully cached the directory " << GREEN << songsDirectory << NC << endl << "Run `" << GREEN << "lmus run" << NC << "` to experience LiteMus!" << endl;
        return 0;
    }
    else if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "help")) {
      litemusHelper(NC);
      return 0;
    }
    else if (argc == 2 && std::string(argv[1]) == "--clear-cache") {
      if (remove(songDirCache.c_str()) == 0 && remove(cacheDebugFile.c_str()) == 0) {
        cout << YELLOW << BOLD << "Removed songDirectory.txt and log files from cache!" << NC << endl;
        cout << endl << "Run `" << GREEN << "lmus run" << NC << "` to add new directory cache!!" << endl;
        return 0;
      } else {
        cout << ERROR << BOLD << "[DIR-ERROR] Something went wrong while trying to remove songDirectory.txt" << endl;
        return 1;
      }
    }
    else if (argc == 2 && std::string(argv[1]) == "run") {
    songDirMain(songDirCache, cacheLitemusDir);
    std::string songsDirectory = read_file_to_string(songDirCache);
    lmus_cache_main(songsDirectory, homeDir, cacheLitemusDir, cacheInfoDir, cacheInfoFile, cacheArtistDirectory, songDirCache, cacheDebugFile);     
    ncursesSetup();

    // Cache directory and song information
    std::vector<std::string> allInodes = loadPreviousInodes(cacheInfoFile);
    int songsSize = allInodes.size();
    std::vector<std::string> allArtists = parseArtists(cacheArtistDirectory);
    int artistsSize = allArtists.size();
    auto [songCrudeTitles, songPaths, songDurations] = listSongs(cacheDirectory, allArtists[0], songsDirectory); // default to the first artist
    size_t maxTitleLength = getMaxSongTitleLength(songCrudeTitles, songDurations);
    auto songTitles = getTitlesWithWhiteSpaces(songCrudeTitles, songDurations, maxTitleLength);
    // Check if songs are found
    if (allArtists.empty() || songTitles.empty() || songPaths.empty()) {
        printw("No songs found in directory.\n");
        refresh();
        endwin();
        cout << ERROR << BLD << "[ERROR] Null / Corrupt metadata in files. Exiting Litemus with code 255" << NC << endl;
        cout << "[NOTE] Ensure that the mp3 files in your directory have proper metadata embedded in them!" << endl;
        return -1;
    }

    // Initialize artist menu
    ITEM** artistItems = createItems("artist", allArtists, songTitles, songDurations);
    MENU* artistMenu = new_menu(artistItems);

    // Initialize song menu 
    ITEM** songItems = createItems("song", allArtists, songTitles, songDurations);
    MENU* songMenu = new_menu(songItems);
    songTitles = songCrudeTitles;

    // Window dimensions and initialization

    WINDOW* title_win = newwin(title_height, title_width, 0, 0);
    wbkgd(title_win, COLOR_PAIR(GREY_BACKGROUND_COLOR));
    box(title_win, 0, 0);
    wrefresh(title_win);

    WINDOW* artist_menu_win = newwin(menu_height, menu_width, 1, 0);
    WINDOW* song_menu_win = newwin(menu_height, menu_width + 29, 1, menu_width);
    WINDOW* status_win = newwin(10, 300, LINES - 2, 0);

    // Set menus to their respective windows
    ncursesMenuSetup(artistMenu, artist_menu_win, menu_height, menu_width, "artist"); 
    ncursesMenuSetup(songMenu, song_menu_win, menu_height, menu_width, "song");
      

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
    timeout(1);
    nodelay(artist_menu_win, TRUE);

    // Flag to track first enter press
    bool firstEnterPressed = false;
    bool showingArtists = true;  // Start with artist menu in focus
    bool showingLyrics = false;
    bool isMuted = false;
    bool updateSongMenu = false;

    highlightFocusedWindow(artistMenu, true);
    highlightFocusedWindow(songMenu, false);
    while (true) {
        int ch = getch();
        if (ch != ERR) {
            switch (ch) {
                case '1':
                  handleKeyEvent_1(artistMenu, songMenu, artist_menu_win, showingArtists, menu_height, menu_width); 
                  break;
                case 9:  // Tab to switch between menus
                    updateSongMenu = false;
                    showingArtists = !showingArtists;
                    handleKeyEvent_tab(artistMenu, songMenu, artist_menu_win, song_menu_win, showingArtists, menu_height, menu_width);
                    break;
                case KEY_DOWN:
                case 'k': {
                  move_menu_down(artistMenu, songMenu, showingArtists); // ncurses helpers
                  if (showingArtists) {
                   updateSongMenu = true; 
                  }
                  break;
                }
                case KEY_UP:
                case 'j':{
                    move_menu_up(artistMenu, songMenu, showingArtists); // ncurses helpers
                    if (showingArtists) {
                      updateSongMenu = true;
                    }
                    break;
                }
                case KEY_RIGHT:
                  seekSong(music, 5, 1); // 1 is bool for true ->it will forward (sfml helpers)
                  break;
                case KEY_LEFT:
                    seekSong(music, 5, 0); // sfml helpers
                    break;
                case '/': // string search 
                {
                  handleKeyEvent_slash(artistMenu, songMenu, showingArtists);
                  if (showingArtists) {
                    updateSongMenu = true; 
                  }
                  break;
                }
                case 10:  // Enter key
                    if (!showingArtists) {
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
                      case 'f':  // Fast-forward (skip 60 seconds)
                          seekSong(music, 60, 1);
                          break;
                      case 'g':  // Rewind (go back 60 seconds)
                          seekSong(music, 60 , 0);
                          break;
                      case 'r':  // Restart current song
                          music.stop();
                          music.play();
                          break;
                      case '9':  // Volume up
                          adjustVolume(music, 10.f); // sfml helpers
                          break;
                      case '0':  // Volume down
                          adjustVolume(music, -10.f); // sfml helpers
                          break;
                      case 'm':
                          toggleMute(music, isMuted); // sfml helpers
                          isMuted = !isMuted;
                          break;
                      case 'n':  // Next song
                          if (firstEnterPressed) {
                          nextSong(music, songPaths, currentSongIndex);
                          currentSong = songTitles[currentSongIndex];
                          auto resultGA = findCurrentGenreArtist(cacheDirectory, currentSong, currentLyrics);
                          currentGenre = resultGA.first;
                          currentArtist = resultGA.second;
                          updateStatusBar(status_win, currentSong, currentArtist, currentGenre,  music, firstEnterPressed, showingLyrics);
                        }
                      break;
                  case 'b':  // Previous song
                      if (firstEnterPressed) {
                        previousSong(music, songPaths, currentSongIndex);
                        currentSong = songTitles[currentSongIndex];
                        auto resultGA = findCurrentGenreArtist(cacheDirectory, currentSong, currentLyrics);
                        currentGenre = resultGA.first;
                        currentArtist = resultGA.second;
                        updateStatusBar(status_win, currentSong, currentArtist, currentGenre, music, firstEnterPressed, showingLyrics);
                    }
                    break;
                case '2':  // Display help window
                    displayWindow(artist_menu_win, "help");
                    break;
                case '3':
                    if (currentLyrics != "") {
                      // Assuming you have all the necessary variables defined and initialized
                      displayLyricsWindow(artist_menu_win, currentLyrics, currentSong, currentArtist, menu_height, menu_width, music,
                                          status_win, firstEnterPressed, showingLyrics, song_menu_win, songMenu, currentGenre, showingArtists);
                    
                    } else {
                      displayWindow(artist_menu_win, "LyricErr");
                      std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                    werase(artist_menu_win);
                    ncursesMenuSetup(artistMenu, artist_menu_win, menu_height, menu_width, "artist");
                    set_menu_format(artistMenu, menu_height, 0);
                    post_menu(artistMenu);
                    highlightFocusedWindow(artistMenu, showingArtists);
                    showingLyrics = false;
                    break;
                case '4':
                    printSessionDetails(artist_menu_win, songsDirectory, artistsSize, songsSize);
                    break;
                case 'q':  // Quit
                if (showExitConfirmation(song_menu_win)) {
                    quitFunc(music, allArtists, songTitles, artistItems, songItems, artistMenu, songMenu);
                    ncursesWinControl(artist_menu_win, song_menu_win, status_win, title_win, "delete");
                    endwin();
                    return 0;
                }
                break;
                case 'x': // force exit
                    quitFunc(music, allArtists, songTitles, artistItems, songItems, artistMenu, songMenu);
                    ncursesWinControl(artist_menu_win, song_menu_win, status_win, title_win, "delete");
                    endwin();
                    return 0;
                break; 
            }
        }

        if (updateSongMenu) {
          ITEM* artItem = current_item(artistMenu);
          int artselectedIndex = item_index(artItem); 
          post_menu(artistMenu);
          post_menu(songMenu);
          box(menu_win(artistMenu), 0, 0);
          const char* selectedArtist = allArtists[artselectedIndex].c_str();

          // Update song menu with songs of the selected artist
          auto [newCrudeSongTitles, newSongPaths, newSongDurations] = listSongs(cacheDirectory, selectedArtist, songsDirectory);
          size_t maxTitleLength = getMaxSongTitleLength(newCrudeSongTitles, newSongDurations);
          auto newSongTitles = getTitlesWithWhiteSpaces(newCrudeSongTitles, newSongDurations, maxTitleLength);
          for (size_t i = 0; i < songTitles.size(); ++i) {
              free_item(songItems[i]);
          }
          free_menu(songMenu);
          werase(menu_win(songMenu)); 
          songItems = new ITEM*[newSongTitles.size() + 1];
          for (size_t i = 0; i < newSongTitles.size(); ++i) {  
              songItems[i] = new_item(strdup(newSongTitles[i].c_str()), "");
          }
          songItems[newSongTitles.size()] = nullptr;
          songMenu = new_menu(songItems);
          ncursesMenuSetup(songMenu, song_menu_win, menu_height, menu_width, "song"); 
          set_menu_format(songMenu, menu_height, 0);
          set_menu_fore(songMenu, A_NORMAL);
          post_menu(songMenu);

          songTitles = newCrudeSongTitles;
          songPaths = newSongPaths;
          songDurations = newSongDurations;
          wrefresh(menu_win(artistMenu));
          wrefresh(menu_win(songMenu));
        }

        // Handle song transition when current song ends
        if (music.getStatus() == sf::Music::Stopped && firstEnterPressed) {
            nextSong(music, songPaths, currentSongIndex);
            currentSong = songTitles[currentSongIndex];
            auto resultGA = findCurrentGenreArtist(cacheDirectory, currentSong, currentLyrics);
            currentGenre = resultGA.first;
            currentArtist = resultGA.second;
            updateStatusBar(status_win, currentSong, currentArtist, currentGenre, music, firstEnterPressed, showingLyrics);
        }

        // Update status bar and refresh windows
        updateStatusBar(status_win, currentSong, currentArtist, currentGenre, music, firstEnterPressed, showingLyrics);
        ncursesWinLoop(artistMenu, songMenu, artist_menu_win, song_menu_win, status_win, title_win, title_content); 
        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Optional delay
    }

    // Clean up and exit
    unpost_menu(artistMenu);
    unpost_menu(songMenu);
    quitFunc(music, allArtists, songTitles, artistItems, songItems, artistMenu, songMenu);
    ncursesWinControl(artist_menu_win, song_menu_win, status_win, title_win, "delete");
    endwin();

    return 0;
  }
  else {
      cout << "Unknown command: " << std::string(argv[1]) << endl;
      cout << "Run `" << GREEN << "lmus --help" << NC << "` to get helpful commands to run LiteMus!" << endl;
      return 1;
  }
  if (argc > 2) {
      cout << ERROR << BOLD << "[ERROR] Invalid use of LiteMus" << NC << endl;
      return -1;
  } else {
    return 1;
  }

}
