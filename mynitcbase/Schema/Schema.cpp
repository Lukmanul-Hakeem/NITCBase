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