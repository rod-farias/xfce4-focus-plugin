#ifndef __FOCUS_H__
#define __FOCUS_H__

#include <libxfce4panel/libxfce4panel.h>

#include "focus-inhibit.h"
#include "focus-dnd.h"

G_BEGIN_DECLS

typedef struct _FocusPlugin FocusPlugin;

struct _FocusPlugin
{
  XfcePanelPlugin *plugin;

  FocusInhibit *inhibit;
  FocusDnd     *dnd;

  GtkWidget *ebox;
  GtkWidget *box;
  GtkWidget *btn_stay_awake;
  GtkWidget *img_stay_awake;
  GtkWidget *btn_arrow;
  GtkWidget *img_arrow;
  GtkWidget *popup_window;

  /* the dropdown's own controls -- the checkboxes are only a selection
   * of what to include next time Focus mode is (re)started, they do
   * not themselves turn the underlying features on or off; see
   * focus_activate()/focus_deactivate(). */
  GtkWidget *check_stay_awake;
  GtkWidget *check_dnd;
  /* shown next to check_dnd when Do Not Disturb is supposed to be on
   * (session active and the box checked) but notifications were
   * actually re-enabled from outside this plugin -- purely
   * informational, it does not fight that change back; see
   * focus_update_dnd_warning(). */
  GtkWidget *icon_dnd_warning;
  /* the "no bell" glyph next to check_dnd -- like img_stay_awake, its
   * color is baked in at render time (see render_bell_disabled_pixbuf())
   * rather than following the theme automatically, so it needs the same
   * manual refresh on a theme change; see focus_style_updated_cb(). */
  GtkWidget *img_dnd_bell;
  GtkWidget *duration_scale;
  GtkWidget *duration_label;
  GtkWidget *duration_remaining_label;

  /* labels swap between Cancel/Start (while off) and Stop/Restart
   * (while active) -- see focus_update_popup_buttons(). */
  GtkWidget *btn_cancel;
  GtkWidget *btn_ok;

  /* only shown while active: closes the dropdown without changing
   * anything, same as Esc or clicking outside it. */
  GtkWidget *btn_continue;

  /* the checkbox/duration selection as it was when the dropdown was
   * last opened, so a Cancel click can discard whatever the user
   * changed since then without touching the live session. */
  gboolean snapshot_stay_awake;
  gboolean snapshot_dnd;
  gint     snapshot_duration_index;

  /* whether Focus mode (whichever of stay-awake/do-not-disturb was
   * selected) is currently actually applied -- toggled only by the
   * main panel button, the dropdown's Ok button, or the duration
   * timer expiring, never by the checkboxes themselves. */
  gboolean session_active;

  /* fires when the selected duration elapses, auto-deactivating; 0
   * (no timer) while stopped or while "Forever" is selected. */
  guint duration_timer_id;

  /* monotonic-clock deadline the above timer is counting down to (0 if
   * none), used only to render duration_remaining_label; and the 1Hz
   * ticker that keeps that label live while a deadline is set. */
  gint64 session_end_monotonic_us;
  guint  countdown_timer_id;
};

void focus_save (XfcePanelPlugin *plugin, FocusPlugin *fp);

G_END_DECLS

#endif /* !__FOCUS_H__ */
