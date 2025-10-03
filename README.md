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
| `O`         | Open File (Placeholder)|
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

Run the executable from the `build` directory:

```bash
./comic-reader
```

Place a `.cbz` file named `test.cbz` in the `build` directory or use the `O` key to load a file (note: file dialog is not yet implemented).
