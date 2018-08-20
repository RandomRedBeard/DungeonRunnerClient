/*
 * point.cpp
 *
 *  Created on: Jan 10, 2018
 *      Author: thomasjansen
 */

#include "point.h"

point::point() {
	x = y = z = -1;
}

point::point(int tx, int ty) :
		x(tx), y(ty) {
	z = -1;
}

point::point(int tx, int ty, int tz) :
		x(tx), y(ty), z(tz) {
}

point::point(char* buffer) {
	x = y = z = -1;
	char* after = strchr(buffer, POINT_SEP);
	if (!after) {
		return;
	}

	int ind = after - buffer;
	*(buffer + ind) = '\0';

	unsigned int bufferlen = strlen(buffer);
	unsigned int afterlen = strlen(after);

	for (unsigned int i = 0; i < bufferlen; i++) {
		if (!isdigit(*(buffer + i))) {
			return;
		}
	}

	for (unsigned int i = 0; i < afterlen; i++) {
		if (!isdigit(*(after + i))) {
			return;
		}
	}


	int x, y;
	x = strtol(buffer, NULL, 0);
	y = strtol(buffer + ind + 1, NULL, 0);

	setPoint(x, y);
}

void point::setPoint(int tx, int ty) {
	setX(tx);
	setY(ty);
}

void point::setPoint(int tx, int ty, int tz) {
	setX(tx);
	setY(ty);
	setZ(tz);
}

void point::setX(int tx) {
	x = tx;
}

int point::getX() {
	return x;
}

void point::setY(int ty) {
	y = ty;
}

int point::getY() {
	return y;
}

void point::setZ(int tz) {
	z = tz;
}

int point::getZ() {
	return z;
}

void point::rand_rc(int xlim, int ylim) {
	x = rand() % xlim;
	y = rand() % ylim;
}

void point::rand_rc(int xlim, int ylim, int zlim) {
	x = rand() % xlim;
	y = rand() % ylim;
	z = rand() % zlim;
}

double point::dist(point pt) {
	return sqrt(pow(pt.getX() - x, 2) + pow(pt.getY() - y, 2));
}

int point::l1dist(point pt) {
	return abs(pt.getX() - x) + abs(pt.getY() - y);
}

void point::operator =(point pt) {
	x = pt.getX();
	y = pt.getY();
	z = pt.getZ();
}

bool point::operator ==(point pt) {
	return (x == pt.getX() && y == pt.getY() && z == pt.getZ());
}

bool point::operator !=(point pt) {
	return (x != pt.getX() || y != pt.getY() || z != pt.getZ());
}

int point::index(int height, int width) {
	return x * width + y;
}

