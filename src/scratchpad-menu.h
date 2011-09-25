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

#ifndef __SCRATCHPAD_MENU_H__
#define	__SCRATCHPAD_MENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SCRATCHPAD_MENU_TYPE            (scratchpad_menu_get_type ())
#define SCRATCHPAD_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCRATCHPAD_MENU_TYPE, ScratchpadMenu))
#define SCRATCHPAD_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCRATCHPAD_MENU_TYPE, ScratchpadMenuClass))
#define IS_SCRATCHPAD_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCRATCHPAD_MENU_TYPE))
#define IS_SCRATCHPAD_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCRATCHPAD_MENU_TYPE))

typedef struct _ScratchpadMenu ScratchpadMenu;
typedef struct _ScratchpadMenuClass ScratchpadMenuClass;

struct _ScratchpadMenu
{
  GtkMenuItem parent_instance;
};

struct _ScratchpadMenuClass
{
  GtkMenuItemClass parent_class;

  void (*copy) (ScratchpadMenu *menu);
};

GType scratchpad_menu_get_type (void) G_GNUC_CONST;
  
GtkWidget*  scratchpad_menu_new  (GtkAccelGroup *accel_group);
                                            
G_END_DECLS

#endif /* __SCRATCHPAD_MENU_H__ */
