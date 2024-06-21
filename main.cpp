#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include "nlohmann/json.hpp"
#include "headers/exitError.h"
#include "headers/executeCmd.h"
#include "headers/sanitize.h"
#include "headers/directoryUtils.hpp"

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

// DIRECTORY VARIABLES
const string songDirectory = "/home/s1dd/Downloads/Songs/";
const string cacheDirectory = songDirectory + ".cache/";
const string cacheLitemusDirectory = cacheDirectory + "litemus/";
const string cacheSortDirectory = cacheLitemusDirectory + "songs/";
const string cacheInfoDirectory = cacheLitemusDirectory + "info/";
const string artistsFilePath = cacheInfoDirectory + "artists.json";

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

// Function to store metadata in JSON format
void storeMetadataJSON(const string& fileName, json& artistsArray, json& songsInfoArray) {
    string metadataCmd = "ffprobe -v quiet -print_format json -show_format \"" + fileName + "\"";
    string metadataInfo = executeCommand(metadataCmd);

    auto metadata = json::parse(metadataInfo);

    string artist = metadata["format"]["tags"].contains("artist") ? metadata["format"]["tags"]["artist"].get<string>() : "Unknown_Artist";
    string album = metadata["format"]["tags"].contains("album") ? metadata["format"]["tags"]["album"].get<string>() : "Unknown_Album";
    string title = metadata["format"]["tags"].contains("title") ? metadata["format"]["tags"]["title"].get<string>() : "Unknown_Title";

    string trueArtist = artist;
    string trueTitle = title;

    artist = sanitizeString(artist);
    album = sanitizeString(album);
    title = sanitizeString(title);

    // Check if artist is already in the array
    if (find(artistsArray.begin(), artistsArray.end(), trueArtist) == artistsArray.end()) {
        artistsArray.push_back(trueArtist);
    }

    string artistDir = ".cache/litemus/songs/" + artist;
    createDirectory(artistDir);

    string albumDir = artistDir + "/" + album;
    createDirectory(albumDir);

    string songFileName = albumDir + "/" + title + ".cache";
    ofstream outFile(songFileName, ios::trunc);
    if (outFile.is_open()) {
        outFile << metadataInfo;
        outFile.close();
    } else {
        printErrorAndExit("[ERROR] Unable to open file for writing: " + songFileName);
    }

    // Add song info to songsInfoArray
    json songInfo = {
        {"file_name", songFileName},
        {"artist", trueArtist},
        {"album", album},
        {"title", trueTitle}
    };
    songsInfoArray.push_back(songInfo);
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

// Function to store song count, inodes, and song names
void storeSongCountAndInodes(const string& infoDirectory, int songCount, const vector<string>& inodes, const vector<string>& songNames, const json& songsInfoArray) {
    json infoJson;
    infoJson["song_count"] = songCount;
    infoJson["inodes"] = inodes;
    infoJson["song_names"] = songNames; // Add song names to the JSON object
    infoJson["songs_info"] = songsInfoArray;

    string infoFilePath = infoDirectory + "/song_cache_info.json";
    ofstream outFile(infoFilePath, ios::trunc);
    if (outFile.is_open()) {
        outFile << infoJson.dump(4);
        outFile.close();
    } else {
        printErrorAndExit("[ERROR] Unable to open file for writing: " + infoFilePath);
    }
}

// Function to save song names to a JSON file
void storeSongsJSON(const string& filePath, const vector<string>& songNames) {
    json songsJson = songNames;
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

int main() { 
    changeDirectory(songDirectory);
    createDirectory(cacheDirectory);
    createDirectory(cacheLitemusDirectory);
    createDirectory(cacheSortDirectory);
    createDirectory(cacheInfoDirectory);

    vector<string> inodes = getInodes();
    vector<string> songNames;

    if (inodes.empty()) {
        cerr << RED << "No inodes found." << RESET << endl;
        return 1;
    }

    // Load previous inodes from song_cache_info.json if it exists
    vector<string> previousInodes;
    json artistsArray;
    int previousSongCount = 0;
    json songsInfoArray;
    string infoFilePath = cacheInfoDirectory + "/song_cache_info.json";
    ifstream infoFile(infoFilePath);
    if (infoFile.is_open()) {
        json infoJson;
        infoFile >> infoJson;
        previousSongCount = infoJson["song_count"];
        previousInodes = infoJson["inodes"].get<vector<string>>();
        songsInfoArray = infoJson["songs_info"];
        infoFile.close();
    }

    // Compare current inodes with previous inodes
    if (compareInodeVectors(inodes, previousInodes)) {
        cout << PINK << BOLD << "[CACHE] No changes in song files. Exiting without caching." << RESET << endl;
    } else {
        int cachedSongCount = 0;

        for (const string& inode : inodes) {
            string fileName = getFileNameFromInode(inode);
            cout << BLUE << "==> " << fileName << RESET << endl;
            songNames.push_back(fileName); // Store song names
            storeMetadataJSON(fileName, artistsArray, songsInfoArray);
            cachedSongCount++;
        }

        storeSongCountAndInodes(cacheInfoDirectory, cachedSongCount, inodes, songNames, songsInfoArray);

        cout << endl << GREEN << BOLD << "[SUCCESS] Total of " << cachedSongCount << " songs have been cached in " << cacheSortDirectory << RESET << endl;
        cout << PINK << BOLD << "[CACHE] Songs' cache has been stored in " << cacheInfoDirectory << RESET << endl;
    }

    saveArtistsToFile(artistsArray, artistsFilePath);
    storeSongsJSON(cacheInfoDirectory + "/song_names.json", songNames); // Store song names in JSON file
    printArtists(artistsArray);
    cout << PINK << BOLD << "[SONG] Play functions go here" << endl;

    return 0;
}
