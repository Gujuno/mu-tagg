# mu-tagg

A modern Qt-based audio tag editor with drag-and-drop support, inspired by MP3tag. This application provides an intuitive interface for editing metadata tags in audio files with support for multiple formats and advanced features.
Inspired by MP3tag. Created using Cursor AI Code Editor.  

This project is not affiliated with, associated with, or endorsed by Mp3tag or its creators. All trademarks and copyrights belong to their respective owners. This is an independent project developed for educational and personal use.

## Features

### Core Functionality
- **Drag and Drop Support**: Drop MP3/FLAC files or entire folders directly into the application window
- **Multi-Format Support**: Edit tags in MP3 (ID3v2) and FLAC (Vorbis Comments) files
- **Batch Editing**: Select multiple files and edit tags simultaneously
- **Real-time Preview**: See changes immediately in the file list before saving

### Tag Fields Supported
- **Basic Metadata**: Title, Artist, Album, Year, Track Number, Genre
- **Extended Metadata**: Album Artist, Composer, Disk Number, Comments
- **Cover Art**: Add, remove, and view embedded album artwork
- **Mixed Selection Handling**: Intelligent handling of files with different tag values

### User Interface
- **Split-panel Layout**: Tag editor panel on the left, file list on the right
- **Resizable Interface**: Adjustable splitter with automatic size persistence
- **Customizable Table**: Reorderable and resizable columns with state persistence
- **Keyboard Shortcuts**: 
  - `Ctrl+S` - Save changes
  - `Delete` - Remove selected files from list
  - `Ctrl+O` - Open files dialog
- **Context Menus**: Right-click on cover art to add/remove images

### File Management
- **Menu Integration**: File menu with "Open Files" and "Open Directory" options
- **Recursive Directory Scanning**: Automatically finds all audio files in subdirectories
- **File Filtering**: Supports `.mp3` and `.flac` files
- **Non-destructive Editing**: Changes are only applied when explicitly saved

### Configuration & Persistence
- **Automatic Settings**: Window size, position, and layout are automatically saved
- **Table Configuration**: Column order, width, and visibility are preserved
- **Splitter Position**: Panel sizes are remembered between sessions
- **Settings Location**: Configuration stored in `~/.config/mu-tagg/mu-tagg.conf`

## Supported Audio Formats

### MP3 Files
- **Tag Format**: ID3v2
- **Cover Art**: Embedded via APIC frames
- **Supported Fields**: All standard ID3v2 fields plus extended metadata

### FLAC Files
- **Tag Format**: Vorbis Comments (Xiph)
- **Cover Art**: Embedded via FLAC Picture metadata
- **Supported Fields**: All Vorbis Comment fields including custom metadata

## Requirements

### System Dependencies
- **Qt5**: Widgets module (GUI framework)
- **TagLib**: Audio metadata library
- **CMake**: Build system (version 3.10 or higher)
- **C++17**: Compiler with C++17 support

### Installation by Platform

#### Arch Linux
```bash
sudo pacman -S qt5-base taglib cmake make gcc
```

#### Ubuntu/Debian
```bash
sudo apt-get install qtbase5-dev libtag1-dev cmake make g++
```

#### Fedora/RHEL
```bash
sudo dnf install qt5-qtbase-devel taglib-devel cmake make gcc-c++
```

#### macOS (with Homebrew)
```bash
brew install qt5 taglib cmake
```

## Build Instructions

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd mu-tagg
   ```

2. **Create build directory and compile**:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Install (optional)**:
   ```bash
   # Copy executable to system path
   sudo cp mu-tagg /usr/local/bin/
   
   # Install desktop file (Linux)
   sudo cp ../mu-tagg.desktop /usr/share/applications/
   ```

## Usage

### Basic Workflow

1. **Launch the application**:
   ```bash
   ./mu-tagg
   ```

2. **Add files**:
   - Drag and drop audio files or folders into the window
   - Use `File → Open Files...` to select specific files
   - Use `File → Open Directory...` to scan a folder recursively

3. **Edit tags**:
   - Select one or more files in the table
   - Edit fields in the left panel
   - Changes are applied to all selected files
   - Use `[Mixed]` indicator to see when files have different values

4. **Manage cover art**:
   - Right-click on the cover art area
   - Choose "Add Cover Art..." to select an image file
   - Choose "Remove Cover Art" to delete embedded artwork

5. **Save changes**:
   - Press `Ctrl+S` or click "Save Changes"
   - Tags are written back to the audio files
   - No confirmation dialog - changes are applied immediately

### Advanced Features

#### Multi-File Selection
- Select multiple files to edit them simultaneously
- Fields showing `[Mixed]` indicate different values across files
- Only explicitly changed fields are updated when saving

#### Table Management
- Drag column headers to reorder them
- Resize columns by dragging their edges
- All layout changes are automatically saved

#### Keyboard Navigation
- Use arrow keys to navigate the file list
- `Delete` key removes selected files from the list (doesn't delete files)
- `Tab` to move between input fields

## Technical Details

### Architecture
- **Framework**: Qt5 Widgets for cross-platform GUI
- **Audio Library**: TagLib for metadata reading/writing
- **Build System**: CMake with C++17 standard
- **Settings**: QSettings for configuration persistence

### File Structure
```
mu-tagg/
├── src/
│   ├── main.cpp          # Application entry point
│   ├── MainWindow.h      # Main window class declaration
│   └── MainWindow.cpp    # Main window implementation
├── CMakeLists.txt        # Build configuration
├── mu-tagg.desktop       # Desktop integration file
└── README.md            # This file
```

### Key Classes
- **MainWindow**: Main application window with all UI components
- **AudioFileInfo**: Data structure holding file metadata
- **TagLib Integration**: Direct use of TagLib APIs for MP3 and FLAC handling

### Error Handling
- Graceful handling of corrupted or unsupported files
- Debug logging for troubleshooting (use `qDebug()` output)
- Non-blocking UI during file operations

## Contributing

This is an open-source project. Contributions are welcome! Areas for improvement include:

- **Additional Formats**: Support for more audio formats (OGG, M4A, etc.)
- **Enhanced UI**: More advanced editing features and visual improvements
- **Performance**: Optimizations for large file collections
- **Localization**: Multi-language support
- **Testing**: Unit tests and integration tests

## License

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org/>

---

**Note**: This application is designed for audio file metadata editing. Always backup your files before making bulk changes, and test on a small sample first. 
**Note:** This is a minimal prototype. For more features (more formats, etc.), contributions are welcome! 
