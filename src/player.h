/*
 * player.h
 *
 *  Created on: Apr 9, 2018
 *      Author: thomasjansen
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <queue>
#include <thread>
#include <mutex>

#include "globals.h"
#include "log.h"
#include "iores/Socket.h"
#include "point.h"
#include "itemres/item.h"
#include "itemres/weapon.h"
#include "itemres/armor.h"
#include "itemres/bow.h"
#include "itemres/arrow.h"

class player {
	char* name;
	point pt;
	int lvl;
	int curhp, maxhp;

	int curxp, nextxp;

	int state;

	std::vector<item*> inventory;
	std::queue<arrow*> quiver;
	weapon* mainhand, *offhand;
	armor* body;

	int findItem(const char*);

	void calculateHealth();
public:
	player();
	virtual ~player();

	void setName(const char*);
	const char* getName();
	void setPt(point);
	point getPt();
	void setLvl(int);
	int getLvl();
	void setCurHp(int);
	int getCurHp();
	void setMaxHp(int);
	int getMaxHp();

	int getCurXp();
	int getNextXp();

	int getAc();

	int pickUp(item*);
	item* drop(const char*);
	int equip(const char*, const char*);
	int unequip(const char*);

	int melee();
	int range();
	int takeDamage(int);

	void addXp(int);
	bool isLevel();
	void levelUp();

	int sprintInventory(char*, int);

	item* getItem(int);
};

#endif 
/* PLAYER_H_ */
