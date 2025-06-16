#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
using namespace std;
int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;

  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer,7000); 
  char message[] = "lukman";

  memcpy(buffer+20,message,7);
  Disk::writeBlock(buffer,7000);
  
  unsigned char buffer2[BLOCK_SIZE];
  Disk::readBlock(buffer2,7000);
  char message2[10];

  memcpy(message2,buffer2+20,7);
  cout << message2 <<endl;

  // exercise solution

  for(int i=0;i<4;i++){
    printf("Reading From BMAP %d\n",i+1);
    Disk::readBlock(buffer,i);
    for(auto val : buffer)cout << val << " ";
    cout << endl;
  }
  
 
  return 0;
}