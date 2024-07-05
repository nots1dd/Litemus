#include "../sfml_helpers.hpp"

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

void toggleMute(sf::Music& music, bool isMuted) {
  if (!isMuted) {
    adjustVolume(music, -100.f);
  } else {
    adjustVolume(music, 100.f);
  }
}

void seekSong(sf::Music& music, int seekVal, bool forward) {
  if (forward) {
    music.setPlayingOffset(music.getPlayingOffset() + sf::seconds(seekVal));
  } else {
    if (music.getPlayingOffset() > sf::seconds(seekVal)) {
          music.setPlayingOffset(music.getPlayingOffset() - sf::seconds(seekVal));
    } else {
          music.setPlayingOffset(sf::seconds(0));
    }
  }
}

std::string getSongDuration(const std::string& songPath) {
    sf::Music music;
    if (!music.openFromFile(songPath)) {
        return "00:00";
    }
    sf::Time duration = music.getDuration();
    int seconds = static_cast<int>(duration.asSeconds());
    int minutes = seconds / 60;
    seconds %= 60;
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
    return std::string(buffer);
}



