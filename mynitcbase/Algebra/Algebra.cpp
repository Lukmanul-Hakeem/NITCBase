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


int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char attrVal[][ATTR_SIZE]){
    // if relName is equal to "RELATIONCAT" or "ATTRIBUTECAT"
    // return E_NOTPERMITTED;

    if(strcmp(relName,RELCAT_RELNAME) == 0 || strcmp(relName,ATTRCAT_RELNAME) == 0)return E_NOTPERMITTED;

    // get the relation's rel-id using OpenRelTable::getRelId() method
    int relId = OpenRelTable::getRelId(relName);

    if(relId == E_RELNOTOPEN)return E_RELNOTOPEN;

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(relId,&relCatEntry);

    if(relCatEntry.numAttrs != nAttrs)return E_NATTRMISMATCH;

    // if relation is not open in open relation table, return E_RELNOTOPEN
    // (check if the value returned from getRelId function call = E_RELNOTOPEN)
    // get the relation catalog entry from relation cache
    // (use RelCacheTable::getRelCatEntry() of Cache Layer)

    /* if relCatEntry.numAttrs != numberOfAttributes in relation,
       return E_NATTRMISMATCH */

    // let recordValues[numberOfAttributes] be an array of type union Attribute

    Attribute record[nAttrs];

    /*
        Converting 2D char array of record values to Attribute array recordValues
     */
    for(int i=0;i<nAttrs;i++)
    {
        // get the attr-cat entry for the i'th attribute from the attr-cache
        // (use AttrCacheTable::getAttrCatEntry())

        // let type = attrCatEntry.attrType;

        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId,i,&attrCatEntry);

        if (attrCatEntry.attrType == NUMBER)
        {
            // if the char array record[i] can be converted to a number
            // (check this using isNumber() function)
            if(isNumber(attrVal[i]))
            {
                /* convert the char array to numeral and store it
                   at recordValues[i].nVal using atof() */

                record[i].nVal = atof(attrVal[i]);
            }
            else
            {
                return E_ATTRTYPEMISMATCH;
            }
        }
        else if (attrCatEntry.attrType == STRING)
        {
            strcpy(record[i].sVal,attrVal[i]);
        }
    }


    // insert the record by calling BlockAccess::insert() function
    // let retVal denote the return value of insert call

    int retVal = BlockAccess::insert(relId,record);

    

    

    return retVal;
}

