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

#include <string.h>
#include <codeslayer/codeslayer-utils.h>
#include "scratchpad-engine.h"
#include "scratchpad-pane.h"

typedef struct
{
  gchar         *file_path;
  unsigned long  line_number;
} Tag;


static void scratchpad_engine_class_init  (ScratchpadEngineClass *klass);
static void scratchpad_engine_init        (ScratchpadEngine      *engine);
static void scratchpad_engine_finalize    (ScratchpadEngine      *engine);

static void copy_action                   (ScratchpadEngine      *engine);
static gchar* get_header                  (ScratchpadEngine      *engine, 
                                           gint                   line_number);
                                                   
#define SCRATCHPAD_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), SCRATCHPAD_ENGINE_TYPE, ScratchpadEnginePrivate))

typedef struct _ScratchpadEnginePrivate ScratchpadEnginePrivate;

struct _ScratchpadEnginePrivate
{
  CodeSlayer *codeslayer;
  GtkWidget  *menu;
  GtkWidget  *pane;
};

G_DEFINE_TYPE (ScratchpadEngine, scratchpad_engine, G_TYPE_OBJECT)

static void
scratchpad_engine_class_init (ScratchpadEngineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) scratchpad_engine_finalize;
  g_type_class_add_private (klass, sizeof (ScratchpadEnginePrivate));
}

static void
scratchpad_engine_init (ScratchpadEngine *engine) {}

static void
scratchpad_engine_finalize (ScratchpadEngine *engine)
{
  G_OBJECT_CLASS (scratchpad_engine_parent_class)->finalize (G_OBJECT(engine));
}

ScratchpadEngine*
scratchpad_engine_new (CodeSlayer *codeslayer,
                       GtkWidget  *menu, 
                       GtkWidget  *pane)
{
  ScratchpadEnginePrivate *priv;
  ScratchpadEngine *engine;

  engine = SCRATCHPAD_ENGINE (g_object_new (scratchpad_engine_get_type (), NULL));
  priv = SCRATCHPAD_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  priv->menu = menu;
  priv->pane = pane;

  g_signal_connect_swapped (G_OBJECT (menu), "copy",
                            G_CALLBACK (copy_action), engine);

  return engine;
}

static void 
copy_action (ScratchpadEngine *engine)
{
  ScratchpadEnginePrivate *priv;
  CodeSlayerEditor *editor;
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  gint line_number;
  gchar *header;              
  gchar *text;
  
  priv = SCRATCHPAD_ENGINE_GET_PRIVATE (engine);
  
  editor = codeslayer_get_active_editor (priv->codeslayer);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
  
  line_number = gtk_text_iter_get_line (&start);
  
  header = get_header (engine, line_number);
  
  scratchpad_pane_add_text (SCRATCHPAD_PANE (priv->pane), header, text);
  scratchpad_pane_create_links (SCRATCHPAD_PANE (priv->pane));
  
  codeslayer_show_side_pane (priv->codeslayer, GTK_WIDGET (priv->pane));
  
  if (text != NULL)
    g_free (text);
    
  g_free (header);      
}

static gchar*
get_header (ScratchpadEngine *engine, 
            gint              line_number)
{
  ScratchpadEnginePrivate *priv;
  CodeSlayerDocument *document;
  const gchar *file_path;
  gchar *header;

  priv = SCRATCHPAD_ENGINE_GET_PRIVATE (engine);
  
  document = codeslayer_get_active_editor_document (priv->codeslayer);
  file_path = codeslayer_document_get_file_path (document);

  header = g_strdup_printf ("%s:%d", file_path, ++line_number);

  return header;
}
