#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <list>
#include <string>

#include "BaseThread.h"
#include "StringSequence.h"

struct EndPoint {
public:
	int sock;
	sockaddr_in addr;
};

class ServerThread;

class TcpServer : public BaseThread {
public:
	TcpServer(int port, int max_peers_);
	virtual ~TcpServer();
	void stop();

protected:
	virtual void execute(void*);
	int getIdThreadFree();

public:
	std::list<ServerThread*> server_threads;
protected:
	EndPoint ep_local;
	int max_peers;
	bool flag_stop;
};

class ServerThread : public BaseThread {
public:
	ServerThread(int id_thread_, EndPoint* ep_local_, const EndPoint& ep_peer_);
	virtual ~ServerThread();
	bool isConnected(){return this->flag_connected;}
	int send(const std::vector<char>& message);
	int send(const std::string& message);
	int getIdThread(){return this->id_thread;}
	void stop();

protected:
	virtual void execute(void*);

public:
	StringSequence ss;

protected:
	int id_thread;
	EndPoint* ep_local;
	EndPoint ep_peer;
	const static int SIZE_BUF= 100000;
	char buf[SIZE_BUF];
	bool flag_connected;
	bool flag_stop;
};
