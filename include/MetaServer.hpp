#ifndef RPCSERVER_HREADER
#define RPCSERVER_HREADER
#include <thread>
#include <unordered_map>
#include <vector>
#include "RdmaSocket.hpp"
#include "Configuration.hpp"
#include "RPCClient.hpp"
#include "mempool.hpp"
#include "global.h"
//ccy add start
#include "common.h"
//ccy add end
#include "filesystem.hpp"
#include "TxManager.hpp"

using namespace std;

typedef unordered_map<uint32_t, int> Thread2ID;
typedef unordered_map<uint32_t, ObjMeta> ObjMetaMap;

typedef struct {
	uint64_t send;
	uint16_t NodeID;
	uint16_t offset;
} RPCTask;

class MetaServer {
private:
	thread *wk;
	Configuration *conf;
	RdmaSocket *socket; 
    //ccy mod start
	/*MemoryManager *mem;
	uint64_t mm;
	TxManager *tx;
	RPCClient *client;
	int ServerCount;
	FileSystem *fs;*/
    //ccy mod end
	int cqSize;
	Thread2ID th2id;
	vector<RPCTask*> tasks;
	bool UnlockWait;
	void Worker(int id);
	void ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID, uint16_t offset);
	void ProcessQueueRequest();
public:
	
    //ccy add start
	MetaServer(int cqsize);
    void parseMessage(char *bufferRequest, char *bufferResponse);
    uint16_t Handle_Obj_Post(uint16_t size);
    ObjMeta* Handle_Obj_Get(uint16_t oid);
    uint16_t Handle_Obj_Put(uint16_t oid);
    uint16_t Handle_Obj_Delete(uint16_t oid);
    uint16_t assign_global_oid();
    uint16_t assign_one_node();
    Segment* alloc_segment_on_node(uint16_t oid, uint16_t nodeid);
    Add_New_Node();
    Remove_Node();
    Get_Node_Status();
	//ccy add end 

	//RPCServer(int cqSize);
	RdmaSocket* getRdmaSocketInstance();
	//MemoryManager* getMemoryManagerInstance();
	RPCClient* getRPCClientInstance();
	//TxManager* getTxManagerInstance();
	uint64_t ContractReceiveBuffer(GeneralSendBuffer *send, GeneralReceiveBuffer *recv);
	void RequestPoller(int id);
	int getIDbyTID();
	~MetaServer();
};

#endif