#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstring> 

using namespace std;

// Define the hardcoded default image path
// NOTE: Make sure you have a file named 'default_cover.jpg' in the project directory
const string DEFAULT_ART_PATH = "default_cover.jpg"; 

// --- DATA STRUCTURES ---

class Node {
public:
    Node* next;
    Node* prev;
    string title;
    string artist;
    float duration;
    string filePath;
    string albumArtPath; 
    bool isFavorite;

    Node(string t, string a, float d, string p, string ap, bool f = false) 
        : title(t), artist(a), duration(d), filePath(p), albumArtPath(ap), isFavorite(f), next(nullptr), prev(nullptr) {}
    
    ~Node() {}
};

class MusicPlaylist {
private:
    Node* head; 
    Node* tail;
    int size;

    void saveToCSV() {
        fstream file;
        // The file is overwritten every time, ensuring favorites and order are saved
        file.open("Dontrey.csv", ios::out); 
        if (!file.is_open()) return;

        if (!head) { file.close(); return; }

        Node* temp = head;
        do {
            // Format: Title,Artist,Duration,FilePath,AlbumArtPath,IsFav (1 or 0)
            file << temp->title << "," << temp->artist << "," << temp->duration << "," << temp->filePath << "," << temp->albumArtPath << "," << temp->isFavorite << "\n";
            temp = temp->next;
        } while (temp != head);
        file.close();
    }

public:
    Node* current;

    // The playlist is implemented as a CIRCULAR DOUBLY LINKED LIST
    MusicPlaylist() : head(nullptr), tail(nullptr), current(nullptr), size(0) {}

    ~MusicPlaylist() {
        if (!head) return;
        Node* temp = head->next;
        while (temp != head) {
            Node* next = temp->next;
            delete temp;
            temp = next;
        }
        delete head;
    }

    bool isEmpty() const {
        return head == nullptr;
    }

    int getSize() const {
        return size;
    }
    
    void addSong(string title, string artist, float duration, string filePath, string albumArtPath, bool isFavorite = false) {
        if (albumArtPath.empty()) {
            albumArtPath = DEFAULT_ART_PATH;
        }

        Node* newNode = new Node(title, artist, duration, filePath, albumArtPath, isFavorite);

        if (head == nullptr) {
            head = tail = newNode;
            tail->next = head;
            head->prev = tail;
            current = head; // Set the new song as current if the playlist was empty
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            newNode->next = head;
            head->prev = newNode;
            tail = newNode;
        }
        size++;
        saveToCSV();
    }
    
    int GetListIndex(Node* target) {
        if (!head || !target) return -1;
        Node* temp = head;
        int index = 0;
        do {
            if (temp == target) return index;
            temp = temp->next;
            index++;
        } while (temp != head);
        return -1;
    }
    
    bool deleteNodeAtIndex(int index) {
        if (!head || index < 0 || index >= size) return false;

        Node* temp = head;
        for (int i = 0; i < index; i++) temp = temp->next;

        if (temp == current) {
            if (size > 1) current = temp->next;
            else current = nullptr; 
        }

        if (size == 1) {
            delete temp;
            head = tail = nullptr;
            size = 0;
            saveToCSV();
            return true; // Indicates the list is now empty
        } else {
            if (temp == head) head = head->next;
            if (temp == tail) tail = tail->prev;

            temp->prev->next = temp->next;
            temp->next->prev = temp->prev;
            delete temp;
        }
        size--;
        saveToCSV();
        return false; // Indicates the list is NOT empty
    }

    bool toggleFavorite() {
        if (current) {
            current->isFavorite = !current->isFavorite;
            saveToCSV();
            return current->isFavorite;
        }
        return false;
    }

    void shufflePlaylist() {
        if (!head || size < 2) return;
        
        // Use a vector to collect nodes for shuffling metadata
        vector<Node*> nodes;
        Node* temp = head;
        do {
            nodes.push_back(temp);
            temp = temp->next;
        } while (temp != head);
        
        // Fisher-Yates shuffle algorithm, swapping content between nodes
        for (int i = size - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            
            // Swap the contents (title, artist, path, etc.) between nodes[i] and nodes[j]
            swap(nodes[i]->title, nodes[j]->title);
            swap(nodes[i]->artist, nodes[j]->artist);
            swap(nodes[i]->duration, nodes[j]->duration);
            swap(nodes[i]->filePath, nodes[j]->filePath);
            swap(nodes[i]->albumArtPath, nodes[j]->albumArtPath); 
            swap(nodes[i]->isFavorite, nodes[j]->isFavorite);
        }
        saveToCSV();
    }

    void playNext() { if (current) current = current->next; }
    void playPrevious() { if (current) current = current->prev; }

    void playByIndex(int index) {
        if (!head || index < 0 || index >= size) return;
        Node* temp = head;
        for(int i=0; i<index; i++) temp = temp->next;
        current = temp;
    }

    void loadFromCSV() {
        // FIX APPLIED HERE: Used "Dontrey.csv" to match saveToCSV()
        ifstream file("Dontrey.csv"); 
        if (!file.is_open()) return;
        string line;
        
        while (getline(file, line)) {
            // Trim whitespace (needed especially if the file has trailing spaces)
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
            if (line.empty()) continue; 

            stringstream ss(line);
            string t, a, dStr, p, ap, fStr;
            
            // Attempt to read all fields separated by comma
            if (!getline(ss, t, ',') || !getline(ss, a, ',') || !getline(ss, dStr, ',') || 
                !getline(ss, p, ',') || !getline(ss, ap, ',') || !getline(ss, fStr, ',')) {
                continue; 
            }

            if (t.empty() || p.empty()) continue; 

            float d = 0; try { d = stof(dStr); } catch(...) {}
            bool f = (fStr == "1");

            if (ap.empty()) ap = DEFAULT_ART_PATH; 
            
            addSong(t, a, d, p, ap, f); 
        }
        file.close();
    }

    string getListString(const char* filter, vector<int>& mapIndices) {
        if (!head) return "";
        string res = "";
        string search = filter;
        transform(search.begin(), search.end(), search.begin(), ::tolower);
        Node* temp = head;
        int realIndex = 0;
        mapIndices.clear();
        do {
            string entry = temp->title + " - " + temp->artist;
            string lower = entry;
            transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (search.empty() || lower.find(search) != string::npos) {
                res += entry + ";";
                mapIndices.push_back(realIndex);
            }
            temp = temp->next;
            realIndex++;
        } while (temp != head);
        if (!res.empty()) res.pop_back();
        return res;
    }

    string getFavoritesString() {
        if (!head) return "";
        string res = "";
        Node* temp = head;
        do {
            if (temp->isFavorite) res += temp->title + " - " + temp->artist + ";";
            temp = temp->next;
        } while (temp != head);
        if (!res.empty()) res.pop_back();
        return res;
    }
};

// --- GLOBAL NOTIFICATION SYSTEM ---
string notificationMsg = "";
int notificationTimer = 0;

void ShowNotification(string msg) {
    notificationMsg = msg;
    notificationTimer = 120; // 2 seconds (60 FPS * 2 seconds)
}

// --- COLORS ---
Color BG_COLOR = { 253, 240, 240, 255 };      
Color PANEL_BG = { 205, 180, 180, 255 };      
Color TEXT_COLOR = { 60, 40, 40, 255 };      

// --- AUDIO MANAGEMENT GLOBALS ---
Music currentMusic = { 0 };
bool isMusicLoaded = false;

// --- ALBUM ART GLOBALS ---
Texture2D currentAlbumArt = { 0 }; 
Texture2D defaultPlaceholder = { 0 }; // The dark gray circle generated in memory

// --- FIX: Robust image updating function ---
// Safely unloads previous custom art and loads the new one or falls back to defaults.
void LoadAndUpdateAlbumArt(const char* imagePath, Texture2D placeholder) {
    // 1. Unload the previous custom texture if one was loaded.
    // We check if currentAlbumArt is initialized (id != 0) AND is NOT the generic placeholder.
    if (currentAlbumArt.id != 0 && currentAlbumArt.id != placeholder.id) {
        UnloadTexture(currentAlbumArt); 
        currentAlbumArt = { 0 }; 
    }

    // 2. Attempt to load the SPECIFIC song art
    if (imagePath && strlen(imagePath) > 0 && FileExists(imagePath) && strcmp(imagePath, DEFAULT_ART_PATH.c_str()) != 0) {
        Texture2D tempTexture = LoadTexture(imagePath);
        if (tempTexture.id != 0) { 
            currentAlbumArt = tempTexture;
            return;
        }
        ShowNotification("Failed to load custom album art!");
    } 

    // 3. Fallback 1: Load default_cover.jpg if it exists
    if (FileExists(DEFAULT_ART_PATH.c_str())) {
        Texture2D defaultCover = LoadTexture(DEFAULT_ART_PATH.c_str());
        if (defaultCover.id != 0) {
            // Check if we are already using a default cover to avoid unnecessary reloads
            if (currentAlbumArt.id != defaultCover.id) {
                currentAlbumArt = defaultCover;
            }
            return;
        }
    }

    // 4. Final Fallback: Use the memory placeholder (gray circle)
    currentAlbumArt = placeholder;
}

void UnloadCurrentMusic() {
    if (isMusicLoaded) {
        StopMusicStream(currentMusic); // Stop before unloading
        UnloadMusicStream(currentMusic);
        isMusicLoaded = false;
    }
}

void LoadAndPlayMusic(Node* song, Texture2D placeholder) {
    if (!song) {
        UnloadCurrentMusic();
        LoadAndUpdateAlbumArt(nullptr, placeholder); 
        return;
    }

    UnloadCurrentMusic();

    if (FileExists(song->filePath.c_str())) {
        currentMusic = LoadMusicStream(song->filePath.c_str());
        // Set volume to max (or a decent default) just in case
        SetMusicVolume(currentMusic, 1.0f); 
        PlayMusicStream(currentMusic);
        isMusicLoaded = true;
    } else {
        ShowNotification("Audio file not found! Cannot play.");
        isMusicLoaded = false;
    }
    
    // Update the image
    LoadAndUpdateAlbumArt(song->albumArtPath.c_str(), placeholder); 
}

string FormatTime(float time) {
    if (time < 0) time = 0;
    int minutes = (int)time / 60;
    int seconds = (int)time % 60;
    string s = to_string(seconds);
    if (s.length() < 2) s = "0" + s;
    return to_string(minutes) + ":" + s;
}

// --- MAIN ---
int main() {
    // Initialization
    const int screenWidth = 1000;
    const int screenHeight = 650;
    
    // Initialize required systems
    InitAudioDevice();
    InitWindow(screenWidth, screenHeight, "Don Trey");
    SetTargetFPS(60);
    srand(time(0));

    // 1. Setup Generic Placeholder Art (Dark Gray Circle generated in memory)
    Image genericImage = GenImageColor(200, 200, DARKGRAY);
    defaultPlaceholder = LoadTextureFromImage(genericImage);
    UnloadImage(genericImage);

    // 2. Initial Art Load: Use the robust function to set currentAlbumArt
    LoadAndUpdateAlbumArt(DEFAULT_ART_PATH.c_str(), defaultPlaceholder);

    float rotationAngle = 0.0f;

    MusicPlaylist playlist;
    
    // Load existing songs (if any)
    playlist.loadFromCSV(); 

    // UI States (FIXED: Declared here to resolve compilation error)
    bool isPlaying = false; 
    bool isLooping = false; 
    bool showAddWindow = false;
    bool isBatchMode = false;
    bool showExitConfirm = false;

    // List States
    int scrollIndex = 0;
    int activeIndex = -1;
    int favScroll = 0;
    int favActive = -1;

    // Inputs
    char titleInput[128] = "";
    char artistInput[128] = "";
    char durInput[128] = "";
    char pathInput[128] = ""; 
    char albumArtPathInput[128] = ""; 
    char searchInput[128] = "";

    bool editTitle = false;
    bool editArtist = false;
    bool editDur = false;
    bool editPath = false; 
    bool editAlbumArtPath = false; 
    bool editSearch = false;
    
    // SEEK BAR State
    float timePlayed = 0.0f; 
    float timeLength = 0.0f; 

    vector<int> filteredIndices;

    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
    // Set the overall application background color style
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xfdf0f0ff); 
    // Set the default button color style for elements like 'Add Song' and 'Delete'
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xfdf0f0ff);
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0x000000ff);


    // If songs were loaded from CSV, load the current one (head)
    if (playlist.current) {
        LoadAndPlayMusic(playlist.current, defaultPlaceholder);
        // Start paused to prevent immediate playback until user clicks play
        PauseMusicStream(currentMusic);
        isPlaying = false; 
    }

while (!WindowShouldClose()) {

        if (isMusicLoaded) {
            UpdateMusicStream(currentMusic);
            timePlayed = GetMusicTimePlayed(currentMusic);
            timeLength = GetMusicTimeLength(currentMusic);

            if (isPlaying && timePlayed >= (timeLength - 0.1f) && timeLength > 0) {
                
                if (playlist.current) {
                    
                    // Get actual playlist size using the new getSize() method
                    int playlistSize = playlist.getSize();
                    
                    if (playlistSize > 1) {
                        // Case 1: Multiple songs exist - ALWAYS move to the next song
                        playlist.playNext();
                        LoadAndPlayMusic(playlist.current, defaultPlaceholder);
                        isPlaying = true; // Keep playing state active
                        ShowNotification("Playing next track.");
                        
                    } else if (playlistSize == 1 && isLooping) {
                        // Case 2: Only one song, but looping is ON (repeat the same song)
                        // Seek back to start and replay
                        SeekMusicStream(currentMusic, 0.0f);
                        PlayMusicStream(currentMusic); // Ensure it starts playing
                        isPlaying = true;
                        ShowNotification("Song repeated.");
                        
                    } else {
                        // Case 3: Only one song, looping is OFF. Stop playback.
                        StopMusicStream(currentMusic);
                        isPlaying = false;
                        ShowNotification("Playlist ended.");
                    }
                }
            }

        } else {
            timePlayed = 0.0f;
            timeLength = 0.0f;
        }

        // VISUAL ROTATION LOGIC
        if (isPlaying && isMusicLoaded) { 
            rotationAngle += 1.0f;
            if (rotationAngle > 360.0f) rotationAngle -= 360.0f;
        } else {
            rotationAngle *= 0.99f;
        }
        
        BeginDrawing();
        ClearBackground(BG_COLOR);
        DrawText("DonTrey", 355, 20, 50, TEXT_COLOR);

        // ============================================================
        // LEFT PANEL: PLAYER
        // ============================================================
        Rectangle leftPanel = { 40, 100, 350, 500 };
        DrawRectangleRec(leftPanel, PANEL_BG);
        // REMOVED: DrawText("Now Playing", leftPanel.x + 20, leftPanel.y + 20, 24, TEXT_COLOR); 
        // -> The text is moved below the image drawing for top layering priority.

        // CIRCULAR ROTATING IMAGE DRAWING
        int imageSize = 240;
        float imageRadius = (float)imageSize / 2.0f; // This is 135.0f
        
        int imageX = leftPanel.x + (leftPanel.width - imageSize) / 2; 
        int imageY = leftPanel.y + 60; 
        Vector2 imageCenter = { (float)imageX + imageSize/2, (float)imageY + imageSize/2 };

        if (currentAlbumArt.id != 0) {
            
            // 1. Draw the rotating square texture
            DrawTexturePro(
                currentAlbumArt, (Rectangle){ 0.0f, 0.0f, (float)currentAlbumArt.width, (float)currentAlbumArt.height },
                (Rectangle){ imageCenter.x, imageCenter.y, (float)imageSize, (float)imageSize },
                (Vector2){ imageRadius, imageRadius }, rotationAngle, WHITE
            );
            
            // 2. Draw a THICK ring MASK using the PANEL_BG color to crop the rotating corners.
            // Inner radius: imageRadius (135.0f)
            // Outer radius: imageRadius + 5.0f (Total 140.0f) - This is much thinner!
            DrawRing(imageCenter, imageRadius, imageRadius + 50.0f, 0, 360, 0, PANEL_BG);
        }
        
        // 3. Draw the visual border and center pin (Drawn LAST, on top of the mask)
        
        // Draw the black vinyl groove line (border) - This is the desired thin line.
        DrawCircleLines(imageCenter.x, imageCenter.y, imageRadius, BLACK);
        
        // Draw the white center pin
        DrawCircle(imageCenter.x, imageCenter.y, 25, RAYWHITE);

        // MOVE: Draw the "Now Playing" title here to ensure it is always on the top layer of the panel.
        DrawText("Now Playing", leftPanel.x + 20, leftPanel.y + 20, 24, TEXT_COLOR);


        if (playlist.current) {
            DrawText(playlist.current->title.c_str(), leftPanel.x + 20, leftPanel.y + 350, 22, BLACK);
            DrawText(playlist.current->artist.c_str(), leftPanel.x + 20, leftPanel.y + 380, 18, DARKGRAY);
            
            // SEEK BAR 
            Rectangle sliderBounds = { leftPanel.x + 20, leftPanel.y + 415, 310, 15 };
            
            DrawText(FormatTime(timePlayed).c_str(), sliderBounds.x, sliderBounds.y - 18, 14, DARKGRAY);
            DrawText(FormatTime(timeLength).c_str(), sliderBounds.x + sliderBounds.width - 40, sliderBounds.y - 18, 14, DARKGRAY);

            if (isMusicLoaded) {
                float newTime = timePlayed; 
                
                // Allow user to scrub the music stream
                if (GuiSlider(sliderBounds, NULL, NULL, &newTime, 0.0f, timeLength)) {
                    if (newTime != timePlayed) {
                        SeekMusicStream(currentMusic, newTime);
                        timePlayed = newTime;
                    }
                }
            } else {
                // Disabled slider visual
                DrawRectangleRec(sliderBounds, LIGHTGRAY); 
            }

            // --- FAVORITE BUTTON (UPDATED: Using GuiButton with conditional style) ---
            Rectangle favButtonBounds = { leftPanel.x + 15, leftPanel.y + 440, 40, 40 };

            // 1. Set the background color to match the dark player controls (Shuffle, etc.)
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x4a4a4aff);
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x646464ff); // Slightly lighter on hover
            GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x323232ff); // Darker on click

            // 2. Set the icon (text) color based on favorite status
            // Raygui Icon 164 is the heart.
            if (playlist.current->isFavorite) {
                // Song is a favorite: Use a highly visible color (e.g., RED)
                GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xff0000ff); 
                GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xff0000ff); 
                GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, 0xcc0000ff);
            } else {
                // Song is NOT a favorite: Use a light color (WHITE)
                GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xffffffff); 
                GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xffffffff); 
                GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, 0xccccccff);
            }

            // 3. Draw the Raygui Button
            if (GuiButton(favButtonBounds, "#186#")) {
                bool isFav = playlist.toggleFavorite();
                if(isFav) ShowNotification("Song Marked as Favorite!");
                else ShowNotification("Song Unmarked from Favorites.");
            }

            // 4. Reset Button Styles for the standard controls section
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xfdf0f0ff);
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0x000000ff);
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x828282ff);
            GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x5a5a5aff);
            // --- END OF FAVORITE BUTTON UPDATE ---
            
        } else {
            DrawText("No Song Playing", leftPanel.x + 80, leftPanel.y + 170, 20, DARKGRAY);
            // Draw a disabled placeholder slider
            Rectangle sliderBounds = { leftPanel.x + 20, leftPanel.y + 415, 310, 15 };
            DrawRectangleRec(sliderBounds, LIGHTGRAY); 
        }

        // PLAYER CONTROLS 
        float cY = leftPanel.y + 440; 
        float cX = leftPanel.x + 60; 


        // --- SCOPED STYLE: 2. Set Shared Style for Prev, Play, Next (Light/Add Song Style) ---
        // These styles were reset by the Favorite button logic above. Re-set them here.
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xfdf0f0ff); // Light background
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0x000000ff); // Black icons

        // PREVIOUS BUTTON (Uses Light style)
        if (GuiButton((Rectangle){ cX + 25, cY, 40, 40 }, "#129#")) {
            if (playlist.current) { 
                playlist.playPrevious();
                LoadAndPlayMusic(playlist.current, defaultPlaceholder);
                isPlaying = true;
            }
        }
        
        // PLAY / PAUSE BUTTON (Uses Light style)
        const char* playIcon = isPlaying ? "#132#" : "#131#";
        if (GuiButton((Rectangle){ cX + 80, cY, 60, 40 }, playIcon)) {
            if (playlist.current) { 
                if (isMusicLoaded) { 
                    isPlaying = !isPlaying;
                    if (isPlaying) ResumeMusicStream(currentMusic);
                    else PauseMusicStream(currentMusic);
                } else { 
                    // Attempt to load and play if it wasn't loaded before
                    LoadAndPlayMusic(playlist.current, defaultPlaceholder);
                    isPlaying = true;
                }
            } else {
                ShowNotification("No songs in playlist to play!");
            }
        }
        
        // NEXT BUTTON (Uses Light style)
        if (GuiButton((Rectangle){ cX + 155, cY, 40, 40 }, "#134#")) {
            if (playlist.current) { 
                playlist.playNext();
                LoadAndPlayMusic(playlist.current, defaultPlaceholder);
                isPlaying = true;
            }
        } 

        // --- SCOPED STYLE: 3. Shuffle Button (Dark/Play Selected Style) ---
        // Override the Light style just for the Shuffle button
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x4a4a4aff); // Dark Gray
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xffffffff); // White icon (FF)

        // SHUFFLE BUTTON (Uses Dark style)
        if (GuiButton((Rectangle){ cX + 230, cY, 40, 40 }, "#175#")) {
            playlist.shufflePlaylist();
            ShowNotification("Playlist Shuffled!");
        }
        
        // --- FINAL RESET: 4. Reset style back to the application default ---
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xfdf0f0ff);
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0x000000ff);
        
        
        // ============================================================
        // RIGHT PANEL: MANAGER (The rest of the buttons here use the default style)
        // ============================================================
        Rectangle rightPanel = { 430, 100, 530, 500 };
        DrawRectangleRec(rightPanel, PANEL_BG);
        DrawText("Playlist Management", rightPanel.x + 20, rightPanel.y + 20, 24, TEXT_COLOR);

        // Search text label
        DrawText("Search:", rightPanel.x + 20, rightPanel.y + 60, 16, TEXT_COLOR);
        // Search input box
        if (GuiTextBox((Rectangle){ rightPanel.x + 80, rightPanel.y + 55, 430, 30 }, searchInput, 128, editSearch)) editSearch = !editSearch;
        
        // List Labels
        DrawText("Current Playlist", rightPanel.x + 20, rightPanel.y + 95, 16, TEXT_COLOR);
        DrawText("Favorite Songs", rightPanel.x + 275, rightPanel.y + 95, 16, TEXT_COLOR);

        // List Views
        string listStr = playlist.getListString(searchInput, filteredIndices);
        GuiListView((Rectangle){ rightPanel.x + 20, rightPanel.y + 115, 235, 280 }, listStr.c_str(), &scrollIndex, &activeIndex);
        string favStr = playlist.getFavoritesString();
        GuiListView((Rectangle){ rightPanel.x + 275, rightPanel.y + 115, 235, 280 }, favStr.c_str(), &favScroll, &favActive);

        // ACTION BUTTONS (These use the default style: 0xfdf0f0ff)
        float bY = rightPanel.y + 440; 
        
        if (GuiButton((Rectangle){ rightPanel.x + 20, bY, 100, 30 }, "Add Song")) { 
            showAddWindow = true; 
            isBatchMode = false; 
            // Reset fields for fresh entry
            titleInput[0] = '\0'; artistInput[0] = '\0'; durInput[0] = '\0'; 
            pathInput[0] = '\0'; albumArtPathInput[0] = '\0'; 
        }
        if (GuiButton((Rectangle){ rightPanel.x + 130, bY, 100, 30 }, "Batch Add")) { 
            showAddWindow = true; 
            isBatchMode = true; 
            // Reset fields for fresh entry
            titleInput[0] = '\0'; artistInput[0] = '\0'; durInput[0] = '\0'; 
            pathInput[0] = '\0'; albumArtPathInput[0] = '\0'; 
        }

        // DELETE SELECTED 
        if (GuiButton((Rectangle){ rightPanel.x + 240, bY, 100, 30 }, "Delete")) {
            if (activeIndex > -1 && activeIndex < filteredIndices.size()) {
                
                // Determine if the song being deleted is the current playing song
                bool wasCurrent = (playlist.current && filteredIndices[activeIndex] == playlist.GetListIndex(playlist.current)); 
                
                bool listEmpty = playlist.deleteNodeAtIndex(filteredIndices[activeIndex]);
                activeIndex = -1;

                if (listEmpty) {
                    UnloadCurrentMusic();
                    isPlaying = false; 
                    LoadAndUpdateAlbumArt(nullptr, defaultPlaceholder); 
                    ShowNotification("Playlist is empty. Playback stopped.");
                } else if (wasCurrent) {
                    // Current song deleted, playlist.current has already advanced to the next one
                    LoadAndPlayMusic(playlist.current, defaultPlaceholder); 
                    isPlaying = true;
                    ShowNotification("Song Deleted. Playing next track.");
                } else {
                    ShowNotification("Song Deleted.");
                }
            }
        }
        
        // PLAY SELECTED SONG (Uses scoped dark style)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x4a4a4aff);
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xffffffff);
        if (GuiButton((Rectangle){ rightPanel.x + 350, bY, 160, 30 }, "Play Selected")) {
            if (activeIndex > -1 && filteredIndices.size() > 0) { 
                playlist.playByIndex(filteredIndices[activeIndex]);
                LoadAndPlayMusic(playlist.current, defaultPlaceholder);
                isPlaying = true;
            } else {
                ShowNotification("Please add and select a song!");
            }
        }
        // Reset GUI button styles
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xfdf0f0ff);
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0x000000ff);

        if (GuiButton((Rectangle){ screenWidth - 100, 20, 80, 30 }, "EXIT")) showExitConfirm = true;

        // --- MODALS and NOTIFICATIONS ---
        if (notificationTimer > 0) {
            DrawRectangle(screenWidth/2 - 150, 60, 300, 40, Fade(DARKGREEN, 0.8f));
            DrawText(notificationMsg.c_str(), screenWidth/2 - MeasureText(notificationMsg.c_str(), 20)/2, 70, 20, WHITE);
            notificationTimer--;
        }

        // --- ADD SONG MODAL ---
        if (showAddWindow) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));
            Rectangle box = { screenWidth/2 - 200, screenHeight/2 - 220, 400, 440 }; 
            DrawRectangleRec(box, RAYWHITE);

            DrawText(isBatchMode ? "Batch Add Mode" : "Add New Song", box.x + 20, box.y + 15, 20, BLACK);

            GuiLabel((Rectangle){box.x + 50, box.y + 50, 100, 20}, "Title:");
            if (GuiTextBox((Rectangle){box.x + 50, box.y + 70, 300, 30}, titleInput, 128, editTitle)) editTitle = !editTitle;

            GuiLabel((Rectangle){box.x + 50, box.y + 110, 100, 20}, "Artist:");
            if (GuiTextBox((Rectangle){box.x + 50, box.y + 130, 300, 30}, artistInput, 128, editArtist)) editArtist = !editArtist;

            GuiLabel((Rectangle){box.x + 50, box.y + 170, 100, 20}, "Duration (Sec):");
            if (GuiTextBox((Rectangle){box.x + 50, box.y + 190, 300, 30}, durInput, 128, editDur)) editDur = !editDur;
            
            GuiLabel((Rectangle){box.x + 50, box.y + 230, 100, 20}, "Audio File Path:");
            if (GuiTextBox((Rectangle){box.x + 50, box.y + 250, 300, 30}, pathInput, 128, editPath)) editPath = !editPath;

            GuiLabel((Rectangle){box.x + 50, box.y + 300, 100, 20}, "Album Art Path (Optional):");
            if (GuiTextBox((Rectangle){box.x + 50, box.y + 320, 300, 30}, albumArtPathInput, 128, editAlbumArtPath)) editAlbumArtPath = !editAlbumArtPath;
            DrawText("Leave blank for 'default_cover.jpg'", box.x + 55, box.y + 355, 14, GRAY);

            if (GuiButton((Rectangle){box.x + 50, box.y + 380, 140, 30}, isBatchMode ? "Add & Next" : "Save")) {
                if (string(titleInput) != "" && string(pathInput) != "") {
                    float d = 0; try { d = stof(durInput); } catch(...) {}
                    
                    bool wasEmpty = playlist.isEmpty();
                    playlist.addSong(titleInput, artistInput, d, pathInput, albumArtPathInput);
                    
                    // Load the new song immediately if it's the first one
                    if (wasEmpty && playlist.current) {
                        LoadAndPlayMusic(playlist.current, defaultPlaceholder);
                        isPlaying = true; 
                    }

                    ShowNotification("Song Added!");
                    // Clear inputs for next song or next run
                    titleInput[0] = '\0'; artistInput[0] = '\0'; durInput[0] = '\0'; 
                    pathInput[0] = '\0'; albumArtPathInput[0] = '\0'; 
                    if (!isBatchMode) showAddWindow = false;
                } else {
                    ShowNotification("Title and Audio Path Required!");
                }
            }
            if (GuiButton((Rectangle){box.x + 210, box.y + 380, 140, 30}, isBatchMode ? "Done" : "Cancel")) showAddWindow = false;
        }

        // --- EXIT CONFIRMATION MODAL ---
        if (showExitConfirm) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));
            int choice = GuiMessageBox((Rectangle){ screenWidth/2 - 125, screenHeight/2 - 50, 250, 100 },
                "Confirm Exit", "Are you sure you want to exit?", "Yes;No");
            if (choice == 1) break; // User selected "Yes"
            if (choice == 2 || choice == 0) showExitConfirm = false; // User selected "No" or closed the box
        }

        EndDrawing();
    }
    
    // Final Cleanup (Ensures no memory leaks from textures/audio)
    UnloadCurrentMusic();
    if (currentAlbumArt.id != 0 && currentAlbumArt.id != defaultPlaceholder.id) UnloadTexture(currentAlbumArt); 
    UnloadTexture(defaultPlaceholder); 
    CloseAudioDevice();
    CloseWindow();
    return 0;
}