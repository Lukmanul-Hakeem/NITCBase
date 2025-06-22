#include "AttrCacheTable.h"

#include <cstring>

AttrCacheEntry* AttrCacheTable::attrCache[MAX_OPEN];

int AttrCacheTable::getAttrCatEntry(int recId,int offset,AttrCatEntry* attrBuffer){
    if(recId<0 || recId>MAX_OPEN)return E_OUTOFBOUND;
    if(attrCache[recId] == nullptr)return E_RELNOTOPEN;

   for(AttrCacheEntry* attr = attrCache[recId];attr != nullptr;attr = attr->next){
        if(attr->attrCatEntry.offset == offset){
            *attrBuffer = attr->attrCatEntry;
            return SUCCESS;
        }
   }

   return E_ATTRNOTEXIST;
}

void AttrCacheTable::recordToAttrCatEntry(union Attribute* attr,AttrCatEntry*attrBuffer){
    strcpy(attrBuffer->relName,attr[ATTRCAT_REL_NAME_INDEX].sVal);
    strcpy(attrBuffer->attrName,attr[ATTRCAT_ATTR_NAME_INDEX].sVal);
    attrBuffer->offset = attr[ATTRCAT_OFFSET_INDEX].nVal;
    attrBuffer->attrType = attr[ATTRCAT_ATTR_TYPE_INDEX].nVal;
    attrBuffer->rootBlock = attr[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
}
