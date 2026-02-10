# PRD: GTimer GTK 2 to GTK 4 + Wayland Migration

## Overview

Migrate GTimer from GTK 2 to GTK 4 + Libadwaita for modern Linux environments (Ubuntu 24.04+, Wayland by default). This is a ground-up rewrite — not a port of the legacy codebase. The legacy code in `legacy/` serves as a behavioral reference only.

**Key documents:**
- `USER_INTERFACE.md` — Complete UI specification (authoritative for all UI decisions)
- `AGENTS.md` — Coding style and conventions
- `STATUS.md` — Project progress tracking

---

## 1. Port to GTK 4 and Libadwaita

### 1.1 Dependencies

**Target:** GTK 4.12+ + Libadwaita 1.4+ + SQLite 3

**Widget Migration Map:**

| GTK 2 Widget | GTK 4 Equivalent | Notes |
|--------------|------------------|-------|
| `GtkVBox` / `GtkHBox` | `GtkBox` | Use `gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing)` |
| `GtkTable` | `GtkGrid` | `gtk_grid_attach()` replaces table attach |
| `GtkMenu` | `GMenuModel` + `GtkPopoverMenu` | Menu models decoupled from UI |
| `GtkStatusbar` | `AdwToastOverlay` | Statusbar deprecated in GTK 4 |
| `GtkTreeView` / `GtkCList` | `GtkColumnView` | Complete rewrite with `GListStore` model |
| `GtkOptionMenu` / `GtkCombo` | `GtkDropDown` | Modern replacement |
| `GtkToolbar` | `AdwHeaderBar` | Toolbar deprecated; actions in header bar |

### 1.2 Critical GTK 4 Behavioral Changes

- **No direct struct access**: All `->` access to GTK structs must use accessor functions
- **Event handling**: Use `g_signal_connect()` and event controllers (not `gtk_signal_connect`)
- **Menu system**: Use `GAction` + `GMenu` + `GtkApplicationWindow` (not GtkUIManager)
- **Drawing**: Use Cairo/GSK rendering (not `gdk_draw_*`)
- **Windowing**: Use `gtk_native_get_surface()` (not `widget->window`)

### 1.3 Libadwaita Integration

**Window Architecture (see USER_INTERFACE.md Section 2 for full spec):**
```
AdwApplicationWindow
└── AdwToolbarView
    ├── AdwHeaderBar (top)
    ├── AdwToastOverlay
    │   └── GtkScrolledWindow → GtkColumnView (task list)
    └── Footer label ("Today: H:MM:SS")
```

---

## 2. Build System

### 2.1 Meson + Ninja

**Build system:** Meson (replaces Autotools)

```meson
project('gtimer', 'c',
  version: '3.0.0',
  meson_version: '>= 1.0.0',
  default_options: [
    'warning_level=2',
    'c_std=gnu11',
  ]
)

gtk4_dep = dependency('gtk4', version: '>= 4.12.0')
adwaita_dep = dependency('libadwaita-1', version: '>= 1.4.0')
sqlite_dep = dependency('sqlite3')
```

### 2.2 Directory Structure

```
gtimer/
├── meson.build              # Root build file
├── src/                     # Application source (UI layer)
│   ├── meson.build
│   ├── main.c               # Application entry point
│   ├── gtimer-app.c         # GtkApplication subclass
│   ├── gtimer-window.c      # Main window (AdwApplicationWindow)
│   └── ...                  # Dialog and UI source files
├── core/                    # Core library (data, models, services)
│   ├── meson.build
│   ├── db-manager.c         # SQLite database operations
│   ├── models.c             # Data models
│   ├── task-list-model.c    # Task list GObject model
│   ├── timer-service.c      # Timer logic
│   ├── idle-monitor.c       # Idle detection abstraction
│   └── ...
├── tests/                   # Test files
│   ├── meson.build
│   ├── test-db.c
│   └── ...
├── data/                    # Resources and desktop files
│   ├── meson.build
│   ├── org.craigknudsen.GTimer.desktop
│   ├── org.craigknudsen.GTimer.metainfo.xml
│   ├── style.css
│   └── icons/
├── legacy/                  # Old GTK 2 source (reference only)
└── po/                      # Translations
    └── meson.build
```

---

## 3. Wayland Support

### 3.1 X11 Replacements

All X11-specific code is eliminated. GTK 4 handles Wayland natively.

| Old (X11) | New (GTK 4/Wayland) |
|-----------|---------------------|
| `XIconifyWindow()` | `gtk_window_minimize()` |
| `XRaiseWindow()` | `gtk_window_present()` |
| `XGetWindowAttributes()` | `gtk_window_get_default_size()` |
| `XSetInputFocus()` | `gtk_widget_grab_focus()` |
| `GtkStatusIcon` (system tray) | `GNotification` (persistent notification) |

### 3.2 Idle Detection

**Problem:** GTimer needs idle detection to pause timers. X11 used XScreenSaver extension. Wayland has no standard idle detection protocol.

**Solution:** Abstract behind an interface with multiple backends:

| Backend | API | When Used |
|---------|-----|-----------|
| Wayland/GNOME | `org.gnome.Mutter.IdleMonitor` D-Bus | GNOME on Wayland (primary) |
| X11 | XScreenSaver extension | XWayland fallback |
| Dummy | No-op | When neither available; shows warning |

### 3.3 Background Operation

When the window is minimized/closed with active timers:
- Use `GNotification` for persistent notification showing elapsed time
- Use `org.freedesktop.portal.Background` to keep running

---

## 4. Data Model

### 4.1 Time Tracking Approach

**IMPORTANT:** GTimer uses a **daily-total** time tracking model, NOT individual start/stop entries.

The legacy application tracked time as a single accumulated total per task per day. The new version preserves this simple approach:

- Each task stores a **total number of seconds** for each calendar day it was used
- When timing starts, the app records the start timestamp in memory
- When timing stops (or periodically via auto-save), elapsed seconds are added to that day's total
- There are **no individual "time entry" records** with start/end timestamps
- The cut/copy/paste buffer, idle detection revert, and manual adjustments all operate on daily totals

This means the `entries` table is **not** a log of individual timing sessions. It is a daily aggregation:

```sql
-- One row per task per day (not per start/stop session)
CREATE TABLE IF NOT EXISTS daily_time (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  task_id INTEGER NOT NULL REFERENCES tasks(id),
  date TEXT NOT NULL,           -- ISO 8601 date: 'YYYY-MM-DD'
  seconds INTEGER DEFAULT 0,   -- Total seconds worked that day
  UNIQUE(task_id, date)
);
```

**Annotations** are stored separately with timestamps:

```sql
CREATE TABLE IF NOT EXISTS annotations (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  task_id INTEGER NOT NULL REFERENCES tasks(id),
  created_at INTEGER NOT NULL,  -- Unix timestamp (UTC)
  text TEXT NOT NULL
);
```

**Task table** tracks current timing state:

```sql
CREATE TABLE IF NOT EXISTS tasks (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  project_id INTEGER REFERENCES projects(id),
  name TEXT NOT NULL,
  is_hidden INTEGER DEFAULT 0,
  is_timing INTEGER DEFAULT 0,
  last_start_time INTEGER,      -- Unix timestamp when current session started
  created_at INTEGER DEFAULT (strftime('%s', 'now'))
);
```

**Projects table:**

```sql
CREATE TABLE IF NOT EXISTS projects (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  created_at INTEGER DEFAULT (strftime('%s', 'now'))
);
```

### 4.2 Time Tracking Operations

| Operation | What Happens |
|-----------|-------------|
| Start timing | Set `is_timing=1`, record `last_start_time=now()` |
| Stop timing | Calculate `elapsed = now() - last_start_time`. Add `elapsed` to today's `daily_time.seconds`. Set `is_timing=0`. |
| Auto-save (while timing) | Calculate elapsed so far, flush to `daily_time`, update `last_start_time=now()` |
| +N minutes | Add N*60 to today's `daily_time.seconds` |
| -N minutes | Subtract N*60 from today's `daily_time.seconds` (floor at 0) |
| Set to zero | Save today's seconds to cut/paste buffer, set today's `daily_time.seconds=0` |
| Cut | Save today's seconds to buffer, set today to 0 |
| Paste | Add buffer to today's seconds |
| Midnight rollover | If timing spans midnight, split: add remaining seconds to old day, start new day at 0 |

### 4.3 "Today" and "Total" Display Values

| Column | Calculation |
|--------|-------------|
| Today | `daily_time.seconds` for current date + (if timing: `now() - last_start_time`) |
| Total | `SUM(daily_time.seconds)` across all dates + (if timing: `now() - last_start_time`) |

### 4.4 Data Storage Location

- **Primary:** `$XDG_DATA_HOME/gtimer/gtimer.db` (SQLite)
- **Fallback:** `~/.local/share/gtimer/gtimer.db`
- **Override:** `-dir` command-line option

### 4.5 No Legacy Data Migration

Legacy GTimer data (XML format in `~/.gtimer/`) is **not** migrated. This is a clean start. The legacy importer code that exists in `core/` can be removed.

---

## 5. Packaging: Flatpak

### 5.1 Flatpak Manifest

**File:** `org.craigknudsen.GTimer.json`

```json
{
  "app-id": "org.craigknudsen.GTimer",
  "runtime": "org.gnome.Platform",
  "runtime-version": "46",
  "sdk": "org.gnome.Sdk",
  "command": "gtimer",
  "finish-args": [
    "--socket=wayland",
    "--socket=fallback-x11",
    "--share=ipc",
    "--filesystem=xdg-data/gtimer:create",
    "--talk-name=org.freedesktop.portal.Background",
    "--talk-name=org.gnome.Mutter.IdleMonitor"
  ],
  "modules": [
    {
      "name": "gtimer",
      "buildsystem": "meson",
      "sources": [
        {
          "type": "git",
          "url": "https://github.com/craigk5n/gtimer.git",
          "branch": "main"
        }
      ]
    }
  ]
}
```

### 5.2 AppStream Metadata

**File:** `data/org.craigknudsen.GTimer.metainfo.xml`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>org.craigknudsen.GTimer</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-2.0+</project_license>
  <name>GTimer</name>
  <summary>A time tracking application</summary>
  <description>
    <p>GTimer is a time tracking tool for projects and tasks.</p>
  </description>
  <launchable type="desktop-id">org.craigknudsen.GTimer.desktop</launchable>
  <content_rating type="oars-1.1" />
  <releases>
    <release version="3.0.0" date="2026-XX-XX">
      <description>
        <p>Complete rewrite with GTK 4 and Libadwaita, Wayland support</p>
      </description>
    </release>
  </releases>
</component>
```

---

## 6. Implementation Phases

### Phase 1: Foundation
1. Meson build system with GTK 4 + Libadwaita + SQLite
2. `GTimerApplication` and `GTimerWindow` classes
3. SQLite database schema (daily-total model)
4. Basic task CRUD operations

### Phase 2: Core UI
1. Task list with `GtkColumnView` (Project, Task, Today, Total columns)
2. AdwHeaderBar with Start/Stop/New/Edit/Annotate buttons + hamburger menu
3. Double-click to toggle timing
4. Footer with "Today" total
5. AdwToast for transient messages

### Phase 3: Task Operations
1. All dialog windows (New/Edit/Delete task, New/Edit project, Annotate, Unhide)
2. Context menu (right-click)
3. Cut/Copy/Paste time buffer
4. Time increment/decrement (+/-1, 5, 30 minutes, set to zero)
5. Hide/Unhide tasks
6. Column sorting

### Phase 4: Reports & Preferences
1. Report configuration dialog
2. Report display window with search, save, print
3. AdwPreferencesDialog (auto-save, animate, idle settings, time settings)
4. Keyboard shortcuts + GtkShortcutsWindow

### Phase 5: Advanced Features
1. Idle detection (Wayland + X11 backends)
2. Auto-save
3. Resume timing on startup
4. Midnight rollover handling
5. Background notification for active timers

### Phase 6: Distribution
1. SVG app icon
2. Desktop file and AppStream metadata
3. Flatpak manifest
4. Accessibility labels
5. Dark mode / high-DPI verification

---

## 7. Testing Checklist

### Functionality
- [ ] Create/Edit/Delete projects
- [ ] Create/Edit/Delete tasks
- [ ] Start/Stop timing (single and multiple tasks)
- [ ] Double-click toggle timing
- [ ] Time increment/decrement
- [ ] Cut/Copy/Paste time
- [ ] Hide/Unhide tasks
- [ ] Annotations
- [ ] Report generation (all types, formats, rounding)
- [ ] Idle detection triggers prompt
- [ ] Auto-save
- [ ] Data persists after restart
- [ ] Midnight rollover

### Platforms
- [ ] Native Wayland (GNOME 46+)
- [ ] XWayland (fallback mode)
- [ ] X11 session (legacy)

### UI/UX
- [ ] Dark mode renders correctly
- [ ] High-DPI display (200% scaling)
- [ ] Keyboard navigation works
- [ ] All keyboard shortcuts functional
- [ ] Screen reader accessible
- [ ] Window state persists (size, columns, sort)

### Distribution
- [ ] Flatpak builds successfully
- [ ] Runs from Flatpak sandbox
- [ ] Idle detection works in Flatpak

---

## 8. Reference Resources

- GTK 4 Reference: https://docs.gtk.org/gtk4/
- Libadwaita Documentation: https://gnome.pages.gitlab.gnome.org/libadwaita/doc/
- GNOME HIG: https://developer.gnome.org/hig/
- XDG Desktop Portal: https://flatpak.github.io/xdg-desktop-portal/
- Meson: https://mesonbuild.com/
- Flatpak: https://docs.flatpak.org/
- AppStream: https://www.freedesktop.org/software/appstream/docs/

---

**Document Version:** 2.0
**Target Release:** GTimer 3.0.0
**Last Updated:** 2026-02-09
