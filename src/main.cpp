#include <raylib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <zip.h>

struct ComicPage {
    Texture2D texture;
    std::string filename;
};

std::vector<ComicPage> pages;
int currentPage = 0;
bool helpVisible = true;
bool mangaMode = false;
bool doublePage = false;
float zoom = 1.0f;
float rotation = 0.0f;
Vector2 offset = {0.0f, 0.0f};

void LoadComic(const char* filepath);
void UnloadComic();
void DrawHelp();

int main() {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Comic Reader");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        if (IsKeyPressed(KEY_H)) helpVisible = !helpVisible;

        int pageIncrement = doublePage ? 2 : 1;
        bool nextPagePressed = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_N);
        bool prevPagePressed = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_P);

        if (mangaMode) {
            if (nextPagePressed) currentPage -= pageIncrement;
            if (prevPagePressed) currentPage += pageIncrement;
        } else {
            if (nextPagePressed) currentPage += pageIncrement;
            if (prevPagePressed) currentPage -= pageIncrement;
        }

        if (IsKeyPressed(KEY_EQUAL)) zoom += 0.1f;
        if (IsKeyPressed(KEY_MINUS)) zoom -= 0.1f;
        if (IsKeyPressed(KEY_M)) mangaMode = !mangaMode;
        if (IsKeyPressed(KEY_D)) doublePage = !doublePage;
        if (IsKeyPressed(KEY_F)) {
            if (!pages.empty()) {
                float screenW = GetScreenWidth();
                float screenH = GetScreenHeight();
                float combinedW = 0;
                float maxH = 0;

                if (doublePage && currentPage + 1 < pages.size()) {
                    combinedW = pages[currentPage].texture.width + pages[currentPage+1].texture.width;
                    maxH = std::max(pages[currentPage].texture.height, pages[currentPage+1].texture.height);
                } else {
                    combinedW = pages[currentPage].texture.width;
                    maxH = pages[currentPage].texture.height;
                }
                
                zoom = std::min(screenW / combinedW, screenH / maxH);
            } else {
                zoom = 1.0f;
            }
            offset = {0.0f, 0.0f};
            rotation = 0.0f;
        }
        
        // if (IsKeyPressed(KEY_O)) {
        //     // File dialog logic would go here.
        //     // For now, we'll just load a test file.
        //     LoadComic("test.cbz");
        // }
        
        if (IsFileDropped()) {
            std::cout << "Detected file drop" << std::endl;
            FilePathList files = LoadDroppedFiles();
            if (files.count == 1) {
                LoadComic(files.paths[0]);
            }
            UnloadDroppedFiles(files);
        }

        if (IsKeyPressed(KEY_R)) rotation += 90.0f;
        if (IsKeyPressed(KEY_Q)) break;

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            offset.x += GetMouseDelta().x;
            offset.y += GetMouseDelta().y;
        }

        currentPage = std::max(0, std::min((int)pages.size() - 1, currentPage));
        zoom = std::max(0.1f, zoom);

        // Draw
        BeginDrawing();
        ClearBackground(BLACK);

        if (!pages.empty()) {
            if (doublePage && currentPage + 1 < pages.size()) {
                int page1_idx = mangaMode ? currentPage + 1 : currentPage;
                int page2_idx = mangaMode ? currentPage : currentPage + 1;

                Texture2D tex1 = pages[page1_idx].texture;
                Texture2D tex2 = pages[page2_idx].texture;

                float scaled_w1 = tex1.width * zoom;
                float scaled_h1 = tex1.height * zoom;
                float scaled_w2 = tex2.width * zoom;
                float scaled_h2 = tex2.height * zoom;

                float total_w = scaled_w1 + scaled_w2;
                
                float start_x = (GetScreenWidth() - total_w) / 2.0f;
                
                Vector2 origin = { 0, 0 };

                Rectangle dest1 = { start_x + offset.x, (GetScreenHeight() - scaled_h1) / 2.0f + offset.y, scaled_w1, scaled_h1 };
                DrawTexturePro(tex1, {0, 0, (float)tex1.width, (float)tex1.height}, dest1, origin, rotation, WHITE);

                Rectangle dest2 = { start_x + scaled_w1 + offset.x, (GetScreenHeight() - scaled_h2) / 2.0f + offset.y, scaled_w2, scaled_h2 };
                DrawTexturePro(tex2, {0, 0, (float)tex2.width, (float)tex2.height}, dest2, origin, rotation, WHITE);

            } else {
                Texture2D tex = pages[currentPage].texture;
                float scaled_w = tex.width * zoom;
                float scaled_h = tex.height * zoom;
                Vector2 origin = { scaled_w / 2, scaled_h / 2 };
                Rectangle dest = { GetScreenWidth() / 2.0f + offset.x, GetScreenHeight() / 2.0f + offset.y, scaled_w, scaled_h };
                DrawTexturePro(tex, {0, 0, (float)tex.width, (float)tex.height}, dest, origin, rotation, WHITE);
            }
        } else {
            DrawText("Drop a file to start reading", GetScreenWidth()/2.0, GetScreenHeight()/2.0, 20, WHITE);
        }

        if (helpVisible) {
            DrawHelp();
        }

        EndDrawing();
    }

    UnloadComic();
    CloseWindow();

    return 0;
}

void LoadComic(const char* filepath) {
    UnloadComic();
    zip_t* za = zip_open(filepath, 0, NULL);
    if (!za) {
        std::cerr << "Failed to open zip archive: " << filepath << std::endl;
        return;
    }

    int numEntries = zip_get_num_entries(za, 0);
    for (int i = 0; i < numEntries; i++) {
        zip_stat_t zs;
        zip_stat_index(za, i, 0, &zs);

        std::string name = zs.name;
        if (name.find(".jpg") != std::string::npos || name.find(".png") != std::string::npos) {
            zip_file_t* zf = zip_fopen_index(za, i, 0);
            if (!zf) continue;

            std::vector<unsigned char> buffer(zs.size);
            zip_fread(zf, buffer.data(), zs.size);
            zip_fclose(zf);

            Image image = LoadImageFromMemory(GetFileExtension(name.c_str()), buffer.data(), buffer.size());
            pages.push_back({LoadTextureFromImage(image), name});
            UnloadImage(image);
        }
    }
    zip_close(za);

    std::sort(pages.begin(), pages.end(), [](const ComicPage& a, const ComicPage& b) {
        return a.filename < b.filename;
    });
}

void UnloadComic() {
    for (auto& page : pages) {
        UnloadTexture(page.texture);
    }
    pages.clear();
}

void DrawHelp() {
    DrawRectangle(10, 10, 250, 230, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(10, 10, 250, 230, BLUE);
    DrawText("Controls:", 20, 20, 10, BLACK);
    DrawText("H: Toggle Help", 20, 40, 10, BLACK);
    DrawText("N/Right: Next Page", 20, 60, 10, BLACK);
    DrawText("P/Left: Prev Page", 20, 80, 10, BLACK);
    DrawText("=: Zoom In", 20, 100, 10, BLACK);
    DrawText("-: Zoom Out", 20, 120, 10, BLACK);
    DrawText("M: Manga Mode", 20, 140, 10, BLACK);
    DrawText("D: Double Page", 20, 160, 10, BLACK);
    DrawText("F: Fit to Screen", 20, 180, 10, BLACK);
    // DrawText("O: Open File", 20, 200, 10, BLACK);
    DrawText("R: Rotate", 20, 200, 10, BLACK);
    DrawText("Q: Quit", 20, 220, 10, BLACK);
}
