# GTimer

GTimer is a lightweight, modern time tracking application for the GNOME desktop, built with GTK 4 and Libadwaita. It allows you to track time spent on various tasks, organize them into projects, and generate detailed reports.

![GTimer](data/icons/hicolor/scalable/apps/us.k5n.GTimer.svg)

## Features

- **Simplified Tracking**: Uses a daily-total model for robust and easy management.
- **Modern UI**: Clean, Libadwaita-based interface with a secondary action bar for quick access.
- **Real-time Search**: Quickly filter your task list by task or project name.
- **Background Notifications**: Stay informed of active timers even when the app is in the background.
- **Idle Detection**: Automatically pauses timers when you're away (supports Wayland and X11).
- **Auto-save & Rollover**: Automatically saves data every minute and handles midnight transitions seamlessly.
- **Flexible Reports**: Generate Plain Text or HTML reports for any time period (daily, weekly, monthly, yearly).
- **Keyboard Friendly**: Full support for GNOME-compliant keyboard shortcuts.

## Building and Running

### Prerequisites

- GTK 4
- Libadwaita 1.1 or later
- SQLite 3
- Meson and Ninja
- (Optional) libXss for X11 idle detection fallback

### Build

```bash
meson setup build
meson compile -C build
```

### Run Locally

For the application to find its GSettings schema when running from the build directory:

```bash
GSETTINGS_SCHEMA_DIR=data ./build/src/gtimer
```

### Install

```bash
sudo meson install -C build
```

## License

GTimer is released under the GPL-2.0 License.
