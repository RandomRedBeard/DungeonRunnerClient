/*
 * item.h
 *
 *  Created on: Jan 10, 2018
 *      Author: thomasjansen
 */

#ifndef ITEM_H_
#define ITEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../point.h"

class item {
	char* name;
	point pt;
	char* id;
	int lvl;
	char icon;
public:
	item();
	item(const char*, char*, int);
	virtual ~item();

	virtual void setName(const char*);
	virtual const char* getName();
	virtual void setPt(point);
	virtual point getPt();
	virtual void setId(char*);
	virtual char* getId();
	virtual void setLvl(int);
	virtual int getLvl();
	virtual void setIcon(char);
	virtual char getIcon();
};

#endif /* ITEM_H_ */
