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


// int BlockAccess::insert(int relId, Attribute *record) {
//     // get the relation catalog entry from relation cache
//     // ( use RelCacheTable::getRelCatEntry() of Cache Layer)

//     RelCatEntry relCatEntry;
//     RelCacheTable::getRelCatEntry(relId,&relCatEntry);

//     int blockNum = relCatEntry.firstBlk;

//     // rec_id will be used to store where the new record will be inserted
//     RecId rec_id = {-1, -1};

//     int numOfSlots =relCatEntry.numSlotsPerBlk;
//     int numOfAttributes = relCatEntry.numAttrs;

//     int prevBlockNum = -1;

//     printf("hello-1\n");

//     /*
//         Traversing the linked list of existing record blocks of the relation
//         until a free slot is found OR
//         until the end of the list is reached
//     */
//     while (blockNum != -1) {
//         // create a RecBuffer object for blockNum (using appropriate constructor!)
//         RecBuffer insertBlock(blockNum);
//         HeadInfo header;
//         insertBlock.getHeader(&header);

//         // get header of block(blockNum) using RecBuffer::getHeader() function

//         // get slot map of block(blockNum) using RecBuffer::getSlotMap() function
        
//         unsigned char* slotMap;
//         insertBlock.getSlotMap(slotMap);
//         // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
//         // (Free slot can be found by iterating over the slot map of the block)
//         /* slot map stores SLOT_UNOCCUPIED if slot is free and
//            SLOT_OCCUPIED if slot is occupied) */

//         for(int i = 0;i<numOfAttributes;i++){
//             if(slotMap[i] == SLOT_UNOCCUPIED){
//                 rec_id.block = blockNum;
//                 rec_id.slot = i;
//                 break;
//             }
//         }

//         /* if a free slot is found, set rec_id and discontinue the traversal
//            of the linked list of record blocks (break from the loop) */

//         if(rec_id.block != -1 && rec_id.slot != -1)break;

//         /* otherwise, continue to check the next block by updating the
//            block numbers as follows:
//               update prevBlockNum = blockNum
//               update blockNum = header.rblock (next element in the linked
//                                                list of record blocks)
//         */

//         prevBlockNum = blockNum;
//         blockNum = header.rblock;

//     }

//     printf("hello1\n");

//     //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
//     if(rec_id.block == -1 && rec_id.slot == -1){
//         // if relation is RELCAT, do not allocate any more blocks
//         //     return E_MAXRELATIONS;

//         if(relId == RELCAT_RELID)return E_MAXRELATIONS;

//         // Otherwise,
//         // get a new record block (using the appropriate RecBuffer constructor!)
//         // get the block number of the newly allocated block
//         // (use BlockBuffer::getBlockNum() function)
//         // let ret be the return value of getBlockNum() function call

//         RecBuffer recordBlock;
//         int ret = blockNum = recordBlock.getBlockNum();

//         if (ret == E_DISKFULL) {
//             return E_DISKFULL;
//         }

//         // Assign rec_id.block = new block number(i.e. ret) and rec_id.slot = 0
//         rec_id.block = ret;
//         rec_id.slot = 0;

//         /*
//             set the header of the new record block such that it links with
//             existing record blocks of the relation
//             set the block's header as follows:
//             blockType: REC, pblock: -1
//             lblock
//                   = -1 (if linked list of existing record blocks was empty
//                          i.e this is the first insertion into the relation)
//                   = prevBlockNum (otherwise),
//             rblock: -1, numEntries: 0,
//             numSlots: numOfSlots, numAttrs: numOfAttributes
//             (use BlockBuffer::setHeader() function)
//         */

//         HeadInfo header;
//         header.blockType = REC;
//         header.lblock = prevBlockNum;
//         header.numAttrs = numOfAttributes;
//         header.numEntries = 0;
//         header.pblock = -1;
//         header.rblock = -1;

//         recordBlock.setHeader(&header);

//         /*
//             set block's slot map with all slots marked as free
//             (i.e. store SLOT_UNOCCUPIED for all the entries)
//             (use RecBuffer::setSlotMap() function)
//         */

//         unsigned char recordBlockSlotMap[numOfSlots];
//         for(int i=0;i<numOfSlots;i++)recordBlockSlotMap[i] = SLOT_UNOCCUPIED;
//         recordBlock.setSlotMap(recordBlockSlotMap);

//         // if prevBlockNum != -1
//         if(prevBlockNum != -1){
//             // create a RecBuffer object for prevBlockNum
//             // get the header of the block prevBlockNum and
//             // update the rblock field of the header to the new block
//             // number i.e. rec_id.block
//             // (use BlockBuffer::setHeader() function)

//             RecBuffer preBlock(prevBlockNum);
//             HeadInfo prevBlockHeader;
//             preBlock.getHeader(&prevBlockHeader);
//             prevBlockHeader.rblock = rec_id.block;
//             preBlock.setHeader(&prevBlockHeader);


//         }
//         else{
//             // update first block field in the relation catalog entry to the
//             // new block (using RelCacheTable::setRelCatEntry() function)

//             relCatEntry.firstBlk = rec_id.block;
//             RelCacheTable::setRelCatEntry(relId, &relCatEntry);
//         }

//         relCatEntry.lastBlk = rec_id.block;
//         RelCacheTable::setRelCatEntry(relId,&relCatEntry);

//         // update last block field in the relation catalog entry to the
//         // new block (using RelCacheTable::setRelCatEntry() function)
//     }

//     printf("hello2\n");

//     // create a RecBuffer object for rec_id.block
//     // insert the record into rec_id'th slot using RecBuffer.setRecord())

//     RecBuffer newBlock(rec_id.block);
//     newBlock.setRecord(record,rec_id.slot);

//     printf("hello3\n");

//     /* update the slot map of the block by marking entry of the slot to
//        which record was inserted as occupied) */
//     // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
//     // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)
//     unsigned char* newBlockSlotMap;
//     newBlock.getSlotMap(newBlockSlotMap);
//     newBlockSlotMap[rec_id.slot] = SLOT_OCCUPIED;
//     newBlock.setSlotMap(newBlockSlotMap);

//     printf("hello4\n");

//     // increment the numEntries field in the header of the block to
//     // which record was inserted
//     // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
//     HeadInfo newBlockHeader;
//     newBlock.getHeader(&newBlockHeader);
//     newBlockHeader.numEntries++;
//     newBlock.setHeader(&newBlockHeader);

//     printf("hello5\n");


//     // Increment the number of records field in the relation cache entry for
//     // the relation. (use RelCacheTable::setRelCatEntry function)

//     relCatEntry.numRecs++;
//     RelCacheTable::setRelCatEntry(relId,&relCatEntry);

//     printf("hello66\n");

//     return SUCCESS;
// }



int BlockAccess::insert(int relId, Attribute *record) {
  RelCatEntry relCatEntry;
  RelCacheTable::getRelCatEntry(relId, &relCatEntry);

  int curBlockNum = relCatEntry.firstBlk;
  RecId rec_id = {-1, -1};

  int numOfSlots = relCatEntry.numSlotsPerBlk;
  int numOfAttrs = relCatEntry.numAttrs;
  int prevBlockNum = -1;

  // find block/slot to insert
  while(curBlockNum != -1) {
    RecBuffer curRecBuffer(curBlockNum);
    HeadInfo curHead;
    curRecBuffer.getHeader(&curHead);
    unsigned char curSlotMap[numOfSlots];
    curRecBuffer.getSlotMap(curSlotMap);

    int freeSlot = -1;
    for(int i=0; i<numOfSlots; i++) {
      if(curSlotMap[i] == SLOT_UNOCCUPIED) {
        freeSlot = i;
        break;
      }
    }

    if(freeSlot != -1) {
      rec_id = {curBlockNum, freeSlot};
      break;
    }

    prevBlockNum = curBlockNum;
    curBlockNum = curHead.rblock;
  }
 
  // if new block is needed to insert
  if(rec_id.block == -1 && rec_id.slot == -1) {
    if(relId == RELCAT_RELID)
      return E_MAXRELATIONS;

    RecBuffer newBlockBuffer;
    int newBlockNum = newBlockBuffer.getBlockNum();
    if(newBlockNum == E_DISKFULL)
      return E_DISKFULL;
    
    rec_id.block = newBlockNum, rec_id.slot = 0;

    HeadInfo newHeader;
    newHeader.blockType = REC, newHeader.pblock = -1, newHeader.rblock = -1, newHeader.numSlots = numOfSlots;
    newHeader.numAttrs = numOfAttrs, newHeader.numEntries = 1, newHeader.lblock = prevBlockNum;
    newBlockBuffer.setHeader(&newHeader);

    unsigned char newBlockSlotMap[numOfSlots];
    for(int k=0; k<numOfSlots; k++)
      newBlockSlotMap[k] = SLOT_UNOCCUPIED;
    newBlockBuffer.setSlotMap(newBlockSlotMap);

    if(prevBlockNum == -1) {
      relCatEntry.firstBlk = newBlockNum;
      RelCacheTable::setRelCatEntry(relId, &relCatEntry);
    }
    else {
      HeadInfo prevHead;
      RecBuffer prevBlockBuffer(prevBlockNum);
      prevBlockBuffer.getHeader(&prevHead);
      prevHead.rblock = newBlockNum;
      prevBlockBuffer.setHeader(&prevHead);
    }

    relCatEntry.lastBlk = newBlockNum;
    RelCacheTable::setRelCatEntry(relId, &relCatEntry);
  } 
  
  RecBuffer recBuffer(rec_id.block);
  recBuffer.setRecord(record, rec_id.slot);
  
  unsigned char slotMap[numOfSlots];
  recBuffer.getSlotMap(slotMap);
  slotMap[rec_id.slot] = SLOT_OCCUPIED;
  recBuffer.setSlotMap(slotMap);

  HeadInfo head;
  recBuffer.getHeader(&head);
  head.numEntries += 1;
  recBuffer.setHeader(&head);

  relCatEntry.numRecs += 1;
  RelCacheTable::setRelCatEntry(relId, &relCatEntry);
  
  return SUCCESS;
}
