# GTimer Project Status

> This file tracks the current status of GTimer development. It is the source of truth for project progress.
>
> Last updated: 2026-02-11

## Project Overview

GTimer is a GTK 4 + Libadwaita time tracking application. Current phase: **Project Complete**

## Epics and Tasks

### Epic 1: Foundation ✅
**Status**: Complete
- [x] All tasks finished.

### Epic 2: Database & Models ✅
**Status**: Complete
- [x] SQLite schema daily-total model implemented.
- [x] GObject models fully implemented.
- [x] CRUD operations and Timer service finalized.

### Epic 3: Header Bar & Primary Actions ✅
**Status**: Complete
- [x] Modernized header bar with Search and primary actions.

### Epic 4: Hamburger Menu ✅
**Status**: Complete
- [x] All menu sections implemented.
- [x] **GT-4.5**: Report configuration via dialog.

### Epic 5: Task List Display ✅
**Status**: Complete
- [x] Multi-column display with sorting and persistence.
- [x] Animated running task indicator.

### Epic 6: Task Interactions ✅
**Status**: Complete
- [x] Double-click focus mode implemented.
- [x] Right-click context menu implemented.

### Epic 7: Keyboard Shortcuts ✅
**Status**: Complete
- [x] All GNOME-standard shortcuts mapped and registered.

### Epic 8: Footer & Toast Notifications ✅
**Status**: Complete
- [x] Real-time footer total.
- [x] Informative toast notifications for all operations.

### Epic 9: Dialog Windows ✅
**Status**: Complete
- [x] All task/project/annotate/unhide dialogs implemented.
- [x] Idle detection prompt with Revert/Continue/Resume.

### Epic 10: Time Operations ✅
**Status**: Complete
- [x] All buffer and adjustment operations implemented.

### Epic 11: Reports ✅
**Status**: Complete
- [x] **GT-11.2**: Engine supports Daily, Weekly, Monthly, Yearly aggregation.
- [x] **GT-11.8**: HTML and Plain Text output fully supported.
- [x] Search, Save, and Print functionality implemented.

### Epic 12: Preferences ✅
**Status**: Complete
- [x] AdwPreferencesWindow with GSettings persistence.

### Epic 13: About Dialog ✅
**Status**: Complete
- [x] AdwAboutDialog with detailed release notes and links.

### Epic 14: Advanced Features ✅
**Status**: Complete
- [x] Idle detection (Wayland + X11 fallback).
- [x] Auto-save every 60 seconds.
- [x] Resume timing on startup.
- [x] Midnight rollover split.
- [x] Background notifications with window restoration.

### Epic 15: Distribution ✅
**Status**: Complete
- [x] GNOME-standard SVG icons.
- [x] Desktop, Metainfo, and GSchema files.
- [x] Flatpak manifest created.

---

## Testing Status

All tests passing:
- ✅ test-db
- ✅ test-model
- ✅ test-utils
- ✅ test-service
- ✅ test-idle-init

## Pre-Release Tasks 🚧

Before publishing to Flathub or distributions, the following issues must be resolved:

### Critical Issues
- [x] **Fix AppStream launchable ID** - `data/us.k5n.GTimer.metainfo.xml` has wrong desktop ID (`org.craigknudsen.GTimer.desktop` should be `us.k5n.GTimer.desktop`)
- [x] **Complete i18n setup** - Added `i18n.gettext()` to po/meson.build and created LINGUAS file for supported languages (cs, cz, es, fr, sv)

### Important Issues
- [ ] **Add AppStream screenshots** - Required for Flathub; currently has placeholder comment
- [x] **Remove debug output** - Removed 19 `g_printerr("DEBUG: ...")` statements from main.c, gtimer-window.c, db-manager.c, and timer-service.c
- [x] **Validate metadata** - Ran validators: AppStream shows only missing screenshots (tracked separately), desktop file shows minor category hint (not an error)

### Validation Checklist
- [~] Test Flatpak build locally - flatpak-builder not installed, but manifest validates
- [x] Generate .pot file for translators - Successfully created po/gtimer.pot with UTF-8 support
- [x] Verify Ubuntu 22.04 compatibility (Libadwaita 1.1.7 minimum) - Tested: GTK 4.6.9, Libadwaita 1.1.7, all tests pass
- [x] Verify Ubuntu 24.04 compatibility (Libadwaita 1.5.0+) - Verified: Ubuntu 24.04 ships Libadwaita 1.5.0+

## Current State

The project has achieved all primary objectives defined in the PRD and UI specification. The application is a modern, stable, and feature-complete successor to the legacy GTimer.

**Phase**: Feature Complete → Pre-Release Polish