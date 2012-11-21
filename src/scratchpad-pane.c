/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <gtksourceview/gtksourceview.h>
#include "scratchpad-pane.h"

typedef struct
{
  gchar *file_path;
  gint   line_number;
  gint   start_offset;
  gint   end_offset;
} Link;

static void scratchpad_pane_class_init  (ScratchpadPaneClass *klass);
static void scratchpad_pane_init        (ScratchpadPane      *pane);
static void scratchpad_pane_finalize    (ScratchpadPane      *pane);

static void preferences_changed_action  (ScratchpadPane      *pane);

#define SCRATCHPAD_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), SCRATCHPAD_PANE_TYPE, ScratchpadPanePrivate))

typedef struct _ScratchpadPanePrivate ScratchpadPanePrivate;

struct _ScratchpadPanePrivate
{
  CodeSlayer             *codeslayer;
  CodeSlayerPreferences  *preferences;
  CodeSlayerEditorLinker *linker;
  GtkWidget              *text_view;
  GtkTextBuffer          *buffer;
  gulong                  initialize_preferences_id;
  gulong                  editor_preferences_changed_id;
};

G_DEFINE_TYPE (ScratchpadPane, scratchpad_pane, GTK_TYPE_VBOX)

static void
scratchpad_pane_class_init (ScratchpadPaneClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) scratchpad_pane_finalize;
  g_type_class_add_private (klass, sizeof (ScratchpadPanePrivate));
}

static void
scratchpad_pane_init (ScratchpadPane *pane) 
{
  ScratchpadPanePrivate *priv;
  GtkWidget *scrolled_window;
  
  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);
  
  priv->text_view = gtk_source_view_new ();
  priv->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->text_view));
  gtk_text_buffer_create_tag (priv->buffer, "header", "weight", PANGO_WEIGHT_BOLD, NULL);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), priv->text_view);

  gtk_box_pack_start (GTK_BOX (pane), scrolled_window, TRUE, TRUE, 0);
}

static void
scratchpad_pane_finalize (ScratchpadPane *pane)
{
  ScratchpadPanePrivate *priv;
  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);

  g_signal_handler_disconnect (priv->preferences, priv->initialize_preferences_id);
  g_signal_handler_disconnect (priv->preferences, priv->editor_preferences_changed_id);
  
  g_object_unref (priv->linker);
  
  G_OBJECT_CLASS (scratchpad_pane_parent_class)->finalize (G_OBJECT(pane));
}

GtkWidget*
scratchpad_pane_new (CodeSlayer *codeslayer)
{
  ScratchpadPanePrivate *priv;
  GtkWidget *pane;

  pane = g_object_new (scratchpad_pane_get_type (), NULL);
  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);
  priv->codeslayer = codeslayer;
  priv->preferences = codeslayer_get_preferences (codeslayer);
  
  priv->linker = codeslayer_get_editor_linker (codeslayer, GTK_TEXT_VIEW (priv->text_view));

  priv->initialize_preferences_id = g_signal_connect_swapped (G_OBJECT (priv->preferences), "initialize-preferences",
                                                              G_CALLBACK (preferences_changed_action), SCRATCHPAD_PANE (pane));
  
  priv->editor_preferences_changed_id = g_signal_connect_swapped (G_OBJECT (priv->preferences), "editor-preferences-changed",
                                                                  G_CALLBACK (preferences_changed_action), SCRATCHPAD_PANE (pane));
  
  return pane;
}

static void
preferences_changed_action (ScratchpadPane *pane)
{
  ScratchpadPanePrivate *priv;
  
  gdouble editor_tab_width;
  gboolean enable_automatic_indentation;
  gboolean insert_spaces_instead_of_tabs;
  gchar *fontname;
  PangoFontDescription *font_description;
  
  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);
  
  editor_tab_width = codeslayer_preferences_get_double (priv->preferences,
                                                        CODESLAYER_PREFERENCES_EDITOR_TAB_WIDTH);
  gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (priv->text_view), editor_tab_width);
  gtk_source_view_set_indent_width (GTK_SOURCE_VIEW (priv->text_view), -1);

  enable_automatic_indentation = codeslayer_preferences_get_boolean (priv->preferences,
                                                                     CODESLAYER_PREFERENCES_EDITOR_ENABLE_AUTOMATIC_INDENTATION);
  gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW (priv->text_view), 
                                   enable_automatic_indentation);
  gtk_source_view_set_indent_on_tab (GTK_SOURCE_VIEW (priv->text_view),
                                     enable_automatic_indentation);

  insert_spaces_instead_of_tabs = codeslayer_preferences_get_boolean (priv->preferences,
                                                                      CODESLAYER_PREFERENCES_EDITOR_INSERT_SPACES_INSTEAD_OF_TABS);
  gtk_source_view_set_insert_spaces_instead_of_tabs (GTK_SOURCE_VIEW (priv->text_view),
                                                     insert_spaces_instead_of_tabs);

  fontname = codeslayer_preferences_get_string (priv->preferences,
                                                CODESLAYER_PREFERENCES_EDITOR_FONT);
  font_description = pango_font_description_from_string (fontname);
  
  if (fontname)
    g_free (fontname);
  
  gtk_widget_override_font (GTK_WIDGET (priv->text_view), font_description);
  pango_font_description_free (font_description);  
}

void
scratchpad_pane_add_text (ScratchpadPane *pane, 
                          const gchar    *header,
                          const gchar    *text)
{
  ScratchpadPanePrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  gchar *format_text;

  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->text_view));

  format_text = g_strconcat ("\n", header, "\n\n", NULL);

  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, text, -1);
  
  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, format_text, -1, "header", NULL);

  g_free (format_text);
}                                       
