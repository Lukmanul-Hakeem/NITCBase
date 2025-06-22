#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTable::OpenRelTable(){

    for(int i=0;i<MAX_OPEN;i++){
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }
    
    // adding Relation catalog and Attribute catalog table to the cache because they have been used most, almost for everey query we use them

    RecBuffer relCatBlock(RELCAT_BLOCK);
    Attribute relCatAttribute[RELCAT_NO_ATTRS];

    relCatBlock.getRecord(relCatAttribute,RELCAT_SLOTNUM_FOR_RELCAT);

    RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatAttribute,&relCacheEntry.relCatEntry);

    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

    // allocate this on the heap because we want it to persist outside this function

    RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(struct RelCacheEntry));
    *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;


    // Attribute attrCatAttribute[ATTRCAT_NO_ATTRS];

    relCatBlock.getRecord(relCatAttribute,RELCAT_SLOTNUM_FOR_ATTRCAT);
    RelCacheEntry attrCatCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatAttribute,&attrCatCacheEntry.relCatEntry);
    attrCatCacheEntry.recId.block = RELCAT_BLOCK;
    attrCatCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
    RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(struct RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatCacheEntry;



    RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    struct AttrCacheEntry* head = nullptr;
    struct AttrCacheEntry* prev = nullptr;

    for(int i=0;i<6;i++){
        attrCatBlock.getRecord(attrCatRecord,i);
        struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(struct AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&attrCacheEntry->attrCatEntry);
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;
        attrCacheEntry->next = nullptr;
        
        if(head == nullptr)head = attrCacheEntry;

        if(prev != nullptr)prev->next = attrCacheEntry;

        prev = attrCacheEntry;
    }

    AttrCacheTable::attrCache[RELCAT_RELID] = head;

    head = nullptr;
    prev = nullptr;

    for(int i=6;i<12;i++){
        attrCatBlock.getRecord(attrCatRecord,i);
        struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(struct AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&attrCacheEntry->attrCatEntry);
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;
        attrCacheEntry->next = nullptr;

        if(head == nullptr)head = attrCacheEntry;

        if(prev != nullptr)prev->next = attrCacheEntry;

        prev = attrCacheEntry;

    }

    AttrCacheTable::attrCache[ATTRCAT_RELID] = head;

}



OpenRelTable::~OpenRelTable() {
  for(int i=0; i<MAX_OPEN; i++) {
    free(RelCacheTable::relCache[i]);

    AttrCacheEntry* prevAttrCacheEntry = nullptr;
    AttrCacheEntry* curAttrCacheEntry = AttrCacheTable::attrCache[i];

    while(curAttrCacheEntry) {
      prevAttrCacheEntry = curAttrCacheEntry;
      curAttrCacheEntry = curAttrCacheEntry->next;
      free(prevAttrCacheEntry);
    }
  }
}