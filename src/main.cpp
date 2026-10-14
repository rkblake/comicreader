#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <memory>
#include <raylib.h>
#include <string>
#include <vector>
#include <zip.h>

namespace {

std::string ToLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

bool IsImageFile(const std::string &name) {
    std::string lowered = ToLower(name);
    return lowered.ends_with(".jpg") || lowered.ends_with(".jpeg") || lowered.ends_with(".png") ||
           lowered.ends_with(".webp");
}

// Case-insensitive, digit-aware comparison so "page2" sorts before "page10".
bool NaturalLess(const std::string &a, const std::string &b) {
    size_t i = 0;
    size_t j = 0;
    while (i < a.size() && j < b.size()) {
        const bool digitA = std::isdigit(static_cast<unsigned char>(a[i])) != 0;
        const bool digitB = std::isdigit(static_cast<unsigned char>(b[j])) != 0;
        if (digitA && digitB) {
            const size_t startA = i;
            const size_t startB = j;
            while (i < a.size() && std::isdigit(static_cast<unsigned char>(a[i]))) { i++; }
            while (j < b.size() && std::isdigit(static_cast<unsigned char>(b[j]))) { j++; }

            std::string numberA = a.substr(startA, i - startA);
            std::string numberB = b.substr(startB, j - startB);
            const size_t firstA = numberA.find_first_not_of('0');
            const size_t firstB = numberB.find_first_not_of('0');
            numberA = (firstA == std::string::npos) ? "0" : numberA.substr(firstA);
            numberB = (firstB == std::string::npos) ? "0" : numberB.substr(firstB);

            if (numberA.size() != numberB.size()) { return numberA.size() < numberB.size(); }
            if (numberA != numberB) { return numberA < numberB; }
        } else {
            const char lowerA = static_cast<char>(std::tolower(static_cast<unsigned char>(a[i])));
            const char lowerB = static_cast<char>(std::tolower(static_cast<unsigned char>(b[j])));
            if (lowerA != lowerB) { return lowerA < lowerB; }
            i++;
            j++;
        }
    }
    return (i == a.size()) && (j != b.size());
}

} // namespace

struct ComicPage {
    Texture2D texture;
    std::string filename;
    bool isSpread = false;
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
        currentPage = 0;
        offset = {0.0f, 0.0f};

        int err = 0;
        std::unique_ptr<zip_t, decltype(&zip_close)> za(zip_open(filepath, 0, &err), &zip_close);
        if (!za) {
            zip_error_t zerr;
            zip_error_init_with_code(&zerr, err);
            std::cerr << "Failed to open zip archive: " << zip_error_strerror(&zerr) << std::endl;
            zip_error_fini(&zerr);
            return;
        }

        int numEntries = zip_get_num_entries(za.get(), 0);
        for (int i = 0; i < numEntries; i++) {
            zip_stat_t zs;
            if (zip_stat_index(za.get(), i, 0, &zs) == -1) {
                std::cerr << "Failed to stat zip archive entry " << i << "." << std::endl;
                UnloadComic();
                return;
            }

            std::string name = zs.name;
            if (!IsImageFile(name)) { continue; }

            zip_file_t *zf = zip_fopen_index(za.get(), i, 0);
            if (!zf) { continue; }

            std::vector<unsigned char> buffer(zs.size);
            zip_int64_t bytesRead = zip_fread(zf, buffer.data(), zs.size);
            zip_fclose(zf);
            if (bytesRead < 0 || static_cast<zip_uint64_t>(bytesRead) != zs.size) {
                std::cerr << "Failed to read image entry: " << name << std::endl;
                continue;
            }
            if (buffer.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
                std::cerr << "Image entry too large: " << name << std::endl;
                continue;
            }

            std::string extension = ToLower(GetFileExtension(name.c_str()));
            Image image = LoadImageFromMemory(extension.c_str(), buffer.data(),
                                              static_cast<int>(buffer.size()));
            if (image.data == nullptr) {
                std::cerr << "Failed to decode image: " << name << std::endl;
                continue;
            }

            bool isSpread = image.width > image.height;
            Texture2D texture = LoadTextureFromImage(image);
            UnloadImage(image);
            if (texture.id == 0) {
                std::cerr << "Failed to upload texture: " << name << std::endl;
                continue;
            }

            pages.push_back({texture, name, isSpread});
        }

        std::sort(pages.begin(), pages.end(), [](const ComicPage &a, const ComicPage &b) {
            return NaturalLess(a.filename, b.filename);
        });

        currentPage = 0;
        offset = {0.0f, 0.0f};
        helpVisible = false;
    }

    void UnloadComic() {
        for (auto &page : pages) { UnloadTexture(page.texture); }
        pages.clear();
    }

    bool Update() {
        if (IsKeyPressed(KEY_H)) { helpVisible = !helpVisible; }

        // Clamp before any indexing into pages so stale state can never go out of bounds.
        if (!pages.empty()) {
            currentPage = std::clamp(currentPage, size_t{0}, pages.size() - 1);
        } else {
            currentPage = 0;
        }

        size_t pageIncrement =
            (!pages.empty() && doublePage && !pages[currentPage].isSpread) ? 2 : 1;
        bool nextPagePressed = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_N);
        bool prevPagePressed = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_P);

        if (!pages.empty()) {
            if (nextPagePressed) {
                currentPage += pageIncrement;
                offset = {0.0f, 0.0f};
            }
            if (prevPagePressed && currentPage >= pageIncrement) {
                currentPage -= pageIncrement;
                offset = {0.0f, 0.0f};
            }
        }

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
                float combinedW = 0.0f;
                float maxH = 0.0f;

                if (doublePage && !pages[currentPage].isSpread && currentPage + 1 < pages.size()) {
                    combinedW =
                        pages[currentPage].texture.width + pages[currentPage + 1].texture.width;
                    maxH = pages[currentPage].texture.height > pages[currentPage + 1].texture.height
                               ? pages[currentPage].texture.height
                               : pages[currentPage + 1].texture.height;
                } else {
                    combinedW = pages[currentPage].texture.width;
                    maxH = pages[currentPage].texture.height;
                }

                if (combinedW > 0.0f && maxH > 0.0f) {
                    zoom = std::min(screenW / combinedW, screenH / maxH);
                }
            } else {
                zoom = 1.0f;
            }
            offset = {0.0f, 0.0f};
            rotation = 0.0f;
        }

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

        if (!pages.empty()) { currentPage = std::clamp(currentPage, size_t{0}, pages.size() - 1); }
        zoom = std::clamp(zoom, 0.1f, 10.0f);

        // Draw
        BeginDrawing();
        ClearBackground(BLACK);

        if (!pages.empty()) {
            if (doublePage && !pages[currentPage].isSpread && currentPage + 1 < pages.size()) {
                size_t page1_idx = mangaMode ? currentPage + 1 : currentPage;
                size_t page2_idx = mangaMode ? currentPage : currentPage + 1;

                Texture2D tex1 = pages[page1_idx].texture;
                Texture2D tex2 = pages[page2_idx].texture;

                float scaled_w1 = tex1.width * zoom;
                float scaled_h1 = tex1.height * zoom;
                float scaled_w2 = tex2.width * zoom;
                float scaled_h2 = tex2.height * zoom;

                float total_w = scaled_w1 + scaled_w2;
                float max_scaled_h = scaled_h1 > scaled_h2 ? scaled_h1 : scaled_h2;

                float start_x = (GetScreenWidth() - total_w) / 2.0f;
                float top_y = (GetScreenHeight() - max_scaled_h) / 2.0f + offset.y;

                Rectangle dest1 = {start_x + scaled_w1 / 2.0f + offset.x, top_y + scaled_h1 / 2.0f,
                                   scaled_w1, scaled_h1};
                DrawTexturePro(tex1, {0, 0, (float)tex1.width, (float)tex1.height}, dest1,
                               {scaled_w1 / 2, scaled_h1 / 2}, rotation, WHITE);

                Rectangle dest2 = {start_x + scaled_w1 + scaled_w2 / 2.0f + offset.x,
                                   top_y + scaled_h2 / 2.0f, scaled_w2, scaled_h2};
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

    static void DrawHelp() {
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

    void SetNotification(const std::string &newNotification) {
        notification = newNotification;
        notificationVisible = true;
        notificationDuration = 2.0f;
    }

    void DrawNotification() {
        const int fontSize = 10;
        const int y = GetScreenHeight() - 55;
        const int boxWidth = MeasureText(notification.c_str(), fontSize) + 20;
        DrawRectangle(10, y, boxWidth, 35, Fade(SKYBLUE, 0.9f));
        DrawRectangleLines(10, y, boxWidth, 35, BLUE);
        DrawText(notification.c_str(), 20, y + 13, fontSize, BLACK);
    }
};

int main() {
    ComicReader reader;

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Comic Reader");

    // Size the window to ~90% of the current monitor when that information is available.
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);
    if (monitorWidth > 0 && monitorHeight > 0) {
        SetWindowSize(static_cast<int>(monitorWidth * 0.9f),
                      static_cast<int>(monitorHeight * 0.9f));
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (reader.Update()) { break; }
    }

    reader.UnloadComic();
    CloseWindow();

    return 0;
}
