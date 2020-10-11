#ifndef METAMESSAGE_HEADER
#define METAMESSAGE_HEADER
#include <global.h>

using namespace std;


#define MAX_SEGMENT_COUNT 1024   /*MAX count of sengment index in a object. */
#define MAX_OBJ_NAME_LENGTH  256        
#define SEGMENT_SIZE (4*1024*1024)

static uint16_t SEG_NO_COUNTER;
static uint16_t OBJ_NO_COUNTER;

//client发送给metaserver或者dataServer的
typedef struct : GeneralSendBuffer  {
    uint16_t objsize;
    uint16_t oid;//创建时用不到，其他操作会用
    unsigned char name[MAX_OBJ_NAME_LENGTH];
} onvm_request_post_obj;

//dataServer接收的信息
typedef struct : GeneralSendBuffer {
    uint16_t seg_id;
    //uint16_t segoff /* segment 在数据节点上的offset */
} SegmentCreateRequest;

typedef struct {
    ONVM_REPLY_STATUS status;
    uint16_t nr_seg; /*Number of Segs this object have */
    uint16_t oid;
    segment_info segments[MAX_SEGMENT_COUNT]; /*base of segment info array */
} onvm_relpy;

typedef struct {
    uint16_t seg_id;
    uint16_t node_id;
}segment_info;


typedef struct
{
	uint16_t oid;
	unsigned char name[MAX_OBJ_NAME_LENGTH];
	time_t timeLastModified;
	uint16_t size;//obj字节数
    uint16_t nr_seg;
	segment_info segments[MAX_SEGMENT_COUNT];
} ms_onvm_object;

#endif