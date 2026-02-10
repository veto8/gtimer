# GTimer UI Design Document

**For AI Coding Agents (Claude Code, Cursor, etc.)**
**Target: GTK 4 + Libadwaita, GNOME HIG compliant**

---

## 1. Overview

GTimer is a lightweight, native-feeling desktop time tracker for GNOME. Users create **tasks** (optionally organized under **projects**), start/stop timing them, manually adjust time, add annotations, hide tasks, and generate reports.

**Core philosophy**: Fast keyboard + mouse operation. Minimal friction for starting/stopping timing. Modern GNOME look and feel with Libadwaita.

**Key departures from legacy GTK 2 version:**
- AdwHeaderBar with hamburger menu replaces traditional menu bar
- Header bar actions replace traditional toolbar
- AdwToast replaces GtkStatusbar for transient messages
- Persistent footer label for "Today" total
- GtkPrintOperation replaces manual print command entry
- `xdg-open` replaces manual browser configuration
- No splash screen
- No manual version check (use AppStream/Flatpak)
- SVG/symbolic icons replace XPM bitmaps
- Keyboard shortcuts remapped to avoid GNOME conflicts

---

## 2. Main Window Layout

```
┌──────────────────────────────────────────────────────────────────┐
│ [▶ Start] [■ Stop All]    GTimer          [Search] [≡ Menu]     │ ← AdwHeaderBar
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  Project         │ Task                      │ Today   │ Total   │
│ ─────────────────┼───────────────────────────┼─────────┼──────── │
│ ● GTimer Dev     │ Application Development   │ 1:05:35 │ 29:18:33│ ← running (● indicator)
│   SysAdmin       │ Security                  │ 0:05:05 │ 25:41:13│
│   ...            │ ...                       │ ...     │ ...     │
│                                                                  │
│                  ┌─────────────────────────────┐                 │
│                  │ ✓ Task added               │                  │ ← AdwToast (transient)
│                  └─────────────────────────────┘                 │
├──────────────────────────────────────────────────────────────────┤
│                                               Today: 1:10:40    │ ← Footer label
└──────────────────────────────────────────────────────────────────┘
```

**Widget Hierarchy:**
```
AdwApplicationWindow
└── AdwToolbarView
    ├── AdwHeaderBar (top bar)
    │   ├── [Left]  Start/Stop toggle button + Stop All button
    │   ├── [Title] AdwWindowTitle ("GTimer" / subtitle: current task)
    │   └── [Right] Search button + Hamburger menu button
    ├── GtkBox (vertical, main content)
    │   ├── AdwToastOverlay
    │   │   └── GtkScrolledWindow
    │   │       └── GtkColumnView (task list)
    │   └── GtkBox (footer)
    │       └── GtkLabel ("Today: H:MM:SS", right-aligned)
    └── (no bottom bar — footer is inside content)
```

---

## 3. Header Bar

The header bar replaces both the legacy menu bar and toolbar. It uses `AdwHeaderBar`.

### Left Side — Primary Actions
| Widget | Icon | Label | Tooltip | Behavior |
|--------|------|-------|---------|----------|
| GtkButton | `media-playback-start-symbolic` / `media-playback-pause-symbolic` | — | "Start/Stop timing selected task" | Toggles timing on selected task. Icon changes based on whether selected task is running. |
| GtkButton | `media-playback-stop-symbolic` | — | "Stop all timers" | Stops all currently running tasks. Insensitive when nothing is timing. |

### Center — Title
- `AdwWindowTitle` with title "GTimer"
- Subtitle shows the currently-timed task name (if exactly one is running), or "N tasks running" if multiple, or blank if none

### Right Side — Secondary Actions
| Widget | Icon | Tooltip | Behavior |
|--------|------|---------|----------|
| GtkButton | `list-add-symbolic` | "New task" | Opens new task dialog |
| GtkButton | `document-edit-symbolic` | "Edit task" | Opens edit dialog for selected task |
| GtkButton | `edit-paste-symbolic` | "Annotate task" | Opens annotation dialog for selected task |
| GtkMenuButton | `open-menu-symbolic` | "Main menu" | Opens hamburger menu (see Section 4) |

Button sensitivity:
- Start button: insensitive if no task selected
- Stop All button: insensitive if nothing timing
- Edit/Annotate buttons: insensitive if no task selected

---

## 4. Hamburger Menu Structure

The hamburger menu uses `GMenuModel` with `GtkPopoverMenu`. Organized into sections separated by dividers.

```
┌──────────────────────────┐
│ Task                     │
│   Start Timing      Alt+S│
│   Stop Timing       Alt+X│
│   Stop All Timing   Alt+T│
│ ─────────────────────────│
│   Hide          Ctrl+Sh+H│
│   Unhide...     Ctrl+Sh+U│
│   Delete                 │
│ ─────────────────────────│
│ Time Adjustment          │
│   +1 Minute              │
│   +5 Minutes       Ctrl+I│
│   +30 Minutes            │
│   −1 Minute              │
│   −5 Minutes       Ctrl+D│
│   −30 Minutes            │
│   Set to Zero            │
│ ─────────────────────────│
│ Edit                     │
│   Cut Time         Ctrl+X│
│   Copy Time        Ctrl+C│
│   Paste Time       Ctrl+V│
│   Clear Buffer           │
│ ─────────────────────────│
│ Project                  │
│   New Project...         │
│   Edit Project...        │
│ ─────────────────────────│
│ Report                   │
│   Daily...               │
│   Weekly...              │
│   Monthly...             │
│   Yearly...              │
│ ─────────────────────────│
│ Preferences...           │
│ ─────────────────────────│
│ Keyboard Shortcuts       │
│ About GTimer             │
│ ─────────────────────────│
│ Save               Ctrl+S│
│ Quit               Ctrl+Q│
└──────────────────────────┘
```

**Notes:**
- "Keyboard Shortcuts" opens a `GtkShortcutsWindow` (standard GNOME pattern)
- "About GTimer" opens `AdwAboutDialog` (standard Libadwaita)
- "Preferences..." opens a `AdwPreferencesDialog` (see Section 12)
- Time adjustment and Edit (cut/copy/paste) are submenus or inline sections
- Legacy "Visit Website" and "View Changelog" move into the About dialog (standard GNOME pattern)

---

## 5. Right-Click Context Menu

Right-clicking a task row shows a `GtkPopoverMenu` with items identical to the Task section of the hamburger menu:

```
┌──────────────────────────┐
│ Start Timing        Alt+S│
│ Stop Timing         Alt+X│
│ Stop All Timing     Alt+T│
│ ─────────────────────────│
│ New Task...        Ctrl+N│
│ Edit Task...       Ctrl+E│
│ Annotate...   Ctrl+Sh+A  │
│ ─────────────────────────│
│ Hide           Ctrl+Sh+H │
│ Delete                   │
│ ─────────────────────────│
│ +1 Minute                │
│ +5 Minutes         Ctrl+I│
│ +30 Minutes              │
│ −1 Minute                │
│ −5 Minutes         Ctrl+D│
│ −30 Minutes              │
│ Set to Zero              │
│ ─────────────────────────│
│ Cut Time           Ctrl+X│
│ Copy Time          Ctrl+C│
│ Paste Time         Ctrl+V│
└──────────────────────────┘
```

Use the same `GAction` handlers as the hamburger menu.

---

## 6. Task List

The task list is the primary content area. Uses `GtkColumnView` with `GtkSingleSelection`.

### Columns

| Column  | Default Width | Justification | Sortable | Content |
|---------|---------------|---------------|----------|---------|
| Project | 150px         | Left          | Yes      | Project name. Running tasks show an indicator (see Visual States). |
| Task    | Expand (fill) | Left          | Yes      | Task name |
| Today   | 80px          | Right         | Yes      | Time spent today (`H:MM:SS`) |
| Total   | 80px          | Right         | Yes      | Cumulative all-time total (`H:MM:SS`) |

### Column Sort Behavior

Clicking a column header sorts the list. Clicking the same column again cycles through sort modes:

| Column  | 1st Click | 2nd Click | 3rd Click | 4th Click |
|---------|-----------|-----------|-----------|-----------|
| Project | Name A→Z  | Name Z→A  | Creation ↑ | Creation ↓ |
| Task    | Name A→Z  | Name Z→A  | Creation ↑ | Creation ↓ |
| Today   | Time ↓ (highest first) | Time ↑ | — | — |
| Total   | Time ↓ (highest first) | Time ↑ | — | — |

A toast message confirms the active sort (e.g., "Sorted by project name").

### Mouse Interactions

| Action | Behavior |
|--------|----------|
| Single click | Select row |
| Double click | Toggle timing on that task (start if stopped, stop if running). Does **not** stop other running tasks. |
| Right-click | Show context menu (see Section 5) |

### Row Visual States

| State | Visual Treatment |
|-------|-----------------|
| Stopped | Normal text, no indicator in Project column |
| Running | Accent-colored dot or animated spinner in Project column. Today time in accent color or bold. |
| Selected | Standard GTK selection highlight |

---

## 7. Footer

A simple `GtkBox` at the bottom of the content area, below the task list.

| Element | Alignment | Content |
|---------|-----------|---------|
| GtkLabel | Right | `"Today: H:MM:SS"` — total time across all tasks for today |

Updated every second while any task is running.

---

## 8. Toast Notifications

Transient messages use `AdwToast` via `AdwToastOverlay`. Auto-dismiss after ~3 seconds (Libadwaita default). No manual dismiss needed.

**Messages shown as toasts:**

| Event | Toast Text |
|-------|-----------|
| Task created | "Task added" |
| Task updated | "Task updated" |
| Task deleted | "Task removed" |
| Task hidden | "Task hidden" |
| Annotation added | "Annotation added" |
| Sort changed | "Sorted by {criterion}" |
| Cut time | "Cut/Paste buffer: HH:MM:SS" |
| Copy time | "Cut/Paste buffer: HH:MM:SS" |
| Paste time | "Cut/Paste buffer: HH:MM:SS" |
| Buffer cleared | "Cut/Paste buffer cleared" |
| Set to zero | "Cut/Paste buffer: HH:MM:SS" |
| File saved | "Data saved" |
| Decrement (at zero) | "Cannot decrement below zero" |

---

## 9. Keyboard Shortcuts

### Remapped Shortcuts (GNOME HIG compliant)

Legacy shortcuts that conflict with GNOME conventions have been remapped.

#### Application

| Action | Shortcut | Notes |
|--------|----------|-------|
| Save | `Ctrl+S` | |
| Quit | `Ctrl+Q` | |

#### Timer Control

| Action | Shortcut | Notes |
|--------|----------|-------|
| Start Timing | `Alt+S` | Start selected task |
| Stop Timing | `Alt+X` | Stop selected task |
| Stop All | `Alt+T` | Stop all running tasks |

#### Task Management

| Action | Shortcut | Legacy | Notes |
|--------|----------|--------|-------|
| New Task | `Ctrl+N` | `Ctrl+N` | Unchanged |
| Edit Task | `Ctrl+E` | `Ctrl+E` | Unchanged |
| Annotate | `Ctrl+Shift+A` | `Ctrl+A` | **Remapped** — `Ctrl+A` is "Select All" in GNOME |
| Hide Task | `Ctrl+Shift+H` | `Ctrl+H` | **Remapped** — `Ctrl+H` is "Find/Replace" in some GNOME apps |
| Unhide Tasks | `Ctrl+Shift+U` | `Ctrl+U` | **Remapped** for consistency with Hide |
| Delete Task | `Ctrl+Delete` | `Ctrl+R` | **Remapped** — `Ctrl+R` is "Reload" in GNOME convention. `Ctrl+Delete` is more intuitive. |

#### Time Adjustment

| Action | Shortcut | Legacy |
|--------|----------|--------|
| +1 Minute | `Shift+Ctrl+I` | Same |
| +5 Minutes | `Ctrl+I` | Same |
| +30 Minutes | `Ctrl+Alt+I` | Same |
| −1 Minute | `Shift+Ctrl+D` | Same |
| −5 Minutes | `Ctrl+D` | Same |
| −30 Minutes | `Ctrl+Alt+D` | Same |
| Set to Zero | `Ctrl+Alt+0` | Same |

#### Edit (Time Buffer)

| Action | Shortcut | Notes |
|--------|----------|-------|
| Cut Time | `Ctrl+X` | Cuts today's time to buffer, zeros task |
| Copy Time | `Ctrl+C` | Copies today's time to buffer |
| Paste Time | `Ctrl+V` | Adds buffer to selected task (does NOT clear buffer) |

**Note:** The GtkShortcutsWindow (accessible via hamburger menu → "Keyboard Shortcuts") displays all of these in a grouped, searchable window per GNOME convention.

---

## 10. Cut/Copy/Paste Time Buffer

A single global buffer stores time in seconds. This feature allows moving time between tasks.

| Operation | Buffer Effect | Task Effect | Toast |
|-----------|---------------|-------------|-------|
| Cut (`Ctrl+X`) | Set to task's today time | Task today → 0 | "Cut/Paste buffer: HH:MM:SS" |
| Copy (`Ctrl+C`) | Set to task's today time | No change | "Cut/Paste buffer: HH:MM:SS" |
| Paste (`Ctrl+V`) | Unchanged (allows repeat paste) | Add buffer to task today | "Cut/Paste buffer: HH:MM:SS" |
| Clear Buffer | Set to 0 | No change | "Cut/Paste buffer cleared" |
| Set to Zero (`Ctrl+Alt+0`) | Set to task's today time (before zero) | Task today → 0 | "Cut/Paste buffer: HH:MM:SS" |

**Implementation note:** Paste does NOT clear the buffer. This allows pasting the same time to multiple tasks.

---

## 11. Dialog Windows

All dialogs use Libadwaita patterns. Modal dialogs are `AdwDialog` subclasses.

### 11.1 New Task Dialog

- **Type:** `AdwDialog` (modal)
- **Title:** "New Task"
- **Fields:**
  - Project dropdown (`GtkDropDown`) — lists all projects, plus "None" option. Only shown if projects exist.
  - Task name entry (`AdwEntryRow`) — pre-filled with "Unnamed Task" (auto-selected for easy overwriting)
- **Buttons:** Cancel, Add (`suggested-action` style)
- **Behavior:** Trims trailing whitespace from input. Focus starts on name field.

### 11.2 Edit Task Dialog

- **Type:** `AdwDialog` (modal)
- **Title:** "Edit Task"
- **Fields:**
  - Project dropdown (`GtkDropDown`) — same as New Task
  - Task name entry (`AdwEntryRow`) — pre-filled with current name
- **Buttons:** Cancel, Save (`suggested-action` style)
- **Behavior:** Trims trailing whitespace. Focus starts on name field.

### 11.3 New Project Dialog

- **Type:** `AdwDialog` (modal)
- **Title:** "New Project"
- **Fields:**
  - Project name entry (`AdwEntryRow`) — pre-filled with "Unnamed Project" (auto-selected)
- **Buttons:** Cancel, Add (`suggested-action` style)

### 11.4 Edit Project Dialog

- **Type:** `AdwDialog` (modal)
- **Title:** "Edit Project"
- **Fields:**
  - Project name entry (`AdwEntryRow`) — pre-filled with current name
- **Buttons:** Cancel, Save (`suggested-action` style)

### 11.5 Annotate Dialog

- **Type:** `AdwDialog` (modal)
- **Title:** "Annotate: {Task Name}"
- **Fields:**
  - Multi-line text view (`GtkTextView`) with word wrapping, inside a scrolled window
- **Buttons:** Cancel, Save (`suggested-action` style)
- **Behavior:** Annotation stored with UTC timestamp.

### 11.6 Unhide Tasks Dialog

- **Type:** `AdwDialog` (modal)
- **Title:** "Unhide Tasks"
- **Content:**
  - List of hidden tasks with checkboxes, showing `[Project] Task Name`
  - All tasks pre-checked by default
- **Buttons:**
  - Cancel
  - Select All / Select None (toggle or dual buttons)
  - Unhide (`suggested-action` style) — unhides checked tasks

### 11.7 Delete Confirmation Dialog

- **Type:** `AdwAlertDialog`
- **Heading:** "Delete Task?"
- **Body:** "This will permanently delete '{Task Name}' and all its time entries and annotations."
- **Buttons:** Cancel, Delete (`destructive-action` style)

### 11.8 Report Configuration Dialog

- **Type:** `AdwDialog` (modal)
- **Title:** "Generate Report"
- **Fields:**

| Field | Widget | Options |
|-------|--------|---------|
| Report Type | `GtkDropDown` | Daily, Weekly, Monthly, Yearly |
| Time Range | `GtkDropDown` | Today, This Week, Last Week, This & Last Week, Last Two Weeks, This Month, Last Month, This Year, Last Year |
| Output Format | `GtkDropDown` | Plain Text, HTML |
| Include | `GtkDropDown` | Hours Worked, Annotations, Hours & Annotations |
| Rounding | `GtkDropDown` | None, 1 Minute, 5 Minutes, 10 Minutes, 15 Minutes, 30 Minutes, 1 Hour |
| Tasks | Checkboxes | Multi-select list of all tasks. All selected by default. |

- **Buttons:** Cancel, Generate (`suggested-action` style)

### 11.9 Report Display Window

- **Type:** `AdwWindow` (non-modal, separate window)
- **Title:** "Report"
- **Content:**
  - Non-editable `GtkTextView` in a `GtkScrolledWindow` displaying the report
  - Search bar (`GtkSearchBar` with `GtkSearchEntry`) — toggled with `Ctrl+F`
  - Search supports "Find Next" via `Enter` or `Ctrl+G`
- **Header bar buttons:**
  - Save (opens `GtkFileDialog` to save report to disk)
  - Print (uses `GtkPrintOperation` for system print dialog)
- **Default size:** 600x600

### 11.10 About Dialog

- **Type:** `AdwAboutDialog` (standard Libadwaita)
- **Properties:**
  - Application name: "GTimer"
  - Application icon: `org.craigknudsen.GTimer`
  - Version: current version string
  - Developer name and copyright
  - Website URL (opens via `xdg-open`)
  - License: GPL-2.0+
  - Release notes / changelog section (replaces legacy changelog window)

### 11.11 Idle Detection Prompt

- **Type:** `AdwAlertDialog`
- **Heading:** "Idle Detected"
- **Body:** "You have been idle for {N} minutes (since {HH:MM})."
- **Buttons (3 choices):**
  - **Revert** — Remove the idle time from all running tasks. Saves removed time to cut/paste buffer. Stops timing.
  - **Continue** — Ignore idle detection, keep accumulated time as-is. Continue timing.
  - **Resume** (`suggested-action` style) — Remove idle time from running tasks, save it to buffer, and resume timing from the point idle started.
- **Behavior:** Modal/blocking. Cannot be dismissed without choosing an option.

---

## 12. Preferences Dialog

Replaces the legacy Options menu toggles and browser configuration with a `AdwPreferencesDialog`.

### General Page

| Setting | Widget | Default | Notes |
|---------|--------|---------|-------|
| Auto Save | `AdwSwitchRow` | On | Automatically save data periodically |
| Animate Running Tasks | `AdwSwitchRow` | On | Show animated indicator for running tasks |
| Resume Timing on Startup | `AdwSwitchRow` | Off | Resume tasks that were timing when app was last closed |

### Idle Detection Page

| Setting | Widget | Default | Notes |
|---------|--------|---------|-------|
| Enable Idle Detection | `AdwSwitchRow` | On | Master toggle |
| Idle Threshold | `AdwSpinRow` | 15 minutes | Minutes of inactivity before idle prompt. Range: 1–120. |

### Time Page

| Setting | Widget | Default | Notes |
|---------|--------|---------|-------|
| Day Start (Midnight Offset) | `AdwSpinRow` | 00:00 | Hour offset for when "today" begins (e.g., 04:00 means the day rolls over at 4 AM). Useful for night-shift workers. |
| Week Starts On | `GtkDropDown` | Monday | First day of the week for weekly reports. Options: Monday through Sunday. |

---

## 13. Visual States & Icons

### Application Icon
- SVG icon: `org.craigknudsen.GTimer.svg`
- Follows GNOME icon guidelines (symbolic and full-color variants)

### Symbolic Icons (from Adwaita icon theme — do NOT bundle)
| Usage | Icon Name |
|-------|-----------|
| Start timing | `media-playback-start-symbolic` |
| Stop/Pause timing | `media-playback-pause-symbolic` |
| Stop all | `media-playback-stop-symbolic` |
| New task | `list-add-symbolic` |
| Edit task | `document-edit-symbolic` |
| Annotate | `edit-paste-symbolic` |
| Delete | `user-trash-symbolic` |
| Menu | `open-menu-symbolic` |
| Search | `system-search-symbolic` |

### Running Task Indicator
- A small colored dot or `GtkSpinner` in the Project column for running tasks
- If "Animate Running Tasks" is enabled in Preferences: use a spinning animation
- If disabled: use a static accent-colored dot
- Multiple tasks can run simultaneously, each showing its own indicator

### Window Title
- `AdwWindowTitle` title: "GTimer"
- `AdwWindowTitle` subtitle: current task name if one running, "{N} tasks running" if multiple, empty if none

---

## 14. Configuration & State Persistence

The following state is saved across sessions (stored in SQLite database or GSettings):

### Window State
| Property | Notes |
|----------|-------|
| Window width | Restore on next launch |
| Window height | Restore on next launch |
| Column widths | All 4 column widths (Project, Task, Today, Total) |
| Sort column | Which column is sorted |
| Sort direction | Ascending/descending |

### User Preferences (see Section 12)
| Property | Default |
|----------|---------|
| Auto save enabled | `true` |
| Animate enabled | `true` |
| Idle detection enabled | `true` |
| Idle threshold (minutes) | `15` |
| Midnight offset (hour) | `0` |
| Week start day | `1` (Monday) |
| Resume on startup | `false` |
| Last timed task IDs | (empty) — populated on close for resume feature |

---

## 15. Command-Line Options

| Option | Argument | Description |
|--------|----------|-------------|
| `-dir` | path | Specify data directory (overrides XDG default) |
| `-resume` | — | Resume timing tasks from last session |
| `-midnight` | HHMM | Set midnight offset (e.g., `0400` for 4 AM day boundary) |
| `-start` | taskname | Start timing the named task immediately |
| `-weekstart` | 0–6 | First day of week (0=Sunday, 1=Monday, ..., 6=Saturday) |
| `-v`, `--version` | — | Print version and exit |
| `-h`, `--help` | — | Print help and exit |

**Dropped from legacy:** `-nosplash` (no splash screen in modern version)

---

## 16. Report System Details

### Report Types

| Type | Grouping | Header Format |
|------|----------|---------------|
| Daily | By date | `MM/DD/YY DayName` |
| Weekly | By week | `Week MM/DD/YY to MM/DD/YY` |
| Monthly | By month | `Month MM/YYYY` |
| Yearly | By year | `Year YYYY` |

All reports end with a **Totals** section showing grand totals per task.

### Report Content

Each group shows:
- Task lines: `HH:MM:SS  [Project] Task Name`
- Group total: `Total: HH:MM:SS`
- If annotations included: indented annotation text with UTC timestamp

### Time Range Options

| Range | Description |
|-------|-------------|
| Today | Current day (respects midnight offset) |
| This Week | Current week (respects week start day) |
| Last Week | Previous week |
| This & Last Week | Both current and previous week |
| Last Two Weeks | Previous two complete weeks |
| This Month | Current calendar month |
| Last Month | Previous calendar month |
| This Year | Current calendar year |
| Last Year | Previous calendar year |

### Rounding Options

| Option | Behavior |
|--------|----------|
| None | Exact seconds |
| 1 Minute | Round to nearest minute |
| 5 Minutes | Round to nearest 5 minutes |
| 10 Minutes | Round to nearest 10 minutes |
| 15 Minutes | Round to nearest 15 minutes |
| 30 Minutes | Round to nearest 30 minutes |
| 1 Hour | Round to nearest hour |

### Output Formats

| Format | Description |
|--------|-------------|
| Plain Text | Monospace-formatted text suitable for terminal or text editor |
| HTML | Formatted HTML with tables, suitable for opening in a browser |

---

## 17. Idle Detection

### How It Works
1. A 1-second periodic check monitors user activity
2. On Wayland: uses `org.gnome.Mutter.IdleMonitor` D-Bus interface
3. On X11/XWayland: uses XScreenSaver extension (fallback)
4. If neither available: idle detection is disabled with a warning in Preferences

### Idle Flow
1. User goes idle while tasks are timing
2. After `idle_threshold` minutes of inactivity, user returns
3. Idle detection prompt appears (see Section 11.11)
4. User chooses Revert, Continue, or Resume
5. If Revert/Resume: idle time saved to cut/paste buffer

### Button Behavior

| Choice | Running Tasks | Cut/Paste Buffer | Timing After |
|--------|---------------|-------------------|--------------|
| Revert | Remove idle time, stop timing | Set to removed idle time | Stopped |
| Continue | Keep all time | Unchanged | Still running |
| Resume | Remove idle time, keep timing | Set to removed idle time | Still running |

---

## 18. Accessibility

Follow GNOME accessibility guidelines:

- All interactive widgets have accessible labels (`gtk_accessible_update_property`)
- Task list rows announce: "{Task Name}, {Project}, {Today Time}, {Total Time}, {Running/Stopped}"
- Header bar buttons have tooltips
- Full keyboard navigation: Tab through sections, arrow keys within list
- High contrast mode supported via Adwaita theming
- Screen reader compatible (ATK/AT-SPI2)
- Respect system font size and scaling settings

---

## 19. Dropped Legacy Features

| Feature | Reason | Replacement |
|---------|--------|-------------|
| Splash screen | Modern apps start instantly; GNOME HIG discourages splash screens | None |
| Browser configuration dialog | `xdg-open` handles URL opening correctly on all modern Linux | `xdg-open` |
| "Check for New Version" menu item | Flatpak/package managers handle updates; AppStream provides release info | AppStream metadata |
| Manual print command dialog | Outdated; requires user to know print commands | `GtkPrintOperation` (system print dialog) |
| Changelog window | Separate window for changelog is unusual in GNOME | Changelog in `AdwAboutDialog` release notes |
| XPM bitmap icons | Low-resolution, not scalable | SVG icons + Adwaita symbolic icons |
| Handle box (draggable menu bar) | Not available in GTK 4, not GNOME HIG | `AdwHeaderBar` |
| GtkStatusbar | Deprecated in GTK 4 | `AdwToast` + footer label |
| Traditional toolbar | Deprecated in GTK 4 | Header bar buttons |
| GtkStatusIcon (system tray) | Removed in GTK 4 | Background portal + persistent notification |

---

## 20. Implementation Notes for Coding Agents

1. **Target:** GTK 4.12+ and Libadwaita 1.4+. Use `AdwApplicationWindow`, `AdwHeaderBar`, `AdwDialog`, `AdwToast`, `AdwAboutDialog`, `AdwPreferencesDialog`.

2. **Task list:** Use `GtkColumnView` with `GtkColumnViewColumn` and `GtkSignalListItemFactory` for custom row rendering. Model: `GListStore` of `GTimerTask` GObjects.

3. **Menu system:** Use `GAction` + `GMenu` + `GMenuModel`. All actions registered on the `GtkApplication` or `GtkApplicationWindow` action maps. Same actions used by header bar buttons, hamburger menu, context menu, and keyboard shortcuts.

4. **Double-click behavior:** Toggle timing on the clicked task only. Do NOT auto-stop other running tasks. This was an explicit user decision.

5. **Context menu:** Use `GtkPopoverMenu` with the same `GMenuModel` as the Task section of the hamburger menu.

6. **Idle detection:** Abstract behind an interface. Implement Wayland backend (GNOME Mutter D-Bus), X11 fallback (XScreenSaver), and dummy (disabled) backend. Select automatically at runtime.

7. **Data storage:** SQLite database in `$XDG_DATA_HOME/gtimer/`. Backward-compatible import from legacy `~/.gtimer/` XML files.

8. **Print:** Use `GtkPrintOperation` for the system print dialog. No manual command entry.

9. **URLs:** Use `gtk_show_uri()` or `g_app_info_launch_default_for_uri()` which delegates to `xdg-open`.

10. **Icons:** Do NOT bundle Adwaita icons. Reference them by standard name. Only bundle the application icon SVG.

11. **Keyboard shortcuts:** Register accelerators via `gtk_application_set_accels_for_action()`. Show via `GtkShortcutsWindow`.

12. **Time format:** Always display as `H:MM:SS` (no leading zero on hours). Internal storage in seconds.

13. **Internationalization:** Use `gettext()` for all user-visible strings. Support UTF-8 throughout.

14. **Dark mode:** Automatic via Libadwaita/`AdwStyleManager`. No special handling needed.

15. **Notifications:** When timer is running and window is minimized/closed, show a persistent notification with elapsed time. Use `GNotification` API.
