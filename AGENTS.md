# Agent Instructions for Comic Reader

This document provides guidelines for AI agents working on this C++ codebase.

## Build, Lint, and Test

- **Build:** Use CMake. From the root directory:
  ```bash
  cmake -S . -B build
  cmake --build build
  ```
- **Run:**
  ```bash
  ./build/comic-reader
  ```
- **Linting:** No linter is configured. Adhere to the style guide below. The compiler uses `-Wall -Wextra -pedantic`.
- **Testing:** No test suite exists. Please add tests for new functionality.

## Code Style Guidelines

- **Formatting:** Use 4-space indentation. Place opening braces on the same line as the function or control statement.
- **Naming:**
  - `Structs` and `functions` use `PascalCase` (e.g., `ComicPage`, `LoadComic`).
  - `variables` use `camelCase` (e.g., `currentPage`).
- **Types:** Use standard C++ types.
- **Error Handling:** Report errors to `std::cerr`.
- **Dependencies:** The project uses `raylib` and `libzip`.
