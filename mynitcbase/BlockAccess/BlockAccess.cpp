#include "BlockAccess.h"
#include <stdio.h>

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

int BlockAccess::renameRelation(char* oldRelName,char* newRelName){

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute attrVal;
    strcpy(attrVal.sVal,newRelName);
    RecId newRelationId = BlockAccess::linearSearch(RELCAT_RELID,(char*)RELCAT_ATTR_RELNAME,attrVal,EQ);
    if(newRelationId.block != -1 && newRelationId.slot != -1)return E_RELEXIST;

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    strcpy(attrVal.sVal,oldRelName);
    RecId oldRelationId = BlockAccess::linearSearch(RELCAT_RELID,(char*)RELCAT_ATTR_RELNAME,attrVal,EQ);

    if(oldRelationId.block == -1 && oldRelationId.slot == -1)return E_RELNOTEXIST;


    RecBuffer relCatalog(oldRelationId.block); // i see a chance for error here.
    Attribute record[RELCAT_NO_ATTRS];
    relCatalog.getRecord(record,oldRelationId.slot);
    strcpy(record[RELCAT_REL_NAME_INDEX].sVal,newRelName);
    relCatalog.setRecord(record,oldRelationId.slot);

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    int numAttr = record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    RecId searchIndex;

    for(int i=0;i<numAttr;i++){
        searchIndex = BlockAccess::linearSearch(ATTRCAT_RELID,(char*)ATTRCAT_ATTR_RELNAME,attrVal,EQ);
        RecBuffer currentBlock(searchIndex.block);
        currentBlock.getRecord(record,searchIndex.slot);
        strcpy(record[ATTRCAT_REL_NAME_INDEX].sVal,newRelName);
        currentBlock.setRecord(record,searchIndex.slot);
    }

    return SUCCESS;


}

int BlockAccess::renameAttribute(char* relName,char* oldAttrName,char* newAttrName){

    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute attrVal;
    strcpy(attrVal.sVal,relName);
    RecId relCatalog = BlockAccess::linearSearch(RELCAT_RELID, (char*)RELCAT_ATTR_RELNAME, attrVal, EQ);

    if(relCatalog.block == -1 && relCatalog.slot == -1)return E_RELNOTEXIST;


    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    RecId linearSearchIndex;
    Attribute record[ATTRCAT_NO_ATTRS];
    RecId attrToRenameRecId;
    attrToRenameRecId.block = -1;
    attrToRenameRecId.slot = -1;
    while(true){
        linearSearchIndex = BlockAccess::linearSearch(ATTRCAT_RELID,(char*)ATTRCAT_ATTR_RELNAME,attrVal,EQ);
        if(linearSearchIndex.block == -1 || linearSearchIndex.slot == -1)break;

        RecBuffer attrCatalog(linearSearchIndex.block);
        attrCatalog.getRecord(record,linearSearchIndex.slot);

        if(strcmp(record[ATTRCAT_ATTR_NAME_INDEX].sVal,newAttrName) == 0)return E_ATTREXIST;

        if(strcmp(record[ATTRCAT_ATTR_NAME_INDEX].sVal,oldAttrName) == 0){
            attrToRenameRecId.block = linearSearchIndex.block;
            attrToRenameRecId.slot = linearSearchIndex.slot;
        }
    }

    if(attrToRenameRecId.block == -1 && attrToRenameRecId.slot == -1)return E_ATTRNOTEXIST;

    RecBuffer updateBlock(attrToRenameRecId.block);
    updateBlock.getRecord(record,attrToRenameRecId.slot);
    strcpy(record[ATTRCAT_ATTR_NAME_INDEX].sVal, newAttrName);
    updateBlock.setRecord(record, attrToRenameRecId.slot);

    return SUCCESS;
}
