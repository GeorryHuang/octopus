#include "RPCServer.hpp"
#include <sys/wait.h>  
#include <sys/types.h>

RPCServer *server;

/* Catch ctrl-c and destruct. */
void Stop (int signo) {
    delete server;
    Debug::notifyInfo("DMFS is terminated, Bye.");
    _exit(0);
}
int main() {
    signal(SIGINT, Stop);
    server = new RPCServer(2);
    char *p = (char *)server->getMemoryManagerInstance()->getDataAddress();
    // void RPCServer::ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID, uint16_t offset) {
    uint64_t bufferRecv = server->getMemoryManagerInstance()->getClientMessageAddress();
    GeneralSendBuffer *send = (GeneralSendBuffer*)bufferRecv;
    send->message = ONVM_CREATE;
    server->ProcessRequest(send, 0 , 0);
}