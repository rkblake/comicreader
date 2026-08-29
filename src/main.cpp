#include <algorithm>
#include <iostream>
#include <raylib.h>
#include <string>
#include <vector>
#include <zip.h>

struct ComicPage {
    Texture2D texture;
    std::string filename;
    bool isSpread;
};

struct ComicReader {
    std::vector<ComicPage> pages;
    size_t currentPage = 0;
    float zoom = 1.0f;
    float rotation = 0.0f;
    Vector2 offset = {0.0f, 0.0f};
    float notificationDuration = 0.0f;
    std::string notification;
    bool notificationVisible = false;
    bool helpVisible = true;
    bool mangaMode = false;
    bool doublePage = false;

    void LoadComic(const char *filepath) {
        UnloadComic();
        int err = 0;
        zip_t *za = zip_open(filepath, 0, &err);
        if (!za) {
            zip_error_t zerr;
            zip_error_init_with_code(&zerr, err);
            std::cerr << "Failed to open zip archive: " << zip_error_strerror(&zerr) << std::endl;
            zip_error_fini(&zerr);
            return;
        }

        int numEntries = zip_get_num_entries(za, 0);
        for (int i = 0; i < numEntries; i++) {
            zip_stat_t zs;
            if (zip_stat_index(za, i, 0, &zs) == -1) {
                std::cerr << "Failed to stat zip archive." << std::endl;
                return;
            }

            std::string name = zs.name;
            if (name.ends_with(".jpeg") || name.ends_with(".jpg") || name.ends_with(".png") ||
                name.ends_with(".webp")) {
                zip_file_t *zf = zip_fopen_index(za, i, 0);
                if (!zf) { continue; }

                std::vector<unsigned char> buffer(zs.size);
                zip_fread(zf, buffer.data(), zs.size);
                zip_fclose(zf);

                Image image = LoadImageFromMemory(GetFileExtension(name.c_str()), buffer.data(),
                                                  buffer.size());
                bool isSpread = image.width > image.height ? true : false;
                pages.push_back({LoadTextureFromImage(image), name, isSpread});
                UnloadImage(image);
            }
        }
        zip_close(za);

        std::sort(pages.begin(), pages.end(),
                  [](const ComicPage a, const ComicPage b) { return a.filename < b.filename; });

        currentPage = 0;
        helpVisible = false;
    }

    void UnloadComic() {
        for (auto &page : pages) { UnloadTexture(page.texture); }
        pages.clear();
    }

    bool Update() {
        if (IsKeyPressed(KEY_H)) { helpVisible = !helpVisible; }

        size_t pageIncrement =
            (!pages.empty() && doublePage && !pages[currentPage].isSpread) ? 2 : 1;
        bool nextPagePressed = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_N);
        bool prevPagePressed = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_P);

        // TODO: is right button being next page for both rtl and ltr better UX?
        // if (mangaMode) {
        // if (nextPagePressed && currentPage >= pageIncrement) currentPage -= pageIncrement;
        // if (prevPagePressed) currentPage += pageIncrement;
        // } else {
        if (!pages.empty()) {
            if (nextPagePressed) { currentPage += pageIncrement; }
            if (prevPagePressed && currentPage >= pageIncrement) { currentPage -= pageIncrement; }
        }

        // }

        if (IsKeyPressed(KEY_EQUAL)) { zoom += 0.1f; }
        if (IsKeyPressed(KEY_MINUS)) { zoom -= 0.1f; }
        if (IsKeyPressed(KEY_M)) {
            mangaMode = !mangaMode;
            SetNotification(mangaMode ? "Enabled Manga Mode" : "Disabled Manga Mode");
        }
        if (IsKeyPressed(KEY_D)) {
            doublePage = !doublePage;
            SetNotification(doublePage ? "Enabled Double Page Mode" : "Disabled Double Page Mode");
        }
        if (IsKeyPressed(KEY_F)) {
            if (!pages.empty()) {
                float screenW = GetScreenWidth();
                float screenH = GetScreenHeight();
                float combinedW = 0;
                float maxH = 0;

                if (doublePage && !pages[currentPage].isSpread && currentPage + 1 < pages.size()) {
                    combinedW =
                        pages[currentPage].texture.width + pages[currentPage + 1].texture.width;
                    maxH = std::max(pages[currentPage].texture.height,
                                    pages[currentPage + 1].texture.height);
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
        //     LoadComic("test.cbz");
        // }

        // TODO: handle multiple files/folders dropped
        if (IsFileDropped()) {
            FilePathList files = LoadDroppedFiles();
            if (files.count == 1) { LoadComic(files.paths[0]); }
            UnloadDroppedFiles(files);
        }

        if (IsKeyPressed(KEY_R)) { rotation += 90.0f; }
        if (IsKeyPressed(KEY_Q)) { return true; }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            offset.x += GetMouseDelta().x;
            offset.y += GetMouseDelta().y;
        }

        currentPage = std::max(0UL, std::min(pages.size() - 1, currentPage));
        zoom = std::clamp(zoom, 0.1f, 10.0f);

        // Draw
        BeginDrawing();
        ClearBackground(BLACK);

        if (!pages.empty()) {
            if (doublePage && !pages[currentPage].isSpread && currentPage + 1 < pages.size()) {
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

                Rectangle dest1 = {start_x + offset.x + scaled_w1 / 2.0f,
                                   GetScreenHeight() / 2.0f + offset.y, scaled_w1, scaled_h1};
                DrawTexturePro(tex1, {0, 0, (float)tex1.width, (float)tex1.height}, dest1,
                               {scaled_w1 / 2, scaled_h1 / 2}, rotation, WHITE);

                Rectangle dest2 = {start_x + scaled_w2 * 1.5f + offset.x,
                                   GetScreenHeight() / 2.0f + offset.y, scaled_w2, scaled_h2};
                DrawTexturePro(tex2, {0, 0, (float)tex2.width, (float)tex2.height}, dest2,
                               {scaled_w2 / 2, scaled_h2 / 2}, rotation, WHITE);

            } else {
                Texture2D tex = pages[currentPage].texture;
                float scaled_w = tex.width * zoom;
                float scaled_h = tex.height * zoom;
                Vector2 origin = {scaled_w / 2, scaled_h / 2};
                Rectangle dest = {GetScreenWidth() / 2.0f + offset.x,
                                  GetScreenHeight() / 2.0f + offset.y, scaled_w, scaled_h};
                DrawTexturePro(tex, {0, 0, (float)tex.width, (float)tex.height}, dest, origin,
                               rotation, WHITE);
            }
        } else {
            DrawText("Drop a file to start reading", GetScreenWidth() / 2, GetScreenHeight() / 2,
                     20, WHITE);
        }

        if (helpVisible) { DrawHelp(); }

        if (notificationVisible) {
            DrawNotification();
            notificationDuration -= GetFrameTime();
            if (notificationDuration <= 0) { notificationVisible = false; }
        }

        EndDrawing();

        return false;
    }

    void DrawHelp() {
        DrawRectangle(10, 10, 250, 230, Fade(SKYBLUE, 0.9f));
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

    void SetNotification(const std::string notification) {
        this->notification = notification;
        notificationVisible = true;
        notificationDuration = 2.0f;
    }

    void DrawNotification() {
        DrawRectangle(10, GetScreenHeight() - 55, 250, 35, Fade(SKYBLUE, 0.9f));
        DrawRectangleLines(10, GetScreenHeight() - 55, 250, 35, BLUE);
        DrawText(notification.c_str(), 20, GetScreenHeight() - 42, 10, BLACK);
    }
};

int main() {
    ComicReader reader;

    // const int width = GetMonitorWidth(0);
    // const int height = GetMonitorHeight(0);

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1920, 1080, "Comic Reader");
    SetTargetFPS(30);

    while (!WindowShouldClose()) {
        if (reader.Update()) { break; }
    }

    reader.UnloadComic();
    CloseWindow();

    return 0;
}
