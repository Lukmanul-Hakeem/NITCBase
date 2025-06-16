#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;

  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer,7000); 
  char message[] = "lukman";
  std::cout << message;

  memcpy(buffer+20,message,7);
  Disk::writeBlock(buffer,7000);
  
  unsigned char buffer2[BLOCK_SIZE];
  Disk::readBlock(buffer2,7000);
  char message2[10];

  memcpy(message2,buffer2+20,7);
  std::cout << message2;
  
  // StaticBuffer buffer;
  // OpenRelTable cache;

  return 0;
}