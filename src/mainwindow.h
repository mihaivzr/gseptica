/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mainwindow.h
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
 
#ifndef SEPT_MAINWINDOW_H
#define SEPT_MAINWINDOW_H

#include <gtk/gtk.h>

enum CardSuit { csClub = 0, csDiamond = 1, csHeart = 2, csLeaf = 3 };
enum GameEndType { getWin, getDraw, getLost };

struct MainWindowData
{
	GtkWidget *window, *drawarea;
	GdkPixbuf *cards_res;
	int card_width, card_height, res_width, res_height;

	gchar sel_ailevel_menu[128];
	int games_won, games_lost, games_drawn;

	GtkLabel *label_stats;
	int window_width, window_height;
	gboolean window_maximized;
};

GtkWidget* main_window_create ();
void main_window_data_cleanup ();

void draw_card(GdkDrawable *draws, GdkGC* gc, int x, int y, int card_suit, int card_nr);
void draw_cards_back(GdkDrawable *draws, GdkGC* gc, int x, int y);

void increment_game_stats(GameEndType get);

#endif /*SEPT_MAINWINDOW_H*/
