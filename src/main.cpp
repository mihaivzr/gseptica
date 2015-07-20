/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.cpp
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
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "mainwindow.h"
#include "defines.h"

GladeXML *GladeXml;

char *installation_path(const char *relPath)
{
#ifndef SEPT_DEBUG
	
#ifndef WIN32
	return g_strdup_printf(DATA_DIR"/gseptica/%s", relPath);
#else
	char *exeDir = g_win32_get_package_installation_directory(NULL, NULL);
	char *path = g_strdup_printf("%s/share/gseptica/%s", exeDir, relPath);
	g_free(exeDir);
	return path;
#endif

#else
	return g_strdup(relPath);
#endif
}

void open_url(const char *url)
{
#ifndef WIN32
	gnome_vfs_url_show(url);
#else
	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#endif
}


static void on_aboutdialog_url_activated (GtkAboutDialog *about, const gchar *url, 
										 gpointer data)
{
	open_url(url);
}

static void on_aboutdialog_email_activated (GtkAboutDialog *about, const gchar *url, 
										 	gpointer data)
{
	GString *s = g_string_new("mailto:");
	g_string_append(s, url);
	open_url(s->str);
	g_string_free(s, TRUE);
}


int main (int argc, char *argv[])
{
	int return_val = 0;
	
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gtk_set_locale ();
	gtk_init (&argc, &argv);

	gnome_vfs_init();
	gtk_about_dialog_set_url_hook(&on_aboutdialog_url_activated, NULL, NULL);
	gtk_about_dialog_set_email_hook(&on_aboutdialog_email_activated, NULL, NULL);

	gchar *glade_path = installation_path(GLADE_FILE);
	GladeXml = glade_xml_new(glade_path, NULL, NULL);
	if (GladeXml != NULL)
	{
		GtkWidget *window = main_window_create();
		if (window != NULL)
		{
			gtk_widget_show (window);
			gtk_main ();
		}else
			return_val = 2;
		main_window_data_cleanup();
		g_object_unref(GladeXml);
	}else
	{
		g_printerr("Error loading %s \nThe application might not be correctly installed.\n", glade_path);
		return_val = 1;
	}
	g_free(glade_path);
	return return_val;
}
