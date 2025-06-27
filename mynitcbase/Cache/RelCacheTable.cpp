#include "RelCacheTable.h"

#include <cstring>


RelCacheEntry* RelCacheTable::relCache[MAX_OPEN];

int RelCacheTable::getRelCatEntry(int recIndex, RelCatEntry* relCacheBuffer){
    if(recIndex < 0 || recIndex >= MAX_OPEN)return E_OUTOFBOUND;
    if(relCache[recIndex] == nullptr)return E_RELNOTOPEN;

    *relCacheBuffer = relCache[recIndex]->relCatEntry;
    return SUCCESS;
}


void RelCacheTable::recordToRelCatEntry(union Attribute* record,RelCatEntry* relCatEntry){
    strcpy(relCatEntry->relName,record[RELCAT_REL_NAME_INDEX].sVal);
    relCatEntry->firstBlk = record[RELCAT_FIRST_BLOCK_INDEX].nVal;
    relCatEntry->lastBlk = record[RELCAT_LAST_BLOCK_INDEX].nVal;
    relCatEntry->numAttrs = record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    relCatEntry->numRecs = record[RELCAT_NO_RECORDS_INDEX].nVal;
    relCatEntry->numSlotsPerBlk = record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal;

}

int RelCacheTable::getSearchIndex(int relId,RecId*searchIndex){
    if(relId < 0 && relId > MAX_OPEN)return E_OUTOFBOUND;
    if(relCache[relId] == nullptr)return E_RELNOTOPEN;
    *searchIndex = relCache[relId]->searchIndex;
    return SUCCESS;
}

int RelCacheTable::setSearchIndex(int relId,RecId*searchIndex){
    if(relId < 0 && relId > MAX_OPEN)return E_OUTOFBOUND;
    if(relCache[relId] == nullptr)return E_RELNOTOPEN;
    relCache[relId]->searchIndex = *searchIndex;
    return SUCCESS;
}

int RelCacheTable::resetSearchIndex(int relId){
    RecId reset = {-1,-1};
    RelCacheTable::setSearchIndex(relId,&reset);
    return SUCCESS;
}
