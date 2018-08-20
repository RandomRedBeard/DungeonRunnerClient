/*
 * itemgen.h
 *
 *  Created on: Apr 14, 2018
 *      Author: thomasjansen
 */

#include "../globals.h"

#include "item.h"

#include "weapon.h"
#include "shortsword.h"
#include "longsword.h"
#include "axe.h"

#include "armor.h"
#include "leatherarmor.h"
#include "ironarmor.h"
#include "steelarmor.h"

#include "arrow.h"
#include "woodarrow.h"

#include "bow.h"
#include "woodbow.h"

#define NUMBER_OF_ITEMS 4
#define NUMBER_OF_WEAPONS 3
#define NUMBER_OF_ARMORS 3
#define NUMBER_OF_ARROWS 1
#define NUMBER_OF_BOWS 1

item* createItem(char*, int);

weapon* createWeapon(char*, int);

armor* createArmor(char*, int);

arrow* createArrow(char*, int);

bow* createBow(char*, int);


