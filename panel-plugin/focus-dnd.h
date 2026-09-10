#ifndef __FOCUS_DND_H__
#define __FOCUS_DND_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define FOCUS_TYPE_DND (focus_dnd_get_type ())
G_DECLARE_FINAL_TYPE (FocusDnd, focus_dnd, FOCUS, DND, GObject)

/* Wraps xfce4-notifyd's own "Do Not Disturb" setting (the xfconf
 * property that its preferences dialog and notification-area menu
 * already toggle), so muting notifications from this plugin behaves
 * exactly like muting them from xfce4-notifyd itself -- and the two
 * stay in sync no matter which one changed it. */

FocusDnd *focus_dnd_new (void);

void     focus_dnd_set_active (FocusDnd *dnd,
                                gboolean  active);
gboolean focus_dnd_get_active (FocusDnd *dnd);

/* emits "changed" (no arguments; call focus_dnd_get_active() to read the
 * new value) whenever the underlying setting changes, from any source --
 * this plugin's button, xfce4-notifyd's own dialog, or xfconf-query. */

G_END_DECLS

#endif /* !__FOCUS_DND_H__ */
