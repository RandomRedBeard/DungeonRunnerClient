/*
 * player.cpp
 *
 *  Created on: Apr 9, 2018
 *      Author: thomasjansen
 */

#include "player.h"

player::player(){
	name = (char*) nullptr;
	lvl = 1;

	calculateHealth();
	curhp = maxhp;

	curxp = 0;
	nextxp = 100;

	state = STATE_OK;

	mainhand = offhand = (weapon*) nullptr;
	body = (armor*) nullptr;
}

player::~player() {
	if (name) {
		free(name);
	}

	item* it;
	while (inventory.size() != 0) {
		it = inventory.front();
		inventory.erase(inventory.begin());
		delete (it);
	}

	arrow* ar;
	while (quiver.size() != 0) {
		ar = quiver.front();
		quiver.erase(quiver.begin());
		delete (ar);
	}
}

int player::findItem(const char* id) {
	item* it;
	for (unsigned int i = 0; i < inventory.size(); i++) {
		it = inventory[i];
		if (strcmp(id, it->getId()) == 0) {
			return i;
		}
	}

	return -1;
}

void player::calculateHealth() {
	maxhp = 100 + (10 * (lvl - 1));
}

int player::sprintItem(item* it, char* buffer, int len) {
	bow* b = dynamic_cast<bow*>(it);
	weapon* w = dynamic_cast<weapon*>(it);
	armor* ar = dynamic_cast<armor*>(it);
	int l = 0;
	if (b) {
		l = snprintf(buffer, len, "%s %s lvl(%d) %d-%d", b->getName(), b->getId(), b->getLvl(), b->getRangeMaxDmg(), b->getRangeMinDmg());
		if (b == mainhand) {
			strcat(buffer, " (m)");
		}
		else if (b == offhand) {
			strcat(buffer, " (o)");
		}

		strcat(buffer, "\n");

		return l;
	} else if (w) {
		l = snprintf(buffer, len, "%s %s lvl(%d) %d-%d", w->getName(), w->getId(), w->getLvl(), w->getMaxDmg(), w->getMinDmg());
		if (w == mainhand) {
			strcat(buffer, " (m)");
		}
		else if (w == offhand) {
			strcat(buffer, " (o)");
		}

		strcat(buffer, "\n");

		return l;
	} else if (ar) {
		l = snprintf(buffer, len, "%s %s lvl(%d) %d", ar->getName(), ar->getId(), ar->getLvl(), ar->getAc());
		if (ar == body) {
			strcat(buffer, " (b)");
		}

		strcat(buffer, "\n");

		return l;
	}
}

void player::setName(const char* buffer) {
	if (name) {
		free(name);
	}

	name = (char*) malloc(strlen(buffer) + 1);
	name = strcpy(name, buffer);
}

const char* player::getName() {
	return (const char*) name;
}

void player::setPt(point p) {
	pt = p;
}

point player::getPt() {
	return pt;
}

void player::setLvl(int l) {
	lvl = l;
}

int player::getLvl() {
	return lvl;
}

void player::setCurHp(int chp) {
	curhp = chp;
}

int player::getCurHp() {
	return curhp;
}

void player::setMaxHp(int mhp) {
	maxhp = mhp;
}

int player::getMaxHp() {
	return maxhp;
}

int player::getCurXp(){
	return curxp;
}

int player::getNextXp(){
	return nextxp;
}

int player::getAc() {
	return body?body->getAc():0;
}

int player::pickUp(item* it) {
	arrow* ar = dynamic_cast<arrow*>(it);
	if (ar) {
		quiver.push_back(ar);
	} else {
		inventory.push_back(it);
	}
	return 0;
}

item* player::drop(const char* id) {
	int index = findItem(id);
	if (index < 0) {
		return (item*) nullptr;
	}

	item* it = inventory[index];
	inventory.erase(inventory.begin() + index);

	if (it == mainhand) {
		mainhand->unequip();
		mainhand = (weapon*) nullptr;
	} else if (it == offhand) {
		offhand->unequip();
		offhand = (weapon*) nullptr;
	} else if (it == body) {
		body->unequip();
		body = (armor*) nullptr;
	}
	
	calculateHealth();

	return it;
}

int player::equip(const char* id, const char* part) {
	int index = findItem(id);
	if (index < 0) {
		return -1;
	}

	item* it = inventory[index];
	weapon* w = dynamic_cast<weapon*>(it);
	armor* a = dynamic_cast<armor*>(it);

	if ((w && w->isEquip()) || (a && a->isEquip())) {
		return -1;
	}

	if (w && strcmp(part, MAINHAND_OP) == 0) {
		if (mainhand) {
			mainhand->unequip();
		}
		mainhand = w;
		mainhand->equip();
	} else if (w && strcmp(part, OFFHAND_OP) == 0) {
		if (offhand) {
			offhand->unequip();
		}
		offhand = w;
		offhand->equip();
	} else if (a && strcmp(part, BODY_OP) == 0) {
		if (body) {
			body->unequip();
		}
		body = a;
		body->equip();
	} else {
		return -1;
	}

	calculateHealth();

	return 0;
}

int player::unequip(const char* part) {
	if (mainhand && strcmp(part, MAINHAND_OP) == 0) {
		mainhand->unequip();
		mainhand = (weapon*) nullptr;
	} else if (offhand && strcmp(part, OFFHAND_OP) == 0) {
		offhand->unequip();
		offhand = (weapon*) nullptr;
	} else if (body && strcmp(part, BODY_OP) == 0) {
		body->unequip();
		body = (armor*) nullptr;
	} else {
		return -1;
	}

	calculateHealth();

	return 0;
}

int player::melee() {
	int dmg = 0;
	if (mainhand) {
		dmg += mainhand->melee();
	}
	if (offhand) {
		dmg += offhand->melee();
	}

	return dmg;
}

int player::range() {
	if (quiver.size() == 0) {
		return -1;
	}

	bow* b = dynamic_cast<bow*>(mainhand);
	if (!b) {
		return -1;
	}

	arrow* ar = quiver.front();
	int dmg = b->rangeAttack(ar);
	if (ar->getNumberArrows() <= 0) {
		quiver.erase(quiver.begin());
		delete (ar);
	}

	return dmg;
}

int player::takeDamage(int dmg) {
	return curhp -= dmg;
}

void player::addXp(int xp) {
	curxp += xp;
}

bool player::isLevel() {
	return curxp >= nextxp;
}

void player::levelUp() {
	lvl++;

	nextxp += 100;

	calculateHealth();

	curhp = maxhp;
}

int player::sprintInventory(char* buffer, int len) {
	char tempBuffer[STD_LEN];

	int val = 0;
	int index = 0;

	for (item* it : inventory) {
		memset(tempBuffer, 0, STD_LEN);
		snprintf(tempBuffer, STD_LEN, "%d: ", index++);
		val += sprintItem(it, (tempBuffer + strlen(tempBuffer)), STD_LEN);
		if (val >= len) {
			return val;
		}
		strcat(buffer, tempBuffer);
	}

	int narrows = 0;
	for (arrow* ar : quiver) {
		narrows += ar->getNumberArrows();
	}

	memset(tempBuffer, 0, STD_LEN);
	snprintf(tempBuffer, STD_LEN, "\nArrows: %d\n", narrows);
	strcat(buffer, tempBuffer);

	return val;
}

item* player::getItem(int index) {
	if (index < 0 || index >= inventory.size()) {
		return (item*)nullptr;
	}

	return inventory[index];
}

bool player::attemptRange(point pt) {
	if (!dynamic_cast<bow*>(mainhand)) {
		return false;
	}

	if (quiver.size() == 0) {
		return false;
	}

	/*
	 * range check
	 */

	return true;
}
