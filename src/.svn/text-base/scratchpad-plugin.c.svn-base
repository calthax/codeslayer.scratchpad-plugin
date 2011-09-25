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

#include <codeslayer/codeslayer.h>
#include "scratchpad-engine.h"
#include "scratchpad-menu.h"
#include "scratchpad-pane.h"
#include <gtk/gtk.h>
#include <gmodule.h>
#include <glib.h>

G_MODULE_EXPORT void activate   (CodeSlayer *codeslayer);
G_MODULE_EXPORT void deactivate (CodeSlayer *codeslayer);

static ScratchpadEngine *engine;
static GtkWidget *menu;
static GtkWidget *pane;

G_MODULE_EXPORT void
activate (CodeSlayer *codeslayer)
{
  GtkAccelGroup *accel_group;
  accel_group = codeslayer_get_menubar_accel_group (codeslayer);
  menu = scratchpad_menu_new (accel_group);
  pane = scratchpad_pane_new ();

  engine = scratchpad_engine_new (codeslayer, menu, pane);

  codeslayer_add_to_menubar (codeslayer, GTK_MENU_ITEM (menu));
  codeslayer_add_to_side_pane (codeslayer, pane, "ScratchPad");
}

G_MODULE_EXPORT void 
deactivate (CodeSlayer *codeslayer)
{
  codeslayer_remove_from_menubar (codeslayer, GTK_MENU_ITEM (menu));
  codeslayer_remove_from_side_pane (codeslayer, pane);
  g_object_unref (engine);
}
