#ifndef MAIN_HPP
#define MAIN_HPP

#include <iostream>
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
void storeMetadataJSON(const string& fileName, json& artistsArray, json& songsInfoArray);
void saveArtistsToFile(const json& artistsArray, const string& filePath);
void printArtists(const json& artistsArray);
void storeSongCountAndInodes(const string& infoDirectory, int songCount, const vector<string>& inodes, const vector<string>& songNames, const json& songsInfoArray);
void storeSongsJSON(const string& filePath, const vector<string>& songNames);
bool compareInodeVectors(const vector<string>& vec1, const vector<string>& vec2);
int lmus_cache_main(const std::string songDirectory);

#endif // MAIN_HPP
