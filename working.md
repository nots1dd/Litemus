```plaintext
  +---------------------------------------------------------------+
  |                       Input: songDirectory                     |
  |                     (Directory of .mp3 files)                  |
  +---------------------------------------------------------------+
                                   |
                                   v
          +---------------------------------------------------+
          |                Initialize Directory               |
          |             Create Cache Directories              |
          +------------------------+--------------------------+
                                   |
                                   v
          +---------------------------------------------------+
          |                Retrieve Inodes                    |
          |                (getInodes())                      |
          +------------------------+--------------------------+
                                   |
                                   v
          +---------------------------------------------------+
          |            Compare with Previous Inodes           |
          |     (loadPreviousInodes(), compareInodeVectors()) |
          +------------------------+--------------------------+
                                   |
          +------------------------+--------------------------+
          |                        |                         |
          v                        v                         v
+-----------------+   No Changes Detected     +----------------------+
|                 | <------------------------ |                      |
|                 |                           |                      |
|  Console Output |                           |                      |
|  (No changes in |                           |                      |
|   song files)   |                           |                      |
|                 |                           |                      |
+-----------------+                           |                      |
                                   +----------------------+
                                   |                      |
                                   v                      |
          +---------------------------------------------------+
          |                Process Each Inode                 |
          |                Retrieve File Name,                |
          |            Extract Metadata with ffprobe,         |
          |               Store in SongMetadata               |
          |              (getFileNameFromInode(),             |
          |              storeMetadataJSON())                 |
          +------------------------+--------------------------+
                                   |
          +------------------------+--------------------------+
          |                        |                         |
          v                        v                         v
+-----------------+            +----------------+      +----------------+
|                 |            |                |      |                |
|                 |            |  Update JSON   |      |  Store Songs   |
|    Store Artists |           |    Metadata    |      |storeSongsJSON()|
|saveArtistsToFile()   |       | (artists.json) |      |                |
|                 |            |                |      |                |
|                 |            |                |      |                |
+-----------------+            +----------------+      +----------------+
                                   |
                                   v
          +---------------------------------------------------+
          |              Save Current Inodes                 |
          |              (saveCurrentInodes())               |
          +------------------------+--------------------------+
                                   |
                                   v
  +---------------------------------------------------------------+
  |          Output: JSON Files with Cached Metadata               |
  |   artists.json, song_names.json, song_cache_info.json          |
  +---------------------------------------------------------------+
                                   |
                                   v
  +---------------------------------------------------------------+
  |                 Console Output: Success Messages              |
  |     (Total songs cached, caching directories, success status) |
  +---------------------------------------------------------------+

```
### Caching System Overview

The caching system implemented in `lmus_cache.hpp` aims to organize and store metadata of music files (specifically `.mp3` files) found in a specified directory. It collects metadata such as artist name, album details, track information, and other relevant data using `ffprobe` to analyze each `.mp3` file. The system then stores this metadata in JSON format, organized by artist, album, disc, and track.

### Components of the Caching System

1. **Directory Setup:**
   - The system expects a directory containing `.mp3` files as input (`songDirectory`).

2. **Metadata Extraction:**
   - **`getInodes()`**: Retrieves the inode (unique identifier) of each `.mp3` file in the directory.
   - **`getFileNameFromInode()`**: Maps an inode back to the corresponding file name.

3. **Metadata Storage:**
   - **`storeMetadataJSON()`**: Executes `ffprobe` on each file to extract metadata such as artist, album, title, disc number, track number, release date, genre, and lyrics (if available). This metadata is then stored in `SongMetadata` structures.

4. **Artists and Songs JSON Creation:**
   - **`saveArtistsToFile()`**: Saves a JSON array of unique artist names to a file (`artists.json`).
   - **`storeSongsJSON()`**: Organizes `SongMetadata` into a structured JSON format (`song_names.json`) based on artist, album, disc, and track.

5. **Caching Mechanism:**
   - **Comparison of Inodes**: Compares the current set of inodes with previously cached inodes to determine if any changes (new files or deleted files) have occurred.
   - **File Caching**: If changes are detected, the system proceeds to cache metadata and updates the cache files (`artists.json`, `song_names.json`, and `song_cache_info.json`).

6. **Output and Logging:**
   - Provides console output to indicate the progress and status of the caching process.
   - Displays success messages upon successful caching and alerts for errors or issues encountered.

### Expected Output

Upon running the caching system:

- **Initial Run:**
  - It scans the directory for `.mp3` files, extracts metadata using `ffprobe`, and organizes this data into JSON files (`artists.json` and `song_names.json`).

- **Subsequent Runs:**
  - It compares the current set of files (by inodes) with previously cached files (`song_cache_info.json`).
  - If changes are detected (new files or deletions), it updates the JSON files accordingly and outputs the number of songs cached.

- **Console Output:**
  - Displays informative messages using ANSI escape sequences for color coding (`RED`, `GREEN`, `BLUE`, `PINK`, `YELLOW`, `BOLD`).
  - Indicates the number of songs processed and cached, along with any errors encountered during the process.

### Example Scenario

Suppose you have a directory `music/` containing various `.mp3` files. When you run `lmus_cache_main("music/")`, the system will:

- Analyze each `.mp3` file using `ffprobe`.
- Create JSON files (`artists.json`, `song_names.json`) containing structured metadata organized by artist, album, disc, and track.
- Compare current and previous states to determine changes in the file set.
- Output success messages if caching is successful or error messages if issues arise.

### Conclusion

There might (and is) a much better way of handling caching and updating songs in a directory, but this is what I thought would be the most practical way of doing it and has accomplised what I set out to do: A cache system that is NOT reliant on the string of the song filename, rather a permanent address (for UNIX systems) that always points toward a file in the UNIX filesystem.

->It might not also be the wisest option to have JSON files as a database, so I will consider shifting it to a YAML file in the future but since these files only contain strings with some unexpected shell special characters (like $, /), these do not pose a problem yet.
