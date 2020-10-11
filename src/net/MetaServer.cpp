#include "MetaServer.hpp"
// __thread struct  timeval startt, endd;
//ccy add start
MetaServer::MetaServer(int _cqSize) :cqSize(_cqSize), {
	
	socket = new RdmaSocket(cqSize, 0, 0), conf, true, 0);
	client = new RPCClient(conf, socket, mem, (uint64_t)mm);
	//TODO:initialize MetaServer
	socket->RdmaListen();
}
MetaServer::~MetaServer() {
	Debug::notifyInfo("Stop MetaServer.");
	delete conf;
	for (int i = 0; i < cqSize; i++) {
		wk[i].detach();
	}
	delete wk;
	delete socket;
	Debug::notifyInfo("MetaServer is closed successfully.");
}
void MetaServer::add_onvm_object(ms_onvm_object *obj)
{
	object_map.insert(pair<uint16_t,ms_onvm_object*>(obj->oid,obj));
}

ms_onvm_object* MetaServer::find_onvm_object(uint16_t oid)
{
	ms_onvm_object *obj = object_map.find(oid);
	if(obj == NULL){
		Debug::notifyInfo("this object is not found!");
		return NULL;
	}
	return obj;
}


void MetaServer::del_onvm_object(uint16_t oid){
	object_map.erase(oid);
}

/**
 * 在堆内构建一个一个ms_onvm_object结构体并返回它的引用;通过rdma联系所有的node构建这个obj的segment
 * 
 * 
*/
ms_onvm_object *MetaServer::alloc_onvm_object(char * recvBuffer, char* response, unsigned char *objName, uint16_t objSize)
{
	ms_onvm_object *obj;
	obj = malloc(sizeof(*obj));
	if(obj){
		memset(obj, 0, sizeof(ms_onvm_object));
		//INIT_LIST_HEAD(&obj->next); 对比hotpot, 这个list有存在的必要吗？
	}else {
		return -1;
	}

	//这个nr_established可以用来作是否完成所有seg创建的判断，但现阶段用不上
	int  nr_established = 0;
	int seg_size = ceil(obj->size/SEGMENT_SIZE);
	obj->oid = assign_global_oid();
	int i;
	for(i = 0; i < seg_size; i++){
		segment_info *seg_p = obj->segments[i];
		seg_p->node_id = assgin_one_node();
		seg_p->seg_id = assign_global_seg_id()
		// char * recvBuffer, char* response,  ms_onvm_object *obj, int index; uint16_t node_id; uint16_t seg_id
		if(!establish_node(recvBuffer, response, obj, i, seg_p->node_id, seg_p->seg_id))
			nr_established++;
	}

	obj->nr_seg=seg_size;
	obj->size = objSize;
	memcpy(obj->name, objName, MAX_OBJ_NAME_LENGTH);
	obj->timeLastModified = 0;
	return obj;
}
uint16_t MetaServer::assign_global_seg_id()
{
	return SEG_NO_COUNTER++;
}

uint16_t MetaServer::assign_global_oid()
{
	return OBJ_NO_COUNTER++;
}

//FIXME:这。。。。
uint16_t MetaServer::assgin_one_node()
{
	return 1
};

segment_info *MetaServer::get_segment_info(uint16_t oid)
{
	ms_onvm_object *obj = object_map.find(oid);
	if(obj == NULL){
		Debug::notifyInfo("this object is not found!");
		return NULL;
	}
	return obj->pos_info;
}

// int MetaServer::init_new_onvm_object(struct ms_onvm_object *obj)
// {
// 	int index,  nr_established = 0;
// 	uint16_t seg_id, node_id, oid;

// 	oid = assign_global_oid();
// 	for(index = 0; index < ceil(obj->size/SEGMENT_SIZE); index++){
// 		node_id = assgin_one_node();
// 		seg_id = assign_global_seg_id();	
// 		obj->oid = oid;
// 		if(!establish_node(obj, index, node_id, seg_id))
// 			nr_established++;
// 	}
// }


/**
 * 为obj分配在不同的node上分配segment，并赋值到ms_ovnm_object对象中。
 * send: RDMA的发送buffer
 * response: RDMA的response的buffer
 * obj: 对象结构信息
 * index: obj的第index个segement
 * node_id: 存放这个segment的node_id
 * seg_id: 这个segment的id
 * return 0 if success, !=0 failed
 * 
*/
int MetaServer::establish_node(char * recvBuffer, char* response,  ms_onvm_object *obj, int index; uint16_t node_id; uint16_t seg_id)
{
	SegmentCreateRequest *request = (SegmentCreateRequest*)sendBufer;
	onvm_reply *reply = (onvm_reply*)response;

	request->message = MESSAGE_ALLOC_SEG_AT_DS;
	request->seg_id = seg_id;
	//size = sizeof(request.message) + sizeof(request.seg_id);
	//TODO:send alloc segment request to DS
	/*发送时传入node_id，结构体的起始64位地址，以及结构体的字节数目
	*/


	if(0!=this.socket->RdmaSend(node_id, (uint64_t)SegmentCreateRequest, sizeof(SegmentCreateRequest))){
		Debug::notifyError("Alloc segment failed! Reason: RdmaSend failed");
		return -1;
	}
	if(0!=this.socket->RdmaReceive(node_id, (uint64_t)reply, sizeof(onvm_reply))){
		Debug::notifyError("Alloc segment failed! Reason: RdmaReceive failed");
		return -1;
	}
	
	if(reply.status != ONVM_REPLY_SUCCESS){
		Debug::notifyError("Alloc segment failed! DataServer reply not ONVM_REPLY_SUCCESS");
		return -1;
	}

	// s = &(obj->pos_info[index]);
	// s->seg_id = seg_id;
	// s->node_id = node_id;
	return 0;
}


// void MetaServer::ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID)
// {
// 	char receiveBuffer[CLIENT_MESSAGE_SIZE];
// 	uint64_t bufferRecv = (uint64_t)send;
// 	GeneralReceiveBuffer *recv = (GeneralReceiveBuffer*)receiveBuffer;
// 	recv->taskID = send->taskID;
// 	recv->message = MESSAGE_RESPONSE;
// 	uint64_t size = send->sizeReceiveBuffer;

// 	switch (send->message)
// 	{
// 	case MESSAGE_POST_OBJ:
// 		/* code */
// 		break;
// 	case MESSAGE_GET_OBJ:
// 		break;
// 	case MESSAGE_PUT_OBJ:
// 		break;
// 	case MESSAGE_DEL_OBJ:
	
// 		break;
// 	case MESSAGE_ALLOC_SEG_AT_DS：
// 		break;
// 	default:
// 		break;
// 	}


// 	if (send->message == MESSAGE_DISCONNECT) {
//         //rdma->disconnect(send->sourceNodeID);
//         return;
//     }



// 	char receiveBuffer[CLIENT_MESSAGE_SIZE];
// 	uint64_t bufferRecv = (uint64_t)send;
// 	ObjectRecvBuffer *recv = (GeneralReceiveBuffer*)receiveBuffer;
// 	recv->taskID = send->taskID;
// 	recv->message = MESSAGE_RESPONSE;
// 	uint64_t size = send->sizeReceiveBuffer;
// 	if (send->message == MESSAGE_DISCONNECT) {
//         //rdma->disconnect(send->sourceNodeID);
//         return;
//     } else if (send->message == MESSAGE_TEST) {
//     	;
//     } else if (send->message == MESSAGE_UPDATEMETA) {
//     	/* Write unlock. */
//     	// UpdateMetaSendBuffer *bufferSend = (UpdateMetaSendBuffer *)send;
//     	// fs->unlockWriteHashItem(bufferSend->key, NodeID, bufferSend->offset);
//     	return;
//     } else if (send->message == MESSAGE_EXTENTREADEND) {
//     	/* Read unlock */
//     	// ExtentReadEndSendBuffer *bufferSend = (ExtentReadEndSendBuffer *)send;
//     	// fs->unlockReadHashItem(bufferSend->key, NodeID, bufferSend->offset);
//     	return;
// 	} else {
// 		ParseMessage((char*)send, receiveBuffer);
// 		//TODO:
// 	}
// }

// void MetaServer::ParseMessage(char *bufferRequest, char *bufferResponse, uint16_t NodeID)
// {
// 	ObjectSendBuffer *bufferObjSend;
// 	ObjectRecvBuffer *bufferObjRecv;
	
// 	ObjectSendBuffer *bufferObjSend = (ObjectSendBuffer *)bufferRequest; /* Send and request. */
//     ObjectRecvBuffer *bufferObjRecv = (ObjectRecvBuffer *)bufferResponse; /* Receive and response. */
//     //bufferGeneralReceive->message = MESSAGE_RESPONSE; /* Fill response message. */
// 	switch(bufferObjSend->message) {
// 		case MESSAGE_POST_OBJ{
// 			Handle_Obj_Post(bufferObjSend,bufferObjRecv,NodeID);
// 			break;
// 		}
// 		case MESSAGE_GET_OBJ{
// 			Handle_Obj_Get(bufferObjSend,bufferObjReceive,NodeID)
// 			break;
// 		}c
// 		case MESSAGE_PUT_OBJ{
// 			Handle_Obj_Put(bufferObjSend,bufferObjReceive,NodeID);
// 			break;
// 		}
// 		case MESSAGE_DEL_OBJ{
// 			Handle_Obj_Delete(bufferObjSend,bufferObjReceive,NodeID);
// 			break;
// 		}
// 	}
// }

/**
 * 
 * 
 * return 0 为正常，非0则是异常
*/

int MetaServer::Handle_Obj_Post(char *recvBuffer, char *responseBuffer, uint16_t NodeID)
{

/*
uint16_t objSize;
    uint16_t oid;//创建时用不到，其他操作会用
    unsigned char name[MAX_OBJ_NAME_LENGTH];
*/
	onvm_request_post_obj *request = (onvm_request_post_obj*)recvBuffer
	uint16_t objSize = request->objSize;
	unsigned char objName[MAX_OBJ_NAME_LENGTH];
	memset(objName,0,MAX_OBJ_NAME_LENGTH);
	memcpy(objName, request->name, MAX_OBJ_NAME_LENGTH);

	obj = find_onvm_object(objName);
    if(!obj)
    {
		if(objSize <= 0)
        {
			Debug::notifyInfo("POST OP Failed!")obj
        	return -1;
		}
		//FIXME: obj没有地方施放，会内存泄漏
    	obj = alloc_onvm_object(recvBuffer, responseBuffer, objName, objSize);
		if(!obj)
		{
			// reply->status = ONVM_POST_OBJ_FAIL;
			//*reply_len = sizeof(unsigned int);
			return -1;
		}
    	// init_new_onvm_object(obj);
		add_onvm_object(obj);       
    }
	
	// for(idx = 0; idx < MAX_SEGMENT_COUNT; idx++){
	// 	segment->seg_id = obj->pos_info[idx].seg_id;
	// 	segment->node_id = obj->pos_info[idx].node_id;
	// 	segment++;
	// 	nr_seg++;
	// }



/*
    ONVM_REPLY_STATUS status;
    uint16_t nr_seg; 
    uint16_t oid;
    segment_info segments[MAX_SEGMENT_COUNT]; 
*/
	onvm_relpy *reply = (onvm_relpy*)responseBuffer;
	memset(reply, 0, sizeof(onvm_reply));


	reply->status = ONVM_REPLY_SUCCESS;
	reply->nr_seg = obj->nr_seg;
	reply->oid = obj->oid;
	memcpy(reply->segments, obj->segments, sizeof(segment_info)*MAX_SEGMENT_COUNT);
	return 0;
}

int MetaServer::Handle_OBJ_GET(char *recvBuffer, char *responseBuffer)
{
	onvm_request *request = (onvm_request_post_obj*)recvBuffer
	ms_onvm_object *obj = find_onvm_object(request->oid);
	onvm_reply *reply = (onvm_reply*)responseBuffer;
	memset(reply,0,sizeof(onvm_reply));
	obj2reply(obj,reply);
}


void obj2reply(ms_onvm_object *obj, char *rep){
	onvm_relpy *reply = (onvm_relpy*)rep;
	memcpy(reply->segments, obj->segments, MAX_SEGMENT_COUNT);
	reply->status= ONVM_REPLY_SUCCESS;
	reply->nr_seg = obj->nr_seg;
	reply->oid = obj->oid;
}

int MetaServer::Handle_OBJ_PUT(char *intput, char *output)
{
	segment_info *seg_info = get_segment_info(oid);
	//TODO:send segmentinfo to client
}

int MetaServer::Handle_Obj_Delete(char *input, char *output)
{
	onvm_request *request = (onvm_request_post_obj*)recvBuffer
	del_onvm_object(request->oid);

	onvm_reply *reply = (onvm_reply*)responseBuffer;
	memset(reply,0,sizeof(onvm_reply));
	reply->status=ONVM_REPLY_SUCCESS;
}
//ccy add end


/**
 * send: 远端通过rdma发送的信息体的起始地址或指针
 * 
 * 
*/
void RPCServer::ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID, uint16_t offset) {
	char reponseBuffer[CLIENT_MESSAGE_SIZE];
	uint64_t bufferRecvAddress = (uint64_t)send;
	GeneralReceiveBuffer *recv = (GeneralReceiveBuffer*)reponseBuffer;
	recv->taskID = send->taskID;
	recv->message = MESSAGE_RESPONSE;
	uint64_t size = send->sizeReceiveBuffer;
	if (send->message == MESSAGE_DISCONNECT) {
        //rdma->disconnect(send->sourceNodeID);
        return;
    } else if (send->message == MESSAGE_TEST) {
    	;
    } else if (send->message == MESSAGE_UPDATEMETA) {
    	/* Write unlock. */
    	// UpdateMetaSendBuffer *bufferSend = (UpdateMetaSendBuffer *)send;
    	// fs->unlockWriteHashItem(bufferSend->key, NodeID, bufferSend->offset);
    	return;
    } else if (send->message == MESSAGE_EXTENTREADEND) {
    	/* Read unlock */
    	// ExtentReadEndSendBuffer *bufferSend = (ExtentReadEndSendBuffer *)send;
    	// fs->unlockReadHashItem(bufferSend->key, NodeID, bufferSend->offset);
    	return;
	} else {
    	// fs->parseMessage((char*)send, reponseBuffer);
		switch(send->message){
			case MESSAGE_POST_OBJ:
		
			int ret =  Handle_Obj_Post(bufferRecvAddress, responseBuffer, NodeId);
			//重置responseBuffer，设置好结果，后面的逻辑会处理好返还给客户端的信息。如果是ret==0，说明成功了，再Handle_Obj_Post内部设置号这个responseBuffer
			if(0 != ret){
				onvm_relpy *recv = (onvm_relpy*)reponseBuffer;
				memset(responseBuffer, 0, CLIENT_MESSAGE_SIZE);
			   recv->status = ONVM_REPLY_SUCCESS
			   recv->
			}
	
			break;
			case MESSAGE_PUT_OBJ:
			int ret =  Handle_OBJ_PUT(bufferRecvAddress, responseBuffer);
			if(0 != ret){
				onvm_relpy *recv = (onvm_relpy*)reponseBuffer;
				memset(responseBuffer, 0, CLIENT_MESSAGE_SIZE);
			   recv->status = ONVM_REPLY_SUCCESS
			}
			break;
			case MESSAGE_DEL_OBJ:
			Handle_OBJ_DE
			int ret =  Handle_Obj_Delete(bufferRecvAddress, responseBuffer);
			if(0 != ret){
				onvm_relpy *recv = (onvm_relpy*)reponseBuffer;
				memset(responseBuffer, 0, CLIENT_MESSAGE_SIZE);
			   recv->status = ONVM_REPLY_SUCCESS
			}
			break;
			case MESSAGE_GET_OBJ:
			int ret =  Handle_OBJ_GET(bufferRecvAddress, responseBuffer);
			if(0 != ret){
				onvm_relpy *recv = (onvm_relpy*)reponseBuffer;
				memset(responseBuffer, 0, CLIENT_MESSAGE_SIZE);
			   recv->status = ONVM_REPLY_SUCCESS
			}
			break;
		}


    	// fs->recursivereaddir("/", 0);
		Debug::debugItem("Contract Receive Buffer, size = %d.", size);
		size -= ContractReceiveBuffer(send, recv);
    	if (send->message == MESSAGE_RAWREAD) {
    		ExtentReadSendBuffer *bufferSend = (ExtentReadSendBuffer *)send;
    		uint64_t *value = (uint64_t *)mem->getDataAddress();
    		// printf("rawread size = %d\n", (int)bufferSend->size);
    		*value = 1;
    		socket->RdmaWrite(NodeID, mem->getDataAddress(), 2 * 4096, bufferSend->size, -1, 1);
    	} else if (send->message == MESSAGE_RAWWRITE) {
    		ExtentWriteSendBuffer *bufferSend = (ExtentWriteSendBuffer *)send;
    		// printf("rawwrite size = %d\n", (int)bufferSend->size);
    		uint64_t *value = (uint64_t *)mem->getDataAddress();
    		*value = 0;
    		socket->RdmaRead(NodeID, mem->getDataAddress(), 2 * 4096, bufferSend->size, 1); // FIX ME.
    		while (*value == 0);
    	}
		Debug::debugItem("Copy Reply Data, size = %d.", size);
    	memcpy((void *)send, reponseBuffer, size);
		Debug::debugItem("Select Buffer.");
    	if (NodeID > 0 && NodeID <= ServerCount) {
			/* Recv Message From Other Server. */
			bufferRecvAddress = bufferRecvAddress - mm;
		} else if (NodeID > ServerCount) {
			/* Recv Message From Client. */
			bufferRecvAddress = 0;
		} 
		Debug::debugItem("send = %lx, recv = %lx", send, bufferRecvAddress);
    		socket->_RdmaBatchWrite(NodeID, (uint64_t)send, bufferRecvAddress, size, 0, 1);
		// socket->_RdmaBatchReceive(NodeID, mm, 0, 2);
		socket->RdmaReceive(NodeID, mm + NodeID * 4096, 0);
		// printf("process end\n");
    }
}

