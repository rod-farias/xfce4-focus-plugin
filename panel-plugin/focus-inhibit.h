#ifndef __FOCUS_INHIBIT_H__
#define __FOCUS_INHIBIT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define FOCUS_TYPE_INHIBIT (focus_inhibit_get_type ())
G_DECLARE_FINAL_TYPE (FocusInhibit, focus_inhibit, FOCUS, INHIBIT, GObject)

FocusInhibit *focus_inhibit_new (void);

/* Requests (or releases) both the screensaver/idle-lock inhibit and the
 * power-manager idle-blank/suspend inhibit. Returns FALSE if activating
 * failed on every backend (e.g. neither xfce4-screensaver/light-locker
 * nor xfce4-power-manager is reachable on the session bus); deactivating
 * always succeeds. */
gboolean focus_inhibit_set_active (FocusInhibit *inhibit,
                                    gboolean      active);

gboolean focus_inhibit_get_active (FocusInhibit *inhibit);

G_END_DECLS

#endif /* !__FOCUS_INHIBIT_H__ */
