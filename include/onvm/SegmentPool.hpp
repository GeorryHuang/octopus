#ifndef SEGMENT_POOL_HEADER
#define SEGMENT_POOL_HEADER

#include "MetaMessage.h"
#include <unordered_map>


/**
 * To generate global segment id. used only in MetaServer
 * 
 * 
*/
class SegmentIdGenerator{
private:
    uint16_t currentId;
public:
    SegmentIdGenerator(){
        currentId = 0;
    }
    uint16_t nextId(){
        return ++currentId;
    }
};

/**
 * A SegmentInfo obj describe where the Segment in(nodeId) and its segmentId
 * Only used in MetaServer.
 * 
*/
class SegmentInfo{
public:
    SegmentInfo(uint16_t nid, uint16_t seg_id){
        this->nodeId = nid;
        this->segmentId = seg_id;
    }
    uint16_t nodeId;
    uint16_t segmentId;
};


class Segment
{
private:
    uint16_t segment_id;
    char *segment_content;

public:
    Segment(uint16_t seg_id)
    {
        this->segment_id = seg_id;
        this->segment_content = (char *)malloc(MAX_SEGMENT_COUNT);
    }
    ~Segment()
    {
        free(segment_content);
    }
};

class SegmentPool
{
private:
    unordered_map<uint16_t, Segment *> segment_map;

public:
    /**
     * @return null if already exist.
     * 
    */
    Segment *alloc_segment(uint16_t segment_id)
    {
        unordered_map<uint16_t, Segment *>::iterator iter;
        iter = segment_map.find(segment_id);
        //If segment already exist!
        if (iter != segment_map.end())
        {
            return NULL;
        }
        return iter->second;
    }

    /**
     * 
     * @return 0 success, if not 0, the segment not exist!
    */
    int release_segment(uint16_t segment_id)
    {
        unordered_map<uint16_t, Segment *>::iterator iter;
        iter = segment_map.find(segment_id);
        //If segment already exist!
        if (iter == segment_map.end())
        {
            return -1;
        }
        Segment *segment_to_release = iter->second;
        segment_map.erase(segment_id);
        delete segment_to_release;
        return 0;
    }

    /**
     * 
     * 
     * @return NULL if not exist.
    */
    Segment *get_segment(uint16_t segment_id, Segment *segment)
    {
        unordered_map<uint16_t, Segment *>::iterator iter;
        iter = segment_map.find(segment_id);
        //If segment already exist!
        if (iter == segment_map.end())
        {
            return NULL;
        }
        return iter->second;
    }
    SegmentPool()
    {
    }
    ~SegmentPool()
    {
        for(auto iter : this->segment_map){
            if(iter!=segment_map.end()){
               delete iter->second; 
            }
        }
    }
};

#endif
