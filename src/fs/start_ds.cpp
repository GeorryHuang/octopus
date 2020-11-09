#include "RPCServer.hpp"
#include <sys/wait.h>  
#include <sys/types.h>
#include <unistd.h>
#include "global.h"

RPCServer *server;

/* Catch ctrl-c and destruct. */
void Stop (int signo) {
    delete server;
    Debug::notifyInfo("DMFS is terminated, Bye.");
    _exit(0);
}
int main() {
    signal(SIGINT, Stop);
    server = new RPCServer(test_thread_count);
    while(true){
        sleep(5);
        // cout<<"DS running"<<endl;
    }
}