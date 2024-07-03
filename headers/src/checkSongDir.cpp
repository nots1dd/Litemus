#include "../checkSongDir.hpp"
#include "../directoryUtils.hpp"

#define MAX_FIELD_LENGTH 50


void init_ncurses() {
    initscr();          // Initialize ncurses
    cbreak();           // Line buffering disabled
    noecho();           // Don't echo while we do getch
    keypad(stdscr, TRUE); // Enable function key detection
}

void draw_title(WINDOW *win, const char *title) {
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 2, title);
    wattroff(win, A_BOLD);
    mvwhline(win, 2, 1, ACS_HLINE, getmaxx(win) - 2);
}

void draw_form_window(WINDOW *win) {
    box(win, 0, 0);        // Draw a box around the window
    draw_title(win, "LiteMus - Light Music Player");
    mvwprintw(win, 4, 2, "Enter Song Directory:");
    wrefresh(win);         // Refresh the window
}

void draw_error_window(WINDOW *win) {
    box(win, 0, 0);        // Draw a box around the window
    draw_title(win, "Error Messages");
    wrefresh(win);         // Refresh the window
}

void draw_error_message(WINDOW *win, const char *message) {
    wclear(win);           // Clear the error window
    box(win, 0, 0);        // Redraw the box around the window
    draw_title(win, "Error Messages");
    wattron(win, COLOR_PAIR(1));  // Turn on color pair 1 (red)
    mvwprintw(win, 4, 2, message);
    wattroff(win, COLOR_PAIR(1)); // Turn off color pair 1
    wrefresh(win);
}

void cleanup(WINDOW *win, const char *song_directory, const std::string& songDirPath) {
    // Ensure directory path ends with '/'
    std::string dirPath = song_directory;
    if (!dirPath.empty() && dirPath.back() != '/') {
        dirPath += '/';
    }

    // Clear the screen completely
    clear();
    refresh();
    endwin();              // End ncurses mode

    // Save the song directory to a file
    std::ofstream file(songDirPath);
    if (file.is_open()) {
        file << dirPath;  // Save the modified directory path
        file.close();
    }

    // Create the directory if it doesn't exist
    struct stat st;
    if (stat(dirPath.c_str(), &st) == -1) {
        if (mkdir(dirPath.c_str(), 0777) != 0) {
            // Handle error if directory creation fails
            mvprintw(LINES - 2, 2, "Failed to create directory %s", dirPath.c_str());
            refresh();
        }
    }
}

bool has_mp3_files(const char *dir_path) {
    DIR *dir;
    struct dirent *ent;
    bool found_mp3 = false;
    if ((dir = opendir(dir_path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".mp3") != NULL) {
                found_mp3 = true;
                break;
            }
        }
        closedir(dir);
    }
    return found_mp3;
}

bool file_exists_and_not_empty(const std::string& path) {
    std::ifstream file(path);
    return file.good() && file.peek() != std::ifstream::traits_type::eof();
}

int songDirMain(const std::string& songDirPath, const std::string& cacheLitemusDir) {
    // Check if the file exists and is not empty
    if (file_exists_and_not_empty(songDirPath)) {
        return 0;
    }
    createDirectory(cacheLitemusDir);

    char song_directory[MAX_FIELD_LENGTH] = "";  // Buffer to hold user input
    WINDOW *main_win, *error_win;
    int ch;
    int dir_length = 0;

    // Initialize ncurses
    init_ncurses();
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);  // Define color pair 1 as red text on black background
        init_pair(2, COLOR_CYAN, COLOR_BLACK); // Define color pair 2 as cyan text on black background
        init_pair(3, COLOR_WHITE, COLOR_BLUE); // Define color pair 3 as white text on blue background
    } 

    // Create the main window for input
    main_win = newwin(12, 60, (LINES - 15) / 2, (COLS - 60) / 2);
    keypad(main_win, TRUE); // Enable function key detection

    // Create the error window below the main window
    error_win = newwin(6, 60, (LINES + 10) / 2, (COLS - 60) / 2);
    draw_error_window(error_win);

    // Draw the form window
    draw_form_window(main_win);

    // Enable cursor
    curs_set(1);

    // Main loop to get user input
    while (true) {
        ch = wgetch(main_win);

        if (ch == KEY_BACKSPACE || ch == 127) {  // Handle backspace
            if (dir_length > 0) {
                dir_length--;
                song_directory[dir_length] = '\0';
                mvwprintw(main_win, 6, 2, "%s ", song_directory);
            }
        } else if (ch == '\n') {  // Handle Enter key
            // Validate the directory
            if (dir_length == 0) {
                draw_error_message(error_win, "Please enter a directory path.");
            } else if (!directory_exists(song_directory)) {
                draw_error_message(error_win, "Directory does not exist.");
            } else if (!has_mp3_files(song_directory)) {
                draw_error_message(error_win, "Directory does not contain any .mp3 files.");
            } else {
                // Clean up and save the input
                cleanup(main_win, song_directory, songDirPath);
                break;  // Exit loop on successful input
            }
        } else if (isprint(ch)) {  // Handle printable characters
            if (dir_length < MAX_FIELD_LENGTH - 1) {
                song_directory[dir_length++] = ch;
                song_directory[dir_length] = '\0';
                mvwprintw(main_win, 6, 2, "%s ", song_directory);
            }
        }

        wrefresh(main_win);  // Refresh the main window
    }

    return 0;
}
