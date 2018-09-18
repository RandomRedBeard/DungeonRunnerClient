/*
 * Socket.cpp
 *
 *  Created on: Nov 17, 2017
 *      Author: thomasjansen
 */

#include "Socket.h"

Socket::Socket(int f) :
		fd(f) {
}

Socket::Socket() {
	fd = -1;
}

Socket::Socket(std::string addr, int port) {
	fd = socket(AF_INET, SOCK_STREAM, 0);
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(addr.c_str());
	serv.sin_port = htons(port);
	serv_len = sizeof(serv);

	char portBuffer[STD_LEN];
	snprintf(portBuffer, STD_LEN, "%d", port);

	struct addrinfo hints , *taddr;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(addr.c_str(), portBuffer, &hints, &taddr);

	connect(fd, (sockaddr*)taddr->ai_addr, taddr->ai_addrlen);
}

Socket::~Socket() {
	closeSocket();
}

int Socket::read(char* buffer, int len) {
	return readln(fd, buffer, len);
}

int Socket::read(char* buffer) {
	return Socket::read(buffer, STD_LEN);
}

std::vector<int> Socket::readList(std::initializer_list<char*> buffers) {
	std::vector<int> rval;

	for (char* buffer : buffers) {
		rval.push_back(read(buffer));
	}

	return rval;
}

int Socket::write(const char* buffer, int len) {
	return writeStream(fd, buffer, len);
}

int Socket::closeSocket() {
#ifdef _WIN32
	return closesocket(fd);
#else
	return close(fd);
#endif
}

int Socket::shutdownSocket() {
	return shutdown(fd, 2);
}
