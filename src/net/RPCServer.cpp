#include "RPCServer.hpp"
// __thread struct  timeval startt, endd;
RPCServer::RPCServer(int _cqSize, bool isMetaServer) :cqSize(_cqSize), {	
	mm = 0;
	UnlockWait = false;
	mem = new MemoryManager(mm, conf->getServerCount(), 2);
	mm = mem->getDmfsBaseAddress();//共享内存起始地址
	Debug::notifyInfo("DmfsBaseAddress = %lx, DmfsTotalSize = %lx",
		mem->getDmfsBaseAddress(), mem->getDmfsTotalSize());
	ServerCount = conf->getServerCount();
	socket = new RdmaSocket(cqSize, mm, mem->getDmfsTotalSize(), conf, true, 0);
	client = new RPCClient(conf, socket, mem, (uint64_t)mm);
	// tx = new TxManager(mem->getLocalLogAddress(), mem->getDistributedLogAddress());
	socket->RdmaListen();
	/* Constructor of file system. */
	fs = new FileSystem((char *)mem->getMetadataBaseAddress(),
              (char *)mem->getDataAddress(),  
              1024 * 20,//最大文件数
              1024 * 30,//最大目录数
              2000,
              conf->getServerCount(),    
              socket->getNodeID());//最后一个参数是本server节点id号
	fs->rootInitialize(socket->getNodeID());
	wk = new thread[cqSize]();
	for (int i = 0; i < cqSize; i++)
		wk[i] = thread(&RPCServer::Worker, this, i);
}
RPCServer::~RPCServer() {
	Debug::notifyInfo("Stop RPCServer.");
	delete conf;
	for (int i = 0; i < cqSize; i++) {
		wk[i].detach();
	}
	delete mem;
	delete wk;
	delete socket;
	// delete tx;
	Debug::notifyInfo("RPCServer is closed successfully.");
}

RdmaSocket* RPCServer::getRdmaSocketInstance() {
	return socket;
}

MemoryManager* RPCServer::getMemoryManagerInstance() {
	return mem;
}

RPCClient* RPCServer::getRPCClientInstance() {
	return client;
}

TxManager* RPCServer::getTxManagerInstance() {
	return tx;
}

/**
 * RPCServer启动后，创建多个线程，每个线程运行一个worker
 * id: 线程的编号，0～n
 * 
*/
void RPCServer::Worker(int id) {
	uint32_t tid = gettid();
	// gettimeofday(&startt, NULL);
	Debug::notifyInfo("Worker %d, tid = %d", id, tid);
	th2id[tid] = id;
	//除了RPCServer之外，MemoryManager也维护了一个tid到id的映射关系map
	mem->setID(id);
	while (true) {
		RequestPoller(id);
	}
}


/**
 * 判断一个ibv_wc是不是一个Unlock Request，如果是，处理对应逻辑并返回true，否则返回false
 * （这个函数截取自作者原来的RequestPoller片段,目前看来他自己注释掉了处理逻辑，只剩下一个壳）
 * Author：Huang Zhuoyue
*/
bool RPCServer::unlockRequest(struct ibv_wc *wc){
	NodeID = wc.imm_data >> 20;
		if (NodeID == 0XFFF) {
			/* Unlock request, process it directly. */
			// uint64_t hashAddress = wc[0].imm_data & 0x000FFFFF;
			// fs->unlockWriteHashItem(0, 0, hashAddress);
			return true;
		}
	return false;
}



/**
 * 根据wc，取出接收缓冲区的地址（因为需要判断是client发送）
 * 
 * 
 * 
*/
uint64_t getBufferRecvAddress(struct ibv_wc *wc){
		NodeID = (uint16_t)(wc.imm_data << 16 >> 16);
		offset = (uint16_t)(wc.imm_data >> 16);
		Debug::debugItem("NodeID = %d, offset = %d", NodeID, offset);
		//这个count并未发现使用的地方，是作者留下的，先注释了
		// count += 1;
		//根据NodeId，来判断发送信息的是server还是client
		if (NodeID > 0 && NodeID <= ServerCount) {
			/* Recv Message From Other Server. */
			return mem->getServerRecvAddress(NodeID, offset);
		} else if (NodeID > ServerCount) {
			/* Recv Message From Client. */
			return mem->getClientMessageAddress(NodeID);
		}
		//作者并没有处理NodeId的else。
		return 0;
}




/**
 * 每一个RDMA监听线程都会持续运行的方法
 * 
 * 
*/
void RPCServer::RequestPoller(int id) {
	struct ibv_wc wc[1];
	uint16_t NodeID;
	uint16_t offset;
	
	int ret = 0
	//这个count并未发现使用的地方，是作者留下的，先注释了
	// int count = 0;
	//远端的接收缓冲区，就是远端发送信息。
	uint64_t bufferRecvAddress;
	// unsigned long diff;
	//获取一个wc(Work completion)，就是rdma work的完成信息
	ret = socket->PollOnce(id, 1, wc);
	//如果是0,说明有异常
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
	} else if (wc[0].opcode == IBV_WC_RECV_RDMA_WITH_IMM) {//这里只处理IBV_WC_RECV_RDMA_WITH_IMM，如果是其他的opcode，就会忽略掉
		//如果是unlockRequest，直接处理完毕
		if(unlockRequest(wc[0])){
			return;
		}

		//得到wc发送完成后的缓冲区64位地址
		bufferRecvAddress = getBufferRecvAddress(wc[0]);
		//照目前逻辑来看，这里是为了复用内存，所以在接收完数据后，直接将接收的缓冲内存当作发送buffer使用。
		GeneralSendBuffer *send = (GeneralSendBuffer*)bufferRecvAddress;
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

/*
 * 1.在MemoryManager内的内存：
 * GeneralSendBuffer * send = |AAAAAAAAAAAAAAAAAAA|
 * 
 * 2.在函数栈内的内存:
 * char receiveBuffer[4096] = |BBBBBBBBB|
 * 
 * 3. 解析|AAAAAAAAAAAAAAAAAAA|中的MESSAGE_TYPE和内容，做出响应处理，并把response RPC构造|BBBBBBBBBB|
 * 
 * 4. 将|BBBBBB|覆盖到|AAAAAAAAAAAAAA|中，此时GeneralSendBuffer * send = |BBBBBBBBAAAAAAAAA|
 * 
 * 5. 计算|BBBBB|AAAAAAAAAAAAAAAAAAAA|中的无效部分，得到正确的size
 *       | size|ContractReceiveBuffer| 
 * 
 * 6. 将send指向的|BBBBBAAAAAAAAAA|部分的BBBBB发送回去
 * 
 * 
 * send: RPCServer从远端接收到消息的buffer起始地址
 * NodeId：远端的id
 * offset：远端发送的offset信息
 * 
 */
void RPCServer::ProcessRequest(GeneralSendBuffer * send, uint16_t NodeID, uint16_t offset) {
	//构建4096字节大小的buffer
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
		//这是octopus的文件系统处理逻辑
    	fs->parseMessage((char*)send, receiveBuffer);
    	// fs->recursivereaddir("/", 0);
		Debug::debugItem("Contract Receive Buffer, size = %d.", size);
		/* 这个函数会返回一个i64,紧接着size会减掉这个值，从推断上来看，因为一开始size来自的是send->sizeReceiveBuffer。所以可以
		断定，是recv这个buffer填好内容后，跟原来的send相比，有一部分是不需要 */
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
		//将返回给发送端的消息体拷贝给send。
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
		//返回消息体给发送端。
    		socket->_RdmaBatchWrite(NodeID, (uint64_t)send, bufferRecv, size, 0, 1);
		// socket->_RdmaBatchReceive(NodeID, mm, 0, 2);
		//为什么这里会有receive，从接收长度为0来看，应该是进行一次ack？再发送response后，进行一次长度为0的挥手？
		socket->RdmaReceive(NodeID, mm + NodeID * 4096, 0);
		// printf("process end\n");
    }
}

int RPCServer::getIDbyTID() {
	uint32_t tid = gettid();
	return th2id[tid];
}


/**
 
 * 
 * 
 * 
*/
uint64_t RPCServer::ContractReceiveBuffer(GeneralSendBuffer *send, GeneralReceiveBuffer *recv) {
	uint64_t length;
	switch (send->message) {
		case MESSAGE_GETATTR: {
			GetAttributeReceiveBuffer *bufferRecv = 
			(GetAttributeReceiveBuffer *)recv;
			if (bufferRecv->attribute.count >= 0 && bufferRecv->attribute.count < MAX_FILE_EXTENT_COUNT)
				length = (MAX_FILE_EXTENT_COUNT - bufferRecv->attribute.count) * sizeof(FileMetaTuple);
			else 
				length = sizeof(FileMetaTuple) * MAX_FILE_EXTENT_COUNT;
			break;
		}
		case MESSAGE_READDIR: {
			ReadDirectoryReceiveBuffer *bufferRecv = 
			(ReadDirectoryReceiveBuffer *)recv;
			if (bufferRecv->list.count >= 0 && bufferRecv->list.count <= MAX_DIRECTORY_COUNT)
				length = (MAX_DIRECTORY_COUNT - bufferRecv->list.count) * sizeof(DirectoryMetaTuple);
			else 
				length = MAX_DIRECTORY_COUNT * sizeof(DirectoryMetaTuple);
			break;
		}
		case MESSAGE_EXTENTREAD: {
			ExtentReadReceiveBuffer *bufferRecv = 
			(ExtentReadReceiveBuffer *)recv;
			if (bufferRecv->fpi.len >= 0 && bufferRecv->fpi.len <= MAX_MESSAGE_BLOCK_COUNT)
				length = (MAX_MESSAGE_BLOCK_COUNT - bufferRecv->fpi.len) * sizeof(file_pos_tuple);
			else 
				length = MAX_MESSAGE_BLOCK_COUNT * sizeof(file_pos_tuple);
			break;
		}
		case MESSAGE_EXTENTWRITE: {
			ExtentWriteReceiveBuffer *bufferRecv = 
			(ExtentWriteReceiveBuffer *)recv;
			if (bufferRecv->fpi.len >= 0 && bufferRecv->fpi.len <= MAX_MESSAGE_BLOCK_COUNT)
				length = (MAX_MESSAGE_BLOCK_COUNT - bufferRecv->fpi.len) * sizeof(file_pos_tuple);
			else 
				length = MAX_MESSAGE_BLOCK_COUNT * sizeof(file_pos_tuple);
			break;
		}
		case MESSAGE_READDIRECTORYMETA: {
			ReadDirectoryMetaReceiveBuffer *bufferRecv = 
			(ReadDirectoryMetaReceiveBuffer *)recv;
			if (bufferRecv->meta.count >= 0 && bufferRecv->meta.count <= MAX_DIRECTORY_COUNT)
				length = (MAX_DIRECTORY_COUNT - bufferRecv->meta.count) * sizeof(DirectoryMetaTuple);
			else 
				length = MAX_DIRECTORY_COUNT * sizeof(DirectoryMetaTuple);
			break;
		}
		default: {
			length = 0;
			break;
		}
	}	
	// printf("contract length = %d", (int)length);
	return length;
}
