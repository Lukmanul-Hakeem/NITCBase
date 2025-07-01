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