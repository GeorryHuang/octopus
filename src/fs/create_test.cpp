#include "RPCServer.hpp"
#include <sys/wait.h>
#include <sys/types.h>
#include "global.h"
#include <atomic>
RPCServer *server;
namespace A{

RdmaSocket *socket;
uint32_t imm;
ExtentReadSendBuffer* bufferSend;
GeneralSendBuffer *send;
uint16_t NodeID = 1;
/* Catch ctrl-c and destruct. */
atomic_long total(0);
}


void Stop(int signo)
{
    Debug::notifyInfo("DMFS is terminated, Bye.");
    Debug::endTimer();
    _exit(0);
}


void hzy_test(int thread_num){
for (int i = 0; i < 5000000; i++)
    {
        A::socket->RdmaWrite(0, (uint64_t)A::send + thread_num * A::bufferSend->size , thread_num * A::bufferSend->size, A::bufferSend->size, A::imm, 0);
        struct ibv_wc wc;
        A::socket->PollCompletion(0, 1, &wc);
    }
    cout<<"Thread "<<i<<" Done!"<<endl;
    A::total++;
    if(A::total == test_thread_count){
        Stop(0);
    }
}

int main()
{
    
    signal(SIGINT, Stop);
    // server = new RPCServer(2);
    //connect DS
    // server->getRdmaSocketInstance()->ONVMConnect(0);
    // char *p = (char *)server->getMemoryManagerInstance()->getDataAddress();
    // void RPCServer::ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID, uint16_t offset) {
    // uint64_t bufferRecv = server->getMemoryManagerInstance()->getClientMessageAddress(0);
    // bool RdmaSocket::RdmaWrite(uint16_t NodeID, uint64_t SourceBuffer, uint64_t DesBuffer, uint64_t BufferSize, uint32_t imm, int TaskID)
    uint64_t mm = 0;
    Configuration *conf = new Configuration();
    MemoryManager *mem = new MemoryManager(mm, conf->getServerCount(), 2);
    mm = mem->getDmfsBaseAddress();
    A::socket = new RdmaSocket(2, mm, mem->getDmfsTotalSize(), conf, true, 0);
    A::socket->ONVMConnect(0);
    cout << "sending create message to DS" << endl;
    uint64_t bufferRecv = mem->getDmfsBaseAddress();
    cout << "sendding message start from:" << bufferRecv << endl;
    A::send = (GeneralSendBuffer *)bufferRecv;
    A::bufferSend = (ExtentReadSendBuffer *)A::send;
    // A::bufferSend->size = test_message_size;
    A::bufferSend->size = 1024;
    A::send->message = ONVM_DS_CREATE;
    uint16_t offset = 0;
    A::imm = A::NodeID << 16 | offset;
    cout << "sending message is :" << A::send->message << endl;

    Debug::startTimer("TestPut");
    thread threads[test_thread_count];
    for(int i=0;i<test_thread_count;i++){
        threads[i] = thread(hzy_test, i);
    }
    while(true);
    

    // server->ProcessRequest(send, 0 , 0);
}



