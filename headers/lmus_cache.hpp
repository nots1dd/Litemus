#ifndef MAIN_HPP
#define MAIN_HPP

#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include "nlohmann/json.hpp"
#include "exitError.h"
#include "executeCmd.h"
#include "directoryUtils.hpp"

using json = nlohmann::json;
using namespace std;

// Function declarations
vector<string> getInodes();
string getFileNameFromInode(const string& inode);
vector<string> loadPreviousInodes(const string& filePath);
void storeMetadataJSON(const string& fileName, json& artistsArray, json& songsInfoArray, const std::string debugFile);
void saveArtistsToFile(const json& artistsArray, const string& filePath);
void saveSongDirToFile(const std::string& songDirPath, const string& songDirectory);
void printArtists(const json& artistsArray);
void storeSongCountAndInodes(const string& infoDirectory, int songCount, const vector<string>& inodes, const vector<string>& songNames, const json& songsInfoArray);
void storeSongsJSON(const string& filePath, const vector<string>& songNames);
bool compareInodeVectors(const vector<string>& vec1, const vector<string>& vec2);
int lmus_cache_main(std::string& songDirectory, const std::string homeDir, const std::string cacheLitemusDirectory, const std::string configLitemusDirectory, const std::string cacheInfoDirectory, const std::string songCacheInfoFile, const std::string artistsFilePath, const std::string songDirPathCache, const std::string debugFile);

#endif // MAIN_HPP
