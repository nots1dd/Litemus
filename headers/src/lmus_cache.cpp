#include "../lmus_cache.hpp"

using json = nlohmann::json;
using namespace std;

// COLORS (ANSI ESCAPE VALUES)
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string BLUE = "\033[34m";
const string PINK = "\033[35m";
const string YELLOW = "\033[33m";
const string BOLD = "\033[1m";

// FILE EXTENSION TO CACHE
const vector<string> extensions = {".mp3", ".wav", ".flac"};


struct SongMetadata {
    string fileName;
    string inode;
    string artist;
    string album;
    string title;
    int disc;
    int track;
    string genre;
    string date;
    string lyrics;
};

// Function to get the list of inodes
vector<string> getInodes() {
    vector<string> inodes;
    for (const string& ext : extensions) {
        const string output_cmd = "ls -i *" + ext + " | awk '{print $1}'";
        string output = executeCommand(output_cmd);
        stringstream ss(output);
        string inode;

        while (getline(ss, inode, '\n')) {
            if (!inode.empty()) {
                inodes.push_back(inode);
            }
        }
    }
    return inodes;
}


// Function to get the file name from an inode
string getFileNameFromInode(const string& inode) {
    string findCmd = "find . -type f -inum " + inode + " | sed 's|\\./||'";
    string result = executeCommand(findCmd);

    stringstream ss(result);
    string fileName;
    getline(ss, fileName, '\n');
    if (fileName.empty()) {
        printErrorAndExit("[ERROR] No file found with the given inode.");
    }

    return fileName;
}

// Function to escape special characters in filename
string escapeSpecialCharacters(const string& fileName) {
    string escapedFileName;
    for (char c : fileName) {
        if (c == '\'') {
            escapedFileName += "'\"'\"'"; // Add a backslash before special characters
        } else {
			escapedFileName += c;
		}
    }
    return escapedFileName;
}



void storeMetadataJSON(const string& inode, const string& fileName, json& artistsArray, vector<SongMetadata>& songMetadata, const string debugFile) {
    // Escape special characters in the filename
    string escapedFileName = escapeSpecialCharacters(fileName);

    // Construct the ffprobe command with the escaped filename
    string metadataCmd = "ffprobe -v quiet -print_format json -show_format '" + escapedFileName + "'";
    string metadataInfo = executeCommand(metadataCmd);

    auto metadata = json::parse(metadataInfo);

    string artist = "Unknown_Artist";
    string album = "Unknown_Album";
    string title = fileName;
    int disc = 1;
    int track = 1;
    string genre = "Unknown Genre";
    string date = "Unknown_Date";
    string lyrics = "";

    // Log the metadata extraction process
    ofstream logFile(debugFile, ios::app);
    if (logFile.is_open()) {
        logFile << "Storing Metadata for: " << fileName << endl;
        logFile << "Inode: " << inode << endl;

        if (metadata["format"]["tags"].contains("artist") || metadata["format"]["tags"].contains("ARTIST")) {
            artist = metadata["format"]["tags"].contains("artist") ? 
                     metadata["format"]["tags"]["artist"].get<string>() : 
                     metadata["format"]["tags"]["ARTIST"].get<string>();
            logFile << "Artist: extracted successfully +" << endl;
        } else {
            logFile << "Artist: extraction failed -" << endl;
        }

        if (metadata["format"]["tags"].contains("album") || metadata["format"]["tags"].contains("ALBUM")) {
            album = metadata["format"]["tags"].contains("album") ? 
                    metadata["format"]["tags"]["album"].get<string>() : 
                    metadata["format"]["tags"]["ALBUM"].get<string>();
            logFile << "Album: extracted successfully +" << endl;
        } else {
            logFile << "Album: extraction failed -" << endl;
        }

        if (metadata["format"]["tags"].contains("title") || metadata["format"]["tags"].contains("TITLE")) {
            title = metadata["format"]["tags"].contains("title") ? 
                    metadata["format"]["tags"]["title"].get<string>() : 
                    metadata["format"]["tags"]["TITLE"].get<string>();
            logFile << "Title: extracted successfully +" << endl;
        } else {
            logFile << "Title: extraction failed -" << endl;
        }

        if (metadata["format"]["tags"].contains("disc") || metadata["format"]["tags"].contains("DISC")) {
            string discStr = metadata["format"]["tags"].contains("disc") ? 
                             metadata["format"]["tags"]["disc"].get<string>() : 
                             metadata["format"]["tags"]["DISC"].get<string>();
            try {
                disc = std::stoi(discStr);
                logFile << "Disc: extracted successfully +" << endl;
            } catch (const std::invalid_argument&) {
                logFile << "Disc: extraction failed -" << endl;
            }
        } else {
            logFile << "Disc: extraction failed -" << endl;
        }

        if (metadata["format"]["tags"].contains("track") || metadata["format"]["tags"].contains("TRACK")) {
            string trackStr = metadata["format"]["tags"].contains("track") ? 
                              metadata["format"]["tags"]["track"].get<string>() : 
                              metadata["format"]["tags"]["TRACK"].get<string>();
            try {
                track = std::stoi(trackStr);
                logFile << "Track: extracted successfully +" << endl;
            } catch (const std::invalid_argument&) {
                logFile << "Track: extraction failed -" << endl;
            }
        } else {
            logFile << "Track: extraction failed -" << endl;
        }

        if (metadata["format"]["tags"].contains("genre") || metadata["format"]["tags"].contains("GENRE")) {
            genre = metadata["format"]["tags"].contains("genre") ? 
                    metadata["format"]["tags"]["genre"].get<string>() : 
                    metadata["format"]["tags"]["GENRE"].get<string>();
            logFile << "Genre: extracted successfully +" << endl;
        } else {
            logFile << "Genre: extraction failed -" << endl;
        }

        if (metadata["format"]["tags"].contains("date") || metadata["format"]["tags"].contains("DATE")) {
            date = metadata["format"]["tags"].contains("date") ? 
                   metadata["format"]["tags"]["date"].get<string>() : 
                   metadata["format"]["tags"]["DATE"].get<string>();
            logFile << "Date: extracted successfully +" << endl;
        } else {
            logFile << "Date: extraction failed -" << endl;
        }

        if (metadata["format"]["tags"].contains("lyrics-XXX") || metadata["format"]["tags"].contains("LYRICS-XXX")) {
            lyrics = metadata["format"]["tags"].contains("lyrics-XXX") ? 
                     metadata["format"]["tags"]["lyrics-XXX"].get<string>() : 
                     metadata["format"]["tags"]["LYRICS-XXX"].get<string>();
            logFile << "Lyrics: extracted successfully +" << endl;
        } else {
            logFile << "Lyrics: extraction failed -" << endl;
        }

        logFile << "--------------------------------------" << endl;
        logFile.close();
    } else {
        cerr << "Unable to open debug log file" << endl;
    }

    // Check if artist is already in the array
    if (find(artistsArray.begin(), artistsArray.end(), artist) == artistsArray.end()) {
        artistsArray.push_back(artist);
    }

    // Store metadata
    songMetadata.push_back({fileName, inode, artist, album, title, disc, track, genre, date, lyrics});
}

// Function to save artists to a file
void saveArtistsToFile(const json& artistsArray, const string& filePath) {
    ofstream outFile(filePath, ios::trunc);
    if (outFile.is_open()) {
        outFile << artistsArray.dump(4);
        outFile.close();
    } else {
        printErrorAndExit("[ERROR] Unable to save artists to file: " + filePath);
    }
}

void saveSongDirToFile(const std::string& songDirPath, const string& songDirectory) {
  std::fstream songDirFile(songDirPath, std::ios::out);
  if (!songDirFile) {
      printErrorAndExit("[ERROR] Unable to save song directory to file: songDirectory.txt");
  }
  songDirFile << songDirectory;
  songDirFile.close();
}

// Function to print artists
void printArtists(const json& artistsArray) {
    cout << "Artists List:" << endl;
    for (const auto& artist : artistsArray) {
        cout << " - " << artist << endl;
    }
}

void storeSongsJSON(const string& filePath, const vector<SongMetadata>& songMetadata, const string debugFile) {
    json songsJson;

    for (const auto& song : songMetadata) {
        // Check if the song has valid metadata to include
        if (!song.artist.empty() && !song.album.empty() && song.disc > 0 && song.track > 0) {
            json songInfo = {
                {"title", song.title},
                {"filename", song.fileName},
                {"inode", song.inode},
                {"disc", song.disc},
                {"track", song.track},
                {"genre", song.genre},
                {"date", song.date},
                {"lyrics", song.lyrics}
            };

            // Ensure the artist exists in the JSON structure
            if (!songsJson.contains(song.artist)) {
                songsJson[song.artist] = json::object();
            }

            // Ensure the album exists under the artist
            if (!songsJson[song.artist].contains(song.album)) {
                songsJson[song.artist][song.album] = json::array();
            }

            // Ensure the disc exists under the album
            while (songsJson[song.artist][song.album].size() < song.disc) {
                songsJson[song.artist][song.album].push_back(json::array());
            }

            // Ensure the track exists under the disc
            while (songsJson[song.artist][song.album][song.disc - 1].size() < song.track) {
                songsJson[song.artist][song.album][song.disc - 1].push_back(json::object());
            }

            // Assign songInfo to the track index
            songsJson[song.artist][song.album][song.disc - 1][song.track - 1] = songInfo;
        } else {
            cerr << "Skipping invalid song metadata: " << song.fileName << endl;
            // Print out the metadata for debugging
            ofstream logFile(debugFile, ios::trunc);
            if (logFile.is_open()) {
                logFile << "Skipping invalid song metadata: " << song.fileName << endl;
                logFile << "Artist: " << song.artist << ", Album: " << song.album << ", Disc: " << song.disc << ", Track: " << song.track << endl;
                logFile.close();
            }
        }
    }

    ofstream outFile(filePath, ios::trunc);
    if (outFile.is_open()) {
        outFile << songsJson.dump(4);
        outFile.close();
    } else {
        printErrorAndExit("[ERROR] Unable to save song names to file: " + filePath);
    }
}

// Function to compare two inode vectors
bool compareInodeVectors(const vector<string>& vec1, const vector<string>& vec2) {
    if (vec1.size() != vec2.size()) {
        return false;
    }
    for (size_t i = 0; i < vec1.size(); ++i) {
        if (vec1[i] != vec2[i]) {
            return false;
        }
    }
    return true;
}

// Function to load previous inodes from file
vector<string> loadPreviousInodes(const string& filePath) {
    vector<string> previousInodes;
    ifstream inFile(filePath);
    if (inFile.is_open()) {
        json inodesJson;
        inFile >> inodesJson;
        inFile.close();
        previousInodes = inodesJson.get<vector<string>>();
    }
    return previousInodes;
}

// Function to save current inodes to file
void saveCurrentInodes(const vector<string>& inodes, const string& filePath) {
    json inodesJson = inodes;
    ofstream outFile(filePath, ios::trunc);
    if (outFile.is_open()) {
        outFile << inodesJson.dump(4);
        outFile.close();
    } else {
        printErrorAndExit("[ERROR] Unable to save inodes to file: " + filePath);
    }
}

void drawProgressBar(WINDOW* win, int y, int x, float progress) {
    int barWidth = 50; // Width of the progress bar
    int pos = barWidth * progress;
    mvwprintw(win, y, x, "[");
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) mvwprintw(win, y, x + 1 + i, "=");
        else mvwprintw(win, y, x + 1 + i, " ");
    }
    mvwprintw(win, y, x + 1 + barWidth, "]");
    wrefresh(win);
}

int lmus_cache_main(std::string& songDirectory, const std::string homeDir, const std::string cacheLitemusDirectory, const std::string configLitemusDirectory, const std::string cacheInfoDirectory, const std::string songCacheInfoFile, const std::string artistsFilePath, const std::string songDirPathCache, const std::string debugFile) {

    // DIRECTORY VARIABLES
    const string cacheDirectory = homeDir + "/.cache/"; 
    cout << BLUE <<  BOLD << "----------------- LITEMUS -- CACHE -- START ------------------" << RESET << endl;
    changeDirectory(songDirectory);
    createDirectory(cacheDirectory);
    createDirectory(cacheLitemusDirectory);
    createDirectory(configLitemusDirectory);
    createDirectory(cacheInfoDirectory);

    vector<string> inodes = getInodes();
    vector<SongMetadata> songMetadata;

    if (inodes.empty()) {
        cerr << RED << "No inodes found." << RESET << endl;
        return 1;
    }

    // Load previous inodes from song_cache_info_file if it exists
    vector<string> previousInodes = loadPreviousInodes(songCacheInfoFile);
    json artistsArray;

    // Compare current inodes with previous inodes
    if (compareInodeVectors(inodes, previousInodes)) {
        cout << PINK << BOLD << "[CACHE] No changes in song files. Exiting without caching." << RESET << endl;
        cout << BLUE << BOLD << "----------------- LITEMUS -- CACHE -- OVER -------------------" << RESET << endl;
    } else {
        int cachedSongCount = 0;
        time_t startTime = time(nullptr); // record the start time
        setlocale(LC_ALL, "");
        initscr();
        noecho();
        cbreak();
        curs_set(0);
        
        // Create windows for TUI
        int height = 10;
        int width = 60;
        int start_y = 1;
        int start_x = (COLS - width) / 2;
        WINDOW* progressWin = newwin(height, width, start_y, start_x);
        box(progressWin, 0, 0);
        mvwprintw(progressWin, 1, 1, "LiteMus Cache Process");
        wrefresh(progressWin);

        int fileWinHeight = 3;
        int fileWinWidth = 100;
        int fileWinStartY = start_y + height + 1;
        int fileWinStartX = start_x - 20;
        WINDOW* fileWin = newwin(fileWinHeight, fileWinWidth, fileWinStartY, fileWinStartX);
        box(fileWin, 0, 0);
        wrefresh(fileWin);

        for (const std::string& inode : inodes) {
            string fileName = getFileNameFromInode(inode);
            string finalFileName = fileName.length() > 75 ? fileName.substr(0, 75) + "..." : fileName;
            // Clear previous filename and print new filename
            wclear(fileWin);
            box(fileWin, 0, 0);
            mvwprintw(fileWin, 1, 1, "==> %s", finalFileName.c_str());
            wrefresh(fileWin);

            storeMetadataJSON(inode, fileName, artistsArray, songMetadata, debugFile);
            cachedSongCount++;
            time_t currentTime = time(nullptr);

            double elapsedSeconds = difftime(currentTime, startTime);
            double songsPerSecond = cachedSongCount / elapsedSeconds;
            double remainingSongs = inodes.size() - cachedSongCount;
            double estimatedTimeRemaining = remainingSongs / songsPerSecond;

            // format the estimated time remaining
            int minutes = static_cast<int>(estimatedTimeRemaining) % 3600 / 60;
            int seconds = static_cast<int>(estimatedTimeRemaining) % 60;
            char timeRemainingStr[32];
            sprintf(timeRemainingStr, "Time to cook: %02d:%02d", minutes, seconds);
            mvwprintw(progressWin, 3, 1, timeRemainingStr);

            float progress = static_cast<float>(cachedSongCount) / inodes.size();
            mvwprintw(progressWin, 2, 1, "Progress: %0.2f%%", progress*100);
            drawProgressBar(progressWin, 5, 1, progress);
        }

        saveArtistsToFile(artistsArray, artistsFilePath);

        // Sort songMetadata by artist, album, disc, and track
        sort(songMetadata.begin(), songMetadata.end(), [](const SongMetadata& a, const SongMetadata& b) {
            if (a.artist != b.artist) return a.artist < b.artist;
            if (a.album != b.album) return a.album < b.album;
            if (a.disc != b.disc) return a.disc < b.disc;
            return a.track < b.track;
        });

        storeSongsJSON(cacheInfoDirectory + "/song_names.json", songMetadata, debugFile);

        // Save current inodes for future comparison
        saveCurrentInodes(inodes, songCacheInfoFile);
        saveSongDirToFile(songDirPathCache, songDirectory);

        endwin();


        cout << endl << GREEN << BOLD << "[SUCCESS] Total of " << cachedSongCount << " songs have been cached!!" << endl;
        cout << PINK << BOLD << "[CACHE] Songs' cache has been stored in " << cacheInfoDirectory << RESET << endl;
        cout << BLUE <<  BOLD << "----------------- LITEMUS -- CACHE -- OVER -------------------" << RESET << endl;
    }

    return 0;
}
