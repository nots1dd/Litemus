#ifndef SFLW_HELPERS_H
#define SFLW_HELPERS_H

#include <SFML/Audio.hpp>
#include <cstring>
#include <iostream>
#include <algorithm>

void playMusic(sf::Music& music, const std::string& songPath);
void nextSong(sf::Music& music, const std::vector<std::string>& songs, int& currentSongIndex);
void previousSong(sf::Music& music, const std::vector<std::string>& songs, int& currentSongIndex);
void adjustVolume(sf::Music& music, float volumeChange);
void toggleMute(sf::Music& music, bool isMuted);
void seekSong(sf::Music& music, int seekVal, bool forward);
std::string getSongDuration(const std::string& songPath);

#endif
