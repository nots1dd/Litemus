#ifndef CHECK_SONG_DIR_HPP
#define CHECK_SONG_DIR_HPP

#include <ncurses.h>
#include <string>
#include <cstring>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>

void init_ncurses();
void draw_background();
void draw_title(WINDOW *win, const char *title);
void draw_form_window(WINDOW *win);
void draw_error_window(WINDOW *win);
void draw_error_message(WINDOW *win, const char *message);
void cleanup(WINDOW *win, const char *song_directory, const std::string& songDirPath);
bool directory_exists(const char *dir_path);
bool has_mp3_files(const char *dir_path);
int songDirMain(const std::string& songDirPath, const std::string& cacheLitemusDir);

#endif // CHECK_SONG_DIR_HPP
