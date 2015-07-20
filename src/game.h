/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * game.h
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

#ifndef SEPT_GAME_H
#define SEPT_GAME_H

#include "ai.h"

#include <gtk/gtk.h>
#include <glib.h>
 
struct Card
{
	Card()
	{
		suit = 0;
		number = 0;
	}
	
	Card(int s, int n)
	{
		suit = s;
		number = n;
	}
	
	int suit;
	int number;
};


enum GameState { gsMyTurn, gsCompTurn, gsDistribute, gsHandEnd, gsHandEndAnim, gsEnd };
enum GameTurn { gtMyTurn, gtOpTurn };

class Game
{
public:
	Game();
	~Game();

	void InitGraphics();
	void Init();

	void DrawExpose();

	void ClickMessage(int x, int y);
	void PutCardAtPositionDown(int x, int y);
	void AttemptBeginDrag(int x, int y);
	void EndDrag(int x, int y);
	void DragMove(int x, int y, gboolean buttonPressed);

	void SetAI(AIEnum ai);
	AIEnum GetAI() { return selectedAI; }
	
	GArray *packet_cards;
	GArray *myhand_cards, *ophand_cards;
	GArray *deck_cards, *mytaken_cards, *optaken_cards;

	
protected:

	void FillSourceDeck();
	void RandomizeSourceDeck();
	void DistributeCards();

	void Draw(GdkDrawable *draws, GdkGC* gc);
	void DrawPacket(GdkDrawable *draws, GdkGC* gc);
	void DrawMyHand(GdkDrawable *draws, GdkGC* gc);
	void DrawOpHand(GdkDrawable *draws, GdkGC* gc);
	void DrawDeckCards(GdkDrawable *draws, GdkGC* gc);
	void DrawMyTakenCards(GdkDrawable *draws, GdkGC* gc);
	void DrawOpTakenCards(GdkDrawable *draws, GdkGC* gc);
	void DrawDoneButton(GdkDrawable *draws, GdkGC* gc);
	void DrawTakenCardsText(GdkDrawable *draws, GdkGC* gc);
	void DragDraw(int x, int y);
	void DrawHandEndAnim();
	void DrawToStPixmap();
	void DrawStPixmap(GdkDrawable *draws, GdkGC* gc);

	void EnsureStPixmap(GdkDrawable *draws);

	void GetDeckPosition(int *deckX, int *deckY);
	void GetMyCardsPosition(int *myhandX, int *myhandY);

	
	void PutMyCardDown(int index);
	gboolean CanPutMyCardDown(int index);

	int GetMyCardAtPosition(int x, int y);

	void ComputerMove();
	void PutOpCardDown(int index);

	int ResponseComputerMove();
	int FirstComputerMove();
	int ContinuationComputerMove();

	gboolean CanComputerContinue();
	
	void ProcessHandEnd();
	void EndGame();
	void CalculatePoints(int *myPoints, int *opPoints);

	static gboolean StateProcessor(gpointer pth);

	gboolean HasContinuation(GArray *hand_cards);
	
	void ClickDoneButton();
	gboolean IsDoneButtonVisible();
	void GetDoneButtonPosition(int *x, int *y, int *btnWidth, int *btnHeight);

	
	int width, height;
	GameState state;
	GameTurn firstTurn;

	gboolean dragging;
	int dragX, dragY;
	Card dragCard;
	int dragCardIndex;

	GdkPixmap *stpixmap;
	GdkGC *stgc;

	int animStep;
	int animStepsNr;

	BaseAI *currentAI;
	AIEnum selectedAI;
	EasyAI GameEasyAI;
	MediumAI GameMediumAI;
	HardAI GameHardAI;

	PangoLayout *donebtnText;
	int timeoutSID;
};

#endif  /*SEPT_GAME_H*/


