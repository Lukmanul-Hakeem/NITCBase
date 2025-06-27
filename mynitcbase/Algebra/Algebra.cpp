#include "Algebra.h"
#include <cstdlib>
#include <cstring>
#include <stdio.h>


bool isNumber(char *str) {
  int len;
  float ignore;
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);
    if(srcRelId == E_RELNOTOPEN)return E_RELNOTOPEN;
    printf("%d\n",srcRelId);

    AttrCatEntry attrEntrySrc;
    if(AttrCacheTable::getAttrCatEntry(srcRelId,attr,&attrEntrySrc) == E_ATTRNOTEXIST)return E_ATTRNOTEXIST;

    int type = attrEntrySrc.attrType;

    Attribute attrVal;

    if(type == NUMBER){
        if(isNumber(strVal)){
            attrVal.nVal = atof(strVal);
        }else {
            return E_ATTRTYPEMISMATCH;
        }
    }else if (type == STRING) {
        strcpy(attrVal.sVal, strVal);
    }

    RelCacheTable::resetSearchIndex(srcRelId);

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId,&relCatEntry);

    AttrCatEntry attrCatEntry;
    for(int i=0;i<relCatEntry.numAttrs;i++){
        AttrCacheTable::getAttrCatEntry(srcRelId,i,&attrCatEntry);
        printf("%s | ",attrCatEntry.attrName);
    }
    printf("\n");


    while(true){


        RecId searchIndex = BlockAccess::linearSearch(srcRelId,attr,attrVal,op);

        if(searchIndex.block != -1 && searchIndex.slot != -1){

            RecBuffer currBlock(searchIndex.block);
            Attribute curRecordAttr[relCatEntry.numAttrs];
            currBlock.getRecord(curRecordAttr,searchIndex.slot);

            for(int i=0;i<relCatEntry.numAttrs;i++){
                AttrCatEntry attrCatEntry;
                AttrCacheTable::getAttrCatEntry(srcRelId,i,&attrCatEntry);

                if(attrCatEntry.attrType == NUMBER){
                    printf("%f | ",curRecordAttr[i].nVal);
                }else{
                    printf("%s | ",curRecordAttr[i].sVal);
                }
            }
            printf("\n");
        }else break;
    }

    return SUCCESS;

}

