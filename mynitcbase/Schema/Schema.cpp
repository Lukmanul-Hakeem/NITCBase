#include "Schema.h"
#include <stdio.h>

#include <cmath>
#include <cstring>


int Schema::openRel(char* relName){
    int ret = OpenRelTable::openRel(relName);
    if(ret >= 0)return SUCCESS;
    return ret;
}

int Schema::closeRel(char*relName){
    if(strcmp(relName,RELCAT_RELNAME) == 0 || strcmp(relName,ATTRCAT_RELNAME) == 0)return E_NOTPERMITTED;


    int relId = OpenRelTable::getRelId(relName);


    if(relId == E_RELNOTOPEN)return E_RELNOTOPEN;


    return OpenRelTable::closeRel(relId);

}



int Schema::renameRel(char* oldRelName,char* newRelName){
    
    if(strcmp(oldRelName,RELCAT_RELNAME) == 0 || strcmp(oldRelName,ATTRCAT_RELNAME) == 0 || strcmp(newRelName,RELCAT_RELNAME) == 0 || strcmp(newRelName,ATTRCAT_RELNAME) == 0)return E_NOTPERMITTED;
    int ret = OpenRelTable::getRelId(oldRelName);
    if(ret != E_RELNOTOPEN) return E_RELOPEN;
    
    return BlockAccess::renameRelation(oldRelName, newRelName);
}

int Schema::renameAttr(char* relName,char* oldName,char* newName){

    if(strcmp(relName,RELCAT_RELNAME) == 0 || strcmp(relName,ATTRCAT_RELNAME) == 0)return E_NOTPERMITTED;
    if(OpenRelTable::getRelId(relName) != E_RELNOTOPEN)return E_RELOPEN;
    return BlockAccess::renameAttribute(relName,oldName,newName);
}


int Schema::deleteRel(char* relName){
    if(strcmp(relName,RELCAT_RELNAME) ==0 || strcmp(relName,ATTRCAT_RELNAME) == 0)return E_NOTPERMITTED;

    int relId = OpenRelTable::getRelId(relName);
    if(relId != E_RELNOTOPEN)return E_RELOPEN;

    return BlockAccess::deleteRelation(relName);
}



int Schema::createRel(char relName[],int nAttrs, char attrs[][ATTR_SIZE],int attrtype[]){

    Attribute relNameAsAttribute;
    strcpy(relNameAsAttribute.sVal,relName);

    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    RecId targetRelId =  BlockAccess::linearSearch(RELCAT_RELID,(char*)RELCAT_ATTR_RELNAME,relNameAsAttribute,EQ);

    if(targetRelId.block != -1 && targetRelId.slot != -1)return E_RELEXIST;
    // printf("relation doesnot exists\n");

    for(int i=0;i<nAttrs;i++){
        for(int j=0;j<nAttrs;j++){
            if(i != j && strcmp(attrs[i],attrs[j]) == 0)return E_DUPLICATEATTR;
        }
    }

    // printf("no duplicate attributes\n");

    Attribute relCatRecord[RELCAT_NO_ATTRS];
    strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,relName);
    relCatRecord[RELCAT_FIRST_BLOCK_INDEX].nVal = -1;
    relCatRecord[RELCAT_LAST_BLOCK_INDEX].nVal = -1;
    relCatRecord[RELCAT_NO_RECORDS_INDEX].nVal = 0;
    relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal = nAttrs;
    relCatRecord[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = floor(2016 / (16*nAttrs + 1));


    int ret = BlockAccess::insert(RELCAT_RELID,relCatRecord);
    if(ret != SUCCESS)return ret;

    // printf("inserted to relcatalog\n");

    for(int i=0; i<nAttrs;i++){

        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatRecord[ATTRCAT_OFFSET_INDEX].nVal = i;
        strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relName);
        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,attrs[i]);
        attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal = attrtype[i];
        attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal = -1;
        attrCatRecord[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = -1;

        ret = BlockAccess::insert(ATTRCAT_RELID,attrCatRecord);
        if(ret != SUCCESS){
            Schema::deleteRel(relName);
            return E_DISKFULL;
        }
    }

    return SUCCESS;

}
