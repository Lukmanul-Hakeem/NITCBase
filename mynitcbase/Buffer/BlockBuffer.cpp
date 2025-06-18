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

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char** buffer){
    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    if(bufferNum == E_BLOCKNOTINBUFFER){
        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
        if(bufferNum == E_OUTOFBOUND)return bufferNum;

        Disk::readBlock(StaticBuffer::blocks[bufferNum],this->blockNum);
    }

    *buffer = StaticBuffer::blocks[bufferNum];

    return SUCCESS;

}

