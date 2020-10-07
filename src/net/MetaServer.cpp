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
void MetaServer::add_onvm_object(struct ms_onvm_object *obj)
{
	object_map[obj->oid] = obj;
}

ms_onvm_object *MetaServer::find_onvm_object(uint16_t oid)
{
	ms_onvm_object *obj = object_map.find(oid);
	if(obj == NULL){
		Debug::notifyInfo("this object is not found!");
		return NULL;
	}
	return obj;
}

ms_onvm_object *MetaServer::alloc_onvm_object(unsigned char *s, uint16_t size)
{
	struct onvm_object *obj;
	obj = malloc(sizeof(*obj));
	if(obj){
		memset(obj, 0, sizeof(*obj));
		memcpy(obj->name, s, MAX_OBJ_NAME_LENGTH);
		//INIT_LIST_HEAD(&obj->next); 对比hotpot, 这个list有存在的必要吗？
	}
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

uint16_t MetaServer::assgin_one_node()
{

};

ms_segment_info *MetaServer::get_segment_info(uint16_t oid)
{
	ms_onvm_object *obj = object_map.find(oid);
	if(obj == NULL){
		Debug::notifyInfo("this object is not found!");
		return NULL;
	}
	return obj->pos_info;
}

int MetaServer::init_new_onvm_object(struct ms_onvm_object *obj)
{
	int index,  nr_established = 0;
	uint16_t seg_id, node_id, oid;

	for(index = 0; index < ceil(obj->size/SEGMENT_SIZE); index++){
		node_id = assgin_one_node();
		seg_id = assign_global_seg_id();
		oid = assign_global_oid();
		obj->oid = oid;
		if(!establish_node(obj, index, node_id, seg_id))
			nr_established++;
	}
}

int MetaServer::establish_node(struct ms_onvm_object *obj, int index; uint16_t node_id; uint16_t seg_id)
{
	struct onvm_request request;
	struct onvm_reply   reply;
	struct ms_segment_info *s;
	unsigned int size;

	request.message = MESSAGE_ALLOC_SEG_AT_DS;
	request.seg_id = seg_id;
	//size = sizeof(request.message) + sizeof(request.seg_id);
	//TODO:send alloc segment request to DS
	//TODO:socket->RdmaSend();
	if(reply.status != ONVM_REPLY_SUCCESS){
		Debug::notifyError("Alloc segment failed!");
		return -1;
	}
	s = &(obj->pos_info[index]);
	s->seg_id = seg_id;
	s->node_id = node_id;

	return 0;
}


void MetaServer::ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID)
{
	char receiveBuffer[CLIENT_MESSAGE_SIZE];
	uint64_t bufferRecv = (uint64_t)send;
	GeneralReceiveBuffer *recv = (GeneralReceiveBuffer*)receiveBuffer;
	recv->taskID = send->taskID;
	recv->message = MESSAGE_RESPONSE;
	uint64_t size = send->sizeReceiveBuffer;

	switch (send->message)
	{
	case MESSAGE_POST_OBJ:
		/* code */
		break;
	case MESSAGE_GET_OBJ:
		break;
	case MESSAGE_PUT_OBJ:
		break;
	case MESSAGE_DEL_OBJ:
	
		break;
	case MESSAGE_ALLOC_SEG_AT_DS：
		break;
	default:
		break;
	}


	if (send->message == MESSAGE_DISCONNECT) {
        //rdma->disconnect(send->sourceNodeID);
        return;
    }



	char receiveBuffer[CLIENT_MESSAGE_SIZE];
	uint64_t bufferRecv = (uint64_t)send;
	ObjectRecvBuffer *recv = (GeneralReceiveBuffer*)receiveBuffer;
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
		ParseMessage((char*)send, receiveBuffer);
		//TODO:
	}
}

void MetaServer::ParseMessage(char *bufferRequest, char *bufferResponse, uint16_t NodeID)
{
	ObjectSendBuffer *bufferObjSend;
	ObjectRecvBuffer *bufferObjRecv;
	
	ObjectSendBuffer *bufferObjSend = (ObjectSendBuffer *)bufferRequest; /* Send and request. */
    ObjectRecvBuffer *bufferObjRecv = (ObjectRecvBuffer *)bufferResponse; /* Receive and response. */
    //bufferGeneralReceive->message = MESSAGE_RESPONSE; /* Fill response message. */
	switch(bufferObjSend->message) {
		case MESSAGE_POST_OBJ{
			Handle_Obj_Post(bufferObjSend,bufferObjRecv,NodeID);
			break;
		}
		case MESSAGE_GET_OBJ{
			Handle_Obj_Get(bufferObjSend,bufferObjReceive,NodeID)
			break;
		}c
		case MESSAGE_PUT_OBJ{
			Handle_Obj_Put(bufferObjSend,bufferObjReceive,NodeID);
			break;
		}
		case MESSAGE_DEL_OBJ{
			Handle_Obj_Delete(bufferObjSend,bufferObjReceive,NodeID);
			break;
		}
	}
}

int MetaServer::Handle_Obj_Post(char *input, char *recv, uint16_t NodeID)
{
	int idx, nr_seg = 0;
	unsigned char *s;
	uint16_t size;
	struct segment_info *segment;
	struct ms_onvm_object *obj;
	struct onvm_request_post_obj *request;
	struct onvm_relpy *reply；

	request = (struct onvm_request_post_obj *)input;
	reply =(struct onvm_relpy *) output;
	
	s = request->name;
	size = request->size;
	obj = find_object(s);
    if(!obj)
    {
		if(objsize <= 0)
        {
			Debug::notifyInfo("POST OP Failed!")obj
        	return -1;
		}
    	obj = alloc_onvm_object(s, size);
		if(!obj)
		{
			reply->status = ONVM_POST_OBJ_FAIL;
			//*reply_len = sizeof(unsigned int);
			goto out;
		}
    	init_new_onvm_object(obj);
		add_onvm_object(obj);       
    }
	segment = &reply->base[0];
	for(idx = 0; idx < MAX_SEGMENT_COUNT; idx++){
		segment->seg_id = obj->pos_info[idx].seg_id;
		segment->node_id = obj->pos_info[idx].node_id;
		segment++;
		nr_seg++;
	}

}

ms_segment_info* MetaServer::Handle_OBJ_GET(uint16_t oid, char *intput, char *output)
{
	ms_segment_info *seg_info = get_segment_info(oid);
	//TODO:send segmentinfo to client
}

ms_segment_info* MetaServer::Handle_OBJ_PUT(uint16_t oid, char *intput, char *output)
{
	ms_segment_info *seg_info = get_segment_info(oid);
	//TODO:send segmentinfo to client
}

ms_segment_info *MetaServer::Handle_Obj_Delete(uint16_t oid, char *input, char *output)
{
	ms_segment_info *seg_info = get_segment_info(oid);
	//TODO:send segmentinfo to client
}
//ccy add end

void RPCServer::Worker(int id) {
	uint32_t tid = gettid();
	// gettimeofday(&startt, NULL);
	Debug::notifyInfo("Worker %d, tid = %d", id, tid);
	th2id[tid] = id;
	mem->setID(id);
	while (true) {
		RequestPoller(id);
	}
}

void RPCServer::RequestPoller(int id) {
	struct ibv_wc wc[1];
	uint16_t NodeID;
	uint16_t offset;
	int ret = 0, count = 0;
	uint64_t bufferRecv;
	// unsigned long diff;
	ret = socket->PollOnce(id, 1, wc);
	if (ret <= 0) {
		/*gettimeofday(&endd, NULL);
		diff = 1000000 * (endd.tv_sec - startt.tv_sec) + endd.tv_usec - startt.tv_usec;
		if (diff > 2000000) {
			printf("ID = %d, Polling, ret = %d\n", id, ret);
			diff = 0;
			gettimeofday(&startt, NULL);
			uint64_t bufferRecv = mem->getClientMessageAddress(2);
			ExtentWriteSendBuffer *send = (ExtentWriteSendBuffer *)bufferRecv;
			socket->RdmaReceive(2, mm + 2 * 4096, 0);
			socket->RdmaReceive(2, mm + 2 * 4096, 0);
			Debug::debugItem("Path = %s, size = %x, offset = %x", send->path, send->size, send->offset);
		}*/
		return;
	} else if (wc[0].opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
		NodeID = wc[0].imm_data >> 20;
		if (NodeID == 0XFFF) {
			/* Unlock request, process it directly. */
			// uint64_t hashAddress = wc[0].imm_data & 0x000FFFFF;
			// fs->unlockWriteHashItem(0, 0, hashAddress);
			return;
		}
		NodeID = (uint16_t)(wc[0].imm_data << 16 >> 16);
		offset = (uint16_t)(wc[0].imm_data >> 16);
		Debug::debugItem("NodeID = %d, offset = %d", NodeID, offset);
		count += 1;
		if (NodeID > 0 && NodeID <= ServerCount) {
			/* Recv Message From Other Server. */
			bufferRecv = mem->getServerRecvAddress(NodeID, offset);
		} else if (NodeID > ServerCount) {
			/* Recv Message From Client. */
			bufferRecv = mem->getClientMessageAddress(NodeID);
		}
		GeneralSendBuffer *send = (GeneralSendBuffer*)bufferRecv;
		switch (send->message) {
			case MESSAGE_TEST: {

			}
			default: {
				// if (id == 0 && UnlockWait == false && (send->message == MESSAGE_ADDMETATODIRECTORY || send->message == MESSAGE_REMOVEMETAFROMDIRECTORY)) {
				// 	/* 
				// 	* When process addmeta or remove meta, lock will be added without release,
				// 	* So we will wait until update meta arrives.
				// 	*/
				// 	UnlockWait = true;
				// 	ProcessRequest(send, NodeID, offset);
				// 	return;
				// } else if (id == 0 && send->message == MESSAGE_DOCOMMIT) {
				// 	UnlockWait = false;
				// 	ProcessRequest(send, NodeID, offset);
				// 	printf("a\n");
				// 	ProcessQueueRequest();
				// 	printf("b\n");
				// 	return;
				// }else if (id == 0 && UnlockWait == true) {
				// 	/* Just push the requests into the queue and return. */
				// 	RPCTask *task = (RPCTask *)malloc(sizeof(RPCTask));
				// 	task->send = (uint64_t)send;
				// 	task->NodeID = NodeID;
				// 	task->offset = offset;
				// 	tasks.push_back(task);
				// 	// printf("process docommit end.");
				// 	return;
				// }
				ProcessRequest(send, NodeID, offset);
				// printf("id = %d,end\n", id);
			}
		}
		
	}
}

void RPCServer::ProcessQueueRequest() {
	for (auto task = tasks.begin(); task != tasks.end(); ) {
		// printf("1\n");
		ProcessRequest((GeneralSendBuffer *)(*task)->send, (*task)->NodeID, (*task)->offset);
		free(*task);
		task = tasks.erase(task);
	}
	// printf("2\n");
}

void RPCServer::ProcessRequest(GeneralSendBuffer *send, uint16_t NodeID, uint16_t offset) {
	char receiveBuffer[CLIENT_MESSAGE_SIZE];
	uint64_t bufferRecv = (uint64_t)send;
	GeneralReceiveBuffer *recv = (GeneralReceiveBuffer*)receiveBuffer;
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
    	fs->parseMessage((char*)send, receiveBuffer);
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
    	memcpy((void *)send, receiveBuffer, size);
		Debug::debugItem("Select Buffer.");
    	if (NodeID > 0 && NodeID <= ServerCount) {
			/* Recv Message From Other Server. */
			bufferRecv = bufferRecv - mm;
		} else if (NodeID > ServerCount) {
			/* Recv Message From Client. */
			bufferRecv = 0;
		} 
		Debug::debugItem("send = %lx, recv = %lx", send, bufferRecv);
    		socket->_RdmaBatchWrite(NodeID, (uint64_t)send, bufferRecv, size, 0, 1);
		// socket->_RdmaBatchReceive(NodeID, mm, 0, 2);
		socket->RdmaReceive(NodeID, mm + NodeID * 4096, 0);
		// printf("process end\n");
    }
}

