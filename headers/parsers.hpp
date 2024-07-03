#ifndef PARSERS_H
#define PARSERS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>

std::vector<std::string> parseArtists(const std::string& artistsFile);
std::pair<std::vector<std::string>, std::vector<std::string>> listSongs(const std::string& cacheFile, const std::string& artistName, const std::string& songDirectory);
std::pair<std::string, std::string> findCurrentGenreArtist(const std::string& cacheFile, const std::string& currentSong, std::string& currentLyrics);
std::vector<std::string> splitStringByNewlines(const std::string& str);
std::string get_home_directory();
std::string read_file_to_string(const std::string& path);
void litemusHelper(const std::string& NC);

#endif
