#include "RPCServer.hpp"
#include <sys/wait.h>  
#include <sys/types.h>

RPCServer *server;

/* Catch ctrl-c and destruct. */
void Stop (int signo) {
    Debug::notifyInfo("DMFS is terminated, Bye.");
    _exit(0);
}
int main() {
    signal(SIGINT, Stop);
    // server = new RPCServer(2);
    //connect DS
    // server->getRdmaSocketInstance()->ONVMConnect(0);
    // char *p = (char *)server->getMemoryManagerInstance()->getDataAddress();
    // void RPCServer::ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID, uint16_t offset) {
    // uint64_t bufferRecv = server->getMemoryManagerInstance()->getClientMessageAddress(0);
	// bool RdmaSocket::RdmaWrite(uint16_t NodeID, uint64_t SourceBuffer, uint64_t DesBuffer, uint64_t BufferSize, uint32_t imm, int TaskID) 
    Configuration *conf = new Configuration();
    MemoryManager *mem = new MemoryManager(mm, conf->getServerCount(), 2);
	uint64_t mm = mem->getDmfsBaseAddress();
    RdmaSocket *socket = new RdmaSocket(2, mm, mem->getDmfsTotalSize(), conf, true, 0);
    socket->ONVMConnect(0);
	cout<<"sending create message to DS"<<endl;
    uint64_t bufferRecv = mem->getClientMessageAddress(0);

    GeneralSendBuffer *send = (GeneralSendBuffer*)bufferRecv;
    send->message = ONVM_CREATE;
	ExtentReadSendBuffer *bufferSend = (ExtentReadSendBuffer *)send;
	send->message = ONVM_DS_CREATE;
		uint16_t offset = 0;
		uint32_t imm = NodeID<<16 | offset;
	socket->RdmaWrite(0, (uint64_t)send, 2*4096, bufferSend->size, imm, 1);
    sleep(3600)
    // server->ProcessRequest(send, 0 , 0);
}