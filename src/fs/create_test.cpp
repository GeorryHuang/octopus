#include "RPCServer.hpp"
#include <sys/wait.h>
#include <sys/types.h>
#include "global.h"
namespace A{
RPCServer *server;
RdmaSocket *socket;
uint32_t imm;
ExtentReadSendBuffer* bufferSend;
GeneralSendBuffer *send;
uint16_t NodeID = 1;
/* Catch ctrl-c and destruct. */
void Stop(int signo)
{
    Debug::notifyInfo("DMFS is terminated, Bye.");
    _exit(0);
}
void hzy_test(int i){
for (int i = 0; i < 5000000; i++)
    {
        socket->RdmaWrite(0, (uint64_t)send, NodeID * CLIENT_MESSAGE_SIZE, bufferSend->size, imm, 0);
        struct ibv_wc wc;
        socket->PollCompletion(0, 1, &wc);
    }
    cout<<"Thread "<<i<<" Done!"<<endl;
}
using namespace A;
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
    socket = new RdmaSocket(2, mm, mem->getDmfsTotalSize(), conf, true, 0);
    socket->ONVMConnect(0);
    cout << "sending create message to DS" << endl;
    uint64_t bufferRecv = mem->getDmfsBaseAddress();
    cout << "sendding message start from:" << bufferRecv << endl;
    GeneralSendBuffer *send = (GeneralSendBuffer *)bufferRecv;
    ExtentReadSendBuffer *bufferSend = (ExtentReadSendBuffer *)send;
    bufferSend->size = sizeof(ExtentReadSendBuffer);
    send->message = ONVM_DS_CREATE;
    uint16_t offset = 0;
    imm = NodeID << 16 | offset;
    cout << "sending message is :" << send->message << endl;


    thread threads[test_thread_count];
    for(int i=0;i<test_thread_count;i++){
        thread(hzy_test, i);
    }
    while(true);
    

    // server->ProcessRequest(send, 0 , 0);
}


}

