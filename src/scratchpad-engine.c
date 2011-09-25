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


static void scratchpad_engine_class_init  (ScratchpadEngineClass  *klass);
static void scratchpad_engine_init        (ScratchpadEngine       *engine);
static void scratchpad_engine_finalize    (ScratchpadEngine       *engine);

static void copy_action                   (ScratchpadEngine       *engine);
static gchar* get_header                  (ScratchpadEngine       *engine);

                                                   
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
  gchar *header;              
  gchar *text;
  
  priv = SCRATCHPAD_ENGINE_GET_PRIVATE (engine);
  
  editor = codeslayer_get_active_editor (priv->codeslayer);
  header = get_header (engine);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
  
  scratchpad_pane_add_text (SCRATCHPAD_PANE (priv->pane), header, text);
  
  if (text != NULL)
    g_free (text);
    
  g_free (header);      
}

static gchar*
get_header (ScratchpadEngine *engine)
{
  ScratchpadEnginePrivate *priv;
  CodeSlayerProject *project;
  CodeSlayerDocument *document;
  const gchar *file_path;
  const gchar *folder_path;
  const gchar *project_name;
  gchar *substr;
  gchar *header;

  priv = SCRATCHPAD_ENGINE_GET_PRIVATE (engine);
  
  document = codeslayer_get_active_editor_document (priv->codeslayer);
  project = codeslayer_get_active_editor_project (priv->codeslayer);
  
  file_path = codeslayer_document_get_file_path (document);
  folder_path = codeslayer_project_get_folder_path (project);
  project_name = codeslayer_project_get_name (project);
  
  substr = codeslayer_utils_substr (file_path, strlen(folder_path) + 1, strlen(file_path));
  
  header = g_strconcat (project_name, " - ", substr, NULL);
  g_free (substr);

  return header;
}
