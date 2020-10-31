#ifndef METASERVER_HEADER
#define METASERVER_HEADER

#include "RPCServer.hpp"
#include "common.hpp"
#include "MetaMessage.h"
#include <unordered_map>
#include "onvm/NVMObjectPool.hpp"
#include "onvm/SegmentPool.hpp"






class MetaServer: public RPCServer{
private:
	// unordered_map<uint32_t, ObjMeta> ObjMetaMap;
	//TODO:继承和网络部分的修改
    NVMObjectPool* nvmObjectPool;
    SegmentPool* segmentPool;
	thread *wk;
	Configuration *conf;
	RPCClient *client;
    unordered_map<uint16_t, ms_onvm_object*> object_map;
    unordered_map<uint16_t, segment_data*> segment_map;
    unordered_map<string, uint16_t> name2oid_map;
    void del_onvm_object(uint16_t oid);
    ms_onvm_object *alloc_onvm_object(char * recvBuffer, char* response, unsigned char *objName, uint16_t objSize);
    void obj2reply(ms_onvm_object *obj, char *rep);
public:	
	MetaServer(int cqsize);
    ms_onvm_object *find_onvm_object(uint16_t);
    ms_onvm_object *find_onvm_object(string);
    // ms_onvm_object *alloc_onvm_object(unsigned char *s, uint16_t size);
    segment_data *get_segment_data(uint16_t oid);
    int init_new_onvm_object( ms_onvm_object *obj);
    int establish_node(char * recvBuffer, char* response,  ms_onvm_object *obj, int index, uint16_t node_id, uint16_t seg_id);
    void add_onvm_object( ms_onvm_object *obj);
    void ParseMessage(char *bufferRequest, char *bufferResponse, uint16_t NodeID);
	void ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID);
    int Handle_Obj_Post(char *recvBuffer, char *responseBuffer, uint16_t NodeID);
    int Handle_Obj_Get(char *recvBuffer, char *responseBuffer);
    int Handle_Obj_Put(char *recvBuffer, char *responseBuffer);
    int Handle_Obj_Delete(char *recvBuffer, char *responseBuffer);
    int Handle_OBJ_ALLOC_SEG_AT_DS(char *recvBuffer, char *responseBuffer);
    int Handle_READ_SEG(char *recvBuffer, char *responseBuffer);
    uint16_t assign_global_oid();
    uint16_t assign_global_seg_id();
    uint16_t assgin_one_node();
    void ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID, uint16_t offset);
    // Segment* alloc_segment_on_node(uint16_t oid, uint16_t nodeid);
    bool Add_New_Node();
    bool Remove_Node();
    //TODO:Get_Node_Status();
	~MetaServer();
};

#endif