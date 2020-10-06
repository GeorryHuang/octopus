#ifndef METAMESSAGE_HEADER
#define METAMESSAGE_HEADER
#include <global.h>

using namespace std;


#define MAX_SEGMENT_COUNT 1024   /*MAX count of sengment index in a object. */
#define MAX_OBJ_NAME_LENGTH  256        
#define SEGMENT_SIZE (4*1024*1024)

static uint16_t SEG_NO_COUNTER;
static uint16_t OBJ_NO_COUNTER;

struct onvm_request_post_obj{
    Message message;
    uint16_t objsize;
    unsigned char name[MAX_OBJ_NAME_LENGTH];
};
struct onvm_request{
    Message message;
    uint16_t seg_id;
    //uint16_t segoff /* segment 在数据节点上的offset */
};

struct onvm_relpy{
    ONVM_REPLY_STATUS status;
    uint16_t nr_seg; /*Number of Segs this object have */
    union{
        struct segment_info base[0]; /*base of segment info array */
        char   data[0;]
    };
};

struct segment_info{
    uint16_t seg_id;
    uint16_t node_id;
};

#endif