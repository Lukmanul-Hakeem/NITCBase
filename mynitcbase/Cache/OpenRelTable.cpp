#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable(){

    for(int i=0;i<MAX_OPEN;i++){
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
        tableMetaInfo[i].free = true;
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


    tableMetaInfo[RELCAT_RELID].free = false;
    strcpy(tableMetaInfo[RELCAT_RELID].relName , RELCAT_RELNAME);

    tableMetaInfo[ATTRCAT_RELID].free = false;
    strcpy(tableMetaInfo[ATTRCAT_RELID].relName , ATTRCAT_RELNAME);

}



OpenRelTable::~OpenRelTable() {

  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }


  if(RelCacheTable::relCache[ATTRCAT_RELID]->dirty){
    RelCacheEntry* attrCache = RelCacheTable::relCache[ATTRCAT_RELID]; 
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    // RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&attrCat);
    
    RelCacheTable::relCatEntryToRecord(&(attrCache->relCatEntry),relCatRecord);
    RecBuffer relCat(attrCache->recId.block);
    relCat.setRecord(relCatRecord,attrCache->recId.slot);
  }

  if(RelCacheTable::relCache[RELCAT_RELID]->dirty){
    RelCacheEntry* relCat = RelCacheTable::relCache[RELCAT_RELID]; 
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    // RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&attrCat);
    
    RelCacheTable::relCatEntryToRecord(&(relCat->relCatEntry),relCatRecord);
    RecBuffer relCatalog(relCat->recId.block);
    relCatalog.setRecord(relCatRecord,relCat->recId.slot);
  }

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


int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
  for(int i=0;i<MAX_OPEN;i++){
    if(strcmp(relName,tableMetaInfo[i].relName) == 0 && !tableMetaInfo[i].free)return i;
  }
  return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry(){
  for(int i=2;i<MAX_OPEN;i++){
    // printf("%d\n",tableMetaInfo[i].free);
    if(tableMetaInfo[i].free)return i;
  }
  return E_CACHEFULL;
}




int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  int existingId = getRelId(relName);
  if(existingId != E_RELNOTOPEN)
    return existingId;

  int relId = getFreeOpenRelTableEntry();
  if(relId == E_CACHEFULL)
    return E_CACHEFULL;

  /****** Setting up Relation Cache entry for the relation ******/

  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  Attribute attrVal;
  strcpy(attrVal.sVal, relName);
  RecId relcatId = BlockAccess::linearSearch(RELCAT_RELID, (char*)RELCAT_ATTR_RELNAME, attrVal, EQ);

  if(relcatId.block == -1 && relcatId.slot == -1)
    return E_RELNOTEXIST;
    
  RecBuffer recBuffer(relcatId.block);
  Attribute record[RELCAT_NO_ATTRS];
  RelCacheEntry relCacheEntry;
  recBuffer.getRecord(record, relcatId.slot);
  RelCacheTable::recordToRelCatEntry(record, &(relCacheEntry.relCatEntry));
  relCacheEntry.recId.block = relcatId.block;
  relCacheEntry.recId.slot = relcatId.slot;

  RelCacheTable::relCache[relId] = (struct RelCacheEntry*)malloc(sizeof(struct RelCacheEntry));
  *RelCacheTable::relCache[relId] = relCacheEntry;

  /****** Setting up Attribute Cache entry for the relation ******/

  AttrCacheEntry* listHead = nullptr;
  AttrCacheEntry* listPrev = nullptr;
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  while(true) {
    RecId searchRes = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)ATTRCAT_ATTR_RELNAME, attrVal, EQ);

    if(searchRes.block != -1 && searchRes.slot != -1) {
      RecBuffer recBuffer(searchRes.block);
      Attribute record[ATTRCAT_NO_ATTRS];
      AttrCacheEntry* attrCacheEntry = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

      recBuffer.getRecord(record, searchRes.slot);
      AttrCacheTable::recordToAttrCatEntry(record, &(attrCacheEntry->attrCatEntry));
      attrCacheEntry->recId.block = searchRes.block;
      attrCacheEntry->recId.slot = searchRes.slot;
      attrCacheEntry->next = nullptr;

      if(listHead == nullptr)
        listHead = attrCacheEntry;
      if(listPrev != nullptr)
        listPrev->next = attrCacheEntry;

      listPrev = attrCacheEntry;
    }
    else break;
  }
  AttrCacheTable::attrCache[relId] = listHead;

  /****** Setting up metadata in the Open Relation Table for the relation******/

  tableMetaInfo[relId].free = false;
  strcpy(tableMetaInfo[relId].relName, relName);

  return relId;
}


int OpenRelTable::closeRel(int relId) {
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) 
    return E_NOTPERMITTED;

  if (relId < 0 || relId >= MAX_OPEN)
    return E_OUTOFBOUND;

  if (tableMetaInfo[relId].free)
    return E_RELNOTOPEN;

  RecId recId;

  if(RelCacheTable::relCache[relId]->dirty){
    recId = RelCacheTable::relCache[relId]->recId;
    Attribute record[RELCAT_NO_ATTRS];
    RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry),record);

    RecBuffer RelCatBlock(recId.block);
    RelCatBlock.setRecord(record,recId.slot);
    // printf("if it didnt work please modify closeRel of OpenRelTable.");
  }
  
  free(RelCacheTable::relCache[relId]);
  RelCacheTable::relCache[relId] = nullptr;

  AttrCacheEntry* curAttrCacheEntry = AttrCacheTable::attrCache[relId];
  while(curAttrCacheEntry) {
    AttrCacheEntry* nextAttrCacheEntry = curAttrCacheEntry->next;
    free(curAttrCacheEntry);
    curAttrCacheEntry = nextAttrCacheEntry;
  }
  AttrCacheTable::attrCache[relId] = nullptr;

  tableMetaInfo[relId].free = true;
  return SUCCESS;

}