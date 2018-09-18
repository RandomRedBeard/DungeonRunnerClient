#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include <curses.h>

#include "globals.h"
#include "itemres/itemgen.h"
#include "topt/topt.h"
#include "iores/Socket.h"
#include "monsterres/monster.h"
#include "player.h"

#if defined (_WIN32) || defined (WIN64)
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"pdcurses.lib")
#endif

#define PORT_ARG 'p'
#define ADDR_ARG 'a'
#define HANDLE_ARG 'h'
#define HEIGHT_ARG 'H'
#define WIDTH_ARG 'W'

#define ARGSTRING "p:a:h:H:W:"

int STATE;

bool REFRESH;

int INP_UP = 'w';
int INP_DOWN = 's';
int INP_LEFT = 'a';
int INP_RIGHT = 'd';
int INP_QUIT = 'q';
int INP_PICKUP = ' ';
int INP_SHOW_INVENTORY = 'i';
int INP_EQUIP = 'e';
int INP_UNEQUIP = 'u';
int INP_DROP = 'D';
int INP_TRAVEL = 't';

int POLL_WAIT_TIMEOUT = -1;
unsigned int MIN_NUMBER_ARROWS = 1;
unsigned int MAX_NUMBER_ARROWS = 6;

#if !defined (_WIN32) && !defined (_WIN64)
unsigned int STD_LEN = 128;
#endif

unsigned int HEIGHT = 25;
unsigned int WIDTH = 80;

double AC_SKEW = .05;

char* LOG_FILE = (char*)"log.txt";

WINDOW* inventoryWin;
WINDOW* targetWin;
WINDOW* borderWin;
WINDOW* mapWin;
WINDOW* hudWin;
WINDOW* messageWin;

std::mutex screenLock;

std::mutex messageLock;
std::condition_variable messageCond;
std::queue<char*> messageQueue;

std::mutex multiplayerLock;

std::vector<monster*> monsters;
std::vector<item*> items;
std::vector<player*> players;

char* MAP;

player* me;

void initMap();

void addMessage(const char* , ...);
void messageThread();
void readerThread(Socket*);

monster* findMonster(const char*);
monster* findMonster(point);
player* findPlayer(const char*);
item* findItem(const char*);
item* findItem(point);

int acceptTravel(Socket*);

int enterMap(Socket*);

int newWeapon(Socket*);
int newArmor(Socket*);
int newBow(Socket*);
int newArrow(Socket*);
int newMonster(Socket*);
int newPlayer(Socket*);

int move(Socket*);
int monsterMove(Socket*);
int playerMove(Socket*);

int pickup(Socket*);
int equip(Socket*);
int unequip(Socket*);

int drop(Socket*);
int dropWeapon(Socket*, const char*);
int dropArmor(Socket*, const char*);
int dropBow(Socket*, const char*);
int dropArrow(Socket*, const char*);

int melee(Socket*);
int playerToMonsterMelee( const char* , const char* , int );
int monsterToPlayerMelee(const char*, const char*,int);

int range(Socket*);
int playerToMonsterRange(const char*, const char*, int);
//monster to player

int killed(Socket*);
int playerKilledMonster(Socket* fd , const char*, const char*);
int monsterKilledPlayer(const char*, const char*);

int level(Socket*);

int showInventory();
int showEquip(Socket*);
int showUnequip(Socket*);
int showDrop(Socket*);

int pmvwaddch(WINDOW* w, point pt, int c) {
	return mvwaddch(w, pt.getX(), pt.getY(), c);
}

void returnCurs() {
	wmove(mapWin, me->getPt().getX(), me->getPt().getY());
}

int getInput(WINDOW* win) {
	/*char buffer[1];
	memset(buffer, 0, 1);
	fread(buffer, 1, 1, stdin);
	return *buffer;*/

	return wgetch(win);
}

void printHud() {
	mvwprintw(hudWin, 0, 0, "%s %d(%d) lvl(%d) Ac(%d) XP=%d/%d ", me->getName(), me->getCurHp(), me->getMaxHp(), me->getLvl(),me->getAc(), me->getCurXp(), me->getNextXp());
	wrefresh(hudWin);
	wrefresh(mapWin);
}

int main(int argc , char** argv ) {
	int opt, port = 5000;
	char* addr = (char*)"127.0.0.1";
	char* handle = nullptr;
	while ((opt = tgetopt(argc, argv, ARGSTRING)) != -1) {
		switch (opt) {
		case PORT_ARG:
			port = strtol(toptarg, nullptr, 0);
			break;
		case ADDR_ARG:
			addr = toptarg;
			break;
		case HANDLE_ARG:
			handle = toptarg;
			break;
		case HEIGHT_ARG:
			HEIGHT = strtol(toptarg, nullptr, 0);
			break;
		case WIDTH_ARG:
			WIDTH = strtol(toptarg, nullptr, 0);
			break;
		}
	}

	if (!handle) {
		printf("%d %d\n", HEIGHT, WIDTH);
		handle = (char*)malloc(17);
		memset(handle, 0, 17);
		printf("Enter Handle: ");
#ifdef _WIN32
		gets_s(handle, 16);
#else
		fgets(handle, 16, stdin);
		*(handle + strlen(handle) - 1) = '\0';
		//return -1;
#endif
	}

#if defined (_WIN32) || defined (_WIN64)
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

	MAP = (char*)malloc((HEIGHT * WIDTH) + 1);
	Socket fd(addr, port);

	me = new player();
	me->setName(handle);
	me->setPt(point(0, 0));

	snprintf(MAP, 128, "%s%c%s%c", JOIN_OP, OP_SEP, handle, OP_SEP);

	printf("Writing\n");

	if (fd.write(MAP, strlen(MAP)) < 0) {
		printf("Write failed\n");
		getchar();
		return -1;
	}

	printf("Ready\n");

	STATE = 0;
	REFRESH = true;

	initscr();
	noecho();
	keypad(stdscr, true);
	keypad(mapWin, true);
	mousemask(ALL_MOUSE_EVENTS, nullptr);
	mouseinterval(0);
	if (!has_mouse()) {
		endwin();
		return -1;
	}

	initMap();
	std::thread message(&messageThread);
	std::thread reader(&readerThread, &fd);

	char buffer[STD_LEN];

	int x, y;

	while (true) {
		x = y = 0;
		int n = getInput(mapWin);

		switch(n){
		case KEY_MOUSE:
			addMessage("Mouse");
			break;
		}

		if (n == INP_UP) {
			x = -1;
		}
		else if (n == INP_DOWN) {
			x = 1;
		}
		else if (n == INP_LEFT) {
			y = -1;
		}
		else if (n == INP_RIGHT) {
			y = 1;
		}
		else if (n == INP_QUIT) {
			fd.shutdownSocket();
			fd.closeSocket();
			STATE = -1;
			messageCond.notify_all();
			message.join();
			reader.join();
			refresh();
			endwin();

			printf("Press any key to exit...");
			getchar();
			return 0;
		}else if (n == INP_PICKUP) {
			snprintf(buffer, STD_LEN, "%s%c", PICKUP_OP, OP_SEP);
			fd.write(buffer, strlen(buffer));
			continue;
		}
		else if (n == INP_SHOW_INVENTORY) {
			showInventory();
			continue;
		}
		else if (n == INP_EQUIP) {
			showEquip(&fd);
			continue;
		}
		else if (n == INP_UNEQUIP) {
			showUnequip(&fd);
		}
		else if (n == INP_DROP) {
			showDrop(&fd);
		}
		else if (n == INP_TRAVEL) {
			snprintf(buffer, STD_LEN, "%s%c", TRAVEL_REQUEST_OP, OP_SEP);
			fd.write(buffer, strlen(buffer));
			continue;
		}

		multiplayerLock.lock();
		point pt = point(me->getPt().getX() + x, me->getPt().getY() + y);
		monster* m = findMonster(pt);
		if (m) {
			snprintf(buffer, STD_LEN, "%s%c%s%c%s%c", MELEE_OP, OP_SEP, MONSTER_OP, OP_SEP, m->getId(), OP_SEP);
			fd.write(buffer,strlen(buffer));
			multiplayerLock.unlock();
			continue;
		}
		multiplayerLock.unlock();

		snprintf(buffer, STD_LEN, "%s%c%d%c%d%c", MOVE_OP, OP_SEP, pt.getX(), POINT_SEP, pt.getY(), OP_SEP);
		fd.write(buffer, strlen(buffer));
	}
	endwin();
}

void initMap() {

	int maxx = getmaxx(stdscr);
	int maxy = getmaxy(stdscr);

	//INVENTORY
	inventoryWin = newwin(maxy, maxx, 0, 0);
	wrefresh(inventoryWin);

	//TARGET
	targetWin = newwin(1, maxx, 0, 0);
	wrefresh(targetWin);

	//BORDER
	borderWin = newwin(HEIGHT+2, WIDTH+2, 1, 0);
	box(borderWin, 0, 0);
	wrefresh(borderWin);

	//MAP
	mapWin = subwin(borderWin,HEIGHT, WIDTH, 2, 1);
	wrefresh(mapWin);

	//HUD

	hudWin = newwin(1, maxx, HEIGHT+3, 0);
	wrefresh(hudWin);

	//MESSAGE
	messageWin = newwin(maxy - (HEIGHT+4), maxx, HEIGHT+4, 0);
	wrefresh(messageWin);


}

void addMessage(const char* format, ...) {
	char* buffer = (char*)malloc(STD_LEN);
	va_list vaList;
	va_start(vaList, format);
	vsnprintf(buffer, STD_LEN, format, vaList);
	va_end(vaList);

	messageLock.lock();
	messageQueue.push(buffer);
	messageCond.notify_all();
	messageLock.unlock();
}

void messageThread() {
	std::unique_lock<std::mutex> ul(messageLock);
	ul.unlock();

	int curx = 0;
	char* buffer;

	while (true) {
		ul.lock();
		while (messageQueue.size() == 0 && STATE == 0) {
			messageCond.wait(ul);
		}

		if (STATE != 0) {
			return;
		}

		screenLock.lock();

		if (curx == getmaxy(messageWin)) {
			curx--;
			wmove(messageWin, 0, 0);
			wdeleteln(messageWin);
			//mvwdeleteln(messageWin, 0, 0);
		}

		buffer = messageQueue.front();
		messageQueue.pop();
		mvwaddstr(messageWin, curx++, 0, buffer);
		free(buffer);
		if (REFRESH) {
			wrefresh(messageWin);
			wrefresh(mapWin);
		}

		screenLock.unlock();

		ul.unlock();
	}
}

void readerThread(Socket* fd) {
	char buffer[STD_LEN];
	while (fd->read(buffer , STD_LEN) > 0 ) {
		if (strcmp(buffer, ENTER_MAP_OP) == 0) {
			enterMap(fd);
		}
		else if (strcmp(buffer, MOVE_OP) == 0) {
			move(fd);
		}
		else if (strcmp(buffer, ACCEPT_TRAVEL_OP) == 0) {
			acceptTravel(fd);
		}
		else if (strcmp(buffer, PICKUP_OP) == 0) {
			pickup(fd);
		}
		else if (strcmp(buffer, EQUIP_OP) == 0) {
			equip(fd);
		}
		else if (strcmp(buffer, UNEQUIP_OP) == 0) {
			unequip(fd);
		}
		else if (strcmp(buffer, DROP_OP) == 0) {
			drop(fd);
		}
		else if (strcmp(buffer, MELEE_OP) == 0) {
			melee(fd);
		}
		else if (strcmp(buffer, KILLED_OP) == 0) {
			killed(fd);
		}
		else if (strcmp(buffer, LEVELUP_OP) == 0) {
			level(fd);
		}
		else {
			addMessage("Server: %s", buffer);
		}
	}

	addMessage("Connection Closed");
}

monster* findMonster(const char* id) {
	for (monster* m : monsters) {
		if (strcmp(m->getId(), id) == 0) {
			return m;
		}
	}

	return (monster*)nullptr;
}

monster* findMonster(point pt) {
	for (monster* m : monsters) {
		if (m->getPt() == pt) {
			return m;
		}
	}

	return (monster*)nullptr;
}

player* findPlayer(const char* name) {
	for (player* p : players) {
		if (strcmp(p->getName(), name) == 0) {
			return p;
		}
	}

	return (player*)nullptr;
}

item* findItem(const char* id) {
	for (item* it : items) {
		if (strcmp(id, it->getId()) == 0) {
			return it;
		}
	}

	return (item*)nullptr;
}

item* findItem(point pt) {
	for (item* it : items) {
		if (it->getPt() == pt) {
			return it;
		}
	}

	return (item*)nullptr;
}

int acceptTravel(Socket* fd) {
	char pt[STD_LEN], buffer[STD_LEN] , height[STD_LEN] , width[STD_LEN];

	
	fd->read(pt);//coor
	fd->read(buffer);//physmap
	fd->read(height); //height
	fd->read(width);//width

	addMessage("%s %s", height, width);

	multiplayerLock.lock();

	me->setPt(point(pt));

	monster* m;
	while (monsters.size() != 0) {
		m = monsters.front();
		monsters.erase(monsters.begin());
		delete(m);
	}

	item* it;
	while (items.size() != 0) {
		it = items.front();
		items.erase(items.begin());
		delete(it);
	}

	/*for ( monster* m : monsters ) {
		delete m;
	}

	monsters.clear();

	for ( item* it : items ) {
		delete it;
	}

	items.clear();*/

	player* p;
	while (players.size() != 0) {
		p = players.front();
		players.erase(players.begin());
		if (p != me) {
			delete(p);
		}
	}

	players.push_back(me);

	multiplayerLock.unlock();

	screenLock.lock();
	fd->read(MAP, (HEIGHT*WIDTH + 1));
	mvwaddstr(mapWin, 0, 0, MAP);
	printHud();
	pmvwaddch(mapWin, me->getPt(), '@');
	returnCurs();
	wrefresh(mapWin);
	screenLock.unlock();

	return 0;
}

int enterMap(Socket* fd) {
	char buffer[STD_LEN];
	if (fd->read(buffer, STD_LEN) < 0) {
		return -1;
	}

	if (strcmp(buffer, WEAPON_OP) == 0) {
		return newWeapon(fd);
	}
	else if (strcmp(buffer, ARMOR_OP) == 0) {
		return newArmor(fd);
	}
	else if (strcmp(buffer, BOW_OP) == 0) {
		return newBow(fd);
	}
	else if (strcmp(buffer, ARROW_OP) == 0) {
		return newArrow(fd);
	}
	else if (strcmp(buffer, MONSTER_OP) == 0) {
		return newMonster(fd);
	}
	else if (strcmp(buffer, PLAYER_OP) == 0) {
		return newPlayer(fd);
	}
	else {
		return -1;
	}
}

int newWeapon(Socket* fd) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], dmg[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(dmg) < 0) {
		return -1;
	}

	point d(dmg);
	weapon* w = new weapon();
	w->setName(name);
	w->setId(id);
	w->setPt(point(pt));
	w->setLvl(strtol(lvl, nullptr, 0));
	w->setMaxDmg(d.getX());
	w->setMinDmg(d.getY());

	addMessage("%s has dropped at %d:%d", name, w->getPt().getX(), w->getPt().getY());

	multiplayerLock.lock();
	items.push_back(w);

	screenLock.lock();
	mvwaddch(mapWin, w->getPt().getX(), w->getPt().getY(), w->getIcon());
	returnCurs();
	if (REFRESH) {
		wrefresh(mapWin);
	}
	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int newArmor(Socket* fd) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], ac[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(ac) < 0) {
		return -1;
	}

	armor* a = new armor();
	a->setName(name);
	a->setId(id);
	a->setLvl(strtol(lvl, nullptr, 0));
	a->setPt(point(pt));
	a->setAc(strtol(ac, nullptr, 0));

	addMessage("%s has dropped at %d %d", name, a->getPt().getX(), a->getPt().getY());
	
	multiplayerLock.lock();
	items.push_back(a);

	screenLock.lock();
	mvwaddch(mapWin, a->getPt().getX(), a->getPt().getY(), a->getIcon());
	returnCurs();
	if (REFRESH) {
		wrefresh(mapWin);
	}
	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int newBow(Socket* fd) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], mdmg[STD_LEN] , rdmg[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(mdmg) < 0) {
		return -1;
	}

	if (fd->read(rdmg) < 0) {
		return -1;
	}

	point md(mdmg);
	point rd(rdmg);

	bow* b = new bow();
	b->setId(id);
	b->setName(name);
	b->setLvl(strtol(lvl, nullptr, 0));
	b->setMaxDmg(md.getX());
	b->setMinDmg(md.getY());
	b->setRangeMaxDmg(rd.getX());
	b->setRangeMinDmg(rd.getY());
	b->setPt(point(pt));

	addMessage("%s has dropped at %d %d", name, b->getPt().getX(), b->getPt().getY());

	multiplayerLock.lock();
	items.push_back(b);

	screenLock.lock();
	mvwaddch(mapWin, b->getPt().getX(), b->getPt().getY(), b->getIcon());
	returnCurs();
	if (REFRESH) {
		wrefresh(mapWin);
	}
	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int newArrow(Socket* fd) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], dmg[STD_LEN] , n[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(dmg) < 0) {
		return -1;
	}

	if (fd->read(n) < 0) {
		return -1;
	}

	point d(dmg);

	arrow* a = new arrow();
	a->setName(name);
	a->setId(id);
	a->setLvl(strtol(lvl, nullptr, 0));
	a->setPt(point(pt));
	a->setNumberArrows(strtol(n, nullptr, 0));
	a->setMaxDmg(d.getX());
	a->setMinDmg(d.getY());

	addMessage("%s dropped at %d %d", name, a->getPt().getX(), a->getPt().getY());

	multiplayerLock.lock();
	items.push_back(a);

	screenLock.lock();
	mvwaddch(mapWin, a->getPt().getX(), a->getPt().getY(), a->getIcon());
	returnCurs();
	if (REFRESH) {
		wrefresh(mapWin);
	}
	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int newMonster(Socket* fd) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], hp[STD_LEN], ac[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(hp) < 0) {
		return -1;
	}

	if (fd->read(ac) < 0) {
		return -1;
	}

	monster* m = new monster();
	m->setId(id);
	m->setName(name);
	m->setPt(point(pt));
	m->setLvl(strtol(lvl, nullptr, 0));
	m->setMaxHp(strtol(hp, nullptr, 0));
	m->setCurHp(strtol(hp, nullptr, 0));
	m->setAc(strtol(ac, nullptr, 0));

	addMessage("%s %s %d is at %d %d", m->getId(), m->getName(),strlen(m->getId()), m->getPt().getX(), m->getPt().getY());

	multiplayerLock.lock();
	monsters.push_back(m);

	screenLock.lock();
	mvwaddch(mapWin, m->getPt().getX(), m->getPt().getY(), *name);
	returnCurs();
	if (REFRESH) {
		wrefresh(mapWin);
	}
	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int newPlayer(Socket* fd) {
	char name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], hp[STD_LEN];

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(hp) < 0) {
		return -1;
	}

	point health(hp);

	player* p = new player();
	p->setName(name);
	p->setPt(point(pt));
	p->setCurHp(health.getX());
	p->setMaxHp(health.getY());
	
	addMessage("%s has joined at %d %d", name, p->getPt().getX(), p->getPt().getY());

	multiplayerLock.lock();
	players.push_back(p);

	screenLock.lock();
	pmvwaddch(mapWin, p->getPt(), '@');
	returnCurs();
	if (REFRESH) {
		wrefresh(mapWin);
	}
	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int move(Socket* fd) {
	char buffer[STD_LEN];
	if (fd->read(buffer) < 0) {
		return -1;
	}

	if (strcmp(buffer, MONSTER_OP) == 0) {
		return monsterMove(fd);
	}
	else if (strcmp(buffer, PLAYER_OP) == 0) {
		return playerMove(fd);
	}
	else {
		return -1;
	}
}

int monsterMove(Socket* fd) {
	char id[STD_LEN], dest[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(dest) < 0) {
		return -1;
	}

	//find monster
	multiplayerLock.lock();
	monster* m=findMonster(id);
	item* it = findItem(m->getPt());

	if (!m) {
		multiplayerLock.unlock();
		return -1;
	}

	screenLock.lock();
	if (!it) {
		pmvwaddch(mapWin, m->getPt(), *(MAP + m->getPt().index(HEIGHT, WIDTH)));
	}
	else {
		pmvwaddch(mapWin, m->getPt(), it->getIcon());
	}
	m->setPt(point(dest));
	pmvwaddch(mapWin, m->getPt(), *(m->getName()));
	returnCurs();
	if (REFRESH) {
		wrefresh(mapWin);
	}
	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int playerMove(Socket* fd) {
	char name[STD_LEN], dest[STD_LEN];

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(dest) < 0) {
		return -1;
	}

	//find monster
	multiplayerLock.lock();
	player* p = findPlayer(name);
	item* it = findItem(p->getPt());

	if (!p) {
		multiplayerLock.unlock();
		return -1;
	}

	screenLock.lock();
	if (!it) {
		pmvwaddch(mapWin, p->getPt(), *(MAP +p->getPt().index(HEIGHT, WIDTH)));
	}
	else {
		pmvwaddch(mapWin, p->getPt(), it->getIcon());
	}
	p->setPt(point(dest));
	pmvwaddch(mapWin, p->getPt(), '@');
	returnCurs();
	if (REFRESH) {
		wrefresh(mapWin);
	}
	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int pickup(Socket* fd) {
	char name[STD_LEN] , id[STD_LEN];

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(id) < 0) {
		return -1;
	}

	multiplayerLock.lock();
	player* p = findPlayer(name);
	item* it = findItem(id);

	if (!p || !it) {
		multiplayerLock.unlock();
		return -1;
	}

	for (unsigned int i = 0; i < items.size(); i++) {
		if (items[i] == it) {
			items.erase(items.begin() + i);
		}
	}

	p->pickUp(it);
	multiplayerLock.unlock();

	addMessage("%s picked up %s", p->getName(), it->getName());


	return 0;
}

int equip(Socket* fd) {
	char name[STD_LEN], id[STD_LEN], part[STD_LEN];

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(part) < 0) {
		return -1;
	}

	multiplayerLock.lock();
	player* p = findPlayer(name);
	if (!p ) {
		multiplayerLock.unlock();
		return -1;
	}

	bool meFlag = false;

	if (p == me) {
		meFlag = true;
	}

	if (p->equip(id, part) < 0) {
		multiplayerLock.unlock();
		return -1;
	}

	multiplayerLock.unlock();

	if (meFlag) {
		screenLock.lock();
		printHud();
		screenLock.unlock();
		addMessage("You have equipped %s to %s", id, part);
	}
	

	return 0; 
}

int unequip(Socket* fd) {
	char name[STD_LEN], eq_part[STD_LEN];

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(eq_part) < 0) {
		return -1;
	}

	multiplayerLock.lock();

	player* p = findPlayer(name);
	if (!p) {
		multiplayerLock.unlock();
		return -1;
	}

	if (p->unequip(eq_part) < 0) {
		multiplayerLock.unlock();
		return -1;
	}

	if (p == me) {
		screenLock.lock();
		printHud();
		screenLock.unlock();
		addMessage("You unequip %s", eq_part);
	}

	multiplayerLock.unlock();

	return 0;
}

int drop(Socket* fd) {
	char p_name[STD_LEN], type[STD_LEN];

	if (fd->read(p_name) < 0) {
		return -1;
	}

	if (fd->read(type) < 0) {
		return -1;
	}

	if (strcmp(type, WEAPON_OP) == 0) {
		return dropWeapon(fd, p_name);
	} else if (strcmp(type, ARMOR_OP) == 0) {
		return dropArmor(fd, p_name);
	} else if (strcmp(type, BOW_OP) == 0) {
		return dropBow(fd, p_name);
	} else if (strcmp(type, ARROW_OP) == 0) {
		return dropArrow(fd, p_name);
	}

	return -1;
}

int dropWeapon(Socket* fd, const char* p_name) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], dmg[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(dmg) < 0) {
		return -1;
	}

	multiplayerLock.lock();

	player* p = findPlayer(p_name);
	if (!p) {
		multiplayerLock.unlock();
		return -1;
	}

	item* it = p->drop(id);

	if (!it) {
		weapon* w = new weapon();
		w->setName(name);
		w->setPt(point(pt));
		w->setLvl(strtol(lvl, nullptr, 0));
		point dmgpt(dmg);
		w->setMaxDmg(dmgpt.getX());
		w->setMinDmg(dmgpt.getY());
		it = w;
	} else {
		it->setPt(point(pt));
	}

	items.push_back(it);

	multiplayerLock.unlock();

	return 0;
}

int dropArmor(Socket* fd, const char* p_name) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], ac[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(ac) < 0) {
		return -1;
	}

	multiplayerLock.lock();

	player* p = findPlayer(p_name);
	if (!p) {
		multiplayerLock.unlock();
		return -1;
	}

	item* it = p->drop(id);

	if (!it) {
		armor* ar = new armor();
		ar->setName(name);
		ar->setPt(p->getPt());
		ar->setLvl(strtol(lvl, nullptr, 0));
		ar->setAc(strtol(ac, nullptr, 0));
		it = ar;
	} else {
		it->setPt(point(pt));
	}

	items.push_back(it);

	multiplayerLock.unlock();
	return 0;
}

int dropBow(Socket* fd, const char* p_name) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], dmg[STD_LEN], rdmg[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(dmg) < 0) {
		return -1;
	}

	if (fd->read(rdmg) < 0) {
		return -1;
	}

	multiplayerLock.lock();

	player* p = findPlayer(p_name);
	if (!p) {
		multiplayerLock.unlock();
		return -1;
	}

	item* it = p->drop(id);

	if (!it) {
		bow* b = new bow();
		b->setId(id);
		b->setName(name);
		b->setPt(p->getPt());
		b->setLvl(strtol(lvl, nullptr, 0));
		point dmgpt(dmg);
		point rdmgpt(rdmg);
		b->setMaxDmg(dmgpt.getX());
		b->setMinDmg(dmgpt.getY());
		b->setRangeMaxDmg(rdmgpt.getX());
		b->setRangeMinDmg(rdmgpt.getY());
		it = b;
	} else {
		it->setPt(point(pt));
	}

	items.push_back(it);

	multiplayerLock.unlock();

	return 0;
}

int dropArrow(Socket* fd, const char* p_name) {
	char id[STD_LEN], name[STD_LEN], pt[STD_LEN], lvl[STD_LEN], dmg[STD_LEN], narrows[STD_LEN];

	if (fd->read(id) < 0) {
		return -1;
	}

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(pt) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	if (fd->read(dmg) < 0) {
		return -1;
	}

	if (fd->read(narrows) < 0) {
		return -1;
	}

	multiplayerLock.lock();

	player* p = findPlayer(p_name);
	if (!p) {
		multiplayerLock.unlock();
		return -1;
	}

	item* it = p->drop(id);

	if (!it) {
		arrow* ar = new arrow();
		ar->setId(id);
		ar->setName(name);
		ar->setPt(p->getPt());
		ar->setLvl(strtol(lvl, nullptr, 0));
		point dmgpt(dmg);
		ar->setMaxDmg(dmgpt.getX());
		ar->setMinDmg(dmgpt.getY());
		ar->setNumberArrows(strtol(narrows, nullptr, 0));
		it = ar;
	} else {
		it->setPt(point(pt));
	}

	items.push_back(it);

	multiplayerLock.unlock();
	return 0;
}

int melee(Socket* fd) {
	char att_type[STD_LEN], att_id[STD_LEN], vic_type[STD_LEN], vic_id[STD_LEN], dmg[STD_LEN];

	if (fd->read(att_type) < 0) {
		return -1;
	}

	if (fd->read(att_id) < 0) {
		return -1;
	}

	if (fd->read(vic_type) < 0) {
		return -1;
	}

	if (fd->read(vic_id) < 0) {
		return -1;
	}

	if (fd->read(dmg) < 0) {
		return -1;
	}

	int d = strtol(dmg, nullptr, 0);

	if (strcmp(att_type, PLAYER_OP) == 0 && strcmp(vic_type, MONSTER_OP) == 0) {
		return playerToMonsterMelee( att_id, vic_id, d);
	}
	else if (strcmp(att_type, MONSTER_OP) == 0 && strcmp(vic_type, PLAYER_OP) == 0) {
		return monsterToPlayerMelee( att_id, vic_id , d);
	}
	
	return -1;
}

int playerToMonsterMelee( const char* att_id, const char* vic_id, int dmg) {
	multiplayerLock.lock();

	player* p = findPlayer(att_id);
	monster* m = findMonster(vic_id);
	if (!p || !m) {
		multiplayerLock.unlock();
		return -1;
	}

	addMessage("You attack %s for %d", vic_id, dmg);

	m->takeDamage(dmg);

	multiplayerLock.unlock();

	return 0;
}

int monsterToPlayerMelee(const char* att_id, const char* vic_id, int dmg) {
	multiplayerLock.lock();

	player* p = findPlayer(vic_id);
	monster* m = findMonster(att_id);
	if (!p || !m) {
		multiplayerLock.unlock();
		return -1;
	}

	addMessage("You were attacked by %s for %d", att_id, dmg);

	screenLock.lock();
	if (REFRESH) {
		printHud();
	}
	screenLock.unlock();

	p->takeDamage(dmg);

	multiplayerLock.unlock();

	return 0;
}

int range(Socket* fd) {
	char attr_type[STD_LEN], attr_id[STD_LEN], vic_type[STD_LEN], vic_id[STD_LEN], dmg[STD_LEN];

	if (fd->read(attr_type) < 0) {
		return -1;
	}

	if (fd->read(attr_id) < 0) {
		return -1;
	}

	if (fd->read(vic_type) < 0) {
		return -1;
	}

	if (fd->read(vic_id) < 0) {
		return -1;
	}

	if (fd->read(dmg) < 0) {
		return -1;
	}

	if (strcmp(attr_type, PLAYER_OP) == 0 && strcmp(vic_type, MONSTER_OP) == 0) {
		return playerToMonsterRange(attr_id, vic_id, strtol(dmg, nullptr, 0));
	}

	return -1;
}

int playerToMonsterRange(const char* p_name, const char* vic_id, int dmg) {
	multiplayerLock.lock();

	player* p = findPlayer(p_name);
	monster* m = findMonster(vic_id);

	if (!p || !m) {
		multiplayerLock.unlock();
		return -1;
	}

	p->range();
	m->takeDamage(dmg);

	if (p == me) {
		addMessage("You hit %s for %d", vic_id, dmg);
	}

	multiplayerLock.unlock();

	return 0;
}

int killed(Socket* fd) {
	char att_type[STD_LEN], att_id[STD_LEN], vic_type[STD_LEN], vic_id[STD_LEN];

	if (fd->read(att_type) < 0) {
		return -1;
	}

	if (fd->read(att_id) < 0) {
		return -1;
	}

	if (fd->read(vic_type) < 0) {
		return -1;
	}

	if (fd->read(vic_id) < 0) {
		return -1;
	}

	if (strcmp(att_type, PLAYER_OP) == 0 && strcmp(vic_type, MONSTER_OP) == 0) {
		return playerKilledMonster(fd, att_id, vic_id);
	}
	else if (strcmp(att_type, MONSTER_OP) == 0 && strcmp(vic_type, PLAYER_OP) == 0) {
		return monsterKilledPlayer(att_id, vic_id);
	}

	return -1;
}

int playerKilledMonster(Socket* fd, const char* att_id, const char* vic_id) {
	char xp[STD_LEN];

	if (fd->read(xp) < 0) {
		return -1;
	}

	multiplayerLock.lock();

	player* p = findPlayer(att_id);
	monster* m = findMonster(vic_id);

	if (!p || !m) {
		multiplayerLock.unlock();
		return -1;
	}

	p->addXp(strtol(xp, nullptr, 0));

	if (p == me) {
		addMessage("You have killed %s", vic_id);
	}

	for (unsigned int i = 0; i < monsters.size(); i++) {
		if (monsters[i] == m) {
			monsters.erase(monsters.begin() + i);
			break;
		}
	}

	item* it = findItem(m->getPt());
	screenLock.lock();
	if (it) {
		pmvwaddch(mapWin, it->getPt(), it->getIcon());
	}
	else {
		pmvwaddch(mapWin, m->getPt(), ' ');
	}

	if (REFRESH) {
		if (p == me){
			printHud();
		}
		wrefresh(mapWin);
	}

	screenLock.unlock();

	delete(m);

	multiplayerLock.unlock();

	return 0;
}

int monsterKilledPlayer(const char* att_id, const char* vic_id) {
	addMessage("%s has died", vic_id);
	return 0;
}

int level(Socket* fd) {
	char name[STD_LEN], lvl[STD_LEN];

	if (fd->read(name) < 0) {
		return -1;
	}

	if (fd->read(lvl) < 0) {
		return -1;
	}

	multiplayerLock.lock();

	player* p = findPlayer(name);
	if (!p) {
		multiplayerLock.unlock();
		return -1;
	}

	p->levelUp();

	screenLock.lock();

	if (p == me && REFRESH) {
		printHud();
		addMessage("You leveled up to lvl %d" , strtol(lvl,nullptr ,0));
	}
	else {
		addMessage("%s leveled up to lvl %d", name, strtol(lvl, nullptr, 0));
	}

	screenLock.unlock();

	multiplayerLock.unlock();

	return 0;
}

int showInventory() {

	char buffer[1024];
	memset(buffer, 0, 1024);
	multiplayerLock.lock();
	
	me->sprintInventory(buffer, 1023);

	multiplayerLock.unlock();

	screenLock.lock();
	REFRESH = false;

	wclear(inventoryWin);
	mvwprintw(inventoryWin, 0, 0, "Inventory\n\n");
	waddstr(inventoryWin, buffer);
	wprintw(inventoryWin,"\nPress any key to continue...");
	wrefresh(inventoryWin);

	screenLock.unlock();
	getInput(inventoryWin);
	screenLock.lock();
	REFRESH = true;

	redrawwin(targetWin);
	wrefresh(targetWin);

	redrawwin(borderWin);
	wrefresh(borderWin);

	redrawwin(hudWin);
	wrefresh(hudWin);

	redrawwin(messageWin);
	wrefresh(messageWin);

	redrawwin(mapWin);
	wrefresh(mapWin);

	screenLock.unlock();

	return 0;
}

int showEquip(Socket* fd ) {
	char buffer[1024];
	memset(buffer, 0, 1024);
	multiplayerLock.lock();

	me->sprintInventory(buffer, 1023);

	multiplayerLock.unlock();

	screenLock.lock();
	REFRESH = false;

	wclear(inventoryWin);
	mvwprintw(inventoryWin, 0, 0, "Inventory\n\n");
	waddstr(inventoryWin, buffer);
	wprintw(inventoryWin,  "\nWhat would you like to equip?");
	wrefresh(inventoryWin);

	screenLock.unlock();
	
	int n = wgetch(inventoryWin) - '0';
	
	multiplayerLock.lock();

	item* it = me->getItem(n);
	if (it && dynamic_cast<armor*>(it)) {
		snprintf(buffer, STD_LEN, "%s%c%s%c%s%c", EQUIP_OP, OP_SEP, it->getId(), OP_SEP, BODY_OP, OP_SEP);
		fd->write(buffer, strlen(buffer));
	}
	else if (it && dynamic_cast<weapon*>(it)) {
		screenLock.lock();
		wprintw(inventoryWin, "\nMainhand(m) or Offhand(o)? ");
		wrefresh(inventoryWin);
		screenLock.unlock();

		n = getInput(inventoryWin);
		if (n == 'm') {
			snprintf(buffer, STD_LEN, "%s%c%s%c%s%c", EQUIP_OP, OP_SEP, it->getId(), OP_SEP, MAINHAND_OP, OP_SEP);
		}
		else if (n == 'o') {
			snprintf(buffer, STD_LEN, "%s%c%s%c%s%c", EQUIP_OP, OP_SEP, it->getId(), OP_SEP, OFFHAND_OP, OP_SEP);
		}
		else {
			memset(buffer, 0, STD_LEN);
		}

		fd->write(buffer, strlen(buffer));
	}

	multiplayerLock.unlock();

	screenLock.lock();	

	REFRESH = true;
	
	redrawwin(targetWin);
	wrefresh(targetWin);

	redrawwin(borderWin);
	wrefresh(borderWin);

	redrawwin(hudWin);
	wrefresh(hudWin);

	redrawwin(messageWin);
	wrefresh(messageWin);

	redrawwin(mapWin);
	wrefresh(mapWin);

	screenLock.unlock();

	return 0;
}

int showUnequip(Socket* fd) {
	char buffer[1024];
	memset(buffer, 0, 1024);
	multiplayerLock.lock();

	me->sprintInventory(buffer, 1023);

	multiplayerLock.unlock();

	screenLock.lock();
	REFRESH = false;

	wclear(inventoryWin);
	mvwprintw(inventoryWin, 0, 0, "Inventory\n\n");
	waddstr(inventoryWin, buffer);
	wprintw(inventoryWin, "\nWhat would you like to unequip? MainHand(m), OffHand(o), Body(b)");
	wrefresh(inventoryWin);

	screenLock.unlock();

	int n = wgetch(inventoryWin);

	if (n == 'm') {
		snprintf(buffer, STD_LEN, "%s%c%s%c", UNEQUIP_OP, OP_SEP, MAINHAND_OP, OP_SEP);
		fd->write(buffer, strlen(buffer));
	} else if (n == 'o') {
		snprintf(buffer, STD_LEN, "%s%c%s%c", UNEQUIP_OP, OP_SEP, OFFHAND_OP, OP_SEP);
		fd->write(buffer, strlen(buffer));
	} else if (n == 'b') {
		snprintf(buffer, STD_LEN, "%s%c%s%c", UNEQUIP_OP, OP_SEP, BODY_OP, OP_SEP);
		fd->write(buffer, strlen(buffer));
	}

	screenLock.lock();

	REFRESH = true;

	redrawwin(targetWin);
	wrefresh(targetWin);

	redrawwin(borderWin);
	wrefresh(borderWin);

	redrawwin(hudWin);
	wrefresh(hudWin);

	redrawwin(messageWin);
	wrefresh(messageWin);

	redrawwin(mapWin);
	wrefresh(mapWin);

	screenLock.unlock();

	return 0;
}

int showDrop(Socket* fd) {
	char buffer[1024];
	memset(buffer, 0, 1024);
	multiplayerLock.lock();

	me->sprintInventory(buffer, 1023);

	multiplayerLock.unlock();

	screenLock.lock();
	REFRESH = false;

	wclear(inventoryWin);
	mvwprintw(inventoryWin, 0, 0, "Inventory\n\n");
	waddstr(inventoryWin, buffer);
	wprintw(inventoryWin, "\nWhat would you like to drop?");
	wrefresh(inventoryWin);

	screenLock.unlock();

	int n = wgetch(inventoryWin) - '0';

	multiplayerLock.lock();

	item* it = me->getItem(n);

	if (it) {
		snprintf(buffer, STD_LEN, "%s%c%s%c", DROP_OP, OP_SEP, it->getId(), OP_SEP);
		fd->write(buffer, strlen(buffer));
	}

	multiplayerLock.unlock();

	screenLock.lock();

	REFRESH = true;

	redrawwin(targetWin);
	wrefresh(targetWin);

	redrawwin(borderWin);
	wrefresh(borderWin);

	redrawwin(hudWin);
	wrefresh(hudWin);

	redrawwin(messageWin);
	wrefresh(messageWin);

	redrawwin(mapWin);
	wrefresh(mapWin);

	screenLock.unlock();

	return 0;
}
