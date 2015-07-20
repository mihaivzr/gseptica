/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mainwindow.cpp
 * Copyright (C) Mihai Varzaru 2009 <mihaiv@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <gtk/gtk.h>
#include <glade/glade-xml.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include <string.h>

#include "mainwindow.h"
#include "game.h"
#include "defines.h"


extern GladeXML *GladeXml;
MainWindowData Mwd;
Game game;


void draw_card(GdkDrawable *draws, GdkGC* gc, int x, int y, int card_suit, int card_nr)
{
	int card_index = card_nr - 1;
	if (card_nr == 11)
		card_index = 0;
	if (card_nr > 11)
		card_index = card_nr - 2;
	int res_X = card_index * Mwd.card_width;
	int res_Y = card_suit * Mwd.card_height;

	gdk_draw_pixbuf(draws, gc, Mwd.cards_res, res_X, res_Y, x, y, Mwd.card_width, 
	    Mwd.card_height, GDK_RGB_DITHER_NONE, 0, 0);
}

void draw_cards_back(GdkDrawable *draws, GdkGC* gc, int x, int y)
{
	int res_X = 2 * Mwd.card_width;
	int res_Y = 4 * Mwd.card_height;
	gdk_draw_pixbuf(draws, gc, Mwd.cards_res, res_X, res_Y, x, y, Mwd.card_width, 
	    Mwd.card_height, GDK_RGB_DITHER_NONE, 0, 0);	
}

static gboolean load_cards_resource()
{
	//Cards proportions are Height / Width = 1.6
	//There are 13 cards in the resource horisontally and 5 vertically
	Mwd.card_width = 80;
	Mwd.card_height = 1.6 * Mwd.card_width; //default 128
	Mwd.res_width = 13 * Mwd.card_width; //defaut 1040
	Mwd.res_height = 5 * Mwd.card_height; //default 640

	GError *err = NULL;
	gchar *cards_path = installation_path(CARDS_FILE);
	Mwd.cards_res = gdk_pixbuf_new_from_file_at_scale(cards_path, 
	    Mwd.res_width, Mwd.res_height, FALSE, &err);
	if (Mwd.cards_res == NULL)
	{
		g_printerr("Error loading cards resource: %s.\n", cards_path);
		if (err != NULL && err->message != NULL)
			g_printerr("%s\n", err->message);
	}
	g_free(cards_path);
	return (Mwd.cards_res != NULL);
}

static gboolean is_mainwnd_maximized()
{
	GdkWindowState state = gdk_window_get_state(Mwd.window->window);
	return ((state & GDK_WINDOW_STATE_MAXIMIZED) != 0);
}

static gboolean get_int_keyval(GKeyFile *config_file, const char *group,
								   const char *key, int *preference)
{
	gint value;
	GError *error = NULL;
	value = g_key_file_get_integer(config_file, group, key, &error);
	if (error != NULL)
	{
		g_error_free(error);
		return FALSE;
	}else
	{
		*preference = value;
		return TRUE;
	}
}

gchar *get_preferences_file_location()
{
	return g_strdup_printf("%s/.gseptica", g_get_home_dir());
}

void load_preferences(gboolean statsOnly)
{
	char *file_name = get_preferences_file_location();
	GKeyFile *config_file = g_key_file_new();

	if (g_key_file_load_from_file(config_file, file_name, G_KEY_FILE_NONE, NULL))
	{
		if (!statsOnly)
		{
			gchar *svalue = g_key_file_get_string(config_file, "Game", "SelAILevelMenu", NULL);
			if (svalue != NULL)
			{
				if (strlen(svalue)<sizeof(Mwd.sel_ailevel_menu) && 
					strncmp(svalue, "rmenuDiffc", strlen("rmenuDiffc"))==0)
				{
					g_strlcpy(Mwd.sel_ailevel_menu, svalue, sizeof(Mwd.sel_ailevel_menu));
				}
				g_free(svalue);
			}
			
			int size_width = -1, size_height=-1;
			get_int_keyval(config_file, "View", "WindowWidth", &size_width);
			get_int_keyval(config_file, "View", "WindowHeight", &size_height);
			if (size_width > 0 && size_height > 0)
			{
				Mwd.window_width = size_width;
				Mwd.window_height = size_height;
				gtk_window_set_default_size(GTK_WINDOW(Mwd.window), size_width, size_height);
			}

			Mwd.window_maximized = g_key_file_get_boolean(config_file, "View", "WindowMaximized", NULL);
			if (Mwd.window_maximized)
				gtk_window_maximize(GTK_WINDOW(Mwd.window));
		}

		get_int_keyval(config_file, "Stats", "GamesWon", &Mwd.games_won);
		get_int_keyval(config_file, "Stats", "GamesLost", &Mwd.games_lost);
		get_int_keyval(config_file, "Stats", "GamesDrawn", &Mwd.games_drawn);
	}

	g_key_file_free(config_file);
	g_free(file_name);
}

void save_preferences(gboolean statsOnly)
{
	char *file_name = get_preferences_file_location();
	GKeyFile *config_file = g_key_file_new();
	g_key_file_load_from_file(config_file, file_name, G_KEY_FILE_NONE, NULL);

	if (!statsOnly)
	{
		g_key_file_set_string(config_file, "Game", "SelAILevelMenu", Mwd.sel_ailevel_menu);
		
		g_key_file_set_integer(config_file, "View", "WindowWidth", Mwd.window_width);
		g_key_file_set_integer(config_file, "View", "WindowHeight", Mwd.window_height);			
		g_key_file_set_boolean(config_file, "View", "WindowMaximized", Mwd.window_maximized);
	}
	g_key_file_set_integer(config_file, "Stats", "GamesWon", Mwd.games_won);
	g_key_file_set_integer(config_file, "Stats", "GamesLost", Mwd.games_lost);
	g_key_file_set_integer(config_file, "Stats", "GamesDrawn", Mwd.games_drawn);

	
	gsize length;
	gchar *data = g_key_file_to_data(config_file, &length, NULL);
	if (data != NULL)
	{
		FILE *f = fopen(file_name, "w");
		if (f != NULL)
		{
			fwrite(data, length, 1, f);
			fclose(f);
		}
		g_free(data);
	}
	
	g_key_file_free(config_file);
	g_free(file_name);
}

void set_menu_from_preferences()
{
	GtkCheckMenuItem *checkMenuItem = GTK_CHECK_MENU_ITEM(glade_xml_get_widget(GladeXml, Mwd.sel_ailevel_menu));
	if (checkMenuItem != NULL && gtk_check_menu_item_get_draw_as_radio(checkMenuItem))
		gtk_check_menu_item_set_active(checkMenuItem, TRUE);
}

void update_game_stats_label()
{
	gchar stats_text[256];
	g_snprintf(stats_text, sizeof(stats_text), _("Won: %d   Lost: %d   Drawn: %d"),
	    Mwd.games_won, Mwd.games_lost, Mwd.games_drawn);
	
	gtk_label_set_text(Mwd.label_stats, stats_text);
}

void increment_game_stats(GameEndType get)
{
	load_preferences(TRUE);

	switch(get)
	{
		case getWin:
			Mwd.games_won++;
			break;
		case getDraw:
			Mwd.games_drawn++;
			break;
		case getLost:
			Mwd.games_lost++;
			break;
	}

	update_game_stats_label();
	
	save_preferences(TRUE);
}

static GtkLabel *add_status_bar_label (const char *text)
{
	GtkWidget* status_bar = glade_xml_get_widget(GladeXml, "statusbar");
	GtkFrame* label_frame;
	GtkLabel* label;
	
	label = GTK_LABEL(gtk_label_new(text));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	
	label_frame = GTK_FRAME(gtk_frame_new(NULL));
	gtk_frame_set_shadow_type(label_frame, GTK_SHADOW_IN);
	
	gtk_container_add (GTK_CONTAINER(label_frame), GTK_WIDGET(label));
	gtk_box_pack_start(GTK_BOX(status_bar), GTK_WIDGET(label_frame), FALSE, FALSE, 3);
	gtk_box_reorder_child(GTK_BOX(status_bar), GTK_WIDGET(label_frame), 0);
	
	gtk_widget_show(GTK_WIDGET(label_frame));
	gtk_widget_show(GTK_WIDGET(label));
	
	return label;
}

static void setup_status_bar()
{
	Mwd.label_stats = add_status_bar_label("Won: 0   Lost: 0   Drawn: 0");
}

static void destroy (GtkWidget *widget, gpointer data)
{
	load_preferences(TRUE);
	save_preferences(FALSE);
	gtk_main_quit ();
}

gboolean on_mainwindow_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
	if (!is_mainwnd_maximized())
	{
		Mwd.window_width = event->width;
		Mwd.window_height = event->height;
	}
	return FALSE;
}

gboolean on_mainwindow_window_state_event(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{	
	Mwd.window_maximized = ((event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0);
	return FALSE;
}

static gboolean on_drawingarea_expose_event (GtkWidget *widget, 
    GdkEventExpose *event, gpointer user_data)
{
	game.DrawExpose();
	return TRUE;
}

static gboolean on_drawingarea_button_press_event(GtkWidget *widget,
	GdkEventButton *event, gpointer user_data)
{
	if (event->type == GDK_2BUTTON_PRESS) //double click
	{
		game.PutCardAtPositionDown((int)event->x, (int)event->y);
		gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);
	}
	if (event->type == GDK_BUTTON_PRESS && event->button==1)
	{
		game.ClickMessage((int)event->x, (int)event->y);
		game.AttemptBeginDrag((int)event->x, (int)event->y);
	}
	return TRUE;
}

static gboolean on_drawingarea_button_release_event(GtkWidget *widget,
	GdkEventButton *event, gpointer user_data)
{
	if (event->type == GDK_BUTTON_RELEASE && event->button==1)
	{
		game.EndDrag((int)event->x, (int)event->y);
	}
	return TRUE;
}

static gboolean on_drawingarea_motion_notify_event(GtkWidget *widget,
	GdkEventMotion *event, gpointer user_data)
{
	gboolean btnPressed = ((event->state & GDK_BUTTON1_MASK) != 0);
	game.DragMove((int)event->x, (int)event->y, btnPressed);
	return TRUE;
}

void on_toolbtnNew_clicked (GtkToolButton *toolbutton, gpointer *user_data)
{
	game.Init();
	gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);
}

void on_menuNew_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	game.Init();
	gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);
}

void on_menuQuit_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_destroy(Mwd.window);
}

void on_rmenuDiffcEasy_toggled (GtkCheckMenuItem *checkmenuitem, gpointer user_data)
{
	if (checkmenuitem->active)
	{
		g_strlcpy(Mwd.sel_ailevel_menu, glade_get_widget_name(GTK_WIDGET(checkmenuitem)), sizeof(Mwd.sel_ailevel_menu));
		game.SetAI(aieEasy);
	}
}

void on_rmenuDiffcMedium_toggled (GtkCheckMenuItem *checkmenuitem, gpointer user_data)
{	
	if (checkmenuitem->active)
	{
		g_strlcpy(Mwd.sel_ailevel_menu, glade_get_widget_name(GTK_WIDGET(checkmenuitem)), sizeof(Mwd.sel_ailevel_menu));
		game.SetAI(aieMedium);
	}
}

void on_rmenuDiffcHard_toggled (GtkCheckMenuItem *checkmenuitem, gpointer user_data)
{
	if (checkmenuitem->active)
	{
		g_strlcpy(Mwd.sel_ailevel_menu, glade_get_widget_name(GTK_WIDGET(checkmenuitem)), sizeof(Mwd.sel_ailevel_menu));
		game.SetAI(aieHard);
	}
}

void on_menuHelpContents_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gchar *help_path = installation_path(HELP_FILE);
	gchar *help_url = g_strdup_printf("file://%s", help_path);
	open_url(help_url);
	g_free(help_url);
	g_free(help_path);
}

static void on_menuAbout_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *aboutdialog;
	aboutdialog = glade_xml_get_widget(GladeXml, "aboutdialog");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutdialog), VERSION);
	gtk_dialog_run(GTK_DIALOG(aboutdialog));
	gtk_widget_hide(aboutdialog);
}

static void on_aboutdialog_close (GtkDialog *dialog, gpointer user_data)
{
	gtk_dialog_response(dialog, GTK_RESPONSE_OK);
}

GtkWidget* main_window_create (void)
{
	memset(&Mwd, 0, sizeof(MainWindowData));
	g_strlcpy(Mwd.sel_ailevel_menu, "rmenuDiffcHard", sizeof(Mwd.sel_ailevel_menu));

	if (!load_cards_resource())
		return NULL;
	
	Mwd.window = glade_xml_get_widget(GladeXml, "mainwindow");
	Mwd.drawarea = glade_xml_get_widget(GladeXml, "drawingarea");

	setup_status_bar();

	g_signal_connect(G_OBJECT (Mwd.window), "destroy", G_CALLBACK (destroy), NULL);
	glade_xml_signal_connect(GladeXml, "on_mainwindow_configure_event", G_CALLBACK(&on_mainwindow_configure_event));
	glade_xml_signal_connect(GladeXml, "on_mainwindow_window_state_event", G_CALLBACK(&on_mainwindow_window_state_event));

	
	glade_xml_signal_connect(GladeXml, "on_drawingarea_expose_event", G_CALLBACK(&on_drawingarea_expose_event));
	glade_xml_signal_connect(GladeXml, "on_drawingarea_button_press_event", G_CALLBACK(&on_drawingarea_button_press_event));
	glade_xml_signal_connect(GladeXml, "on_drawingarea_button_release_event", G_CALLBACK(&on_drawingarea_button_release_event));
	glade_xml_signal_connect(GladeXml, "on_drawingarea_motion_notify_event", G_CALLBACK(&on_drawingarea_motion_notify_event));

	glade_xml_signal_connect(GladeXml, "on_toolbtnNew_clicked", G_CALLBACK(&on_toolbtnNew_clicked));
	glade_xml_signal_connect(GladeXml, "on_menuNew_activate", G_CALLBACK(&on_menuNew_activate));
	glade_xml_signal_connect(GladeXml, "on_menuQuit_activate", G_CALLBACK(&on_menuQuit_activate));

	glade_xml_signal_connect(GladeXml, "on_rmenuDiffcEasy_toggled", G_CALLBACK(&on_rmenuDiffcEasy_toggled));
	glade_xml_signal_connect(GladeXml, "on_rmenuDiffcMedium_toggled", G_CALLBACK(&on_rmenuDiffcMedium_toggled));
	glade_xml_signal_connect(GladeXml, "on_rmenuDiffcHard_toggled", G_CALLBACK(&on_rmenuDiffcHard_toggled));

	glade_xml_signal_connect(GladeXml, "on_menuHelpContents_activate", G_CALLBACK(&on_menuHelpContents_activate));
	glade_xml_signal_connect(GladeXml, "on_menuAbout_activate", G_CALLBACK(&on_menuAbout_activate));
	glade_xml_signal_connect(GladeXml, "on_aboutdialog_close", G_CALLBACK(&on_aboutdialog_close));
	
	load_preferences(FALSE);
	set_menu_from_preferences();
	update_game_stats_label();

	game.InitGraphics();
	game.Init();
	
	return Mwd.window;
}

void main_window_data_cleanup ()
{
	if (Mwd.cards_res != NULL)
		g_object_unref(Mwd.cards_res);
}

