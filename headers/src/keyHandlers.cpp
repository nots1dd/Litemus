#include "../keyHandlers.hpp"
#include "../ncurses_helpers.hpp"

void handleKeyEvent_1(MENU* artistMenu, MENU* songMenu, WINDOW* artist_menu_win, bool showingArtists, int menu_height, int menu_width) {
  werase(artist_menu_win);
  ncursesMenuSetup(artistMenu, artist_menu_win, menu_height, menu_width);
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
      ncursesMenuSetup(artistMenu, artist_menu_win, menu_height, menu_width);
      post_menu(artistMenu);
      post_menu(songMenu);
      box(artist_menu_win, 0, 0);
      highlightFocusedWindow(artistMenu, true);
      highlightFocusedWindow(songMenu, false);
      wrefresh(artist_menu_win);
  } else {
      // Switch focus to song menu
      ncursesMenuSetup(songMenu, song_menu_win, menu_height, menu_width);
      post_menu(songMenu);
      post_menu(artistMenu);
      box(song_menu_win, 0, 0);
      set_menu_format(songMenu, menu_height, 0);
      highlightFocusedWindow(artistMenu, false);
      highlightFocusedWindow(songMenu, true);
      wrefresh(song_menu_win);
  }
}

void handleKeyEvent_slash(MENU* artistMenu, MENU* songMenu, bool showingArtists) {
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
