# Comic Reader

A simple comic book reader for `.cbz` files, built with C++, raylib, and libzip.

## Features

-   Load and display `.cbz` comic archives.
-   Page navigation (next/previous).
-   Zoom in and out.
-   Pan the view by clicking and dragging.
-   Rotate pages.
-   **Manga Mode:** Right-to-left page navigation.
-   **Double-Page Mode:** View two pages side-by-side.
-   **Fit to Screen:** Automatically adjust the zoom to fit the page(s) to the window.
-   Toggleable help overlay.

## Keyboard Shortcuts

| Key         | Action                |
|-------------|-----------------------|
| `H`         | Toggle Help Overlay   |
| `N`, `Right`| Next Page             |
| `P`, `Left` | Previous Page         |
| `M`         | Toggle Manga Mode     |
| `D`         | Toggle Double-Page Mode|
| `F`         | Fit to Screen         |
| `O`         | Open File (Not Implemented)|
| `R`         | Rotate Page           |
| `+`, `=`    | Zoom In               |
| `-`         | Zoom Out              |
| `Q`         | Quit                  |

## Dependencies

-   [raylib](https://www.raylib.com/)
-   [libzip](https://libzip.org/)
-   [CMake](https://cmake.org/)

## Building

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd comic-reader
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake and build the project:**
    ```bash
    cmake ..
    make
    ```

## Usage

Run the executable from the repository root:

```bash
./build/comic-reader
```

Dropping a `.cbz` file onto the window loads it. Drag with the left mouse button to pan.

## Releases

CI publishes prebuilt binaries for Linux, macOS, and Windows.

-   **Linux:** a plain binary and an **AppImage** (`comic-reader-linux.AppImage`) are published.
    raylib and libzip are built statically, so the binary only needs the usual system libraries
    (X11/OpenGL, ALSA, and zlib). To run the AppImage:
    `chmod +x comic-reader-linux.AppImage && ./comic-reader-linux.AppImage`.
-   **macOS / Windows:** built against the platform package manager's raylib and libzip, so those
    libraries must be available at runtime (`brew install raylib libzip`, or vcpkg's
    `x64-windows` packages).
-   **macOS:** the binary is not code-signed, so Gatekeeper may block it. Remove the quarantine
    attribute with `xattr -dr com.apple.quarantine ./comic-reader-macos` or right-click and choose
    *Open*.

