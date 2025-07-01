#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

BlockBuffer::BlockBuffer(int blockNum){
    this->blockNum = blockNum;
}

RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum){}

int BlockBuffer::getHeader(struct HeadInfo* head){
    unsigned char *buffer;
    int ret = BlockBuffer::loadBlockAndGetBufferPtr(&buffer);

    if(ret != SUCCESS)return ret;

    memcpy(&head->blockType,buffer,4);
    memcpy(&head->pblock,buffer+4,4);
    memcpy(&head->lblock,buffer+8,4);
    memcpy(&head->rblock,buffer+12,4);
    memcpy(&head->numEntries,buffer+16,4);
    memcpy(&head->numAttrs,buffer+20,4);
    memcpy(&head->numSlots,buffer+24,4);
    memcpy(&head->reserved,buffer+28,4);

    return SUCCESS;
    
}


int RecBuffer::getRecord(union Attribute* rec,int slot){

    struct HeadInfo head;
    this->getHeader(&head);

    unsigned char *buffer;
    int ret = BlockBuffer::loadBlockAndGetBufferPtr(&buffer);
    if(ret != SUCCESS)return ret;

    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;
    int recordSize = attrCount * ATTR_SIZE;

    unsigned char* slotPointer = buffer + 32 + slotCount + (recordSize * slot);
    memcpy(rec,slotPointer,recordSize);

    return SUCCESS;
}


int RecBuffer::setRecord(Attribute* record,int slot){
    unsigned char* bufferptr;
    int ret = BlockBuffer::loadBlockAndGetBufferPtr(&bufferptr);
    if(ret != SUCCESS)return ret;

    HeadInfo header;
    BlockBuffer::getHeader(&header);
    int numAttrs = header.numAttrs;
    int slotCount = header.numSlots;

    if(slot < 0|| slot >= slotCount)return E_OUTOFBOUND;

    int offset = HEADER_SIZE + slotCount + (numAttrs*ATTR_SIZE*slot);

    memcpy(bufferptr + offset, record,ATTR_SIZE*numAttrs);

    StaticBuffer::setDirtyBit(this->blockNum);
    return SUCCESS;

}



int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char** buffer){
    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    if(bufferNum != E_BLOCKNOTINBUFFER){
        StaticBuffer::metainfo[bufferNum].timeStamp = 0;
        for(int i=0;i<BUFFER_CAPACITY;i++)if(bufferNum != i)StaticBuffer::metainfo[i].timeStamp++;
    }else{
        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
        if(bufferNum == E_OUTOFBOUND)return E_OUTOFBOUND;
        Disk::readBlock(StaticBuffer::blocks[bufferNum],this->blockNum);
    }

    *buffer = StaticBuffer::blocks[bufferNum];

    return SUCCESS;

}

int RecBuffer::getSlotMap(unsigned char*slotMap){
    unsigned char *buffer;
    int retVal = BlockBuffer::loadBlockAndGetBufferPtr(&buffer);

    if(retVal != SUCCESS)return retVal;

    struct HeadInfo headInfo;
    this->getHeader(&headInfo);

    int slotCount = headInfo.numSlots;
    unsigned char* slotMapBuffer = buffer + HEADER_SIZE;

    memcpy(slotMap, slotMapBuffer, slotCount);

    return SUCCESS;

}


int compareAttrs(Attribute attr1, Attribute attr2, int attrType){
    double diff = 0;
    if(attrType == STRING){
        diff = strcmp(attr1.sVal,attr2.sVal);
    }else diff = attr1.nVal - attr2.nVal;

    

    if(diff > 0)return 1;
    else if(diff == 0)return 0;
    else return -1;
}

