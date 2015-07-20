/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ai.cpp
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

#include "ai.h"
#include "game.h"

#include <glib.h>


BaseAI::BaseAI(Game &g):gm(g)
{

}

BaseAI::~BaseAI()
{
}

int BaseAI::StandardContinuationMove()
{
	Card cbase = g_array_index(gm.deck_cards, Card, 0);
	gboolean point_card = (cbase.number == 10 || cbase.number == 11);
	Card *cards = (Card*)gm.ophand_cards->data;

	for(int i=0; i<gm.ophand_cards->len; i++)
		if (cards[i].number == cbase.number || (cards[i].number == 7 && point_card))
			return i;
	return -1;
}

int EasyAI::ResponseMove()
{
	Card *cards = (Card*)gm.ophand_cards->data;
	int ncards = gm.ophand_cards->len;
	Card cbase = g_array_index(gm.deck_cards, Card, 0);
	gboolean continuation = (gm.deck_cards->len > 2);
	if (continuation)
	{
		int cmove = StandardContinuationMove();
		if (cmove >= 0)
			return cmove;
	}else
	{
		int pc7=0, pcbase=0;
		for(int i=0; i<ncards; i++)
			if (cards[i].number == 7) pc7++;
			else if (cards[i].number == cbase.number) pcbase++;

		if (pcbase > 0)
		{
			for(int i=0; i<ncards; i++)
				if (cards[i].number == cbase.number)
					return i;
		}else
		{
			int takesnr = pcbase + pc7;
			double rnd = g_random_double();
			if ((cbase.number == 10 || cbase.number == 11) && takesnr > (rnd < 0.6 ? 1 : 0) )
			{
				for(int i=0; i<ncards; i++)
					if (cards[i].number == cbase.number || cards[i].number==7)
						return i;
			}
		}
	}

	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7)
			return i;
	return 0;
}

int EasyAI::FirstMove()
{
	Card *cards = (Card*)gm.ophand_cards->data;
	int ncards = gm.ophand_cards->len;

	int pc10=0, pc11=0, pc7=0;
	for(int i=0; i<ncards; i++)
		if (cards[i].number == 7) pc7++;
		else if (cards[i].number == 10) pc10++;
		else if (cards[i].number == 11) pc11++;

	gboolean put10 = (pc10 > 0 && pc10 + pc7 > 1);
	gboolean put11 = (pc11 > 0 && pc11 + pc7 > 1);

	for(int i=0; i<ncards; i++)
		if (put10 && cards[i].number == 10)
			return i;
		else if (put11 && cards[i].number == 11)
			return i;

	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7)
			return i;
	
	return 0;
}

int EasyAI::ContinuationMove()
{
	return StandardContinuationMove();
}



int MediumAI::ResponseMove()
{
	Card *cards = (Card*)gm.ophand_cards->data;
	int ncards = gm.ophand_cards->len;
	Card cbase = g_array_index(gm.deck_cards, Card, 0);
	gboolean continuation = (gm.deck_cards->len > 2);

	int pc10=0, pc11=0, pc7=0, pcbase=0;
	for(int i=0; i<ncards; i++)
		if (cards[i].number == 7) pc7++;
		else if (cards[i].number == 10) pc10++;
		else if (cards[i].number == 11) pc11++;
		else if (cards[i].number == cbase.number) pcbase++;

	gboolean take10 = (cbase.number == 10 && pc10 + pc7 > (continuation ? 0 : 1));
	gboolean take11 = (cbase.number == 11 && pc11 + pc7 > (continuation ? 0 : 1));

	for(int i=0; i<ncards; i++)
		if (take10 && cards[i].number == 10)
			return i;
		else if (take11 && cards[i].number == 11)
			return i;
	for(int i=0; i<ncards; i++)
		if ((take10 || take11) && cards[i].number == 7)
			return i;

	if (pcbase > 0 && cbase.number != 10 && cbase.number != 11)
	{
		for(int i=0; i<ncards; i++)
			if (cards[i].number == cbase.number)
				return i;
	}

	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7 && cards[i].number != 10 && cards[i].number != 11)
			return i;
	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7)
			return i;	
	return ncards - 1;
}

int MediumAI::FirstMove()
{
	Card *cards = (Card*)gm.ophand_cards->data;
	int ncards = gm.ophand_cards->len;

	int pc10=0, pc11=0, pc7=0;
	for(int i=0; i<ncards; i++)
		if (cards[i].number == 7) pc7++;
		else if (cards[i].number == 10) pc10++;
		else if (cards[i].number == 11) pc11++;

	gboolean put10 = (pc10 > 0 && pc10 + pc7 > 1);
	gboolean put11 = (pc11 > 0 && pc11 + pc7 > 1);

	for(int i=0; i<ncards; i++)
		if (put10 && cards[i].number == 10)
			return i;
		else if (put11 && cards[i].number == 11)
			return i;

	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7 && cards[i].number != 10 && cards[i].number != 11)
			return i;
	
	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7)
			return i;
	
	return ncards-1;
}

int MediumAI::ContinuationMove()
{
	return StandardContinuationMove();
}



int HardAI::ResponseMove()
{
	Card *cards = (Card*)gm.ophand_cards->data;
	int ncards = gm.ophand_cards->len;
	Card cbase = g_array_index(gm.deck_cards, Card, 0);
	gboolean continuation = (gm.deck_cards->len > 2);

	if (continuation)
	{
		int cmv = StandardContinuationMove();
		if (cmv >= 0)
			return cmv;
	}else
	{
		int pc10=0, pc11=0, pc7=0, pcbase=0;
		for(int i=0; i<ncards; i++)
			if (cards[i].number == 7) pc7++;
			else if (cards[i].number == 10) pc10++;
			else if (cards[i].number == 11) pc11++;
			else if (cards[i].number == cbase.number) pcbase++;

		gboolean take10, take11;
		if (gm.packet_cards->len <= 4)
		{
			int p10hit = 0, p11hit = 0;
			Card *uscards = (Card*)gm.myhand_cards->data;
			int nuscards = gm.myhand_cards->len;
			for(int i=0; i<nuscards; i++)
				if (uscards[i].number == 7) { p10hit++; p11hit++; }
				else if (uscards[i].number == 10) p10hit++;
				else if (uscards[i].number == 11) p11hit++;
			take10 = (cbase.number == 10 && pc10 + pc7 > p10hit);
			take11 = (cbase.number == 11 && pc11 + pc7 > p11hit);
		}else
		{
			double rnd = g_random_double();
			take10 = (cbase.number == 10 && pc10 + pc7 > (rnd < 0.6 ? 1 : 2));
			take11 = (cbase.number == 11 && pc11 + pc7 > (rnd < 0.6 ? 1 : 2));
		}

		for(int i=0; i<ncards; i++)
			if (take10 && cards[i].number == 10)
				return i;
			else if (take11 && cards[i].number == 11)
				return i;
		for(int i=0; i<ncards; i++)
			if ((take10 || take11) && cards[i].number == 7)
				return i;

		if (pcbase > 0 && cbase.number != 10 && cbase.number != 11)
		{
			for(int i=0; i<ncards; i++)
				if (cards[i].number == cbase.number)
					return i;
		}
	}

	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7 && cards[i].number != 10 && cards[i].number != 11)
			return i;
	return 0;
}

int HardAI::FirstMove()
{
	Card *cards = (Card*)gm.ophand_cards->data;
	int ncards = gm.ophand_cards->len;

	int pc10=0, pc11=0, pc7=0;
	for(int i=0; i<ncards; i++)
		if (cards[i].number == 7) pc7++;
		else if (cards[i].number == 10) pc10++;
		else if (cards[i].number == 11) pc11++;

	gboolean put10, put11;
	if (gm.packet_cards->len <= 4)
	{
		int p10hit = 0, p11hit = 0;
		Card *uscards = (Card*)gm.myhand_cards->data;
		int nuscards = gm.myhand_cards->len;
		for(int i=0; i<nuscards; i++)
			if (uscards[i].number == 7) { p10hit++; p11hit++; }
			else if (uscards[i].number == 10) p10hit++;
			else if (uscards[i].number == 11) p11hit++;
		put10 = (pc10 > 0 && pc10 + pc7 > p10hit);
		put11 = (pc11 > 0 && pc11 + pc7 > p11hit);
	}else
	{
		double rnd = g_random_double();
		put10 = (pc10 > 0 && pc10 + pc7 > (rnd < 0.4 ? 1 : 2));
		put11 = (pc11 > 0 && pc11 + pc7 > (rnd < 0.4 ? 1 : 2));
	}

	for(int i=0; i<ncards; i++)
		if (put10 && cards[i].number == 10)
			return i;
		else if (put11 && cards[i].number == 11)
			return i;

	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7 && cards[i].number != 10 && cards[i].number != 11)
			return i;
	for(int i=0; i<ncards; i++)
		if (cards[i].number != 7)
			return i;
	return 0;
}

int HardAI::ContinuationMove()
{
	return StandardContinuationMove();
}



