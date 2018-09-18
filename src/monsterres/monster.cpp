/*
 * monster.cpp
 *
 *  Created on: Apr 3, 2018
 *      Author: thomasjansen
 */

#include "monster.h"

monster::monster() {
	state = STATE_NORM;
	name = (char*)nullptr;
	id = (char*)nullptr;
}

monster::~monster() {
	if (name)
		free(name);

	if (id)
		free(id);
}

void monster::setId(char* tid) {
	if (id) {
		free(id);
	}

	id = (char*)malloc(strlen(tid) + 1);
	strcpy(id, tid);
}

char* monster::getId() {
	return id;
}

void monster::setName(const char* name_t) {
	if (name)
		free(name);

	name = (char*) malloc(strlen(name_t) + 1);
	name = strcpy(name, name_t);
}

const char* monster::getName() {
	return (const char*) name;
}

void monster::setPt(point newpt) {
	pt = newpt;
}

point monster::getPt() {
	return pt;
}

void monster::setLvl(int level) {
	lvl = level;
}

int monster::getLvl() {
	return lvl;
}

void monster::setCurHp(int chp) {
	curhp = chp;
}

int monster::getCurHp() {
	return curhp;
}

void monster::setMaxHp(int mhp) {
	maxhp = mhp;
}

int monster::getMaxHp() {
	return maxhp;
}

void monster::setAc(int a) {
	ac = a;
}

int monster::getAc() {
	return ac;
}

void monster::setState(int newstate) {
	state = newstate;
}

int monster::getState() {
	return state;
}

void monster::setXp(int nxp) {
	xp = nxp;
}

int monster::getXp() {
	return xp;
}

void monster::setMaxDmg(int xdmg) {
	maxdmg = xdmg;
}

int monster::getMaxDmg() {
	return maxdmg;
}

void monster::setMinDmg(int ndmg) {
	mindmg = ndmg;
}

int monster::getMinDmg() {
	return mindmg;
}

int monster::melee() {
	int dmg = mindmg + rand() % (maxdmg - mindmg + 1);

	return dmg;
}

int monster::takeDamage(int dmg) {
	return curhp -= dmg;
}
