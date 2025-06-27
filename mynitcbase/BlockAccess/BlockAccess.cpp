#include "BlockAccess.h"

#include <cstring>
RecId BlockAccess::linearSearch(int relId,char* attrName,union Attribute attr,int op){
    RecId searchIndex;
    int ret = RelCacheTable::getSearchIndex(relId,&searchIndex);

    int block;
    int slot;

    if(searchIndex.block == -1 && searchIndex.slot == -1){
        RelCatEntry relCatEntry;
        RelCacheTable::getRelCatEntry(relId,&relCatEntry);

        block = relCatEntry.firstBlk;
        slot = 0;
    }else{
        block = searchIndex.block;
        slot = searchIndex.slot + 1;
    }

    while(block != -1){
        RecBuffer currentBlock(block);
         
        HeadInfo headInfo;
        currentBlock.getHeader(&headInfo);

        unsigned char slotMap[headInfo.numSlots];
        currentBlock.getSlotMap(slotMap);

        if(headInfo.numSlots <= slot){
            block = headInfo.rblock;
            slot = 0;
            continue;
        }

        if(slotMap[slot] == SLOT_UNOCCUPIED){
            slot++;
            continue;
        }

        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

        Attribute currRecordAttr[headInfo.numAttrs];
        currentBlock.getRecord(currRecordAttr,slot);

        Attribute searchingAttr = currRecordAttr[attrCatEntry.offset];


        int cmpVal = compareAttrs(searchingAttr,attr,attrCatEntry.attrType);

        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {


            RecId foundRecId = {block, slot};
            RelCacheTable::setSearchIndex(relId, &foundRecId);
            return foundRecId;

        }

        slot++;

        

    }

    return RecId{-1, -1};

}
