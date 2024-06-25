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
const string extension = ".mp3";

struct SongMetadata {
    string fileName;
    string inode;
    string artist;
    string album;
    string title;
    int disc;
    int track;
    string genre;
    string lyrics;
};

// Function to get the list of inodes
vector<string> getInodes() {
    const string output_cmd = "ls -i *" + extension + " | awk '{print $1}'";
    string output = executeCommand(output_cmd);
    vector<string> inodes;
    stringstream ss(output);
    string inode;

    while (getline(ss, inode, '\n')) {
        if (!inode.empty()) {
            inodes.push_back(inode);
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
        if (c == '$' || c == '#') {
            escapedFileName += '\\'; // Add a backslash before special characters
        }
        escapedFileName += c;
    }
    return escapedFileName;
}
// Function to store metadata in JSON format
void storeMetadataJSON(const string& inode, const string& fileName, json& artistsArray, vector<SongMetadata>& songMetadata) {
    // Escape special characters in the filename
    string escapedFileName = escapeSpecialCharacters(fileName);

    // Construct the ffprobe command with the escaped filename
    string metadataCmd = "ffprobe -v quiet -print_format json -show_format \"" + escapedFileName + "\"";
    string metadataInfo = executeCommand(metadataCmd);

    auto metadata = json::parse(metadataInfo);

    string artist = metadata["format"]["tags"].contains("artist") ? metadata["format"]["tags"]["artist"].get<string>() : "Unknown_Artist";
    string album = metadata["format"]["tags"].contains("album") ? metadata["format"]["tags"]["album"].get<string>() : "Unknown_Album";
    string title = metadata["format"]["tags"].contains("title") ? metadata["format"]["tags"]["title"].get<string>() : "Unknown_Title";
    int disc = metadata["format"]["tags"].contains("disc") ? stoi(metadata["format"]["tags"]["disc"].get<string>()) : 0;
    int track = metadata["format"]["tags"].contains("track") ? stoi(metadata["format"]["tags"]["track"].get<string>()) : 0;
    string genre = metadata["format"]["tags"].contains("genre") ? metadata["format"]["tags"]["genre"].get<string>() : "";
    string lyrics = metadata["format"]["tags"].contains("lyrics-XXX") ? metadata["format"]["tags"]["lyrics-XXX"].get<string>() : "";

    // Use true values for original metadata
    string trueArtist = artist;
    string trueTitle = title;
    string trueAlbum = album;

    // Sanitize strings
    artist = sanitizeString(artist);
    album = sanitizeString(album);
    title = sanitizeString(title);

    // Check if artist is already in the array
    if (find(artistsArray.begin(), artistsArray.end(), trueArtist) == artistsArray.end()) {
        artistsArray.push_back(trueArtist);
    }

    // Store metadata
    songMetadata.push_back({fileName, inode, trueArtist, trueAlbum, trueTitle, disc, track, genre, lyrics});
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

// Function to print artists
void printArtists(const json& artistsArray) {
    cout << "Artists List:" << endl;
    for (const auto& artist : artistsArray) {
        cout << " - " << artist << endl;
    }
}



void storeSongsJSON(const string& filePath, const vector<SongMetadata>& songMetadata) {
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
            cout << "Artist: " << song.artist << ", Album: " << song.album << ", Disc: " << song.disc << ", Track: " << song.track << endl;

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

int lmus_cache_main(const std::string songDirectory) {
    // DIRECTORY VARIABLES
    const string cacheDirectory = songDirectory + ".cache/";
    const string cacheLitemusDirectory = cacheDirectory + "litemus/";
    const string cacheInfoDirectory = cacheLitemusDirectory + "info/";
    const string artistsFilePath = cacheInfoDirectory + "artists.json";
    const string songCacheInfoFile = cacheInfoDirectory + "song_cache_info.json";

    changeDirectory(songDirectory);
    createDirectory(cacheDirectory);
    createDirectory(cacheLitemusDirectory);
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
    } else {
        int cachedSongCount = 0;

        for (const string& inode : inodes) {
            string fileName = getFileNameFromInode(inode);
            cout << BLUE << "==> " << fileName << RESET << endl;
            storeMetadataJSON(inode, fileName, artistsArray, songMetadata);
            cachedSongCount++;
        }

        saveArtistsToFile(artistsArray, artistsFilePath);

        // Sort songMetadata by artist, album, disc, and track
        sort(songMetadata.begin(), songMetadata.end(), [](const SongMetadata& a, const SongMetadata& b) {
            if (a.artist != b.artist) return a.artist < b.artist;
            if (a.album != b.album) return a.album < b.album;
            if (a.disc != b.disc) return a.disc < b.disc;
            return a.track < b.track;
        });

        storeSongsJSON(cacheInfoDirectory + "/song_names.json", songMetadata);

        // Save current inodes for future comparison
        saveCurrentInodes(inodes, songCacheInfoFile);

        cout << endl << GREEN << BOLD << "[SUCCESS] Total of " << cachedSongCount << " songs have been cached!!" << endl;
        cout << PINK << BOLD << "[CACHE] Songs' cache has been stored in " << cacheInfoDirectory << RESET << endl;
    }

    return 0;
}
