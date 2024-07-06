#include "../parsers.hpp"
#include "../sfml_helpers.hpp"

using json = nlohmann::json;

std::string get_home_directory() {
    const char* homeDir = getenv("HOME");
    if (homeDir == nullptr) {
        // Handle error or fallback
        return "";
    } else {
        return std::string(homeDir);
    }
}

std::vector<std::string> parseArtists(const std::string& artistsFile) {
    std::ifstream file(artistsFile);
    if (!file.is_open()) {
        std::cerr << "Could not open artists file: " << artistsFile << std::endl;
        return {};
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return {};
    }

    std::vector<std::string> artists;
    for (const auto& artist : j) {
        artists.push_back(artist.get<std::string>());
    }

    return artists;
}


std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<std::string>> listSongs(const std::string& cacheFile, const std::string& artistName, const std::string& songsDirectory) {
    std::ifstream file(cacheFile);
    if (!file.is_open()) {
        std::cerr << "Could not open cache file: " << cacheFile << std::endl;
        return {{}, {}, {}};
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return {{}, {}, {}};
    }

    std::vector<std::string> songTitles;
    std::vector<std::string> songPaths;
    std::vector<std::string> songDurations;

    // Iterate over each artist
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it.key() == artistName) {
            auto& albums = it.value();

            // Iterate over each album of the artist
            for (auto albumIt = albums.begin(); albumIt != albums.end(); ++albumIt) {
                std::string albumName = albumIt.key();
                auto& discs = albumIt.value();

                // Iterate over each disc in the album
                for (auto discIt = discs.begin(); discIt != discs.end(); ++discIt) {
                    auto& tracks = *discIt;

                    // Iterate over each track in the disc
                    for (auto trackIt = tracks.begin(); trackIt != tracks.end(); ++trackIt) {
                        auto& songInfo = *trackIt;

                        // Check if the track contains a valid song object
                        if (!songInfo.empty()) {
                            std::string songTitle = songInfo["title"].get<std::string>();
                            std::string songFilename = songInfo["filename"].get<std::string>();
                            std::string songPath = songsDirectory + songFilename;

                            songTitles.push_back(songTitle);
                            songPaths.push_back(songPath);
                            songDurations.push_back(getSongDuration(songPath));
                        }
                    }
                }
            }
        }
    }

    return {songTitles, songPaths, songDurations};
}

std::pair<std::string, std::string> findCurrentGenreArtist(const std::string& cacheFile, const std::string& currentSong, std::string& currentLyrics) {
    std::ifstream file(cacheFile);
    if (!file.is_open()) {
        std::cerr << "Could not open cache file: " << cacheFile << std::endl;
        return {};
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return {};
    }

    for (auto it = j.begin(); it != j.end(); ++it) {
        for (auto albumIt = it.value().begin(); albumIt != it.value().end(); ++albumIt) {
            for (auto discIt = albumIt.value().begin(); discIt != albumIt.value().end(); ++discIt) {
                for (auto trackIt = discIt.value().begin(); trackIt != discIt.value().end(); ++trackIt) {
                    if (trackIt.value()["title"] == currentSong) {
                        std::string genre = trackIt.value()["genre"];
                        std::string artist = it.key();
                        currentLyrics = trackIt.value()["lyrics"];
                        return {genre, artist}; // Return the genre and artist as a pair
                    }
                }
            }
        }
    }

    return {}; // Return empty pair if genre and artist not found
}

std::vector<std::string> splitStringByNewlines(const std::string& str) {
    std::vector<std::string> lines;
    std::istringstream stream(str);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string read_file_to_string(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    if (file.is_open()) {
        buffer << file.rdbuf();
        file.close();
    }
    return buffer.str();
}

void litemusHelper(const std::string& NC) {
    std::cout << NC << "LiteMus - Light Music Player (lmus)" << std::endl
              << std::endl
              << "Commands:" << std::endl << std::endl
              << " run               Run Litemus" << std::endl
              << " --help            Show this help dialog and exit" << std::endl
              << " --remote-cache    Remotely cache songs (dir set in $HOME/.cache/litemus/songDirectory.txt)" << std::endl
              << " --clear-cache     Remove the current chosen directory's cache" << std::endl
              << std::endl;
}

size_t getMaxSongTitleLength(const std::vector<std::string>& songTitles, const std::vector<std::string>& songDurations) {
    size_t maxLength = 0;
    for (size_t i = 0; i < songTitles.size(); ++i) {
        size_t combinedLength = songTitles[i].length() + songDurations[i].length() + 10; // 5 is for spaces and brackets
        if (combinedLength > maxLength) {
            maxLength = combinedLength;
        }
    }
    return maxLength;
}

std::string getRequiredWhitespaces(const std::string& title, size_t maxLength) {
    size_t titleLength = title.length();
    if (titleLength >= maxLength) {
        return "";  // No extra whitespace needed if title length is already max
    } else {
        return std::string(maxLength - titleLength, ' ');
    }
}

std::vector<std::string> getTitlesWithWhiteSpaces(const std::vector<std::string>& songTitles, const std::vector<std::string>& songDurations, size_t maxLength) {
    std::vector<std::string> titlesWithWhitespaces;
    for (size_t i = 0; i < songTitles.size(); ++i) {
        std::string formattedTitle = songTitles[i] + getRequiredWhitespaces(songTitles[i], maxLength) + " " + songDurations[i];
        titlesWithWhitespaces.push_back(formattedTitle);
    }
    return titlesWithWhitespaces;
}

std::string removeWhitespace(const std::string& str) {
    std::string result;
    std::remove_copy_if(str.begin(), str.end(), std::back_inserter(result), ::isspace);
    return result;
}


