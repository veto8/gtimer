# GTimer Project Status

> This file tracks the current status of GTimer development. It is the source of truth for project progress.
>
> Last updated: 2026-02-09

## Project Overview

GTimer is a GTK 4 + Libadwaita time tracking application. Current phase: **Schema Rework + UI Implementation**

## Schema Change Required

The current database schema uses an `entries` table with individual start/stop records. This needs to be reworked to use a **daily-total** model (one row per task per day with accumulated seconds). See PRD.md Section 4 for the target schema. The legacy data importer can also be removed since we are not migrating old data.

## Epics and Tasks

### Epic 1: Foundation ✅
**Status**: Complete
**Description**: Build system, application skeleton, basic window

- [x] **GT-1.1**: Meson build system with GTK 4 + Libadwaita + SQLite
- [x] **GT-1.2**: GTimerApplication and GTimerWindow classes
- [x] **GT-1.3**: AdwApplicationWindow with AdwHeaderBar
- [x] **GT-1.4**: Default window size (500x400)

### Epic 2: Database & Models ⏳
**Status**: Needs Rework
**Description**: SQLite schema and GObject data models

- [x] **GT-2.1**: SQLite database with projects/tasks tables
- [x] **GT-2.2**: GObject models (GTimerTask, GTimerProject)
- [x] **GT-2.3**: Rework schema to daily-total model (`daily_time` table with task_id + date + seconds, replacing `entries` table)
- [x] **GT-2.4**: Separate `annotations` table (task_id + timestamp + text)
- [x] **GT-2.5**: Add `is_hidden` column to tasks
- [x] **GT-2.6**: Remove legacy XML importer code (no data migration needed)
- [x] **GT-2.7**: Task CRUD operations
- [x] **GT-2.8**: Timer service (start/stop logic)

### Epic 3: Header Bar & Primary Actions ✅
**Status**: Complete
**Description**: AdwHeaderBar with action buttons and hamburger menu (replaces legacy menu bar + toolbar)

- [x] **GT-3.1**: AdwHeaderBar with title "GTimer"
- [x] **GT-3.2**: Start/Stop toggle button (left side, icon changes per selected task state)
- [x] **GT-3.3**: Stop All button (left side, insensitive when nothing timing)
- [x] **GT-3.4**: New Task button (right side)
- [x] **GT-3.5**: Edit Task button (right side, insensitive when no selection)
- [x] **GT-3.6**: Annotate button (right side, insensitive when no selection)
- [x] **GT-3.7**: Hamburger menu button (right side)
- [x] **GT-3.8**: Window subtitle shows current task or "N tasks running"

### Epic 4: Hamburger Menu ✅
**Status**: Complete
**Description**: GMenuModel-based hamburger menu (replaces legacy File/Edit/Options/Task/Project/Report/Help menus)

- [x] **GT-4.1**: Task section (Start, Stop, Stop All, Hide, Unhide, Delete)
- [x] **GT-4.2**: Time Adjustment section (+/-1, 5, 30 minutes, Set to Zero)
- [x] **GT-4.3**: Edit section (Cut/Copy/Paste Time, Clear Buffer)
- [x] **GT-4.4**: Project section (New, Edit)
- [ ] **GT-4.5**: Report section (Daily, Weekly, Monthly, Yearly)
- [x] **GT-4.6**: Preferences item (opens AdwPreferencesDialog)
- [x] **GT-4.7**: Keyboard Shortcuts item (opens GtkShortcutsWindow)
- [x] **GT-4.8**: About GTimer item (opens AdwAboutDialog)
- [x] **GT-4.9**: Save and Quit items

### Epic 5: Task List Display ✅
**Status**: Complete
**Description**: GtkColumnView showing Project, Task, Today, Total columns

- [x] **GT-5.1**: GtkColumnView with GListStore model
- [x] **GT-5.2**: Task name column
- [x] **GT-5.3**: Basic time display
- [x] **GT-5.4**: Project column
- [x] **GT-5.5**: Today column (daily accumulated time from daily_time table)
- [x] **GT-5.6**: Total column (sum of all daily_time rows for task)
- [x] **GT-5.7**: Column sorting with cycle behavior (name A-Z, Z-A, creation asc/desc)
- [x] **GT-5.8**: Running task indicator (colored dot or spinner in Project column)
- [x] **GT-5.9**: Window/column size persistence across sessions

### Epic 6: Task Interactions ✅
**Status**: Complete
**Description**: Mouse and keyboard task operations

- [x] **GT-6.1**: Single click to select
- [x] **GT-6.2**: Double-click to toggle timing (stops all other tasks)
- [x] **GT-6.3**: Multiple simultaneous running tasks (via buttons/menu)
- [x] **GT-6.4**: Right-click context menu (GtkPopoverMenu, same actions as Task section of hamburger menu)

### Epic 7: Keyboard Shortcuts ✅
**Status**: Complete
**Description**: GNOME HIG-compliant keyboard shortcuts (remapped from legacy)

- [x] **GT-7.1**: Ctrl+S (Save), Ctrl+Q (Quit)
- [x] **GT-7.2**: Alt+S (Start), Alt+X (Stop), Alt+T (Stop All)
- [x] **GT-7.3**: Ctrl+N (New Task), Ctrl+E (Edit Task)
- [x] **GT-7.4**: Ctrl+Shift+A (Annotate — remapped from Ctrl+A)
- [x] **GT-7.5**: Ctrl+Shift+H (Hide — remapped from Ctrl+H)
- [x] **GT-7.6**: Ctrl+Shift+U (Unhide — remapped from Ctrl+U)
- [x] **GT-7.7**: Ctrl+Delete (Delete — remapped from Ctrl+R)
- [x] **GT-7.8**: Time adjustment shortcuts (Ctrl+I, Shift+Ctrl+I, Ctrl+Alt+I, Ctrl+D, Shift+Ctrl+D, Ctrl+Alt+D, Ctrl+Alt+0)
- [x] **GT-7.9**: Ctrl+X/C/V (Cut/Copy/Paste time)
- [x] **GT-7.10**: GtkShortcutsWindow listing all shortcuts

### Epic 8: Footer & Toast Notifications ✅
**Status**: Complete
**Description**: Footer label for "Today" total + AdwToast for transient messages

- [x] **GT-8.1**: Footer label showing "Today: H:MM:SS" (updates every second)
- [x] **GT-8.2**: AdwToastOverlay wrapping task list
- [x] **GT-8.3**: Toast messages for all operations (task added/updated/deleted/hidden, sort changed, buffer operations, save, etc.)

### Epic 9: Dialog Windows ⏳
**Status**: In Progress
**Description**: All modal dialogs using AdwDialog / AdwAlertDialog

- [x] **GT-9.1**: New Task dialog (AdwDialog with project dropdown + name entry)
- [x] **GT-9.2**: Edit Task dialog
- [x] **GT-9.3**: New Project dialog
- [x] **GT-9.4**: Edit Project dialog
- [x] **GT-9.5**: Annotate dialog (multi-line text view, stored with UTC timestamp)
- [x] **GT-9.6**: Unhide Tasks dialog (checkbox list, select all/none)
- [x] **GT-9.7**: Delete Confirmation dialog (AdwAlertDialog, destructive-action button)
- [ ] **GT-9.8**: Idle Detection prompt (3 buttons: Revert, Continue, Resume)

### Epic 10: Time Operations ✅
**Status**: Complete
**Description**: Cut/Copy/Paste time buffer + increment/decrement

- [x] **GT-10.1**: +5 minutes
- [x] **GT-10.2**: +15 minutes
- [x] **GT-10.3**: +1 minute, +30 minutes
- [x] **GT-10.4**: -1 minute, -5 minutes, -30 minutes (floor at 0)
- [x] **GT-10.5**: Set to Zero (saves to buffer)
- [x] **GT-10.6**: Cut Time (today's time to buffer, zero task)
- [x] **GT-10.7**: Copy Time (today's time to buffer, keep task)
- [x] **GT-10.8**: Paste Time (add buffer to task, buffer NOT cleared)
- [x] **GT-10.9**: Clear Buffer

### Epic 11: Reports ✅
**Status**: Complete
**Description**: Report configuration, generation, and display

- [x] **GT-11.1**: Report Configuration dialog (type, time range, format, rounding, task selection)
- [x] **GT-11.2**: Report generation engine (Daily, Weekly, Monthly, Yearly)
- [x] **GT-11.3**: Report Display window (non-modal, scrolled text view)
- [x] **GT-11.4**: Report search (Ctrl+F, find next)
- [x] **GT-11.5**: Report save to file (GtkFileDialog)
- [x] **GT-11.6**: Report print (GtkPrintOperation)
- [x] **GT-11.7**: Time rounding (None, 1/5/10/15/30 min, 1 hour)
- [x] **GT-11.8**: Plain Text and HTML output formats

### Epic 12: Preferences ✅
**Status**: Complete
**Description**: AdwPreferencesDialog (replaces legacy Options menu toggles)

- [x] **GT-12.1**: General page: Auto Save toggle, Animate toggle, Resume on Startup toggle
- [x] **GT-12.2**: Idle Detection page: Enable toggle, Threshold spinner (1-120 min)
- [x] **GT-12.3**: Time page: Midnight offset (day boundary), Week start day dropdown

### Epic 13: About Dialog ✅
**Status**: Complete
**Description**: AdwAboutDialog with app info

- [x] **GT-13.1**: Basic AdwAboutDialog with version and copyright
- [x] **GT-13.2**: Website URL (opens via xdg-open)
- [ ] **GT-13.3**: Release notes / changelog section
- [x] **GT-13.4**: License info (GPL-2.0+)

### Epic 14: Advanced Features ⏳
**Status**: In Progress
**Description**: Idle detection, auto-save, notifications, persistence

- [x] **GT-14.1**: Idle detection abstraction layer (Wayland, X11, Dummy backends)
- [ ] **GT-14.2**: Wayland idle detection via org.gnome.Mutter.IdleMonitor D-Bus
- [ ] **GT-14.3**: X11 fallback via XScreenSaver extension
- [ ] **GT-14.4**: Auto-save (periodic flush of timing data to SQLite)
- [ ] **GT-14.5**: Resume timing on startup (restore last-timed tasks)
- [ ] **GT-14.6**: Midnight rollover (split time across day boundary)
- [ ] **GT-14.7**: Background notification for active timers (GNotification)
- [x] **GT-14.8**: Window state persistence (size, column widths, sort)
- [x] **GT-14.9**: Animated running-task indicator (spinner or dot, toggleable)

### Epic 15: Distribution ✅
**Status**: Complete
**Description**: Icons, packaging, metadata

- [x] **GT-15.1**: SVG application icon
- [x] **GT-15.2**: Desktop file (us.k5n.GTimer.desktop)
- [x] **GT-15.3**: AppStream metainfo (us.k5n.GTimer.metainfo.xml)
- [ ] **GT-15.4**: Flatpak manifest
- [x] **GT-15.5**: Accessibility labels on all interactive widgets
- [x] **GT-15.6**: Dark mode verification
- [x] **GT-15.7**: High-DPI verification

---

## Current Sprint Focus

**Immediate priority:** Advanced features like Idle Detection improvements (Epic 14) and Reports refinements (Epic 11).

**Next priorities:**
1. Background notifications for active timers (GT-14.7)
2. Auto-save and Resume logic (GT-14.4, GT-14.5)
3. HTML report output (GT-11.8)

## Testing Status

All existing tests passing:
- ✅ test-db: Database operations
- ✅ test-model: Task list model
- ✅ test-utils: Time formatting utilities
- ✅ test-importer: Legacy data import (to be removed — GT-2.6)
- ✅ test-service: Timer service
- ✅ test-idle-init: Idle monitoring

**Note:** Tests will need updating after schema rework (GT-2.3, GT-2.4).

## Known Issues

- Database schema uses individual entries model — needs rework to daily-total model (see Epic 2)
- Legacy importer tests exist but importer is no longer needed

## Documentation

- ✅ USER_INTERFACE.md: Complete UI specification (20 sections, GNOME HIG compliant)
- ✅ PRD.md: Technical requirements and data model
- ✅ AGENTS.md: Coding style and conventions
- ✅ This file: Project status (source of truth)

---

**Legend**:
- ✅ Complete
- ⏳ In Progress / Needs Rework
- 📋 Not Started
