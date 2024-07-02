#include <thread>
#include <chrono>
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


const char* title_content = "  LITEMUS - Light Music player                                                                                                                                                                               ";
int menu_height = 44;
int menu_width = 90;
int title_height = 2; 
int title_width = 208;

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

int main() {
    // Initialize ncurses
    songDirMain(songDirCache, cacheLitemusDir);
    std::string songsDirectory = read_file_to_string(songDirCache);
    lmus_cache_main(songsDirectory, homeDir, cacheLitemusDir, cacheInfoDir, cacheInfoFile, cacheArtistDirectory, songDirCache);     
    ncursesSetup();

    // Cache directory and song information 
    std::vector<std::string> allArtists = parseArtists(cacheArtistDirectory);
    auto [songTitles, songPaths] = listSongs(cacheDirectory, allArtists[0], songsDirectory); // default to the first artist
 

    // Check if songs are found
    if (songTitles.empty() || songPaths.empty() || allArtists.empty()) {
        printw("No songs found in directory.\n");
        refresh();
        endwin();
        return -1;
    }

    // Initialize artist menu
    ITEM** artistItems = createItems("artist", allArtists, songTitles);
    MENU* artistMenu = new_menu(artistItems);

    // Initialize song menu 
    ITEM** songItems = createItems("song", allArtists, songTitles);
    MENU* songMenu = new_menu(songItems);

    // Window dimensions and initialization
   /*  initWinSetup(artistMenu, songMenu); */

    WINDOW* title_win = newwin(title_height, title_width, 0, 0);
    wbkgd(title_win, COLOR_PAIR(GREY_BACKGROUND_COLOR));
    box(title_win, 0, 0);
    wrefresh(title_win);

    WINDOW* artist_menu_win = newwin(menu_height, menu_width, 1, 0);
    WINDOW* song_menu_win = newwin(menu_height, menu_width + 29, 1, menu_width);
    WINDOW* status_win = newwin(10, 300, LINES - 2, 0);

    // Set menus to their respective windows
    ncursesMenuSetup(artistMenu, artist_menu_win, menu_height, menu_width); 
    ncursesMenuSetup(songMenu, song_menu_win, menu_height, menu_width);
      

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
                    showingArtists = !showingArtists;
                    handleKeyEvent_tab(artistMenu, songMenu, artist_menu_win, song_menu_win, showingArtists, menu_height, menu_width); 
                    break;
                case KEY_DOWN:
                case 'k':
                  move_menu_down(artistMenu, songMenu, showingArtists); 
                  break;
                case KEY_UP:
                case 'j':
                    move_menu_up(artistMenu, songMenu, showingArtists);
                    break;
                case KEY_RIGHT:
                  seekSong(music, 5, 1); // 1 is bool for true ->it will forward
                  break;
                case KEY_LEFT:
                    seekSong(music, 5, 0);
                    break;
                case '/': // string search 
                  handleKeyEvent_slash(artistMenu, songMenu, showingArtists);
                  break;   
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
                        auto [newSongTitles, newSongPaths] = listSongs(cacheDirectory, selectedArtist, songsDirectory);
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
                        ncursesMenuSetup(songMenu, song_menu_win, menu_height, menu_width); 
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
                          seekSong(music, 60, 1);
                          break;
                      case 'g':  // Rewind (go back 5 seconds)
                          seekSong(music, 60 , 0);
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
                      case 'm':
                          if (!isMuted) {
                            adjustVolume(music, -100.f);
                          } else {
                            adjustVolume(music, 100.f);
                          }
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
                      mvwprintw(artist_menu_win, 0, 2, "  Lyrics: "); 
                      wrefresh(artist_menu_win);
                      werase(artist_menu_win); 
                      int x,y;
                      getmaxyx(stdscr, y, x); // get the screen dimensions
                      int warning_width = 75; // adjust this to your liking
                      int warning_height = 15;
                      int warning_x = x / 2; // center the input field
                      int warning_y = y / 2; // center the input field
                      std::vector<std::string> lines = splitStringByNewlines(currentLyrics);
                      WINDOW *warning_win = newwin(warning_height, warning_width, warning_y, warning_x);
                      displayWindow(warning_win, "warning");
                      
                      WINDOW* lyrics_win = derwin(artist_menu_win, menu_height - 2, menu_width - 2, 1, 1);
                      // wattron(lyrics_win, COLOR_PAIR(GREY_BACKGROUND_COLOR);
                      mvwprintw(artist_menu_win, 0, 2, " Lyrics: ");

                      int start_line = 0; // To keep track of the starting line for scrolling

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
                          displayWindow(warning_win, "warning");
                          std::this_thread::sleep_for(std::chrono::milliseconds(20));
                          
                      }

                      delwin(lyrics_win);
                      delwin(warning_win);
                    
                    } else {
                      werase(artist_menu_win);
                      box(artist_menu_win, 0, 0);
                      mvwprintw(artist_menu_win, 0, 2, " Artists: ");
                      mvwprintw(artist_menu_win, 2, 20, "NO LYRICS FOR THIS SONG!");
                      mvwprintw(artist_menu_win, 4, 20, "Redirecting to main window...");
                      wrefresh(artist_menu_win);
                      std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                    werase(artist_menu_win);
                    ncursesMenuSetup(artistMenu, artist_menu_win, menu_height, menu_width);
                    set_menu_format(artistMenu, menu_height, 0);
                    post_menu(artistMenu);
                    highlightFocusedWindow(artistMenu, showingArtists);
                    showingLyrics = false;
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
                default:
                    mvwprintw(status_win, 12, 2, "Invalid input.");
            }
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
