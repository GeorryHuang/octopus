#ifndef RPCSERVER_HREADER
#define RPCSERVER_HREADER
#include "RPCServer.hpp"
#include "common.hpp"
#include <unordered_map>
using namespace std;

class MetaServer: public RPCServer{
private:
	unordered_map<uint32_t, ObjMeta> ObjMetaMap;
	//TODO:继承和网络部分的修改
	thread *wk;
	Configuration *conf;
	RdmaSocket *socket;
	RPCClient *client;
public:	
	MetaServer(int cqsize);
    void ParseMessage(char *bufferRequest, char *bufferResponse, uint16_t NodeID);
	void ProcessRequest(ObjectSendBuffer *send, uint16_t NodeID);
    uint16_t Handle_Obj_Post(ObjectSendBuffer *send, uint16_t size);
    ObjMeta* Handle_Obj_Get(uint16_t oid);
    uint16_t Handle_Obj_Put(uint16_t oid);
    uint16_t Handle_Obj_Delete(uint16_t oid);
    uint16_t assign_global_oid();
    uint16_t assign_one_node();
    Segment* alloc_segment_on_node(uint16_t oid, uint16_t nodeid);
    bool Add_New_Node();
    bool Remove_Node();
    //TODO:Get_Node_Status();
	~MetaServer();
};

#endif