#ifndef PARSERS_H
#define PARSERS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <cctype>
#include <nlohmann/json.hpp>

using namespace std;

std::vector<std::string> parseArtists(const std::string& artistsFile);
std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>> listSongs(const std::string& cacheFile, const std::string& artistName, const std::string& songsDirectory); 
std::pair<std::string, std::string> findCurrentGenreArtist(const std::string& cacheFile, const std::string& currentSong, std::string& currentLyrics);
std::vector<std::string> splitStringByNewlines(const std::string& str);
std::string get_home_directory();
std::string read_file_to_string(const std::string& path);
void litemusHelper(const std::string& NC);
std::vector<std::string> getTitlesWithWhiteSpaces(const std::vector<std::string>& songTitles, const std::vector<std::string>& songDurations, size_t maxLength);
size_t getMaxSongTitleLength(const std::vector<std::string>& songTitles, const std::vector<std::string>& songDurations);
std::string removeWhitespace(const std::string& str);
void verboseQuit(const std::string& NC, const std::string& BLUE, const std::string& BOLD);

#endif
