#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

BlockBuffer::BlockBuffer(int blockNum){
    this->blockNum = blockNum;
}

BlockBuffer::BlockBuffer(char blockType){
    int type;
    if(blockType == 'R')type = REC;
    else if(blockType == 'I')type = IND_INTERNAL;
    else if(blockType == 'L')type = IND_LEAF;

    this->blockNum = this->getFreeBlock(type);
}

int BlockBuffer::getFreeBlock(int blockType){
    int blockNum = -1;
    for(int i=0;i<DISK_SIZE;i++){
        if(StaticBuffer::blockAllocMap[i] == UNUSED_BLK){
            blockNum = i;
            break;
        }
    }

    if(blockNum == -1)return E_DISKFULL;

    int bufferNum = StaticBuffer::getFreeBuffer(blockNum);
    this->blockNum = blockNum;
    struct HeadInfo head;
    // header.blockType = blockType;
    head.pblock = -1;
    head.lblock = -1;
    head.rblock = -1;
    head.numAttrs = 0;
    head.numEntries = 0;
    head.numSlots = 0;

    this->setHeader(&head);
    this->setBlockType(blockType);

    return blockNum;
}

int BlockBuffer::setHeader(struct HeadInfo *head){
    unsigned char* bufferPtr;
    int ret = this->loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret != SUCCESS)return ret;

    struct HeadInfo* bufferHeader = (struct HeadInfo*)bufferPtr;
    bufferHeader->blockType = head->blockType;
    bufferHeader->lblock = head->lblock;
    bufferHeader->numAttrs = head->numAttrs;
    bufferHeader->numEntries = head->numEntries;
    bufferHeader->numSlots = head->numSlots;
    bufferHeader->pblock = head->pblock;
    bufferHeader->rblock = head->rblock;

    int setDirtyRes = StaticBuffer::setDirtyBit(this->blockNum);
    if(setDirtyRes != SUCCESS)return setDirtyRes;

    return SUCCESS;
}

int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;
    int loadRet = this->loadBlockAndGetBufferPtr(&bufferPtr);

    if(loadRet != SUCCESS)return loadRet;

    // int32_t* type = (int32_t*)bufferPtr;
    // *type = blockType;

    *(int32_t*)bufferPtr = blockType;
    StaticBuffer::blockAllocMap[this->blockNum] = blockType;

    int retDirty = StaticBuffer::setDirtyBit(this->blockNum);

    if(retDirty != SUCCESS)return retDirty;

    return SUCCESS;

}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = BlockBuffer::loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret != SUCCESS)return ret;

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    // get the header of the block using the getHeader() function
    HeadInfo header;
    BlockBuffer::getHeader(&header);

    int numSlots = header.numSlots;
    unsigned char* newSlotMap = bufferPtr + HEADER_SIZE;

    memcpy(newSlotMap, slotMap, numSlots);

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`

    // update dirty bit using StaticBuffer::setDirtyBit
    ret = StaticBuffer::setDirtyBit(this->blockNum);
    // if setDirtyBit failed, return the value returned by the call
    if(ret != SUCCESS)return ret;

    return SUCCESS;
}

int BlockBuffer::getBlockNum(){
    return this->blockNum;
    //return corresponding block number.
}

RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum){}
RecBuffer::RecBuffer() : BlockBuffer('R'){}

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
