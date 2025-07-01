#include "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer(){
    for(int i=0;i<BUFFER_CAPACITY;i++){
        metainfo[i].free = true;
        metainfo[i].blockNum = -1;
        metainfo[i].dirty = false;
        metainfo[i].timeStamp = -1;
    }
}

StaticBuffer::~StaticBuffer(){
    for(int i=0;i<BUFFER_CAPACITY;i++){
        if(!metainfo[i].free && metainfo[i].dirty){
            Disk::writeBlock(blocks[i],metainfo[i].blockNum);
        }
    }
}

int StaticBuffer::getFreeBuffer(int blockNum){
    if(blockNum < 0 || blockNum > DISK_BLOCKS)return E_OUTOFBOUND;

    for(int i=0;i<BUFFER_CAPACITY;i++)if(!metainfo[i].free)metainfo[i].timeStamp++;



    int bufferNum = -1;
    for(int i=0;i<BUFFER_CAPACITY;i++){
        if(metainfo[i].free){
            bufferNum = i;
            break;
        }
    }

    if(bufferNum == -1){
        int max = -1;
        for(int i=0;i<BUFFER_CAPACITY;i++){
            if(metainfo[i].timeStamp > max){
                bufferNum = i;
                max = metainfo[i].timeStamp;
            }
        }

        if(metainfo[bufferNum].dirty)Disk::writeBlock(blocks[bufferNum],metainfo[bufferNum].blockNum);
    }


    metainfo[bufferNum].free = false;
    metainfo[bufferNum].blockNum = blockNum;
    metainfo[bufferNum].dirty = false;
    metainfo[bufferNum].timeStamp = 0;

    return bufferNum;
}

int StaticBuffer::getBufferNum(int blockNum){
    if(blockNum < 0 || blockNum > DISK_BLOCKS)return E_OUTOFBOUND;

    for(int i=0;i<BUFFER_CAPACITY;i++){
        if(metainfo[i].blockNum == blockNum)return i;
    }

    return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
    int bufferId = StaticBuffer::getBufferNum(blockNum);
    if(bufferId == E_BLOCKNOTINBUFFER)return E_BLOCKNOTINBUFFER;

    if(bufferId == E_OUTOFBOUND)return E_OUTOFBOUND;

    metainfo[bufferId].dirty = true;
    return SUCCESS;
}
