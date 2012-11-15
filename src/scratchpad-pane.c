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

static GList* find_matches               (gchar              *text);
static GList* mark_links                 (ScratchpadPane     *pane, 
                                          GtkTextBuffer      *buffer, 
                                          GList              *matches);
static Link* create_link                 (ScratchpadPane     *pane,
                                          gchar              *text,
                                          GtkTextIter        *begin, 
                                          GtkTextIter        *end);
static gboolean select_link_action       (ScratchpadPane     *pane, 
                                          GdkEventButton     *event);
static gboolean notify_link_action       (ScratchpadPane     *pane, 
                                          GdkEventButton     *event);                                                              
static void clear_links                  (ScratchpadPane     *pane);



#define SCRATCHPAD_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), SCRATCHPAD_PANE_TYPE, ScratchpadPanePrivate))

typedef struct _ScratchpadPanePrivate ScratchpadPanePrivate;

struct _ScratchpadPanePrivate
{
  CodeSlayer            *codeslayer;
  CodeSlayerPreferences *preferences;
  GtkWidget             *text_view;
  GtkTextBuffer         *buffer;
  gulong                 initialize_preferences_id;
  gulong                 editor_preferences_changed_id;
  GList                 *links;
  GtkTextTag            *underline_tag;
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
  
  priv->links = NULL;
  
  priv->text_view = gtk_source_view_new ();
  priv->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->text_view));
  gtk_text_buffer_create_tag (priv->buffer, "header", "weight", PANGO_WEIGHT_BOLD, NULL);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), priv->text_view);

  gtk_box_pack_start (GTK_BOX (pane), scrolled_window, TRUE, TRUE, 0);

  priv->underline_tag = gtk_text_buffer_create_tag (priv->buffer, "underline", "underline", 
                                                    PANGO_UNDERLINE_SINGLE, NULL);

  g_signal_connect_swapped (G_OBJECT (priv->text_view), "button-press-event",
                            G_CALLBACK (select_link_action), pane);
  g_signal_connect_swapped (G_OBJECT (priv->text_view), "motion-notify-event",
                            G_CALLBACK (notify_link_action), pane);
}

static void
scratchpad_pane_finalize (ScratchpadPane *pane)
{
  ScratchpadPanePrivate *priv;
  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);

  g_signal_handler_disconnect (priv->preferences, priv->initialize_preferences_id);
  g_signal_handler_disconnect (priv->preferences, priv->editor_preferences_changed_id);
  
  clear_links (pane);

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

void
scratchpad_pane_create_links (ScratchpadPane *pane)
{
  ScratchpadPanePrivate *priv;
  GtkTextIter start;
  GtkTextIter end;
  gchar *text;
  GList *matches;

  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);

  gtk_text_buffer_get_bounds (priv->buffer, &start, &end);
  
  text = gtk_text_buffer_get_text (priv->buffer, &start, &end, FALSE);
  
  matches = find_matches (text);
  clear_links (pane);
  priv->links = mark_links (pane, priv->buffer, matches);
  g_list_foreach (matches, (GFunc) g_free, NULL);
  g_free (text);
}

static void
clear_links (ScratchpadPane *pane)
{
  ScratchpadPanePrivate *priv;
  GList *list;
  
  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);

  if (priv->links == NULL)
    return;
  
  list = priv->links;
  
  while (list != NULL)
    {
      Link *link = list->data;
      g_free (link->file_path);
      g_free (link);
      list = g_list_next (list);
    }
  
  g_list_free (priv->links);
  priv->links = NULL;
}

static GList*
find_matches (gchar *text)
{
  GList *results = NULL;
  GRegex *regex;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  regex = g_regex_new ("\\s(\\/.*?:\\d+)", 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (g_match_info_matches (match_info))
    {
      gchar *match_text = NULL;
      match_text = g_match_info_fetch (match_info, 1);
      
      g_print ("match %s\n", match_text);
      results = g_list_prepend (results, g_strdup (match_text));
        
      g_free (match_text);
      g_match_info_next (match_info, &error);
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for completion word error: %s\n", error->message);
      g_error_free (error);
    }

  return results;    
}

static GList*
mark_links (ScratchpadPane *pane, 
            GtkTextBuffer *buffer, 
            GList         *matches)
{
  GList *results = NULL;

  while (matches != NULL)
    {
      gchar *match_text = matches->data;
      GtkTextIter start, begin, end;

      gtk_text_buffer_get_start_iter (buffer, &start);
      
      while (gtk_text_iter_forward_search (&start, match_text, 
                                           GTK_TEXT_SEARCH_TEXT_ONLY, 
                                           &begin, &end, NULL))
        {
          Link *link;
          gtk_text_buffer_apply_tag_by_name (buffer, "underline", &begin, &end);
          
          link = create_link (pane, match_text, &begin, &end);
          if (link != NULL)
            results = g_list_prepend (results, link);
          
          start = begin;
          gtk_text_iter_forward_char (&start);
        }
      
      matches = g_list_next (matches);
    }
    
  return results;    
}

static Link*
create_link (ScratchpadPane *pane,
             gchar         *text,
             GtkTextIter   *begin, 
             GtkTextIter   *end)
{
  Link *link = NULL;
  gchar **split, **tmp;
  
  split = g_strsplit (text, ":", 0);  
  
  if (split != NULL)
    {
      tmp = split;

      link = g_malloc (sizeof (Link));
      link->file_path = g_strdup (*tmp);
      tmp++;
      link->line_number = atoi(*tmp);
      link->start_offset = gtk_text_iter_get_offset (begin);
      link->end_offset = gtk_text_iter_get_offset (end);
      
      g_strfreev(split);
    }

  return link;
}

static gboolean
select_link_action (ScratchpadPane  *pane, 
                    GdkEventButton *event)
{
  ScratchpadPanePrivate *priv;
  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);

  if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS))
    {
      GdkWindow *window;
      GtkTextIter iter;
      gint offset, x, y, bx, by;
      GList *list;

      window = gtk_text_view_get_window (GTK_TEXT_VIEW (priv->text_view),
                                         GTK_TEXT_WINDOW_TEXT);
                                                                                                                
      gdk_window_get_device_position (window, event->device, &x, &y, NULL);      
      
      gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (priv->text_view),
                                             GTK_TEXT_WINDOW_TEXT,
                                             x, y, &bx, &by);

      gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (priv->text_view),
                                          &iter, bx, by);
      
      offset = gtk_text_iter_get_offset (&iter);
      
      list = priv->links;
      
      while (list != NULL)
        {
          Link *link = list->data;
          
          if (offset >= link->start_offset && offset <= link->end_offset)
            {
              CodeSlayerDocument *document;
              CodeSlayerProject *project;
              document = codeslayer_document_new ();
              codeslayer_document_set_file_path (document, link->file_path);
              codeslayer_document_set_line_number (document, link->line_number);
              
              project = codeslayer_get_project_by_file_path (priv->codeslayer, 
                                                             link->file_path);
              codeslayer_document_set_project (document, project);
              
              codeslayer_select_editor (priv->codeslayer, document);
              g_object_unref (document);
              return FALSE;
            }
          list = g_list_next (list);
        }
    }

  return FALSE;
}

static gboolean
notify_link_action (ScratchpadPane  *pane, 
                    GdkEventButton *event)
{

  ScratchpadPanePrivate *priv;
  GdkWindow *window;
  GdkCursor *cursor;
  GtkTextIter iter;
  gint x, y, bx, by;

  priv = SCRATCHPAD_PANE_GET_PRIVATE (pane);

  window = gtk_text_view_get_window (GTK_TEXT_VIEW (priv->text_view),
                                     GTK_TEXT_WINDOW_TEXT);
                                                                                                                
  gdk_window_get_device_position (window, event->device, &x, &y, NULL);      

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (priv->text_view),
                                         GTK_TEXT_WINDOW_TEXT,
                                         x, y, &bx, &by);
      
  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (priv->text_view),
                                      &iter, bx, by);

  cursor = gdk_window_get_cursor (window);

  if (gtk_text_iter_has_tag (&iter, priv->underline_tag))
    {
      if (cursor == NULL || gdk_cursor_get_cursor_type (cursor) != GDK_HAND1)
        {
          cursor = gdk_cursor_new (GDK_HAND1);
          gdk_window_set_cursor (window, cursor);
          g_object_unref (cursor);    
        }
    } 
  else 
    {
      if (cursor != NULL && gdk_cursor_get_cursor_type (cursor) != GDK_XTERM)
        {
          cursor = gdk_cursor_new (GDK_XTERM);
          gdk_window_set_cursor (window, cursor);
          g_object_unref (cursor);    
        }
    }
      
  return FALSE;
}
