# Tombo C99

Plain text editor with encryption for Windows, built with C99 and Win32 API.

## Features

  * Tree view sidebar for navigating directories
  * Multi-line text editor with word wrap
  * Open/save plain text (.txt, .md) and encrypted (.chi, .chs) files
  * Unicode text support via configurable encoding list (UTF-8, CP1252, etc.)
  * BOM detection (UTF-8, UTF-16 LE/BE)
  * Password dialog for encrypt/decrypt
  * Password caching with inactivity timer (configurable via `password_timeout`)
  * Tools menu with manual "Forget Password"
  * Basic text search (find next/prev)
  * Safe save by default (writes to temp file, then renames)
  * Optional paranoid mode (read-back verification)

## Build

Requires mingw gcc on Windows:

```
cd tombo_c99
gmake
```

Alternatively zig:

```
zig cc -std=c99 -Wall -Wextra -DTOMBO -I../contrib/TomboCrypt   -lgdi32 -lcomctl32 -lcomdlg32 -lshell32   main.c config.c encoding.c ini.c ../contrib/TomboCrypt/bf01_file.c ../contrib/TomboCrypt/blowfish.c ../contrib/TomboCrypt/md5.c
```


## Usage

Run `tombo.exe`. The left pane shows a file tree; double-click a file to open it, or use keyboard and enter/return key.

### Keyboard Shortcuts

  * Ctrl+N - New file
  * Ctrl+O - Open file
  * Ctrl+S - Save (or Save As if new)
  * Ctrl+Z - Undo
  * Ctrl+F - Find  - FIXME TODO
  * F3 - Find next  - FIXME TODO
  * Shift+F3 - Find previous  - FIXME TODO
  * Shift+Tab/Tab - Switch between tree and editor (or insert tab in editor)

### Configuration

Settings are stored in `tombo.ini` (created on exit):

```ini
[window]
x=100
y=100
w=800
h=600
tree_w=200

[general]
last_dir=C:\path\to\last\folder
safe_save=1
paranoid_save=0
password_timeout=300
persist_window=1
sort_dirs_first=0
encoding_list=utf8,cp1252

[word]
word_wrap=0
```

  * `safe_save` - Write to temp file first, then rename. Default: 1 (on)
  * `paranoid_save` - Read back temp file and verify contents before rename. Default: 0 (off)
  * `password_timeout` - Cache password in memory for N seconds after successful encrypt/decrypt. Inactivity resets the timer. 0 = disabled (prompt every time). Default: 0
  * `persist_window` - Save and restore window position/size across sessions. 0 = always open at default size/position. Default: 1
  * `sort_dirs_first` - Show directories at the top of the tree view. Default: 1
  * `encoding_list` - Comma-separated list of character encodings to try when opening files. First encoding used for saving. BOM detection tried before the list. Default: `utf8`

## License

See individual source files for licensing. Uses rxi/ini (MIT) for INI parsing.
