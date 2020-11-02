#ifndef RDMABUFFER_POOL_HEADER
#define RDMABUFFER_POOL_HEADER
#include "global.h"
#include <string>

/**
 * 收到RDMA信息后，有一个对应区域的buffer，在这里取出数据后，会复用这一块区域，然后把要response的信息继续填入这块内存进行response，这块内存就是RDMABuffer
*/
class RDMABuffer{
private:
    char* rdmaMemory;
    char recvMessage[CLIENT_MESSAGE_SIZE];
    char responseMessage[CLIENT_MESSAGE_SIZE];
public:
    RDMABuffer(uint64_t memoryPointer){
        this->rdmaMemory = (char*)memoryPointer;
        memcpy(recvMessage, (char*)memoryPointer, CLIENT_MESSAGE_SIZE);
    }
};


#endif