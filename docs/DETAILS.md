# xfce4-focus-plugin

A native C/GTK3 panel plugin for the Xfce panel that gives you a single
"Focus mode" you can start for a fixed duration, combining two
optional effects:

- **Stay Awake** — temporarily inhibits screen locking, screen
  blanking (DPMS) and automatic suspend, the same idea as GNOME's
  *Caffeine*. It works by requesting an inhibit over D-Bus from
  whichever of **xfce4-screensaver**/**light-locker** and
  **xfce4-power-manager** are running in the session — no single
  service handles all three on its own, so both are inhibited
  independently and the plugin keeps working even if only one of them
  is present.
- **Do Not Disturb** — temporarily mutes notifications, by toggling
  **xfce4-notifyd**'s own "Do Not Disturb" xfconf setting — the exact
  same switch its own preferences dialog uses.

> **⚠ Early development.** This plugin is under active development,
> has only been tested on a single machine/configuration (Ubuntu
> 26.04, Xfce 4.20), and hasn't had a stable release yet.

## The panel button

A single eye icon: open (with lashes) while Focus mode is active,
closed while it's off. Click it to toggle the whole thing on or off
using whatever is currently selected in the dropdown (see below). A
small arrow next to it opens that dropdown without touching the
running state.

## The dropdown

- **Stay Awake** / **Do Not Disturb** checkboxes, each with its own
  icon (a steaming cup for Stay Awake, a crossed-out bell for Do Not
  Disturb). These only *select* what the next activation will include
  — checking or unchecking one has no immediate effect on your screen
  or notifications. At least one of the two must stay checked;
  unchecking the last one flips the other back on instead.
- A **Do Not Disturb** row also shows a warning icon if notifications
  get re-enabled from outside the plugin (its own settings dialog,
  `xfconf-query`, another instance of this plugin, …) while Focus mode
  still expects them off. It's purely informational — Focus mode never
  fights that change back — and clears itself on Stop, on Restart, or
  if the checkbox is unchecked.
- A **duration scale** with 5 stops — Forever, 15m, 30m, 1h, 2h — that
  snaps to the nearest one as you drag it. The selected stop is shown
  in bold underneath.
- A live **"Time remaining: hh:mm:ss"** countdown, shown only while
  Focus mode is active with a duration other than Forever.
- **Cancel** / **Stop** (left) and **Start** / **Restart** (right),
  same width, centered. Which pair of labels shows depends on whether
  Focus mode is currently active:
  - **Off:** *Cancel* discards any checkbox/duration change made since
    the dropdown was opened (without touching anything live); *Start*
    activates with the current selection.
  - **Active:** *Stop* deactivates immediately; *Restart* re-applies
    the currently checked features and resets the countdown to the
    full selected duration, without deactivating first.
- **Continue**, shown only while active, just closes the dropdown
  without changing anything — the same as pressing Esc or clicking
  outside it.

Deactivation (of everything, regardless of which checkboxes are
selected) always happens the same three ways: clicking the main panel
button while active, clicking Stop, or the duration timer reaching
zero.

## Persistence

The checkbox selection and duration are remembered across panel
restarts and logins. If Focus mode is active when the panel exits, it
resumes automatically (re-requesting the inhibit/DND state) the next
time the panel starts — the inhibit itself is only ever held live over
D-Bus, so this re-request is what makes it survive a panel restart in
practice.

## Requirements

Runtime: a desktop session with a D-Bus session bus. Stay Awake needs
at least one of xfce4-screensaver, light-locker or
xfce4-power-manager running to have any effect; Do Not Disturb needs
xfce4-notifyd (the default Xfce notification daemon). If Stay Awake
can't reach either service when you try to start it, a dialog explains
that instead of silently doing nothing.

Build dependencies are listed under each build option below —
installing from a `.deb` (Option A) does *not* require them on the
machine doing the install, only on the one building the package.

## Building and installing

### Option A: build a `.deb` package

**A `.deb` is tied to the system it was built on** (same distro
release, same architecture, compatible Xfce version). For any target
that doesn't match, use Option B and build on that machine instead.

```bash
sudo apt update && sudo apt install -y \
  build-essential debhelper pkgconf libglib2.0-dev libgtk-3-dev \
  libxfce4panel-2.0-dev libxfce4ui-2-dev libxfconf-0-dev xfce4-dev-tools
dpkg-buildpackage -us -uc -b
sudo apt install ../xfce4-focus-plugin_<version>_amd64.deb
```

If the `.deb` lives under your home directory, `apt`'s `_apt` sandbox
user may not be able to read it (a `700`-permission home directory is
common) — copy it to `/tmp` first if `apt` reports a permission error
acquiring the file.

### Option B: build and install from source directly

```bash
sudo apt update && sudo apt install -y \
  build-essential pkgconf libglib2.0-dev libgtk-3-dev \
  libxfce4panel-2.0-dev libxfce4ui-2-dev libxfconf-0-dev xfce4-dev-tools
./autogen.sh
./configure --prefix=/usr
make
sudo make install
```

Either way, `sudo`/root is unavoidable at install time: the panel only
looks for plugin modules in its system plugin directory (e.g.
`/usr/lib/<triplet>/xfce4/panel/plugins/`), never in any per-user
directory.

### After installing (either option)

Restart the panel:

```bash
xfce4-panel -r
```

Then right-click the panel → **Panel** → **Add New Items…** and add
"Focus".

## Icons

The main button's open/closed eye icons are derived directly from
hand-supplied reference artwork: each pixel's inverted luminance
becomes an alpha channel (the source images are opaque, not
transparent), embedded as small PNGs and recolored at render time to
the current foreground color, so they still adapt to light/dark panel
themes. The "Do Not Disturb" bell icon is a small hand-drawn SVG,
rendered the same way. The "Stay Awake" cup icon uses the active icon
theme's own `caffeine-cup-full-symbolic`, when available.

## Known limitations

- Stay Awake has no effect on a session that runs neither a
  freedesktop/Xfce screensaver service nor xfce4-power-manager (e.g. a
  minimal Xfce install with both removed).
- The D-Bus inhibit and the duration countdown are only held for as
  long as the panel process is running: neither can, or does, survive
  a full logout or reboot on their own — only the *selection* (which
  checkboxes, which duration) and whether Focus mode was left active
  are what actually get persisted and re-applied on the next start.
- The eye icon artwork is a placeholder — good enough to tell the two
  states apart, but expected to be replaced with something better
  later.
