#ifndef METASERVER_HEADER
#define METASERVER_HEADER
#include "RPCServer.hpp"
#include "common.hpp"
#include "MetaMessage.h"
#include <unordered_map>
using namespace std;






class MetaServer: public RPCServer{
private:
	unordered_map<uint32_t, ObjMeta> ObjMetaMap;
	//TODO:继承和网络部分的修改
	thread *wk;
	Configuration *conf;
	RPCClient *client;
    unordered_map<uint16_t, ms_onvm_object*> object_map;
    unordered_map<uint16_t, segment*> segment_map;
public:	
	MetaServer(int cqsize);
    ms_onvm_object *find_onvm_object(uint16_t);
    ms_onvm_object *alloc_onvm_object(unsigned char *s, uint16_t size);
    segment_info *get_segment_info(uint16_t oid);
    int init_new_onvm_object( ms_onvm_object *obj);
    int establish_node( ms_onvm_object *obj, int index; uint16_t node_id; uint16_t seg_id);
    void add_onvm_object( ms_onvm_object *obj);
    void ParseMessage(char *bufferRequest, char *bufferResponse, uint16_t NodeID);
	void ProcessRequest(ObjectSendBuffer *send, uint16_t NodeID);
    void Handle_Obj_Post(char *input, char *output, int sender_id, uint16_t size);
    void Handle_Obj_Get(uint16_t oid);
    void Handle_Obj_Put(uint16_t oid);
    void Handle_Obj_Delete(uint16_t oid);
    uint16_t assign_global_oid();
    uint16_t assign_global_seg_id();
    uint16_t assign_one_node();
    Segment* alloc_segment_on_node(uint16_t oid, uint16_t nodeid);
    bool Add_New_Node();
    bool Remove_Node();
    //TODO:Get_Node_Status();
	~MetaServer();
};

#endif