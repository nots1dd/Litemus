#include "../sflw_helpers.hpp"

void playMusic(sf::Music& music, const std::string& songPath) {
    if (music.getStatus() == sf::Music::Playing) {
        music.stop();
    }
    if (!music.openFromFile(songPath)) {
        std::cerr << "Error loading file" << std::endl;
    } else {
        music.play();
    }
}

void nextSong(sf::Music& music, const std::vector<std::string>& songs, int& currentSongIndex) {
    music.stop();
    currentSongIndex = (currentSongIndex + 1) % songs.size();
    std::string nextSongPath = songs[currentSongIndex];
    playMusic(music, nextSongPath);    
}

void previousSong(sf::Music& music, const std::vector<std::string>& songs, int& currentSongIndex) {
    music.stop();
    currentSongIndex = (currentSongIndex - 1 + songs.size()) % songs.size();
    std::string prevSongPath = songs[currentSongIndex];
    playMusic(music, prevSongPath);
}

void adjustVolume(sf::Music& music, float volumeChange) {
    float currentVolume = music.getVolume();
    currentVolume += volumeChange;
    currentVolume = std::max(0.f, std::min(100.f, currentVolume));
    music.setVolume(currentVolume);
}

