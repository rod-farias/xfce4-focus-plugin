#include <string.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>

#include "focus.h"

#define FOCUS_ICON_COFFEE         "caffeine-cup-full-symbolic"
#define FOCUS_ROW_ICON_SIZE       16
#define FOCUS_ARROW_ICON_SIZE     10

/* Base64-encoded 48x48 PNGs derived from the user-supplied eye3.png
 * (closed/inactive) and eye4.png (open/active) reference artwork.
 * Those originals are fully opaque -- a black icon on a white, not
 * transparent, background -- so each pixel's inverted luminance was
 * used as an alpha channel instead: a lossless silhouette extraction
 * from the same artwork, now with real transparency, saved back out
 * at a modest fixed size to keep this embed small. The RGB channels
 * are irrelevant (recolor_mask_pixbuf() overwrites them with the
 * current foreground color at render time), which is what makes this
 * adapt to light/dark panel themes despite starting from a
 * fixed-color raster instead of a real symbolic SVG. */
static const gchar *eye_closed_mask_base64 =
  "iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAEOUlEQVR42u2YXYhVVRTHf+fOHZuHcVQsepAgFR0/ksReLDPUqBwV"
  "8WFQsEZDSqKHIkpFRRh9kXpIBdPqpSxECn3xyQ9Q8WuIBC0ykTLQsRAqk7QZvXPvub6sJcvFOeeee+fOJHgWDOcy++v/X/u/1tp7"
  "Q2aZZZZZZpllllnNFgzi3OWBWKShjnPlZL5cDGBtDx6kHQgEWBkIXdsQQ6YPKMU4r/R/EWhwi48DngdmAK3AKKBR2nqBbuA8cAI4"
  "BfzhnFAarLjJGeKPAK8ChwRkOeXfP8C3wNyYeQfM8ub3cuDnlIDDhLaTwCu1xmVQpdZLwBRgGzDb9bkMHAN+An4HrgDXTfDmgQnA"
  "QqANaHHjdwMfANekb7Gega5E3wb+cx78DugAmquY8wlgFfCrm6sbmG92IqiH3lU6n8kiJfleAl6LWGQk8D5wALgAdAFbgaelvdH0"
  "bQZWAzcckTX1iAsFPxQ4KBP3yfdTYLhLmUgG+iVG6wUDLO/iaZwQ1n5lYEd/SOigYRJk6vkeYJkLag26acAtB7oo4/pMIK93ErFE"
  "NjkSX9YiJw3YPHDUZJFrwLMGeGDINpmM1CML73cZKDQ7+JzLODmz40uFgN+JfFrwOule4/luySBewzrpGwbkCtP+udkJ+z3sZIqb"
  "u03qipLoTEtCO3Qa8NeBiTETKICjAv4GMEJiohGY5YCH8lcAxlQgscDJqb1SndCGl81iPU42UTWkBfjTLNbqip0NfkumPcGrSqLD"
  "YLkJjI4hfS/XPy4FKZQt7EhYRCcZI8BDI49JwEzgN/l/yRBQMu9WkIWS2GzGH5F4y8VJZ5lhfDZC81EExhpQYYpjhfZ9rwIBreBN"
  "wN+GxNSoXdDMMxQ4Z8CsSiChEhouC5Sdt8MYQiqhJQkEArPmF2bsHiHVkBQD04A7ZqFFCSTUC6cFbDHFTmi/8XF6NmutM+OuAo8m"
  "jLnPG2+ZoOwF5sSQ0P7vRARrOcb7oRTHSuAthpLEVKrTqoLaakDdiiERmIp91UkkyvOaEtsiwNiqvFL63ZHvm7UWs12GxG1gcUTl"
  "1L4vGumo1zQmimZ3tkWAt7/XOvBrqgHvgzoHfOMKSqfbLUu4XW5bcRLa7s421uvNwFcO/KZawHsSgTkSFEyub3VEVFpPAluAH0VW"
  "l4B9wEsuPVqvvyAXISvB1f0B70n4jFAG/gU2yNEh6sqpabkp4tiNIbvTFbqbFVJsTSTUW/OkUvtb1EapvmltOvBJxEXme3PxSR2w"
  "1Vzoi8BjwEfA6669AJwBjgM/iHz+knEjxdvPSDqc6sb2Ah+L5gsRTzYD8pI3y9yiko4NpYT228DXwFMRxXFA3zwtkRlyX+6u4l3o"
  "IvAhMNk5J6gFTH92wz4ptsgxZLrEwyg5J/VJwF+WTNMl561CzDyDbg01PhLn6yGXoM7S0rph5eLTsT3sZZZZZg+73QVX5K1vnMHf"
  "WwAAAABJRU5ErkJggg==";

static const gchar *eye_open_mask_base64 =
  "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";

#define FOCUS_DURATION_COUNT 5

/* mark text drawn on the scale itself (kept short); duration_labels is
 * what the bold status label below it shows for the same stop -- "∞"
 * (an escape rather than the literal character, so the source stays
 * plain ASCII) reads fine at the scale's small mark size, but the
 * label spells it out as "Forever" instead. Marks were tried as the
 * sole indicator (bold + explicit color via Pango markup on the
 * selected one) but GTK discards markup formatting -- color and font
 * weight both -- when it renders scale marks, replacing it with its
 * own CSS-derived attributes regardless (confirmed with a standalone
 * test window); a real GtkLabel isn't subject to that, so the label
 * is what actually shows which stop is selected. duration_seconds is
 * how long each stop runs before auto-deactivating (0 = never, i.e.
 * "Forever"). */
static const gchar *duration_marks[FOCUS_DURATION_COUNT] = { "\xe2\x88\x9e", "15m", "30m", "1h", "2h" };
static const gchar *duration_labels[FOCUS_DURATION_COUNT] = { "Forever", "15m", "30m", "1h", "2h" };
static const guint duration_seconds[FOCUS_DURATION_COUNT] = { 0, 15 * 60, 30 * 60, 60 * 60, 120 * 60 };

/* No stock icon theme ships a "disabled bell" glyph, so this one is
 * drawn here and rasterized at runtime (via GdkPixbufLoader) instead of
 * being installed as a themed icon -- keeps it self-contained in the
 * source, no icon file or install-time build changes needed. "%s" is
 * replaced with the current foreground color so it still matches
 * light/dark themes despite not being a real recolorable symbolic icon. */
static const gchar *bell_disabled_svg_template =
  "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" viewBox=\"0 0 16 16\">"
  "<g fill=\"%s\">"
  "<circle cx=\"8\" cy=\"1.4\" r=\"0.6\"/>"
  "<path d=\"M 8,2.2 C 6,2.2 4.6,3.9 4.6,6.3 L 4.6,8.6 C 4.6,9.8 4.1,10.9 3.3,11.6 L 12.7,11.6"
  " C 11.9,10.9 11.4,9.8 11.4,8.6 L 11.4,6.3 C 11.4,3.9 10,2.2 8,2.2 Z\"/>"
  "<rect x=\"3\" y=\"11.8\" width=\"10\" height=\"1.1\" rx=\"0.55\"/>"
  "<circle cx=\"8\" cy=\"14.1\" r=\"1.05\"/>"
  "<rect width=\"1.3\" height=\"20\" rx=\"0.6\" ry=\"0.6\" x=\"-0.65\" y=\"1\" transform=\"rotate(-45)\"/>"
  "</g></svg>";

static gint
current_duration_index (FocusPlugin *fp)
{
  return CLAMP ((gint) (gtk_range_get_value (GTK_RANGE (fp->duration_scale)) + 0.5), 0, FOCUS_DURATION_COUNT - 1);
}

/* defined further down; focus_load_settings() needs to (re)activate on
 * startup before the rest of the file has introduced them */
static void focus_activate (FocusPlugin *fp);
static void focus_update_main_button_ui (FocusPlugin *fp);
static void focus_update_countdown_label (FocusPlugin *fp);
static void focus_update_dnd_warning (FocusPlugin *fp);
static gchar *get_foreground_hex (GtkWidget *widget);
static void focus_update_stay_awake_icon (FocusPlugin *fp);

static void
focus_load_settings (FocusPlugin *fp)
{
  gchar *file;
  XfceRc *rc;
  gboolean pref_stay_awake = TRUE;
  gboolean pref_dnd = TRUE;
  gint duration_index = 0;
  gboolean was_active = FALSE;

  file = xfce_panel_plugin_save_location (fp->plugin, TRUE);
  if (file != NULL)
    {
      rc = xfce_rc_simple_open (file, TRUE);
      g_free (file);

      if (rc != NULL)
        {
          pref_stay_awake = xfce_rc_read_bool_entry (rc, "PrefStayAwake", TRUE);
          pref_dnd = xfce_rc_read_bool_entry (rc, "PrefDnd", TRUE);
          duration_index = CLAMP (xfce_rc_read_int_entry (rc, "DurationIndex", 0), 0, FOCUS_DURATION_COUNT - 1);
          was_active = xfce_rc_read_bool_entry (rc, "SessionActive", FALSE);

          xfce_rc_close (rc);
        }
    }

  if (!pref_stay_awake && !pref_dnd)
    pref_stay_awake = TRUE;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fp->check_stay_awake), pref_stay_awake);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fp->check_dnd), pref_dnd);
  gtk_range_set_value (GTK_RANGE (fp->duration_scale), (gdouble) duration_index);

  if (was_active)
    focus_activate (fp);
  else
    focus_update_main_button_ui (fp);
}

void
focus_save (XfcePanelPlugin *plugin, FocusPlugin *fp)
{
  gchar *file;
  XfceRc *rc;

  file = xfce_panel_plugin_save_location (plugin, TRUE);
  if (file == NULL)
    return;

  rc = xfce_rc_simple_open (file, FALSE);
  g_free (file);

  if (rc == NULL)
    return;

  xfce_rc_write_bool_entry (rc, "PrefStayAwake", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_stay_awake)));
  xfce_rc_write_bool_entry (rc, "PrefDnd", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_dnd)));
  xfce_rc_write_int_entry (rc, "DurationIndex", current_duration_index (fp));
  xfce_rc_write_bool_entry (rc, "SessionActive", fp->session_active);

  xfce_rc_close (rc);
}

static void
main_button_toggled_cb (GtkToggleButton *button, FocusPlugin *fp);

static void
focus_update_main_button_ui (FocusPlugin *fp)
{
  focus_update_stay_awake_icon (fp);

  gtk_widget_set_tooltip_text (fp->btn_stay_awake,
                                fp->session_active
                                  ? "Focus mode is active\n(click to stop it)"
                                  : "Click to start Focus mode with the settings below");

  g_signal_handlers_block_by_func (fp->btn_stay_awake, main_button_toggled_cb, fp);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fp->btn_stay_awake), fp->session_active);
  g_signal_handlers_unblock_by_func (fp->btn_stay_awake, main_button_toggled_cb, fp);

  gtk_button_set_label (GTK_BUTTON (fp->btn_cancel), fp->session_active ? "Stop" : "Cancel");
  gtk_button_set_label (GTK_BUTTON (fp->btn_ok), fp->session_active ? "Restart" : "Start");
  gtk_widget_set_visible (fp->btn_continue, fp->session_active);

  focus_update_dnd_warning (fp);

  focus_update_countdown_label (fp);
}

/* Purely informational -- Focus mode never fights this back (that was
 * a deliberate choice, not an oversight: see the "reflejo pasivo" vs.
 * "reforzar el compromiso" discussion this was added from). Shown
 * whenever Do Not Disturb is supposed to be enforced right now
 * (session active and its checkbox checked) but the live xfconf value
 * disagrees, which can only mean something outside this plugin turned
 * it back off. Naturally hides itself again on Stop (session_active
 * clears), on Restart (re-asserts the checked state, making the live
 * value agree again) or if the checkbox itself gets unchecked. */
static void
focus_update_dnd_warning (FocusPlugin *fp)
{
  gboolean expected_on = fp->session_active && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_dnd));
  gboolean actual_on = focus_dnd_get_active (fp->dnd);

  gtk_widget_set_visible (fp->icon_dnd_warning, expected_on && !actual_on);
}

static void
focus_apply_icon_size (FocusPlugin *fp)
{
  focus_update_stay_awake_icon (fp);
}

static void
show_inhibit_failure_dialog (FocusPlugin *fp)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                    "Could not inhibit screen lock or power management.");
  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                             "Neither a screensaver service (xfce4-screensaver or "
                                             "light-locker) nor xfce4-power-manager answered on the "
                                             "session bus. Make sure at least one of them is running.");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void
focus_update_countdown_label (FocusPlugin *fp)
{
  gint64 remaining_us;
  gint64 remaining_seconds;
  gchar *text;

  if (!fp->session_active || fp->session_end_monotonic_us <= 0)
    {
      gtk_widget_hide (fp->duration_remaining_label);
      return;
    }

  remaining_us = fp->session_end_monotonic_us - g_get_monotonic_time ();
  /* round up so the display counts down to 00:00:00 rather than
   * jumping straight from 00:00:01 to hidden */
  remaining_seconds = remaining_us > 0 ? (remaining_us + G_USEC_PER_SEC - 1) / G_USEC_PER_SEC : 0;

  text = g_strdup_printf ("Time remaining: %02" G_GINT64_FORMAT ":%02" G_GINT64_FORMAT ":%02" G_GINT64_FORMAT,
                           remaining_seconds / 3600, (remaining_seconds % 3600) / 60, remaining_seconds % 60);
  gtk_label_set_text (GTK_LABEL (fp->duration_remaining_label), text);
  g_free (text);

  gtk_widget_show (fp->duration_remaining_label);
}

static gboolean
countdown_tick_cb (gpointer user_data)
{
  FocusPlugin *fp = user_data;

  focus_update_countdown_label (fp);

  if (!fp->session_active || fp->session_end_monotonic_us <= 0)
    {
      fp->countdown_timer_id = 0;
      return G_SOURCE_REMOVE;
    }

  return G_SOURCE_CONTINUE;
}

static void
focus_stop_countdown_ticker (FocusPlugin *fp)
{
  if (fp->countdown_timer_id != 0)
    {
      g_source_remove (fp->countdown_timer_id);
      fp->countdown_timer_id = 0;
    }
}

static void
focus_cancel_duration_timer (FocusPlugin *fp)
{
  if (fp->duration_timer_id != 0)
    {
      g_source_remove (fp->duration_timer_id);
      fp->duration_timer_id = 0;
    }

  focus_stop_countdown_ticker (fp);
  fp->session_end_monotonic_us = 0;
}

static gboolean focus_duration_expired_cb (gpointer user_data);

static void
focus_restart_duration_timer (FocusPlugin *fp)
{
  guint seconds = duration_seconds[current_duration_index (fp)];

  focus_cancel_duration_timer (fp);

  if (seconds > 0)
    {
      fp->session_end_monotonic_us = g_get_monotonic_time () + (gint64) seconds * G_USEC_PER_SEC;
      fp->duration_timer_id = g_timeout_add_seconds (seconds, focus_duration_expired_cb, fp);
      fp->countdown_timer_id = g_timeout_add_seconds (1, countdown_tick_cb, fp);
    }

  focus_update_countdown_label (fp);
}

/* Turns on whichever of "Stay Awake" / "Do Not Disturb" are currently
 * checked in the dropdown, for the currently selected duration. Called
 * from the main panel button, the dropdown's Ok button, and (to resume
 * across a panel restart) focus_load_settings() -- never from the
 * checkboxes themselves. */
static void
focus_activate (FocusPlugin *fp)
{
  gboolean want_stay_awake = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_stay_awake));
  gboolean want_dnd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_dnd));

  if (!focus_inhibit_set_active (fp->inhibit, want_stay_awake) && want_stay_awake)
    show_inhibit_failure_dialog (fp);

  focus_dnd_set_active (fp->dnd, want_dnd);

  focus_restart_duration_timer (fp);

  fp->session_active = TRUE;
  focus_update_main_button_ui (fp);
}

/* Turns everything off and cancels the duration timer. Called from the
 * main panel button (clicked while active) and when the timer expires. */
static void
focus_deactivate (FocusPlugin *fp)
{
  focus_cancel_duration_timer (fp);

  /* set before the D-Bus/xfconf calls below (rather than after, as the
   * mirror-image focus_activate() does) so that if FocusDnd's
   * "changed" signal is delivered synchronously from within
   * focus_dnd_set_active(), dnd_changed_cb() already sees a stopped
   * session and doesn't mistake our own deactivation for the "someone
   * re-enabled notifications behind Focus mode's back" case. */
  fp->session_active = FALSE;

  focus_inhibit_set_active (fp->inhibit, FALSE);
  focus_dnd_set_active (fp->dnd, FALSE);

  focus_update_main_button_ui (fp);
}

static gboolean
focus_duration_expired_cb (gpointer user_data)
{
  FocusPlugin *fp = user_data;

  fp->duration_timer_id = 0;
  focus_deactivate (fp);

  return G_SOURCE_REMOVE;
}

static void
main_button_toggled_cb (GtkToggleButton *button, FocusPlugin *fp)
{
  if (gtk_toggle_button_get_active (button))
    focus_activate (fp);
  else
    focus_deactivate (fp);
}

/* The checkboxes only select what focus_activate() will turn on next
 * time -- they never touch the inhibit/DND backends themselves. "At
 * least one selected" is still enforced here, purely as a UI rule:
 * unchecking the last checked one flips the other back on instead. */
static void
check_stay_awake_toggled_cb (GtkToggleButton *button, FocusPlugin *fp)
{
  if (!gtk_toggle_button_get_active (button) &&
      !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_dnd)))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fp->check_dnd), TRUE);
}

static void
check_dnd_toggled_cb (GtkToggleButton *button, FocusPlugin *fp)
{
  if (!gtk_toggle_button_get_active (button) &&
      !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_stay_awake)))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fp->check_stay_awake), TRUE);

  focus_update_dnd_warning (fp);
}

/* Do Not Disturb changed from any source -- our own focus_activate()/
 * focus_deactivate() included, which is exactly why this only updates
 * the warning icon (see focus_update_dnd_warning()) rather than the
 * checkbox: the checkbox stays a pure user-set preference, untouched
 * by what's actually happening live. */
static void
dnd_changed_cb (FocusDnd *dnd, FocusPlugin *fp)
{
  focus_update_dnd_warning (fp);
}

static void
duration_scale_changed_cb (GtkRange *range, FocusPlugin *fp)
{
  gint index = CLAMP ((gint) (gtk_range_get_value (range) + 0.5), 0, FOCUS_DURATION_COUNT - 1);
  gchar *markup = g_strdup_printf ("<b>%s</b>", duration_labels[index]);

  gtk_label_set_markup (GTK_LABEL (fp->duration_label), markup);
  g_free (markup);

  /* unlike the checkboxes, a new duration takes effect immediately
   * while active: it only controls how long the current session keeps
   * running, not which features are included, so there is no "wait
   * until Stop/expiry" reason to defer it. */
  if (fp->session_active)
    focus_restart_duration_timer (fp);
}

/* GtkScale drags continuously by default -- dropping the handle a
 * pixel short of a mark would otherwise leave it sitting between two
 * stops instead of snapping to the nearest one. "change-value" fires
 * with the raw proposed value for every kind of interaction (mouse
 * drag, click, keyboard, scroll) before it is applied; rounding it
 * here and applying that ourselves is what makes the slider behave as
 * a 5-stop control instead of a continuous one. */
static gboolean
duration_scale_change_value_cb (GtkRange *range, GtkScrollType scroll, gdouble value, FocusPlugin *fp)
{
  gint index = CLAMP ((gint) (value + 0.5), 0, FOCUS_DURATION_COUNT - 1);

  gtk_range_set_value (range, (gdouble) index);

  return TRUE;
}

static GtkWidget *
create_toggle_button (const gchar *tooltip, GCallback callback, FocusPlugin *fp, GtkWidget **image_out)
{
  GtkWidget *button;
  GtkWidget *image;

  button = gtk_toggle_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_set_focus_on_click (button, FALSE);
  gtk_widget_set_tooltip_text (button, tooltip);

  /* GTK_RELIEF_NONE only flattens the button while it behaves like an
   * ordinary, unchecked GtkButton -- once toggled on, most themes still
   * paint their normal ":checked" pressed-button background (a solid
   * or rounded highlight box) behind it regardless of relief style,
   * since that state is meant to read as "pressed". This button
   * already signals Focus mode's active state through the icon itself
   * (open vs. closed eye, see focus_update_stay_awake_icon()), so that
   * extra highlight box is redundant -- and on light themes in
   * particular it renders as a visibly mismatched beige/gray box
   * behind the icon rather than reading as part of the design. Strip
   * it explicitly, the same way focus-arrow-button already strips
   * padding from the arrow button below. */
  {
    GtkCssProvider *flat_css = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (flat_css,
                                      "button.focus-main-button,"
                                      "button.focus-main-button:checked,"
                                      "button.focus-main-button:active,"
                                      "button.focus-main-button:hover {"
                                      "  background: none;"
                                      "  background-image: none;"
                                      "  border: none;"
                                      "  box-shadow: none;"
                                      "}",
                                      -1, NULL);
    gtk_style_context_add_class (gtk_widget_get_style_context (button), "focus-main-button");
    gtk_style_context_add_provider (gtk_widget_get_style_context (button),
                                     GTK_STYLE_PROVIDER (flat_css),
                                     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (flat_css);
  }

  image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (button), image);

  g_signal_connect (button, "toggled", callback, fp);

  if (image_out != NULL)
    *image_out = image;

  return button;
}

static gchar *
get_foreground_hex (GtkWidget *widget)
{
  GtkStyleContext *style_context = gtk_widget_get_style_context (widget);
  GdkRGBA color;

  gtk_style_context_get_color (style_context, gtk_style_context_get_state (style_context), &color);

  return g_strdup_printf ("#%02x%02x%02x",
                           (gint) (CLAMP (color.red, 0.0, 1.0) * 255),
                           (gint) (CLAMP (color.green, 0.0, 1.0) * 255),
                           (gint) (CLAMP (color.blue, 0.0, 1.0) * 255));
}

static GdkPixbuf *
decode_base64_pixbuf (const gchar *base64)
{
  guchar *data;
  gsize len;
  GdkPixbufLoader *loader;
  GdkPixbuf *pixbuf = NULL;
  GError *error = NULL;

  data = g_base64_decode (base64, &len);

  loader = gdk_pixbuf_loader_new ();
  if (gdk_pixbuf_loader_write (loader, data, len, &error) &&
      gdk_pixbuf_loader_close (loader, &error))
    {
      pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
      if (pixbuf != NULL)
        g_object_ref (pixbuf);
    }
  else
    {
      g_warning ("focus: could not decode embedded eye icon: %s", error->message);
      g_clear_error (&error);
    }

  g_object_unref (loader);
  g_free (data);

  return pixbuf;
}

/* Scales an alpha-only mask (see eye_closed_mask_base64/
 * eye_open_mask_base64 above) to @size and repaints every pixel's RGB
 * with @for_widget's current foreground color, keeping the source
 * alpha -- the same "recolor a fixed-color raster at render time"
 * trick render_bell_disabled_pixbuf() uses, just starting from a
 * decoded raster mask instead of a hand-authored SVG template. */
static GdkPixbuf *
recolor_mask_pixbuf (const gchar *base64, GtkWidget *for_widget, gint size)
{
  GdkPixbuf *decoded;
  GdkPixbuf *scaled;
  GtkStyleContext *style_context;
  GdkRGBA color;
  guchar red, green, blue;
  gint width, height, x, y, rowstride, n_channels;
  guchar *pixels;

  decoded = decode_base64_pixbuf (base64);
  if (decoded == NULL)
    return NULL;

  scaled = gdk_pixbuf_scale_simple (decoded, size, size, GDK_INTERP_BILINEAR);
  g_object_unref (decoded);
  if (scaled == NULL)
    return NULL;

  if (!gdk_pixbuf_get_has_alpha (scaled))
    {
      GdkPixbuf *with_alpha = gdk_pixbuf_add_alpha (scaled, FALSE, 0, 0, 0);
      g_object_unref (scaled);
      scaled = with_alpha;
    }

  style_context = gtk_widget_get_style_context (for_widget);
  gtk_style_context_get_color (style_context, gtk_style_context_get_state (style_context), &color);
  red = (guchar) (CLAMP (color.red, 0.0, 1.0) * 255);
  green = (guchar) (CLAMP (color.green, 0.0, 1.0) * 255);
  blue = (guchar) (CLAMP (color.blue, 0.0, 1.0) * 255);

  width = gdk_pixbuf_get_width (scaled);
  height = gdk_pixbuf_get_height (scaled);
  rowstride = gdk_pixbuf_get_rowstride (scaled);
  n_channels = gdk_pixbuf_get_n_channels (scaled);
  pixels = gdk_pixbuf_get_pixels (scaled);

  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          guchar *p = pixels + y * rowstride + x * n_channels;
          p[0] = red;
          p[1] = green;
          p[2] = blue;
          /* p[3] (alpha) is left as scaled from the source mask */
        }
    }

  return scaled;
}

static void
focus_update_stay_awake_icon (FocusPlugin *fp)
{
  gint size = xfce_panel_plugin_get_icon_size (fp->plugin);
  GdkPixbuf *pixbuf = recolor_mask_pixbuf (fp->session_active ? eye_open_mask_base64 : eye_closed_mask_base64,
                                            fp->btn_stay_awake, size);

  if (pixbuf != NULL)
    {
      gtk_image_set_from_pixbuf (GTK_IMAGE (fp->img_stay_awake), pixbuf);
      g_object_unref (pixbuf);
    }
}

static void
bell_size_prepared_cb (GdkPixbufLoader *loader, gint width, gint height, gpointer user_data)
{
  gint size = GPOINTER_TO_INT (user_data);

  gdk_pixbuf_loader_set_size (loader, size, size);
}

static GdkPixbuf *
render_bell_disabled_pixbuf (GtkWidget *for_widget, gint size)
{
  gchar *hex;
  gchar *svg;
  GdkPixbufLoader *loader;
  GdkPixbuf *pixbuf = NULL;
  GError *error = NULL;

  hex = get_foreground_hex (for_widget);
  svg = g_strdup_printf (bell_disabled_svg_template, hex);
  g_free (hex);

  loader = gdk_pixbuf_loader_new ();
  g_signal_connect (loader, "size-prepared", G_CALLBACK (bell_size_prepared_cb), GINT_TO_POINTER (size));

  if (gdk_pixbuf_loader_write (loader, (const guchar *) svg, strlen (svg), &error) &&
      gdk_pixbuf_loader_close (loader, &error))
    {
      pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
      if (pixbuf != NULL)
        g_object_ref (pixbuf);
    }
  else
    {
      g_warning ("focus: could not render the \"do not disturb\" icon: %s", error->message);
      g_clear_error (&error);
    }

  g_object_unref (loader);
  g_free (svg);

  return pixbuf;
}

static void
focus_update_dnd_bell_icon (FocusPlugin *fp)
{
  GdkPixbuf *pixbuf = render_bell_disabled_pixbuf (fp->img_dnd_bell, FOCUS_ROW_ICON_SIZE);

  if (pixbuf != NULL)
    {
      gtk_image_set_from_pixbuf (GTK_IMAGE (fp->img_dnd_bell), pixbuf);
      g_object_unref (pixbuf);
    }
}

/* Unlike an image set by icon name, a plain pixbuf never gets
 * re-recolored by GTK on its own -- both img_stay_awake
 * (recolor_mask_pixbuf()) and img_dnd_bell (render_bell_disabled_pixbuf())
 * bake the current foreground color into their pixels at the moment
 * they're rendered. Without reacting to "style-updated" (which GTK
 * emits on a widget whenever its resolved style changes, switching the
 * system's light/dark theme included), those baked-in colors would
 * keep showing the old theme -- sometimes invisible against the new
 * background -- until something unrelated happened to redraw them,
 * e.g. clicking the button to activate/deactivate Focus mode. */
static void
focus_style_updated_cb (GtkWidget *widget, FocusPlugin *fp)
{
  focus_update_stay_awake_icon (fp);
  focus_update_dnd_bell_icon (fp);
}

static GtkWidget *
create_check_row (GtkWidget *icon, const gchar *label, GtkWidget **check_out)
{
  GtkWidget *row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

  gtk_box_pack_start (GTK_BOX (row), icon, FALSE, FALSE, 0);

  *check_out = gtk_check_button_new_with_label (label);
  gtk_box_pack_start (GTK_BOX (row), *check_out, FALSE, FALSE, 0);

  return row;
}

/* labeled "Start" while off or "Restart" while active -- either way it
 * (re)applies the checked features and resets the duration countdown
 * to the full selected time, so no separate "restart" codepath is
 * needed. */
static void
popup_ok_clicked_cb (GtkButton *button, FocusPlugin *fp)
{
  focus_activate (fp);
  gtk_widget_hide (fp->popup_window);
}

/* only shown while active -- closes the dropdown without applying or
 * discarding anything, exactly like Esc or an outside click already
 * do (xfce_panel_plugin_popup_window() wires those up on its own). */
static void
popup_continue_clicked_cb (GtkButton *button, FocusPlugin *fp)
{
  gtk_widget_hide (fp->popup_window);
}

/* labeled "Cancel" while off (discards unsaved checkbox/duration
 * edits, same as before) or "Stop" while active (actually ends the
 * session) -- see focus_update_popup_buttons(). */
static void
popup_cancel_clicked_cb (GtkButton *button, FocusPlugin *fp)
{
  if (fp->session_active)
    {
      focus_deactivate (fp);
    }
  else
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fp->check_stay_awake), fp->snapshot_stay_awake);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fp->check_dnd), fp->snapshot_dnd);
      gtk_range_set_value (GTK_RANGE (fp->duration_scale), (gdouble) fp->snapshot_duration_index);
    }

  gtk_widget_hide (fp->popup_window);
}

static void
arrow_clicked_cb (GtkButton *button, FocusPlugin *fp)
{
  fp->snapshot_stay_awake = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_stay_awake));
  fp->snapshot_dnd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fp->check_dnd));
  fp->snapshot_duration_index = current_duration_index (fp);

  /* GtkPopover ends up rendered behind the panel window itself on
   * xfce4-panel (a known limitation: panel plugin windows are docks
   * kept "always above", and a bare GtkPopover doesn't get placed
   * above that) -- xfce_panel_plugin_popup_window() is the panel's own
   * replacement for exactly this case: a plain GtkWindow that it
   * positions, keeps above the panel, and dismisses on outside
   * click/Esc itself. */
  xfce_panel_plugin_popup_window (fp->plugin, GTK_WINDOW (fp->popup_window), fp->btn_arrow);
}

static GtkWidget *
create_popup_window (GtkWidget *style_widget, FocusPlugin *fp)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *icon_coffee;
  GtkWidget *row_stay_awake;
  GtkWidget *row_dnd;
  GtkWidget *button_row;
  GtkSizeGroup *button_size_group;
  GdkPixbuf *bell_pixbuf;
  gint i;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (window), frame);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  gtk_container_set_border_width (GTK_CONTAINER (box), 8);
  gtk_container_add (GTK_CONTAINER (frame), box);

  icon_coffee = gtk_image_new_from_icon_name (FOCUS_ICON_COFFEE, GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size (GTK_IMAGE (icon_coffee), FOCUS_ROW_ICON_SIZE);
  row_stay_awake = create_check_row (icon_coffee, "Stay Awake", &fp->check_stay_awake);
  gtk_box_pack_start (GTK_BOX (box), row_stay_awake, FALSE, FALSE, 0);

  bell_pixbuf = render_bell_disabled_pixbuf (style_widget, FOCUS_ROW_ICON_SIZE);
  fp->img_dnd_bell = gtk_image_new_from_pixbuf (bell_pixbuf);
  g_clear_object (&bell_pixbuf);
  row_dnd = create_check_row (fp->img_dnd_bell, "Do Not Disturb", &fp->check_dnd);

  fp->icon_dnd_warning = gtk_image_new_from_icon_name ("dialog-warning-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size (GTK_IMAGE (fp->icon_dnd_warning), FOCUS_ROW_ICON_SIZE);
  gtk_widget_set_no_show_all (fp->icon_dnd_warning, TRUE);
  gtk_widget_set_tooltip_text (fp->icon_dnd_warning,
                                "Notifications were re-enabled outside this plugin.\n"
                                "They will stay on until you click Restart or the current "
                                "Focus session ends.");
  gtk_box_pack_start (GTK_BOX (row_dnd), fp->icon_dnd_warning, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (box), row_dnd, FALSE, FALSE, 0);

  fp->duration_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, FOCUS_DURATION_COUNT - 1, 1);
  gtk_widget_set_size_request (fp->duration_scale, 200, -1);
  gtk_scale_set_draw_value (GTK_SCALE (fp->duration_scale), FALSE);
  gtk_scale_set_digits (GTK_SCALE (fp->duration_scale), 0);
  for (i = 0; i < FOCUS_DURATION_COUNT; i++)
    gtk_scale_add_mark (GTK_SCALE (fp->duration_scale), (gdouble) i, GTK_POS_BOTTOM, duration_marks[i]);
  gtk_box_pack_start (GTK_BOX (box), fp->duration_scale, FALSE, FALSE, 0);

  fp->duration_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (fp->duration_label), "<b>Forever</b>");
  gtk_box_pack_start (GTK_BOX (box), fp->duration_label, FALSE, FALSE, 0);

  fp->duration_remaining_label = gtk_label_new (NULL);
  gtk_widget_set_no_show_all (fp->duration_remaining_label, TRUE);
  gtk_box_pack_start (GTK_BOX (box), fp->duration_remaining_label, FALSE, FALSE, 0);

  g_signal_connect (fp->duration_scale, "value-changed", G_CALLBACK (duration_scale_changed_cb), fp);
  g_signal_connect (fp->duration_scale, "change-value", G_CALLBACK (duration_scale_change_value_cb), fp);

  gtk_box_pack_start (GTK_BOX (box), gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);

  button_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_halign (button_row, GTK_ALIGN_CENTER);
  gtk_box_pack_start (GTK_BOX (box), button_row, FALSE, FALSE, 0);

  /* kept equal-width via the size group below regardless of which pair
   * of labels (Cancel/Start, Stop/Restart) is currently showing. */
  fp->btn_cancel = gtk_button_new_with_label ("Cancel");
  g_signal_connect (fp->btn_cancel, "clicked", G_CALLBACK (popup_cancel_clicked_cb), fp);
  gtk_box_pack_start (GTK_BOX (button_row), fp->btn_cancel, FALSE, FALSE, 0);

  fp->btn_ok = gtk_button_new_with_label ("Start");
  g_signal_connect (fp->btn_ok, "clicked", G_CALLBACK (popup_ok_clicked_cb), fp);
  gtk_box_pack_start (GTK_BOX (button_row), fp->btn_ok, FALSE, FALSE, 0);

  button_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  gtk_size_group_add_widget (button_size_group, fp->btn_cancel);
  gtk_size_group_add_widget (button_size_group, fp->btn_ok);
  g_object_unref (button_size_group);

  fp->btn_continue = gtk_button_new_with_label ("Continue");
  gtk_widget_set_no_show_all (fp->btn_continue, TRUE);
  g_signal_connect (fp->btn_continue, "clicked", G_CALLBACK (popup_continue_clicked_cb), fp);
  gtk_box_pack_start (GTK_BOX (button_row), fp->btn_continue, FALSE, FALSE, 0);

  gtk_widget_show_all (frame);

  return window;
}

static gboolean
focus_size_changed (XfcePanelPlugin *plugin, gint size, FocusPlugin *fp)
{
  gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, -1);
  focus_apply_icon_size (fp);
  return TRUE;
}

/* On a vertical panel (not deskbar mode) xfce4-panel reports
 * GTK_ORIENTATION_VERTICAL here -- the plugin only gets the panel's
 * narrow thickness to work with, so fp->box must stack the main button
 * above the arrow button instead of placing them side by side, or the
 * arrow gets squeezed down to zero width and disappears. The arrow
 * glyph is swapped the same way GTK's own widgets (e.g. GtkExpander)
 * rotate their disclosure triangle to match: pointing right instead of
 * down still reads as "more options below/beside" once the row becomes
 * a column. */
static void
focus_orientation_changed (XfcePanelPlugin *plugin, GtkOrientation orientation, FocusPlugin *fp)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (fp->box), orientation);
  gtk_image_set_from_icon_name (GTK_IMAGE (fp->img_arrow),
                                 orientation == GTK_ORIENTATION_HORIZONTAL ? "pan-down-symbolic" : "pan-end-symbolic",
                                 GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size (GTK_IMAGE (fp->img_arrow), FOCUS_ARROW_ICON_SIZE);
}

static void
focus_free_data (XfcePanelPlugin *plugin, FocusPlugin *fp)
{
  focus_cancel_duration_timer (fp);
  g_object_unref (fp->inhibit);
  g_object_unref (fp->dnd);
  g_free (fp);
}

static void
focus_construct (XfcePanelPlugin *plugin)
{
  FocusPlugin *fp;

  fp = g_new0 (FocusPlugin, 1);
  fp->plugin = plugin;
  fp->inhibit = focus_inhibit_new ();
  fp->dnd = focus_dnd_new ();

  fp->ebox = gtk_event_box_new ();
  gtk_widget_show (fp->ebox);

  fp->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_show (fp->box);
  gtk_container_add (GTK_CONTAINER (fp->ebox), fp->box);

  fp->btn_stay_awake = create_toggle_button ("Click to start Focus mode with the settings below",
                                              G_CALLBACK (main_button_toggled_cb), fp, &fp->img_stay_awake);
  gtk_widget_show_all (fp->btn_stay_awake);
  gtk_box_pack_start (GTK_BOX (fp->box), fp->btn_stay_awake, FALSE, FALSE, 0);

  fp->btn_arrow = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (fp->btn_arrow), GTK_RELIEF_NONE);
  gtk_widget_set_focus_on_click (fp->btn_arrow, FALSE);
  gtk_widget_set_tooltip_text (fp->btn_arrow, "More options");

  /* GTK_RELIEF_NONE only drops the button's border/background, not the
   * theme's own button padding and minimum size -- both still leave it
   * noticeably wider than the arrow glyph actually needs next to the
   * main icon, so trim them explicitly instead. */
  {
    GtkCssProvider *arrow_css = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (arrow_css,
                                      "button.focus-arrow-button {"
                                      "  padding: 0px 2px;"
                                      "  min-width: 0px;"
                                      "  min-height: 0px;"
                                      "}",
                                      -1, NULL);
    gtk_style_context_add_class (gtk_widget_get_style_context (fp->btn_arrow), "focus-arrow-button");
    gtk_style_context_add_provider (gtk_widget_get_style_context (fp->btn_arrow),
                                     GTK_STYLE_PROVIDER (arrow_css),
                                     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (arrow_css);
  }

  fp->img_arrow = gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size (GTK_IMAGE (fp->img_arrow), FOCUS_ARROW_ICON_SIZE);
  gtk_container_add (GTK_CONTAINER (fp->btn_arrow), fp->img_arrow);
  gtk_box_pack_start (GTK_BOX (fp->box), fp->btn_arrow, FALSE, FALSE, 0);

  fp->popup_window = create_popup_window (fp->btn_arrow, fp);
  g_signal_connect (fp->btn_arrow, "clicked", G_CALLBACK (arrow_clicked_cb), fp);
  gtk_widget_show_all (fp->btn_arrow);

  /* fp->popup_window is a separate top-level, not a descendant of
   * fp->ebox, so it needs its own "style-updated" listener to keep
   * img_dnd_bell in sync with the theme; see focus_style_updated_cb(). */
  g_signal_connect (fp->btn_stay_awake, "style-updated", G_CALLBACK (focus_style_updated_cb), fp);
  g_signal_connect (fp->popup_window, "style-updated", G_CALLBACK (focus_style_updated_cb), fp);

  g_signal_connect (fp->check_stay_awake, "toggled", G_CALLBACK (check_stay_awake_toggled_cb), fp);
  g_signal_connect (fp->check_dnd, "toggled", G_CALLBACK (check_dnd_toggled_cb), fp);
  g_signal_connect (fp->dnd, "changed", G_CALLBACK (dnd_changed_cb), fp);

  gtk_container_add (GTK_CONTAINER (plugin), fp->ebox);

  xfce_panel_plugin_add_action_widget (plugin, fp->btn_stay_awake);
  xfce_panel_plugin_add_action_widget (plugin, fp->btn_arrow);

  g_signal_connect (plugin, "free-data", G_CALLBACK (focus_free_data), fp);
  g_signal_connect (plugin, "save", G_CALLBACK (focus_save), fp);
  g_signal_connect (plugin, "size-changed", G_CALLBACK (focus_size_changed), fp);
  g_signal_connect (plugin, "orientation-changed", G_CALLBACK (focus_orientation_changed), fp);

  focus_orientation_changed (plugin, xfce_panel_plugin_get_orientation (plugin), fp);
  focus_apply_icon_size (fp);
  focus_load_settings (fp);
}

XFCE_PANEL_PLUGIN_REGISTER (focus_construct);
