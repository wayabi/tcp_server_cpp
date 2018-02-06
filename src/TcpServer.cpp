#include "TcpServer.h"

using namespace std;

TcpServer::TcpServer(int port, int max_peers_)
{
	this->max_peers = max_peers_;
	this->flag_stop = false;
	this->ep_local.sock = socket(AF_INET, SOCK_STREAM, 0);
	this->ep_local.addr.sin_family = AF_INET;
	this->ep_local.addr.sin_port = htons(port);
	this->ep_local.addr.sin_addr.s_addr = INADDR_ANY;
	int TRUE = 1;
	setsockopt(this->ep_local.sock, SOL_SOCKET, SO_REUSEADDR, &TRUE, sizeof(TRUE));
}

TcpServer::~TcpServer()
{
	for(list<ServerThread*>::iterator ite = this->server_threads.begin();ite != this->server_threads.end();++ite){
		delete(*ite);
	}
}

void TcpServer::execute(void*)
{
	int ret = bind(this->ep_local.sock, (struct sockaddr*)&(this->ep_local.addr), sizeof(this->ep_local.addr));
	if(ret == -1){
		int err =  errno;
		switch(err){
		case EADDRINUSE:
			printf("EADDRINUSE\n");
			break;
		case EINVAL:
			printf("EINVAL\n");
			break;
		default:
			break;
		}
		printf("bind error(%d)\n", err);
	}
	listen(this->ep_local.sock, this->max_peers);

	EndPoint ep_peer;
	while(!this->flag_stop){
		usleep(10*1000);
		for(list<ServerThread*>::iterator ite = this->server_threads.begin();ite != this->server_threads.end();){
			if(!(*ite)->isConnected()){
printf("join start[%d]\n", (*ite)->getIdThread());
				(*ite)->join();
printf("join end[%d]\n", (*ite)->getIdThread());
				delete(*ite);
				ite = this->server_threads.erase(ite);
			}else{
				++ite;
			}
		}

		if(this->server_threads.size() >= (size_t)(this->max_peers)) continue;

		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		fd_set set;
		FD_ZERO(&set);
		FD_SET(this->ep_local.sock, &set);
		int ret = select(this->ep_local.sock+1, &set, NULL, NULL, &timeout);
		if(ret <= 0) continue;
		if(!FD_ISSET(this->ep_local.sock, &set)) continue;
		EndPoint ep_peer;
		socklen_t len = sizeof(ep_peer.addr);
		ep_peer.sock = accept(this->ep_local.sock, (struct sockaddr*)&(ep_peer.addr), &len);
		int id = this->getIdThreadFree();
		printf("new connection (%s:%d) id[%d]\n", inet_ntoa(ep_peer.addr.sin_addr), ep_peer.addr.sin_port, id);
		ServerThread* thread = new ServerThread(id, &(this->ep_local), ep_peer);
		thread->start(NULL);
		this->server_threads.push_back(thread);
	}

	//end
	printf("end start\n");
	for(list<ServerThread*>::iterator ite = this->server_threads.begin();ite != this->server_threads.end();++ite){
		(*ite)->stop();
	}
	for(list<ServerThread*>::iterator ite = this->server_threads.begin();ite != this->server_threads.end();++ite){
		printf("join start[%d]\n", (*ite)->getIdThread());
		(*ite)->join();
		printf("join end[%d]\n", (*ite)->getIdThread());
		delete(*ite);
	}
	this->server_threads.clear();
	close(this->ep_local.sock);
}

void TcpServer::stop()
{
	this->flag_stop = true;
}

int TcpServer::getIdThreadFree()
{
	const int size_map = 10000;
	char map[size_map];
	memset(map, 0, size_map);
	for(list<ServerThread*>::iterator ite = this->server_threads.begin();ite != this->server_threads.end();++ite){
		map[(*ite)->getIdThread()] = 1;
	}
	int i=0;
	for(;i<size_map;++i){
		if(map[i] == 0) break;
	}
	if(i < size_map) return i;
	return -1;
}

ServerThread::ServerThread(int id_thread_, EndPoint* ep_local_, const EndPoint& ep_peer_)
{
	this->id_thread = id_thread_;
	this->ep_local = ep_local_;
	this->ep_peer = ep_peer_;
	this->flag_connected = true;
	this->flag_stop = false;
}

ServerThread::~ServerThread()
{
}

void ServerThread::execute(void*)
{
	while(!this->flag_stop){
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		fd_set set;
		FD_ZERO(&set);
		FD_SET(this->ep_peer.sock, &set);
		int ret = select(this->ep_peer.sock+1, &set, NULL, NULL, &timeout);
		if(ret <= 0) continue;
		if(!FD_ISSET(this->ep_peer.sock, &set)) continue;

		int len = read(this->ep_peer.sock, this->buf, ServerThread::SIZE_BUF);
		if(len > 0){
			this->ss.lock();
			this->ss.push_back(this->buf, len);
			string s(this->buf, len);
			this->ss.unlock();
		}else{
printf("[%d] closed\n", this->id_thread);
			break;
		}
	}
	close(this->ep_peer.sock);
	this->flag_connected = false;
}

int ServerThread::send(const std::vector<char>& message)
{
	if(!this->flag_connected){
		printf("connection closed@ServerThread::send()\n");
		return -1;
	}
	return write(this->ep_peer.sock, &message[0], message.size());
}

int ServerThread::send(const std::string& message)
{
	if(!this->flag_connected){
		printf("connection closed@ServerThread::send()\n");
		return -1;
	}
	return write(this->ep_peer.sock, message.c_str(), message.size());
}

void ServerThread::stop()
{
	this->flag_stop = true;
}
