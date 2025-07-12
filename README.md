# mu-tagg

A simple Qt-based audio tag editor (MP3, FLAC, etc.) with drag-and-drop support, inspired by MP3tag.

## Features
- Drag and drop MP3/FLAC files or folders into the window
- View and edit tags (Title, Artist, Album, Year, Track, Genre, Comment, Album Artist, Composer, Disk)
- Save changes back to files
- Cover art support (add/remove via right-click)
- Multi-file selection and editing
- Automatic configuration saving (window size, splitter position, column layout)

## Requirements
- Qt5 (Widgets)
- TagLib
- CMake (>= 3.10)
- C++17 compiler

### On Arch Linux
```
sudo pacman -S qt5-base taglib cmake make gcc
```

### On Ubuntu/Debian
```
sudo apt-get install qtbase5-dev libtag1-dev cmake make g++
```

## Build Instructions

1. Clone or download this repository.
2. In the project root, run:

```
mkdir build
cd build
cmake ..
make
```

3. The executable will be `mu-tagg` in the `build` directory.

## Run

```
./mu-tagg
```

## Usage
- Drag and drop MP3 or FLAC files (or folders) into the app window.
- Edit tags directly in the table or use the left-side tag editor panel.
- Use Ctrl+S or click "Save Changes" to write tags back to the files.
- Right-click on files to add/remove cover art.
- Configuration (window size, panel sizes, column layout) is automatically saved.

---

**Note:** This is a minimal prototype. For more features (more formats, etc.), contributions are welcome! 