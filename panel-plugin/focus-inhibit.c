#include <gio/gio.h>

#include "focus-inhibit.h"

#define FOCUS_APP_NAME "Focus panel plugin"
#define FOCUS_REASON   "Requested by the user from the Focus panel plugin"

#define DBUS_CALL_TIMEOUT_MS 2000

#define POWER_BUS_NAME    "org.freedesktop.PowerManagement"
#define POWER_OBJECT_PATH "/org/freedesktop/PowerManagement/Inhibit"
#define POWER_INTERFACE   "org.freedesktop.PowerManagement.Inhibit"

/* Different screensaver daemons answer to different bus names/object
 * paths/interfaces for what is otherwise the same Inhibit(app, reason)
 * -> cookie / UnInhibit(cookie) pair:
 *  - light-locker, gnome-screensaver, mate-screensaver, cinnamon-screensaver
 *    all implement the classic freedesktop.org name.
 *  - xfce4-screensaver (the current default on Xfce >= 4.16) instead
 *    answers to its own org.xfce.ScreenSaver name/interface, though in
 *    practice it exports the object at both the freedesktop and the
 *    Xfce-specific object path.
 * Try each until one actually answers, and remember which one worked
 * for the matching UnInhibit call. */
typedef struct
{
  const gchar *bus_name;
  const gchar *object_path;
  const gchar *interface_name;
} ScreensaverBackend;

static const ScreensaverBackend screensaver_backends[] = {
  { "org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver" },
  { "org.xfce.ScreenSaver",        "/org/freedesktop/ScreenSaver", "org.xfce.ScreenSaver" },
  { "org.xfce.ScreenSaver",        "/org/xfce/ScreenSaver",        "org.xfce.ScreenSaver" },
};

#define N_SCREENSAVER_BACKENDS (G_N_ELEMENTS (screensaver_backends))

struct _FocusInhibit
{
  GObject parent_instance;

  GDBusConnection *connection;

  gboolean active;

  gboolean screensaver_inhibited;
  gint     screensaver_backend_index;
  guint32  screensaver_cookie;

  gboolean power_inhibited;
  guint32  power_cookie;
};

G_DEFINE_TYPE (FocusInhibit, focus_inhibit, G_TYPE_OBJECT)

static gboolean
call_inhibit (GDBusConnection *connection,
              const gchar     *bus_name,
              const gchar     *object_path,
              const gchar     *interface_name,
              guint32         *cookie_out)
{
  GVariant *result;
  GError *error = NULL;

  result = g_dbus_connection_call_sync (connection,
                                         bus_name,
                                         object_path,
                                         interface_name,
                                         "Inhibit",
                                         g_variant_new ("(ss)", FOCUS_APP_NAME, FOCUS_REASON),
                                         G_VARIANT_TYPE ("(u)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         DBUS_CALL_TIMEOUT_MS,
                                         NULL,
                                         &error);

  if (result == NULL)
    {
      g_debug ("focus: Inhibit on %s failed: %s", interface_name, error->message);
      g_clear_error (&error);
      return FALSE;
    }

  g_variant_get (result, "(u)", cookie_out);
  g_variant_unref (result);

  return TRUE;
}

static void
call_uninhibit (GDBusConnection *connection,
                const gchar     *bus_name,
                const gchar     *object_path,
                const gchar     *interface_name,
                guint32          cookie)
{
  GVariant *result;
  GError *error = NULL;

  result = g_dbus_connection_call_sync (connection,
                                         bus_name,
                                         object_path,
                                         interface_name,
                                         "UnInhibit",
                                         g_variant_new ("(u)", cookie),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         DBUS_CALL_TIMEOUT_MS,
                                         NULL,
                                         &error);

  if (result == NULL)
    {
      g_debug ("focus: UnInhibit on %s failed: %s", interface_name, error->message);
      g_clear_error (&error);
      return;
    }

  g_variant_unref (result);
}

static gboolean
try_screensaver_inhibit (FocusInhibit *self)
{
  guint i;

  for (i = 0; i < N_SCREENSAVER_BACKENDS; i++)
    {
      const ScreensaverBackend *backend = &screensaver_backends[i];

      if (call_inhibit (self->connection, backend->bus_name, backend->object_path,
                         backend->interface_name, &self->screensaver_cookie))
        {
          self->screensaver_backend_index = (gint) i;
          self->screensaver_inhibited = TRUE;
          return TRUE;
        }
    }

  return FALSE;
}

static void
screensaver_uninhibit (FocusInhibit *self)
{
  const ScreensaverBackend *backend = &screensaver_backends[self->screensaver_backend_index];

  call_uninhibit (self->connection, backend->bus_name, backend->object_path,
                   backend->interface_name, self->screensaver_cookie);

  self->screensaver_inhibited = FALSE;
}

static gboolean
try_power_inhibit (FocusInhibit *self)
{
  if (!call_inhibit (self->connection, POWER_BUS_NAME, POWER_OBJECT_PATH,
                      POWER_INTERFACE, &self->power_cookie))
    return FALSE;

  self->power_inhibited = TRUE;
  return TRUE;
}

static void
power_uninhibit (FocusInhibit *self)
{
  call_uninhibit (self->connection, POWER_BUS_NAME, POWER_OBJECT_PATH,
                   POWER_INTERFACE, self->power_cookie);

  self->power_inhibited = FALSE;
}

gboolean
focus_inhibit_set_active (FocusInhibit *self,
                           gboolean      active)
{
  g_return_val_if_fail (FOCUS_IS_INHIBIT (self), FALSE);

  active = !!active;

  if (active == self->active)
    return TRUE;

  if (active)
    {
      gboolean screensaver_ok;
      gboolean power_ok;

      if (self->connection == NULL)
        {
          g_warning ("focus: no D-Bus session bus connection available");
          return FALSE;
        }

      screensaver_ok = try_screensaver_inhibit (self);
      power_ok = try_power_inhibit (self);

      if (!screensaver_ok && !power_ok)
        {
          g_warning ("focus: could not inhibit screen lock or power management "
                     "-- no compatible service answered on the session bus");
          return FALSE;
        }

      if (!screensaver_ok)
        g_warning ("focus: no screensaver service answered -- screen may still lock, "
                   "but idle blanking/suspend is inhibited");
      else if (!power_ok)
        g_warning ("focus: xfce4-power-manager did not answer -- screen lock is inhibited, "
                   "but idle blanking/suspend may still occur");

      self->active = TRUE;
    }
  else
    {
      if (self->screensaver_inhibited)
        screensaver_uninhibit (self);

      if (self->power_inhibited)
        power_uninhibit (self);

      self->active = FALSE;
    }

  return TRUE;
}

gboolean
focus_inhibit_get_active (FocusInhibit *self)
{
  g_return_val_if_fail (FOCUS_IS_INHIBIT (self), FALSE);

  return self->active;
}

static void
focus_inhibit_dispose (GObject *object)
{
  FocusInhibit *self = FOCUS_INHIBIT (object);

  /* never leave the session with the screen permanently unlockable or
   * idle-suspend permanently disabled just because the panel plugin
   * went away. */
  focus_inhibit_set_active (self, FALSE);

  g_clear_object (&self->connection);

  G_OBJECT_CLASS (focus_inhibit_parent_class)->dispose (object);
}

static void
focus_inhibit_class_init (FocusInhibitClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = focus_inhibit_dispose;
}

static void
focus_inhibit_init (FocusInhibit *self)
{
  GError *error = NULL;

  self->connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
  if (self->connection == NULL)
    {
      g_warning ("focus: could not connect to the session bus: %s", error->message);
      g_clear_error (&error);
    }
}

FocusInhibit *
focus_inhibit_new (void)
{
  return g_object_new (FOCUS_TYPE_INHIBIT, NULL);
}
