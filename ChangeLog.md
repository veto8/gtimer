# ChangeLog

## [3.0.0] - 2026-02-10
### Added
- Complete rewrite using **GTK 4** and **Libadwaita**.
- Adopted **daily-total** data model for simplified and robust time tracking.
- Modernized UI featuring a centered **Search Entry** for real-time task filtering.
- Secondary **Action Bar** for quick access to core functions.
- **Background Notifications**: Persistent alerts when timers are active and the app is unfocused.
- **Auto-save**: Automatic database synchronization every 60 seconds.
- **Midnight Rollover**: Automatic splitting of time entries across day boundaries.
- **Improved Reports**: Support for Daily, Weekly, Monthly, and Yearly aggregation with HTML export and search.
- **Wayland & X11 support**: Native Wayland idle detection via Mutter D-Bus with XScreenSaver fallback for X11.
- **Flatpak support**: Added manifest for modern distribution.
- New high-resolution **SVG application icons**.

### Changed
- Replaced legacy menu bar and toolbar with a streamlined Header Bar and "More" (⋯) menu.
- Moved build system from Autotools to **Meson**.
- Migration of preferences to **GSettings**.

## [2.0.1] - 2023-05-06
- Header file cleanup; fix email address and URLs.
- Fix compile errors found while using Ubuntu 20.04.
- Disabled "Check for new version" code.

## [2.0.0] - 2010-03-27
- Update to GTK 2.0.
- Inclusion of Debian patches for desktop integration and bug fixes.
