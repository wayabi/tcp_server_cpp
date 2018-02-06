#include <string>
#include <sstream>

#include "BaseThread.h"
#include "StdInThread.h"
#include "TcpServer.h"
#include "Util.h"
#include "ExpandVariable.h"

using namespace std;

int main(int argc, char** argv)
{
	string path_file_variable = "./variable";
	if(argc < 2){
		printf("usage: %s <port_server>\n", *(argv+0));
		return 1;
	}
	bool flag_dump_bin = true;

	StdInThread thread_stdin;
	thread_stdin.start(NULL);
	TcpServer server(atoi(*(argv+1)), 200);
	server.start(NULL);

	while(true){
		thread_stdin.ss.lock();
		vector<char> c = thread_stdin.ss.head();
		if(c.size() > 0){
			string s = Util::trim(string(&c[0], c.size()), "\n");
			s = ExpandVariable::expandVariable(s, path_file_variable);
			thread_stdin.ss.erase();
			if(s == "q"){
				thread_stdin.ss.unlock();
				thread_stdin.stop();
				break;
			}else if(s == "#state"){
				for(
					list<ServerThread*>::iterator ite = server.server_threads.begin();
					ite != server.server_threads.end();++ite
				){
					printf("state[%d]\n", (*ite)->getIdThread());
				}
			}else{
				for(
					list<ServerThread*>::iterator ite = server.server_threads.begin();
					ite != server.server_threads.end();++ite
				){
					if((*ite)->isConnected()){
						printf("[%d] send:%s\n", (*ite)->getIdThread(), s.c_str());
						(*ite)->send(s);
					}
				}
			}
		}
		thread_stdin.ss.unlock();

		for(
			list<ServerThread*>::iterator ite = server.server_threads.begin();
			ite != server.server_threads.end();++ite
		){
			if((*ite)->isConnected()){
				(*ite)->ss.lock();
				while(true){
					vector<char> c = (*ite)->ss.head();
					if(c.size() == 0) break;
					if(flag_dump_bin){
						stringstream ss;
						Util::hexdump(ss, &c[0], c.size());
						cout << "[" << (*ite)->getIdThread() << "] received:" << endl << ss.str() << endl;
					}else{
						string s(&c[0], c.size());
						printf("[%d] received:%s\n", (*ite)->getIdThread(), s.c_str());
					}
					(*ite)->ss.erase();
				}
				(*ite)->ss.unlock();
			}	
		}
		usleep(50*1000);
	}
	server.stop();
	printf("join tcp_server\n");
	server.join();
	printf("join std_thread\n");
	thread_stdin.join();
	return 0;
}
