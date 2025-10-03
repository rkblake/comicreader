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
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Comic Reader");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_ERROR);

    while (!WindowShouldClose()) {
        // Update
        if (IsKeyPressed(KEY_H)) helpVisible = !helpVisible;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_N)) currentPage++;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_P)) currentPage--;
        if (IsKeyPressed(KEY_EQUAL)) zoom += 0.1f;
        if (IsKeyPressed(KEY_MINUS)) zoom -= 0.1f;
        if (IsKeyPressed(KEY_M)) mangaMode = !mangaMode;
        if (IsKeyPressed(KEY_D)) doublePage = !doublePage;
        if (IsKeyPressed(KEY_F)) {
            zoom = 1.0f;
            offset = {0.0f, 0.0f};
            rotation = 0.0f;
        }
        if (IsKeyPressed(KEY_O)) {
            // File dialog logic would go here.
            // For now, we'll just load a test file.
            LoadComic("test.cbz");
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
            Rectangle dest = {offset.x, offset.y, pages[currentPage].texture.width * zoom, pages[currentPage].texture.height * zoom};
            DrawTexturePro(pages[currentPage].texture, {0, 0, (float)pages[currentPage].texture.width, (float)pages[currentPage].texture.height}, dest, {0,0}, rotation, WHITE);
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
    DrawText("O: Open File", 20, 200, 10, BLACK);
    DrawText("R: Rotate", 20, 220, 10, BLACK);
    DrawText("Q: Quit", 20, 240, 10, BLACK);
}
