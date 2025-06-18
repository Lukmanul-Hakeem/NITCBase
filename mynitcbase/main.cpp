#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
using namespace std;
int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;

  RecBuffer relationCatalog(RELCAT_BLOCK);

  HeadInfo relationCat_Header;
  relationCatalog.getHeader(&relationCat_Header);


  for(int i=0;i<relationCat_Header.numEntries;i++){
    Attribute relCatRecord[relationCat_Header.numAttrs];
    relationCatalog.getRecord(relCatRecord,i);

    // printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    if(strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,"Student") == 0){

      int currBlock = ATTRCAT_BLOCK;
      while(currBlock != -1){

        RecBuffer attributeCatalog(currBlock);
        HeadInfo attributeCat_Header;
        attributeCatalog.getHeader(&attributeCat_Header);

        for(int j=0;j<attributeCat_Header.numEntries;j++){

          Attribute attributeRecord[relationCat_Header.numAttrs];
          attributeCatalog.getRecord(attributeRecord,j);
    
          if(strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,attributeRecord[ATTRCAT_REL_NAME_INDEX].sVal)==0 && strcmp(attributeRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Class") == 0){

            strcpy(attributeRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Batch");
            printf("SCHEMA UPDATED SUCCESSFULLY");
            break;

          }
    
        }
      
        currBlock = attributeCat_Header.rblock;

      }

      printf("\n");

    }

    

  }
  
 
  return 0;
}