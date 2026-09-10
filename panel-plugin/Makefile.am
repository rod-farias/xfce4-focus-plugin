plugindir = $(libdir)/xfce4/panel/plugins

plugin_LTLIBRARIES = libfocus.la

libfocus_la_SOURCES = \
	focus.c \
	focus.h \
	focus-inhibit.c \
	focus-inhibit.h \
	focus-dnd.c \
	focus-dnd.h

libfocus_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCONF_CFLAGS) \
	-DG_LOG_DOMAIN=\"focus\"

libfocus_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_construct$$'

libfocus_la_LIBADD = \
	$(GTK_LIBS) \
	$(GLIB_LIBS) \
	$(LIBXFCE4PANEL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCONF_LIBS)
