/*
 * item.cpp
 *
 *  Created on: Jan 10, 2018
 *      Author: thomasjansen
 */

#include "item.h"

item::item() {
	name = (char*) nullptr;
	id = (char*) nullptr;

	lvl = 1;
}

item::item(const char* name_t, char* tid, int l) {
	name = (char*) nullptr;
	id = (char*) nullptr;

	lvl = l;

	setName(name_t);
	setId(tid);
}

item::~item() {
	if (name)
		free(name);
	if (id)
		free(id);
}

void item::setName(const char* name_t) {
	if (name)
		free(name);

	name = (char*) malloc(strlen(name_t) + 1);
	name = strcpy(name, name_t);
}

const char* item::getName() {
	return (const char*) name;
}

void item::setPt(point tpt) {
	pt = tpt;
}

point item::getPt() {
	return pt;
}

void item::setId(char* tid) {
	if (id) { free(id); }

	id = (char*)malloc(strlen(tid)+1);
	strcpy(id, tid);

}

char* item::getId() {
	return id;
}

void item::setLvl(int l) {
	lvl = l;
}

int item::getLvl() {
	return lvl;
}


void item::setIcon(char c) {
	icon = c;
}

char item::getIcon() {
	return icon;
}

int item::snprintItem(char* buffer, int len) {
	return snprintf(buffer, len, "%s %s lvl(%d)\n", name, id, lvl);
}
