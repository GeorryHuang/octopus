#ifndef OBJECT_POOL_HEADER
#define OBJECT_POOL_HEADER

#include "MetaMessage.h"
#include <unordered_map>
#include <list>

#include "SegmentPool.hpp"

class NVMObject
{
private:
    uint16_t object_id;
    string object_name;
    list<SegmentInfo> segmentInfoList;

public:
    NVMObject(uint16_t obj_id, string name)
    {
        this->object_id = obj_id;
        this->object_name = name;
    }

    void appendSegment(SegmentInfo segmentInfo)
    {
        segmentInfoList.push_back(segmentInfo);
    }

    uint16_t getSegmentsCount(){
        return segmentInfoList.size();
    }

    uint16_t getObjectId(){
        return this->object_id;
    }
    

    ~NVMObject()
    {
    }
};

class NVMObjectPool
{
private:
    long object_id_index; //FIXME:用于分配id，目前先用这种办法
    unordered_map<string, uint16_t> name2id_map;
    unordered_map<uint16_t, NVMObject *> object_map;

    bool is_object_exist(string name)
    {
        auto iter = name2id_map.find(name);
        return iter != name2id_map.end();
    }

    bool is_object_exist(uint16_t obj_id)
    {
        auto iter = object_map.find(obj_id);
        return iter != object_map.end();
    }

public:
    /**
     * @return null if already exist.
     * 
    */
    NVMObject *newObject(string name)
    {
        if (!is_object_exist(name))
        {
            this->object_id_index++;
            name2id_map.insert(pair<string, uint16_t>(name, this->object_id_index));
            NVMObject *obj = new NVMObject(this->object_id_index, name);
            object_map.insert(pair<uint16_t, NVMObject *>(this->object_id_index, obj));
            return obj;
        }
        else
        {
            //name already exist!
            return NULL;
        }
    }
    /**
     * 
     * 
     * @return NULL if not exist.
    */
    NVMObject *getObject(string name)
    {
        auto iter = name2id_map.find(name);
        if (iter == name2id_map.end())
        {
            return NULL;
        }
        else
        {
            return getObject(iter->second);
        }
    }

    NVMObject *getObject(uint16_t obj_id)
    {
        auto iter = object_map.find(obj_id);
        if (iter == object_map.end())
        {
            return NULL;
        }
        else
        {
            return iter->second;
        }
    }

    NVMObjectPool()
    {
        this->object_id_index = 0;
    }
    ~NVMObjectPool()
    {
        for (auto iter = object_map.begin(); iter != object_map.end(); iter++)
        {
            delete iter->second;
        }
        object_map.clear();
    }
};

#endif
