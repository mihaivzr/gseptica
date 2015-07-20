/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * game.cpp
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

#include "game.h"
#include "mainwindow.h"
#include "defines.h"

#include <gtk/gtk.h>
#include <glib.h>

extern MainWindowData Mwd;



Game::Game(): 
	GameEasyAI(*this), GameMediumAI(*this), GameHardAI(*this)
{
	packet_cards = g_array_new(FALSE, TRUE, sizeof(Card));
	myhand_cards = g_array_new(FALSE, TRUE, sizeof(Card));
	ophand_cards = g_array_new(FALSE, TRUE, sizeof(Card));
	deck_cards = g_array_new(FALSE, TRUE, sizeof(Card));
	mytaken_cards = g_array_new(FALSE, TRUE, sizeof(Card));
	optaken_cards = g_array_new(FALSE, TRUE, sizeof(Card));
	dragging = false;
	dragX = dragY = 0;
	dragCard.number = -1;
	dragCard.suit = 0;
	dragCardIndex = -1;
	width = height = 0;
	state = gsMyTurn;
	firstTurn = gtMyTurn;
	animStep = animStepsNr = 0;
	SetAI(aieHard);
	donebtnText = NULL;
	timeoutSID = 0;
	stpixmap = NULL;
	stgc = NULL;
}

Game::~Game()
{
	g_array_free(packet_cards, TRUE);
	g_array_free(myhand_cards, TRUE);
	g_array_free(ophand_cards, TRUE);
	g_array_free(deck_cards, TRUE);
	g_array_free(mytaken_cards, TRUE);
	g_array_free(optaken_cards, TRUE);
	if (donebtnText != NULL)
		g_object_unref(donebtnText);
	if (stpixmap != NULL)
	{
		g_object_unref(stgc);
		g_object_unref(stpixmap);	
	}
}

void Game::FillSourceDeck()
{
	g_array_set_size(packet_cards, 0);
	for(int i=0; i<4; i++)
		for(int j=7; j<=14; j++)
		{
			Card card(i, j);
			g_array_append_val(packet_cards, card);
		}
}

void Game::RandomizeSourceDeck()
{
	int length = packet_cards->len;
	for(int i=0; i<400; i++)
	{
		int c1 = g_random_int_range(0, length-1);
		int c2 = g_random_int_range(0, length-1);

		Card aux = g_array_index(packet_cards, Card, c1);
		g_array_index(packet_cards, Card, c1) = g_array_index(packet_cards, Card, c2);
		g_array_index(packet_cards, Card, c2) = aux;
	}
}

void Game::DistributeCards()
{
	int myhn = 4 - myhand_cards->len;
	int ophn = 4 - ophand_cards->len;
	if (myhn + ophn > packet_cards->len)
	{
		myhn = packet_cards->len / 2;
		ophn = packet_cards->len / 2;
	}
	for(int i=0; i<myhn; i++)
	{
		int index = packet_cards->len - 1;
		g_array_append_val(myhand_cards, g_array_index(packet_cards, Card, index));
		g_array_remove_index(packet_cards, index);
	}
	for(int i=0; i<ophn; i++)
	{
		int index = packet_cards->len - 1;
		g_array_append_val(ophand_cards, g_array_index(packet_cards, Card, index));
		g_array_remove_index(packet_cards, index);
	}
}

void Game::Init()
{
	if (timeoutSID > 0)
	{
		g_source_remove(timeoutSID);
		timeoutSID = 0;
	}
	
	g_array_set_size(packet_cards, 0);
	g_array_set_size(myhand_cards, 0);
	g_array_set_size(ophand_cards, 0);
	g_array_set_size(deck_cards, 0);
	g_array_set_size(mytaken_cards, 0);
	g_array_set_size(optaken_cards, 0);
	dragging = false;
	dragCardIndex = -1;
	
	FillSourceDeck();
	RandomizeSourceDeck();
	DistributeCards();

	GameEasyAI.Init();
	GameMediumAI.Init();
	GameHardAI.Init();

	double rnd = g_random_double();
	if (rnd < 0.5)
	{
		state = gsMyTurn;
		firstTurn = gtMyTurn;
	}else
	{
		firstTurn = gtOpTurn;
		state = gsCompTurn;
		timeoutSID = g_timeout_add(0, &StateProcessor, this);
	}
}


void Game::InitGraphics()
{
	donebtnText = gtk_widget_create_pango_layout(Mwd.drawarea, _("Done"));
}

void Game::DrawExpose()
{
	GdkDrawable *draws = Mwd.drawarea->window;
	GdkGC *gc = gdk_gc_new(draws);
	
	DrawToStPixmap();
	DrawStPixmap(draws, gc);

	g_object_unref(gc);
}

void Game::Draw(GdkDrawable *draws, GdkGC* gc)
{
	gdk_drawable_get_size(draws, &width, &height);

	GdkColor color = {0, 0, 20000, 0};
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_gc_set_rgb_bg_color(gc, &color);
	gdk_draw_rectangle(draws, gc, TRUE, 0, 0, width, height);

	DrawPacket (draws, gc);
	
	DrawMyHand (draws, gc);
	DrawOpHand (draws, gc);

	DrawDeckCards (draws, gc);

	DrawMyTakenCards (draws, gc);
	DrawOpTakenCards (draws, gc);

	DrawDoneButton (draws, gc);

	DrawTakenCardsText (draws, gc);		
}

void Game::DrawPacket(GdkDrawable *draws, GdkGC* gc)
{
	if (packet_cards->len > 0)
	{
		int packetX = 20;
		int packetY = (height - Mwd.card_height) / 2;
		
		for(int i=0; i<3; i++)
			draw_cards_back(draws, gc, packetX - i*2, packetY - i*2);
	}
}

void Game::DrawMyHand(GdkDrawable *draws, GdkGC* gc)
{
	int myhandX, myhandY;
	GetMyCardsPosition(&myhandX, &myhandY);
	
	for(int i=0; i<myhand_cards->len; i++)
	{
		if (dragging && dragCardIndex==i)
			continue;
		
		Card card = g_array_index(myhand_cards, Card, i);
		draw_card (draws, gc, myhandX + i*(Mwd.card_width+4), myhandY, card.suit, card.number);
	}
}

void Game::DrawOpHand(GdkDrawable *draws, GdkGC* gc)
{
	int ophandX = (width - ((Mwd.card_width + 4) * 4)) / 2;
	int ophandY = (height/3 - Mwd.card_height)/2;
	
	for(int i=0; i<ophand_cards->len; i++)
	{
		draw_cards_back (draws, gc, ophandX + i*(Mwd.card_width+4), ophandY);
	}
}

void Game::DrawDeckCards(GdkDrawable *draws, GdkGC* gc)
{
	int deckX, deckY;
	GetDeckPosition(&deckX, &deckY);
	
	if (deck_cards->len > 0)
	{
		Card card = g_array_index(deck_cards, Card, deck_cards->len-1);
		draw_card (draws, gc, deckX, deckY, card.suit, card.number);
	}else
	{
		GdkColor color = {0, 0, 40000, 0};
		gdk_gc_set_rgb_fg_color(gc, &color);
		gdk_gc_set_rgb_bg_color(gc, &color);
		gdk_draw_rectangle(draws, gc, TRUE, deckX, deckY, Mwd.card_width, Mwd.card_height);

		GdkColor bcolor = {0, 0, 20000, 0};
		gdk_gc_set_rgb_fg_color(gc, &bcolor);
		gdk_gc_set_rgb_bg_color(gc, &bcolor);
		gdk_draw_rectangle(draws, gc, TRUE, deckX + 5, deckY + 5, Mwd.card_width-10, Mwd.card_height-10);
	}
}

void Game::DrawMyTakenCards(GdkDrawable *draws, GdkGC* gc)
{
	if (mytaken_cards->len > 0)
	{
		int mytakenX = width - Mwd.card_width - 10;
		int mytakenY = height - 30;
		
		for(int i=0; i<2; i++)
			draw_cards_back(draws, gc, mytakenX + i*2, mytakenY + i*2);
	}
}

void Game::DrawOpTakenCards(GdkDrawable *draws, GdkGC* gc)
{
	if (optaken_cards->len > 0)
	{
		int optakenX = width - Mwd.card_width - 10;
		int optakenY = 30 - Mwd.card_height;
		
		for(int i=0; i<2; i++)
			draw_cards_back(draws, gc, optakenX + i*2, optakenY - i*2);
	}
}

void Game::DrawDoneButton(GdkDrawable *draws, GdkGC* gc)
{
	if (IsDoneButtonVisible ())
	{
		int btnWidth, btnHeight, btnX, btnY;
		GetDoneButtonPosition(&btnX, &btnY, &btnWidth, &btnHeight);

		GdkColor color = {0, 0, 60000, 0};
		gdk_gc_set_rgb_fg_color(gc, &color);
		gdk_gc_set_rgb_bg_color(gc, &color);

		gdk_draw_rectangle(draws, gc, TRUE, btnX, btnY, btnWidth, btnHeight);

		int textWidth, textHeight;
		pango_layout_get_pixel_size(donebtnText, &textWidth, &textHeight);
		GdkColor textColor = { 0, 60000, 0, 0};
		gdk_draw_layout_with_colors(draws, gc, btnX + (btnWidth-textWidth)/2, btnY + (btnHeight-textHeight)/2, 
			donebtnText, &textColor, NULL);
	}
}

void Game::DrawTakenCardsText(GdkDrawable *draws, GdkGC* gc)
{
	char mytext[128], optext[128];
	int myPoints, opPoints;
	CalculatePoints(&myPoints, &opPoints);
	g_snprintf(mytext, sizeof(mytext), _("Cards: %d\nPoints: %d"), mytaken_cards->len, myPoints);
	g_snprintf(optext, sizeof(optext), _("Cards: %d\nPoints: %d"), optaken_cards->len, opPoints);

	GdkColor textColor = { 0, 40000, 40000, 60000};
	PangoLayout *mytextLayout = gtk_widget_create_pango_layout(Mwd.drawarea, mytext);
	PangoLayout *optextLayout = gtk_widget_create_pango_layout(Mwd.drawarea, optext);
	int textWidth, textHeight;
	pango_layout_get_pixel_size(mytextLayout, &textWidth, &textHeight);

	int xdisplacement = 80;
	if (textWidth + 20 > xdisplacement)
		xdisplacement = textWidth + 20;
	
	int myX = width - xdisplacement, myY = height - 50 - textHeight;
	int opX = width - xdisplacement, opY = 50;

	gdk_draw_layout_with_colors(draws, gc, myX, myY, mytextLayout, &textColor, NULL);
	gdk_draw_layout_with_colors(draws, gc, opX, opY, optextLayout, &textColor, NULL);
	
	g_object_unref(mytextLayout);
	g_object_unref(optextLayout);
}

void Game::DrawToStPixmap()
{
	EnsureStPixmap(Mwd.drawarea->window);
	Draw(stpixmap, stgc);
}

void Game::DrawStPixmap(GdkDrawable *draws, GdkGC* gc)
{
	gdk_draw_drawable(draws, gc, stpixmap, 0, 0, 0, 0, width, height);
}

void Game::EnsureStPixmap(GdkDrawable *draws)
{
	int draws_width, draws_height;
	gdk_drawable_get_size(draws, &draws_width, &draws_height);
	if (stpixmap != NULL)
	{
		int pix_width, pix_height;
		gdk_drawable_get_size(stpixmap, &pix_width, &pix_height);
		
		if (draws_width != pix_width || draws_height != pix_height)
		{
			g_object_unref(stgc);
			g_object_unref(stpixmap);			
			stpixmap = NULL;
			stgc = NULL;
		}
	}
	if (stpixmap == NULL)
	{
		stpixmap = gdk_pixmap_new(draws, draws_width, draws_height, -1);
		stgc = gdk_gc_new(stpixmap);
	}
}

void Game::GetDeckPosition(int *deckX, int *deckY)
{
	*deckX = (width - Mwd.card_width)/2;
	*deckY = height/3 + (height/3 - Mwd.card_height)/2;
}

void Game::GetMyCardsPosition(int *myhandX, int *myhandY)
{
	*myhandX = (width - ((Mwd.card_width + 4) * 4)) / 2;
	*myhandY = (height * 2) / 3 + (height/3 - Mwd.card_height)/2;	
}

gboolean Game::HasContinuation(GArray *hand_cards)
{
	if (deck_cards->len > 0)
	{
		Card deck_card = g_array_index(deck_cards, Card, 0);
		Card *cards = (Card*)hand_cards->data;
		for(int i=0; i<hand_cards->len; i++)
			if (cards[i].number == 7 || cards[i].number == deck_card.number)
				return TRUE;
		return FALSE;
	}else
		return TRUE;
}


void Game::PutCardAtPositionDown(int x, int y)
{
	if (state == gsMyTurn)
	{
		int index = GetMyCardAtPosition (x, y);
		if (index >= 0)
			PutMyCardDown (index);
	}
}

void Game::PutMyCardDown(int index)
{
	if (state == gsMyTurn && index < myhand_cards->len && index>=0 && CanPutMyCardDown (index))
	{	
		g_array_append_val(deck_cards, g_array_index(myhand_cards, Card, index));
		g_array_remove_index(myhand_cards, index);

		gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);
		state = gsCompTurn;
		timeoutSID = g_timeout_add(700, &StateProcessor, this);
	}
}

gboolean Game::CanPutMyCardDown(int index)
{
	if (deck_cards->len == 0 || firstTurn == gtOpTurn)
		return TRUE;
	Card cbase = g_array_index(deck_cards, Card, 0);
	Card card = g_array_index(myhand_cards, Card, index);
	if (card.number == 7 || card.number == cbase.number)
		return TRUE;
	return FALSE;
}

int Game::GetMyCardAtPosition(int x, int y)
{
	int myhandX, myhandY;
	GetMyCardsPosition(&myhandX, &myhandY);

	for(int i=0; i<myhand_cards->len; i++)
	{
		int xpos = myhandX + (i*(Mwd.card_width+4));
		if (x >= xpos && x < xpos + Mwd.card_width && 
		    y >= myhandY && y < myhandY + Mwd.card_height)
			return i;
	}
	return -1;
}


gboolean Game::StateProcessor(gpointer pth)
{
	Game* th = (Game*)pth;

	switch(th->state)
	{
		case gsCompTurn:
			th->ComputerMove ();
			break;
		case gsHandEnd:
			th->ProcessHandEnd ();			

			th->DrawToStPixmap();
			th->animStep = 0;
			th->animStepsNr = 6;
			th->state = gsHandEndAnim;
			th->timeoutSID = g_timeout_add(1, &StateProcessor, th);
			
			break;
		case gsHandEndAnim:
			if (th->animStep >= th->animStepsNr)
			{
				gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);
				
				if (th->packet_cards->len > 0 || th->myhand_cards->len > 0 || th->ophand_cards->len > 0)
				{
					th->state = gsDistribute;
					th->timeoutSID = g_timeout_add(300, &StateProcessor, th);
				}else
					th->EndGame();			
			}else
			{
				th->DrawHandEndAnim();
				th->timeoutSID = g_timeout_add(50, &StateProcessor, th);
			}
			th->animStep++;
			break;
			
		case gsDistribute:
			th->DistributeCards ();
			gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);
			if (th->firstTurn == gtMyTurn)
			{
				th->state = gsMyTurn;
			}else
			{
				th->state = gsCompTurn;
				th->timeoutSID = g_timeout_add(400, &StateProcessor, th);
			}
			break;
	}
	
	return FALSE;
}

void Game::ComputerMove()
{
	if (firstTurn == gtMyTurn)
	{
		g_assert(myhand_cards->len < ophand_cards->len && ophand_cards->len > 0);

		int opcardIdx = ResponseComputerMove();
		PutOpCardDown (opcardIdx);
		gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);

		if (deck_cards->len > 1 && deck_cards->len%2==0)
		{
			Card ctop = g_array_index(deck_cards, Card, deck_cards->len-1);
			Card cbase = g_array_index(deck_cards, Card, 0);
			if ((ctop.number == cbase.number || ctop.number==7) && HasContinuation(myhand_cards))
				state = gsMyTurn;
		}

		if (state != gsMyTurn)
		{
			state = gsHandEnd;
			timeoutSID = g_timeout_add(700, &StateProcessor, this);		
		}
	}else
	{
		if (deck_cards->len == 0)
		{
			g_assert(ophand_cards->len > 0);
			
			int opcardIdx = FirstComputerMove();
			PutOpCardDown (opcardIdx);
			
			gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);
			state = gsMyTurn;
		}else
		{
			int opcardIdx = -1;
			if (CanComputerContinue())
				opcardIdx = ContinuationComputerMove();
			if (opcardIdx >= 0)
			{
				PutOpCardDown (opcardIdx);
				gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);

				state = gsMyTurn;
			}else
			{
				state = gsHandEnd;
				timeoutSID = g_timeout_add(1, &StateProcessor, this);	
			}
		}
	}
}

int Game::ResponseComputerMove()
{
	return currentAI->ResponseMove();
}

int Game::FirstComputerMove()
{
	return currentAI->FirstMove();
}

int Game::ContinuationComputerMove()
{
	return currentAI->ContinuationMove();
}

gboolean Game::CanComputerContinue()
{
	if (firstTurn == gtOpTurn && deck_cards->len > 1 && deck_cards->len%2==0)
	{
		Card ctop = g_array_index(deck_cards, Card, deck_cards->len-1);
		Card cbase = g_array_index(deck_cards, Card, 0);
		if ((ctop.number == cbase.number || ctop.number==7) && HasContinuation(ophand_cards))
			return TRUE;		
	}
	return FALSE;
}

void Game::PutOpCardDown(int index)
{
	g_array_append_val(deck_cards, g_array_index(ophand_cards, Card, index));
	g_array_remove_index(ophand_cards, index);
}

void Game::ProcessHandEnd()
{
	if (deck_cards->len > 1)
	{
		Card ctop = g_array_index(deck_cards, Card, deck_cards->len-1);
		Card cbase = g_array_index(deck_cards, Card, 0);
		GArray *cards_array;
		if (ctop.number == cbase.number || ctop.number==7)
			cards_array = (firstTurn == gtMyTurn) ? optaken_cards : mytaken_cards;			
		else
			cards_array = (firstTurn == gtMyTurn) ? mytaken_cards : optaken_cards;	

		for(int i=0; i<deck_cards->len; i++)
				g_array_append_val(cards_array, g_array_index(deck_cards, Card, i));

		firstTurn = (cards_array == mytaken_cards) ? gtMyTurn : gtOpTurn;
	}
	g_array_set_size(deck_cards, 0);
}

void Game::EndGame()
{
	state = gsEnd;
	int mypoints, oppoints;
	CalculatePoints(&mypoints, &oppoints);
	const char *endMessage = (mypoints > oppoints) ? _("You won!") : 
		((mypoints < oppoints) ? _("You lost!") : _("Draw!"));

	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(Mwd.window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
	    GTK_BUTTONS_OK, _("Game ended. %s"), endMessage);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	increment_game_stats((mypoints > oppoints) ? getWin : ((mypoints < oppoints) ? getLost : getDraw));
}

void Game::CalculatePoints(int *myPoints, int *opPoints)
{
	int i;
	Card *cards;
	*myPoints = 0;
	cards = (Card*)mytaken_cards->data;
	for(i=0; i<mytaken_cards->len; i++)
		if (cards[i].number == 10 || cards[i].number == 11)
			(*myPoints)++;
	*opPoints = 0;
	cards = (Card*)optaken_cards->data;
	for(i=0; i<optaken_cards->len; i++)
		if (cards[i].number == 10 || cards[i].number == 11)
			(*opPoints)++;
}

void Game::ClickMessage(int x, int y)
{
	if (IsDoneButtonVisible ())
	{
		int btnWidth, btnHeight, btnX, btnY;
		GetDoneButtonPosition(&btnX, &btnY, &btnWidth, &btnHeight);
		if (x >= btnX && x < btnX + btnWidth && y >= btnY && y < btnY + btnHeight)
			ClickDoneButton();
	}
}

void Game::ClickDoneButton()
{
	state = gsHandEnd;
	timeoutSID = g_timeout_add(100, &StateProcessor, this);	
}

gboolean Game::IsDoneButtonVisible()
{
	if (state == gsMyTurn && firstTurn == gtMyTurn && deck_cards->len > 1 && deck_cards->len%2==0)
	{
		Card ctop = g_array_index(deck_cards, Card, deck_cards->len-1);
		Card cbase = g_array_index(deck_cards, Card, 0);
		if ((ctop.number == cbase.number || ctop.number==7) && HasContinuation(myhand_cards))
			return TRUE;
	}
	return FALSE;
}

void Game::GetDoneButtonPosition(int *btnX, int *btnY, int *btnWidth, int *btnHeight)
{
	int textWidth, textHeight;
	pango_layout_get_pixel_size(donebtnText, &textWidth, &textHeight);
	
	*btnWidth = 50;
	*btnHeight = 30;
	if (*btnWidth < textWidth + 20)
		*btnWidth = textWidth + 20;
	if (*btnHeight < textHeight + 10)
		*btnHeight = textHeight + 10;
	int deckX, deckY;
	GetDeckPosition(&deckX, &deckY);
	*btnX = deckX + Mwd.card_width + 20;
	*btnY = (height - *btnHeight) / 2;
}

void Game::AttemptBeginDrag(int x, int y)
{
	if (state == gsMyTurn)
	{
		int index = GetMyCardAtPosition (x, y);
		if (index >= 0 && CanPutMyCardDown(index))
		{
			int myhandX, myhandY;
			GetMyCardsPosition(&myhandX, &myhandY);
			
			int cposX = myhandX + (index * (Mwd.card_width + 4)), cposY = myhandY;
			dragging = true;
			dragX = x - cposX;
			dragY = y - cposY;
			dragCard = g_array_index(myhand_cards, Card, index);
			dragCardIndex = index;

			DrawToStPixmap();
			DragDraw(x, y);
		}
	}
}

void Game::EndDrag(int x, int y)
{
	if (dragging)
	{
		dragging = false;

		int deckX, deckY;
		GetDeckPosition(&deckX, &deckY);		

		gboolean ontarget = (x - dragX < deckX + Mwd.card_width && x - dragX + Mwd.card_width >= deckX && 
		    y - dragY < deckY + Mwd.card_height && y - dragY + Mwd.card_height >= deckY);

		if (ontarget)
		{
			PutMyCardDown(dragCardIndex);
		}
		dragCardIndex = -1;
		
		gdk_window_invalidate_rect(Mwd.drawarea->window, NULL, TRUE);
	}
}

void Game::DragMove(int x, int y, gboolean buttonPressed)
{
	if (dragging)
	{
		if (!buttonPressed)
		{
			EndDrag(x, y);
			return;
		}

		DragDraw(x, y);
	}
}

void Game::DragDraw(int x, int y)
{
	if (dragging)
	{
		GdkDrawable *draws = Mwd.drawarea->window;
		GdkRectangle rect = { 0, 0, width, height };
		gdk_window_begin_paint_rect(draws, &rect);
		GdkGC * gc = gdk_gc_new(draws);

		DrawStPixmap(draws, gc);
		draw_card(draws, gc, x - dragX, y - dragY, dragCard.suit, dragCard.number);
	
		g_object_unref(gc);
		gdk_window_end_paint(draws);
	}
}

void Game::DrawHandEndAnim()
{
	int minPosX = width - 180, maxPosX = width - Mwd.card_width - 10;
	int minPosY = (height - Mwd.card_height)/2, 
		maxPosY = (firstTurn==gtMyTurn) ? height - 50 : 50-Mwd.card_height;
	int cposX = minPosX + (maxPosX - minPosX)*animStep/animStepsNr;
	int cposY = minPosY + (maxPosY - minPosY)*animStep/animStepsNr;

	GdkDrawable *draws = Mwd.drawarea->window;
	GdkRectangle rect = { 0, 0, width, height };
	gdk_window_begin_paint_rect(draws, &rect);
	GdkGC * gc = gdk_gc_new(draws);

	DrawStPixmap(draws, gc);
	draw_cards_back(draws, gc, cposX, cposY);

	g_object_unref(gc);
	gdk_window_end_paint(draws);
}


void Game::SetAI(AIEnum ai)
{
	selectedAI = ai;
	switch(ai)
	{
		case aieEasy:
			currentAI = &GameEasyAI;
			break;
		case aieMedium:
			currentAI = &GameMediumAI;
			break;
		case aieHard:
			currentAI = &GameHardAI;
			break;
	}
}

