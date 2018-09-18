/*
 * armor.cpp
 *
 *  Created on: Jan 10, 2018
 *      Author: thomasjansen
 */

#include "armor.h"

armor::armor() {
	baseAc = 0;
	ac = 0;
	equipped = false;
	setIcon('[');
}

armor::armor(const char* name_t, char* tid, int l, int tac) :
		item(name_t, tid, l), baseAc(tac) {
	ac = 0;
	equipped = false;
	calculateArmor();
}

void armor::setAc(int a) {
	ac = a;
}

int armor::getAc() {
	return ac;
}

void armor::calculateArmor() {
	ac = baseAc * getLvl();
}

void armor::equip() {
	equipped = true;
}

void armor::unequip() {
	equipped = false;
}

bool armor::isEquip() {
	return equipped;
}
