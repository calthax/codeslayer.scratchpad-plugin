/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.remove_group_item
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gdk/gdkkeysyms.h>
#include <codeslayer/codeslayer.h>
#include "scratchpad-menu.h"

static void scratchpad_menu_class_init  (ScratchpadMenuClass *klass);
static void scratchpad_menu_init        (ScratchpadMenu      *menu);
static void scratchpad_menu_finalize    (ScratchpadMenu      *menu);

static void copy_action                 (ScratchpadMenu      *menu);
                                        
enum
{
  COPY,
  LAST_SIGNAL
};

static guint scratchpad_menu_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (ScratchpadMenu, scratchpad_menu, GTK_TYPE_MENU_ITEM)

static void
scratchpad_menu_class_init (ScratchpadMenuClass *klass)
{
  scratchpad_menu_signals[COPY] =
    g_signal_new ("copy", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (ScratchpadMenuClass, copy),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) scratchpad_menu_finalize;
}

static void
scratchpad_menu_init (ScratchpadMenu *menu)
{
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu), "Copy To ScratchPad");
}

static void
scratchpad_menu_finalize (ScratchpadMenu *menu)
{
  G_OBJECT_CLASS (scratchpad_menu_parent_class)->finalize (G_OBJECT (menu));
}

GtkWidget*
scratchpad_menu_new (GtkAccelGroup *accel_group)
{
  GtkWidget *menu;

  menu = g_object_new (scratchpad_menu_get_type (), NULL);
  
  gtk_widget_add_accelerator (menu, "activate", 
                              accel_group, GDK_M, 
                              GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);  

  g_signal_connect_swapped (G_OBJECT (menu), "activate", 
                            G_CALLBACK (copy_action), menu);

  return menu;
}

static void 
copy_action (ScratchpadMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "copy");
}
