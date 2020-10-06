#ifndef METASERVER_HEADER
#define METASERVER_HEADER
#include "RPCServer.hpp"
#include "common.hpp"
#include "MetaMessage.h"
#include <unordered_map>
using namespace std;


typedef struct ms_segment_info
{
	uint16_t seg_id;
	uint16_t node_id;
	//TODO:add sth
};

typedef struct ms_onvm_object
{
	uint16_t oid;
	unsigned char name[MAX_OBJ_NAME_LENGTH];
	time_t timeLastModified;
	uint16_t size;
	ms_segment_info pos_info[MAX_SEGMENT_COUNT_ONE_ObJ];
};
//ccy add end


class MetaServer: public RPCServer{
private:
	unordered_map<uint32_t, ObjMeta> ObjMetaMap;
	//TODO:继承和网络部分的修改
	thread *wk;
	Configuration *conf;
	RdmaSocket *socket;
	RPCClient *client;
    unordered_map<uint16_t, ms_onvm_object> object_map;
public:	
	MetaServer(int cqsize);
    ms_onvm_object *find_onvm_object(unsigned char *s);
    ms_onvm_object *alloc_onvm_object(unsigned char *s, uint16_t size);
    ms_segment_info *get_segment_info(uint16_t oid);
    int init_new_onvm_object(struct ms_onvm_object *obj);
    int establish_node(struct ms_onvm_object *obj, int index; uint16_t node_id; uint16_t seg_id);
    int add_onvm_object(struct ms_onvm_object *obj);
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