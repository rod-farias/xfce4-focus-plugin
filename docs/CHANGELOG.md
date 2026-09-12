# Changelog

All notable changes to this project are documented in this file.

## [0.5.2] - 2026-09-11

### Fixes

- Dropdown arrow was invisible on a vertical panel — the button row now re-orients (and swaps the arrow glyph) to match the panel's orientation.
- The plugin is now marked unique (`X-XFCE-Unique=true`), so only one instance can run across all panels at once instead of independent instances fighting over the same D-Bus inhibit and Do Not Disturb setting.
- Do Not Disturb no longer stays on forever if the plugin is removed from the panel while active.
- Fixed a use-after-free crash on plugin teardown, caused by releasing a reference to xfconf's shared channel object that this plugin never owned.
- The icon shown in "Add New Items…" and Panel Preferences is redrawn with bolder linework so it no longer collapses into an unrecognizable blob at the small size those dialogs use.
- The panel icon (and the dropdown's "no bell" glyph) no longer go stale after a live light/dark theme switch.
- The active-state icon no longer shows the current theme's default checked-button background (a mismatched box on light themes) behind the eye.

### Docs

- README.md trimmed to a short overview; full details moved to [docs/DETAILS.md](docs/DETAILS.md), with screenshots added.

## [0.5.1] - 2026-09-10

First fully functional release.


