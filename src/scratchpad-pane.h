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

#ifndef __SCRATCHPAD_PANE_H__
#define	__SCRATCHPAD_PANE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define SCRATCHPAD_PANE_TYPE            (scratchpad_pane_get_type ())
#define SCRATCHPAD_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCRATCHPAD_PANE_TYPE, ScratchpadPane))
#define SCRATCHPAD_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCRATCHPAD_PANE_TYPE, ScratchpadPaneClass))
#define IS_SCRATCHPAD_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCRATCHPAD_PANE_TYPE))
#define IS_SCRATCHPAD_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCRATCHPAD_PANE_TYPE))

typedef struct _ScratchpadPane ScratchpadPane;
typedef struct _ScratchpadPaneClass ScratchpadPaneClass;

struct _ScratchpadPane
{
  GtkVBox parent_instance;
};

struct _ScratchpadPaneClass
{
  GtkVBoxClass parent_class;
};

GType scratchpad_pane_get_type (void) G_GNUC_CONST;
     
GtkWidget*  scratchpad_pane_new           (CodeSlayer     *codeslayer);

void        scratchpad_pane_add_text      (ScratchpadPane *pane, 
                                           const gchar    *header,
                                           const gchar    *text);

G_END_DECLS

#endif /* __SCRATCHPAD_PANE_H__ */
