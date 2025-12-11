#include <iostream>
#include <string>
#include <vector>

// Raylib and Raygui libraries
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// --- CONFIGURATION ---
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 450
#define MAX_SONGS 5

// IMPORTANT: Replace these with valid paths to your music files.
// Ensure these files exist in the same directory as your executable or use full paths.
#define SONG_PATH_1 "song1.mp3" 
#define SONG_PATH_2 "song2.mp3"
#define SONG_PATH_3 "song3.mp3"
#define SONG_PATH_4 "song4.mp3"
#define SONG_PATH_5 "song5.ogg" 

// Structure to hold song information
struct Song
{
    std::string title;
    Music music;
    bool isLoaded = false;
};

// --- GLOBAL VARIABLES ---
std::vector<Song> playlist;
int currentSongIndex = 0;
bool isPlaying = false;
float volume = 0.5f;

// UI Control variables
int listViewScrollIndex = 0;
int listViewActive = 0; // Index of the selected item in the list view

// Custom function to load music with proper error checking
void LoadPlaylist()
{
    std::string songPaths[MAX_SONGS] = {
        SONG_PATH_1,
        SONG_PATH_2,
        SONG_PATH_3,
        SONG_PATH_4,
        SONG_PATH_5
    };

    std::string songTitles[MAX_SONGS] = {
        "Cosmic Drive (MP3)",
        "Ocean Waves (MP3)",
        "Synthwave Runner (MP3)",
        "Acoustic Chill (MP3)",
        "Epic Orchestral (OGG)"
    };

    for (int i = 0; i < MAX_SONGS; i++)
    {
        Music music = LoadMusicStream(songPaths[i].c_str());

        // Check if the music loaded successfully
        if (music.stream.buffer != NULL)
        {
            Song newSong;
            newSong.title = songTitles[i];
            newSong.music = music;
            newSong.isLoaded = true;
            playlist.push_back(newSong);
            std::cout << "INFO: Successfully loaded song: " << newSong.title << std::endl;
        }
        else
        {
            std::cerr << "WARNING: Failed to load music file: " << songPaths[i] << " - Skipping song." << std::endl;
        }
    }

    if (playlist.empty())
    {
        std::cerr << "ERROR: No songs were loaded! The player cannot run." << std::endl;
    }
    else
    {
        std::cout << "INFO: Successfully loaded " << playlist.size() << " songs into the playlist." << std::endl;
    }
}

// Function to unload all music from the playlist
void UnloadPlaylist()
{
    for (Song& song : playlist)
    {
        if (song.isLoaded)
        {
            UnloadMusicStream(song.music);
        }
    }
}

// Function to switch and start a new song
void PlayNewSong(int newIndex)
{
    if (newIndex >= 0 && newIndex < playlist.size())
    {
        // 1. Stop the currently playing music (if any)
        if (isPlaying && playlist[currentSongIndex].isLoaded)
        {
            StopMusicStream(playlist[currentSongIndex].music);
        }

        currentSongIndex = newIndex;
        listViewActive = newIndex; // Update the UI list selection

        // 2. Play the new song
        if (playlist[currentSongIndex].isLoaded)
        {
            // Rewind the stream before playing the new song
            SeekMusicStream(playlist[currentSongIndex].music, 0.0f);
            PlayMusicStream(playlist[currentSongIndex].music);
            isPlaying = true;
        }
        else
        {
            // If the song didn't load, stop playback state
            isPlaying = false;
        }
    }
}

// --- MAIN PROGRAM ---
int main(void)
{
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib/Raygui Music Player");
    InitAudioDevice(); // Initialize audio device
    
    // Set audio log level to see device initialization
    SetTraceLogLevel(LOG_INFO);

    // Load the playlist and audio
    LoadPlaylist();

    // Check if any songs were loaded
    if (playlist.empty())
    {
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    // Initial setup for the first song
    // We only need to load it, we will play/pause based on UI input
    SetMusicVolume(playlist[currentSongIndex].music, volume); 
    listViewActive = currentSongIndex; // Ensure list view highlights the first song

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose())
    {
        // 1. UPDATE
        // Must be called every frame to buffer and play audio
        if (isPlaying && playlist[currentSongIndex].isLoaded)
        {
            UpdateMusicStream(playlist[currentSongIndex].music);

            // Check for end of song to move to the next track (loop)
            if (GetMusicTimeLength(playlist[currentSongIndex].music) > 0 &&
                GetMusicTimePlayed(playlist[currentSongIndex].music) >= GetMusicTimeLength(playlist[currentSongIndex].music))
            {
                int nextIndex = (currentSongIndex + 1) % playlist.size();
                PlayNewSong(nextIndex);
            }
        }

        // Apply volume changes. This must be outside the isPlaying block.
        SetMusicVolume(playlist[currentSongIndex].music, volume);
        
        // 2. DRAW
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // --- UI Layout definitions ---
            Rectangle nowPlayingPanel = { 20, 20, 260, SCREEN_HEIGHT - 40 };
            Rectangle playlistPanel = { 300, 20, 280, SCREEN_HEIGHT - 40 };

            // --- LEFT PANEL: Now Playing ---
            GuiPanel(nowPlayingPanel, "NOW PLAYING");
            
            // Get progress calculation
            float progress = 0.0f;
            if (playlist[currentSongIndex].isLoaded) {
                 progress = GetMusicTimePlayed(playlist[currentSongIndex].music) / GetMusicTimeLength(playlist[currentSongIndex].music);
                 progress = (progress > 1.0f || progress < 0.0f) ? 0.0f : progress; // Clamp/reset if invalid
            }
            
            // Display current song title
            DrawText(playlist[currentSongIndex].title.c_str(), nowPlayingPanel.x + 20, nowPlayingPanel.y + 30, 20, DARKGRAY);

            // Progress Bar
            Rectangle progressBarRect = { nowPlayingPanel.x + 20, nowPlayingPanel.y + 70, nowPlayingPanel.width - 40, 15 };
            // FIX: GuiProgressBar requires a float pointer for the value
            GuiProgressBar(progressBarRect, NULL, NULL, &progress, 0.0f, 1.0f);

            // Time labels
            float timePlayed = GetMusicTimePlayed(playlist[currentSongIndex].music);
            float timeTotal = GetMusicTimeLength(playlist[currentSongIndex].music);
            DrawText(TextFormat("%02i:%02i", (int)timePlayed / 60, (int)timePlayed % 60), 
                     progressBarRect.x, progressBarRect.y + progressBarRect.height + 5, 10, GRAY);
            DrawText(TextFormat("%02i:%02i", (int)timeTotal / 60, (int)timeTotal % 60), 
                     progressBarRect.x + progressBarRect.width - 25, progressBarRect.y + progressBarRect.height + 5, 10, GRAY);

            // --- Controls (Play/Pause/Stop/Next/Prev) ---
            Rectangle playButtonRect = { nowPlayingPanel.x + 105, nowPlayingPanel.y + 120, 50, 50 };
            Rectangle prevButtonRect = { nowPlayingPanel.x + 40, nowPlayingPanel.y + 120, 50, 50 };
            Rectangle nextButtonRect = { nowPlayingPanel.x + 170, nowPlayingPanel.y + 120, 50, 50 };

            // Play / Pause Button
            if (GuiButton(playButtonRect, isPlaying ? "#146# PAUSE" : "#145# PLAY")) // #146# is the Pause icon, #145# is the Play icon
            {
                if (playlist[currentSongIndex].isLoaded)
                {
                    if (isPlaying)
                    {
                        PauseMusicStream(playlist[currentSongIndex].music);
                    }
                    else
                    {
                        // Check if music finished and restart if so, otherwise just resume
                        if (GetMusicTimeLength(playlist[currentSongIndex].music) > 0 &&
                            GetMusicTimePlayed(playlist[currentSongIndex].music) >= GetMusicTimeLength(playlist[currentSongIndex].music))
                        {
                             SeekMusicStream(playlist[currentSongIndex].music, 0); // Rewind
                        }
                        ResumeMusicStream(playlist[currentSongIndex].music);
                    }
                    isPlaying = !isPlaying;
                }
            }

            // Previous Button
            if (GuiButton(prevButtonRect, "#133#")) // #133# is the Previous icon
            {
                int newIndex = (currentSongIndex - 1 + playlist.size()) % playlist.size();
                PlayNewSong(newIndex);
            }

            // Next Button
            if (GuiButton(nextButtonRect, "#134#")) // #134# is the Next icon
            {
                int newIndex = (currentSongIndex + 1) % playlist.size();
                PlayNewSong(newIndex);
            }
            
            // --- Volume Slider ---
            Rectangle volumeSliderRect = { nowPlayingPanel.x + 20, nowPlayingPanel.y + 200, nowPlayingPanel.width - 40, 20 };
            GuiLabel({volumeSliderRect.x, volumeSliderRect.y - 20, 60, 20}, "Volume");
            // FIX: GuiSliderBar requires a float pointer for the value
            GuiSliderBar(volumeSliderRect, NULL, TextFormat("%i%%", (int)(volume * 100)), &volume, 0.0f, 1.0f);


            // --- RIGHT PANEL: Playlist Management ---
            GuiPanel(playlistPanel, "PLAYLIST MANAGEMENT");

            // Prepare the list view items string
            std::string listItems;
            for (size_t i = 0; i < playlist.size(); ++i)
            {
                // Prepend an index for better list view display
                listItems += TextFormat("%02i. %s", (int)i + 1, playlist[i].title.c_str());
                if (i < playlist.size() - 1) listItems += ";";
            }

            // Playlist List View
            Rectangle listViewRect = { playlistPanel.x + 20, playlistPanel.y + 30, playlistPanel.width - 40, 300 };
            int result = GuiListView(listViewRect, listItems.c_str(), &listViewScrollIndex, &listViewActive);

            // Handle selection change in list view
            if (result == 1) // Item selected/clicked
            {
                if (listViewActive != currentSongIndex)
                {
                    PlayNewSong(listViewActive);
                }
            }


        EndDrawing();
    }

    // De-Initialization
    UnloadPlaylist();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}