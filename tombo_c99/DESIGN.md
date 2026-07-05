# C99 Win32 Note Editor with Encryption

## Recommendation: Fresh implementation

**Why not convert C++ code?**
The C++ Tombo app is ~80% platform-agnostic logic but deeply coupled to C++ patterns (classes, virtual dispatch, STL-like containers). The YAEdit custom editor engine alone is thousands of lines of complex C++. Converting means rewriting almost everything anyway, but with the burden of preserving C++ idioms translated to C. A fresh C99 implementation is cleaner, simpler, and can directly reuse the existing C99 TomboCrypt library (`bf01_file.c`) with zero adaptation.

**What we reuse from C++ codebase:**
- Architecture patterns (tree view + editor pane layout)
- BF01 file format (via `contrib/TomboCrypt/bf01_file.c` + dependencies)
- Menu structure ideas from `Src/MainFrame.h`

## Scope

Medium feature set:
  * Tree view sidebar (file system directories, nested)
  * Multi-line text editor pane
  * Open/save .txt (plain) and .chi (encrypted) files
  * Password dialog for encrypt/decrypt
  * Basic text search (find next/prev)
  * Word wrap toggle
  * INI config file (window size, last directory)

## Architecture

```
tombo_c99/
  main.c          Win32 entry, window proc, message loop, menus, dialogs
  config.c/.h     INI config load/save (rxi/ini adapted for write)
  Makefile         gcc build
```

Shared code reused as-is (NOT copied, linked):
```
contrib/TomboCrypt/bf01_file.c
contrib/TomboCrypt/blowfish.c
contrib/TomboCrypt/md5.c
```

External dependency (vendored copy):
```
tombo_c99/ini.c + ini.h   from C:\code\terminal\colortool_alt_clean (rxi/ini, MIT)
```

## File Details

### `main.c`
Single-file Win32 GUI. Key components:

  * **WinMain**: Register class, create window with `WS_OVERLAPPEDWINDOW`, create child controls (TreeView left, Edit right), message loop
  * **WndProc**: Handle WM_CREATE (init tree + edit), WM_SIZE (layout children), WM_NOTIFY (tree selection), WM_COMMAND (menu actions), WM_DESTROY (save config, exit)
  * **Menu**: File > New, Open, Save, Save As, Exit; Edit > Undo, Cut, Copy, Paste, Find; View > Word Wrap; Help > About
  * **Tree view**: `TV_INSERTSTRUCT` to populate from directory scan. `FindFirstFile`/`FindNextFile` recursion for subdirectories. Show .txt and .chi files. Double-click opens file in editor.
  * **Editor**: `CreateWindow("EDIT", ..., WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL, ...)`. `WM_SETFONT` with fixed-width font. Track dirty state via `EM_GETMODIFY`.
  * **File open/save flow**:
      - Open .txt: `fopen` + `fread` into buffer, `SetWindowText` to edit
      - Open .chi: `fopen` -> `fseek`/`fread` to temp FILE* -> `bf01_decrypt_stream` -> read plaintext -> `SetWindowText`
      - Save .txt: `GetWindowText` length + buffer, `fwrite`
      - Save .chi: `GetWindowText` -> write plaintext to temp FILE* -> `bf01_encrypt_stream` -> write to .chi file
  * **Password dialog**: `DialogBoxParam` with custom `DlgProc`. Two `EDIT` controls (password + confirm). ES_PASSWORD style. Returns password string via `SetWindowLongPtr(DWLP_USER, ...)`.
  * **Find dialog**: Simple modeless dialog with text input + "Find Next" / "Find Previous" buttons. Uses `EM_FINDTEXT` on the edit control.
  * **Config**: Load on startup (window pos, last dir), save on exit. Section `[window]` for geometry, `[general]` for last_dir.

### `config.c`
Thin wrapper around rxi/ini:
  * `config_load(path)` -> returns `ini_t*`
  * `config_get(ini, section, key)` -> `const char*`
  * `config_get_int(ini, section, key, default)` -> `int`
  * For write support: simple `fprintf` to INI format (rxi/ini is read-only, but we can write manually for the few settings we need)

### `Makefile`
```makefile
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -DTOMBO
LDFLAGS = -lgdi32 -lcomctl32 -lcomdlg32

TOMBO_CRYPT = ../contrib/TomboCrypt

SRCS = main.c config.c ini.c $(TOMBO_CRYPT)/bf01_file.c $(TOMBO_CRYPT)/blowfish.c $(TOMBO_CRYPT)/md5.c
OBJS = $(SRCS:.c=.o)
TARGET = tombo.exe

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
```

## Key Design Decisions

  * **Single-file GUI** (`main.c`): Keeps things simple. YAGNI applies - no need for separate tree/editor modules until code gets unwieldy.
  * **Win32 API only** (no MFC, no WTL): Direct API calls. Compiles with gcc via mingw.
  * **Temp files for crypto**: `bf01_encrypt_stream`/`bf01_decrypt_stream` work on `FILE*`. Use `tmpfile()` for in-memory encryption round-trips without temp files on disk.
  * **INI write**: Manual `fprintf` for saving config (only 2-3 settings). No need for a write-capable INI library.
  * **Tree view**: Manual `FindFirstFile`/`FindNextFile` recursion. No `SHBrowseForFolder` - we own the tree and populate it ourselves for full control.
  * **File associations**: Only show .txt and .chi in the tree view. Other files hidden.
  * **Encoding**: UTF-8 with BOM handling. The edit control uses `EM_SETTEXTLEN` for large files. Keep it simple - no Unicode conversion layer.

## Build & Test

  1. `cd tombo_c99 && make` (requires mingw gcc on Windows)
  2. Run `tombo.exe`
  3. Test plain text: File > New, type text, File > Save as .txt, close, reopen
  4. Test encrypted: File > Save as .chi, enter password, close, reopen .chi, enter password
  5. Test tree: Navigate directories, open files from tree
  6. Test search: Edit > Find, search for text
  7. Test word wrap: View > Word Wrap toggle

## Critical Files

  * `C:\code\c\tombo\contrib\TomboCrypt\bf01_file.c` - encryption library (reuse as-is)
  * `C:\code\c\tombo\contrib\TomboCrypt\bf01_file.h` - encryption API
  * `C:\code\c\tombo\contrib\TomboCrypt\blowfish.c` - Blowfish cipher (reuse as-is)
  * `C:\code\c\tombo\contrib\TomboCrypt\md5.c` - MD5 hash (reuse as-is)
  * `C:\code\terminal\colortool_alt_clean\ini.c` + `ini.h` - INI library (vendor copy)

## TODO Items

  * Add option for safe-saving, save to a temp file, once successful, delete old filename and rename temp to old
      * double safe paranoid mode could load temp file from disk and compare contents before delete/rename step
  * Add support for caching password in memory, with auto-forget on an inactivity timer
  * New Folder support, menu and right click
  * Delete New Folder support
  * Delete File support
  * convert text file to encrypted
  * convert encrypted file to text
  * working find/search support
  * Icon support
  * Review file encoding support
  * BOM support, right now see the 3-bytes as (what I suspect is) cp1252
