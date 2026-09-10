SUBDIRS = panel-plugin

desktopdir = $(datadir)/xfce4/panel/plugins
desktop_DATA = focus.desktop

iconsdir = $(datadir)/icons/hicolor/scalable/status
icons_DATA = \
	icons/hicolor/scalable/status/focus-eye-open-symbolic.svg \
	icons/hicolor/scalable/status/focus-eye-closed-symbolic.svg

EXTRA_DIST = \
	$(desktop_DATA) \
	$(icons_DATA)

.PHONY: ChangeLog

ChangeLog: Makefile
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'Git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog
