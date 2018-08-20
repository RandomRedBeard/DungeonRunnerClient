/*
 * arrow.h
 *
 *  Created on: May 18, 2018
 *      Author: thomasjansen
 */

#ifndef ARROW_H_
#define ARROW_H_

#include "item.h"

class arrow: public item {
	int numberOfArrows;
	int baseMax, baseMin;
	int maxdmg, mindmg;

	void calculateDamage();
public:
	arrow();
	arrow(const char*, char*, int, int, int, int);

	void setNumberArrows(int);

	void setMaxDmg(int);
	void setMinDmg(int);

	int getNumberArrows();
	int getMaxDmg();
	int getMinDmg();
	int range();
};

#endif /* ARROW_H_ */
