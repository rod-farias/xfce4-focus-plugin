#include <xfconf/xfconf.h>

#include "focus-dnd.h"

#define NOTIFYD_CHANNEL_NAME "xfce4-notifyd"
#define NOTIFYD_PROPERTY     "/do-not-disturb"

enum
{
  SIGNAL_CHANGED,
  N_SIGNALS,
};

static guint dnd_signals[N_SIGNALS] = { 0, };

struct _FocusDnd
{
  GObject parent_instance;

  XfconfChannel *channel;
  gboolean       owns_xfconf_init;
};

G_DEFINE_TYPE (FocusDnd, focus_dnd, G_TYPE_OBJECT)

static void
property_changed_cb (XfconfChannel *channel,
                      const gchar   *property,
                      const GValue  *value,
                      FocusDnd      *self)
{
  if (g_strcmp0 (property, NOTIFYD_PROPERTY) == 0)
    g_signal_emit (self, dnd_signals[SIGNAL_CHANGED], 0);
}

void
focus_dnd_set_active (FocusDnd *self,
                       gboolean  active)
{
  g_return_if_fail (FOCUS_IS_DND (self));

  if (self->channel == NULL)
    return;

  xfconf_channel_set_bool (self->channel, NOTIFYD_PROPERTY, !!active);
}

gboolean
focus_dnd_get_active (FocusDnd *self)
{
  g_return_val_if_fail (FOCUS_IS_DND (self), FALSE);

  if (self->channel == NULL)
    return FALSE;

  return xfconf_channel_get_bool (self->channel, NOTIFYD_PROPERTY, FALSE);
}

static void
focus_dnd_dispose (GObject *object)
{
  FocusDnd *self = FOCUS_DND (object);

  g_clear_object (&self->channel);

  if (self->owns_xfconf_init)
    {
      xfconf_shutdown ();
      self->owns_xfconf_init = FALSE;
    }

  G_OBJECT_CLASS (focus_dnd_parent_class)->dispose (object);
}

static void
focus_dnd_class_init (FocusDndClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = focus_dnd_dispose;

  dnd_signals[SIGNAL_CHANGED] =
    g_signal_new ("changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
focus_dnd_init (FocusDnd *self)
{
  GError *error = NULL;

  if (!xfconf_init (&error))
    {
      g_warning ("focus: could not initialize xfconf: %s", error->message);
      g_clear_error (&error);
      return;
    }

  self->owns_xfconf_init = TRUE;

  self->channel = xfconf_channel_get (NOTIFYD_CHANNEL_NAME);
  g_signal_connect (self->channel, "property-changed", G_CALLBACK (property_changed_cb), self);
}

FocusDnd *
focus_dnd_new (void)
{
  return g_object_new (FOCUS_TYPE_DND, NULL);
}
