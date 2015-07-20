/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ai.h
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

#ifndef SEPT_AI_H
#define SEPT_AI_H

class Game;

enum AIEnum { aieEasy, aieMedium, aieHard };

class BaseAI
{
public:
	BaseAI(Game &g);
	virtual ~BaseAI();
	
	virtual int Init() {}
	virtual int ResponseMove() = 0;
	virtual int FirstMove() = 0;
	virtual int ContinuationMove() = 0;

protected:
	int StandardContinuationMove();

	Game &gm;
};

class EasyAI: public BaseAI
{
public:
	EasyAI(Game &g):BaseAI(g) {}
	
	virtual int ResponseMove();
	virtual int FirstMove();
	virtual int ContinuationMove();	
};

class MediumAI: public BaseAI
{
public:
	MediumAI(Game &g):BaseAI(g) {}
	
	virtual int ResponseMove();
	virtual int FirstMove();
	virtual int ContinuationMove();	
};

class HardAI: public BaseAI
{
public:
	HardAI(Game &g):BaseAI(g) {}
	
	virtual int ResponseMove();
	virtual int FirstMove();
	virtual int ContinuationMove();	
};


#endif  /*SEPT_AI_H*/
