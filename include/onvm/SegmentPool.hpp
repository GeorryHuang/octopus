#ifndef SEGMENT_POOL_HEADER
#define SEGMENT_POOL_HEADER

#include "MetaMessage.h"
#include <unordered_map>

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
        
    }
};

#endif
