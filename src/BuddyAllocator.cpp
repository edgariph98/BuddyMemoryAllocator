#include "BuddyAllocator.h"
#include <iostream>
#include <math.h>
//importing memcpy
#include <cstring>
using namespace std;

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
  basic_block_size = _basic_block_size, total_memory_size = _total_memory_length;
  start = (char *)malloc(total_memory_size);
  int listNum = (int)(log2(total_memory_size/basic_block_size));
  for(int i = 0; i <= listNum; ++i){
    FreeList.push_back(LinkedList());
  }
  //creating a new block header containing size of all memory
  BlockHeader* totalMemoryBlock = (BlockHeader*)(start);
  totalMemoryBlock->free = true;
  totalMemoryBlock->next = nullptr;
  totalMemoryBlock->block_size = total_memory_size;
  //inserting block to end of free list
  FreeList[listNum].insert(totalMemoryBlock);
}

BuddyAllocator::~BuddyAllocator (){
  for(int i = 0 ; i < FreeList.size();++i){
    FreeList[i].head = nullptr;
  }
	std::free(start);
}
void* BuddyAllocator::alloc(int length) {
  //actual size of block we need to allocate
  int rightLength = length + sizeof(BlockHeader);
  //getting the index for the list that we need to remove a block from for allocation 
  int listIndex = freeListIndex(rightLength);
  //cout << "List Index: " << listIndex << ", for length: " << rightLength <<  endl;

  //checking if the size requested is ess than total memory size
  if(rightLength > total_memory_size){
    cout << "Size cannot be greater than total memory allocated" << endl;
    return nullptr;
  }
  //checking if the list currently has a block available for use
  if(FreeList[listIndex].head){
    BlockHeader* b = FreeList[listIndex].head;
    //removing head from the list where it fits
    FreeList[listIndex].remove(FreeList[listIndex].head);
    b->free = false;
    //returning the address of pinter to free memory not address of our pointer
    return (char *)(b+1);

  }
  //case where the list is empty and we need to start splitting larger blocks to get our size block
  else{
    //serves to count how many times to backtrack and split
    int backtrack = listIndex;;
    //finding first list with a block in it with larger size than we need
    while(FreeList[backtrack].head == nullptr){
      //checking if went to bigger size than memory size and could not find a not nullptr
      if(backtrack >= FreeList.size()){
        return nullptr;
      }
      backtrack++;
    }
    //we start bactracking up to listIndex and splitting up the blocks
    while(listIndex < backtrack){
      split(FreeList[backtrack].head);
      backtrack--;
    }
    //getting memory
    BlockHeader* mem = FreeList[listIndex].head;
    //setting memory to allocated
    mem->free = false;
    //removing mem from linkedlist
    FreeList[listIndex].remove(FreeList[listIndex].head);
    
    //returning pointer to actual free memory
    return (char*)(mem+1);
  }
}
//serves to find the index could be inserted or removed based on its size
int BuddyAllocator::freeListIndex(int length){
  int index = (int)ceil(log2(ceil( (double)length / (basic_block_size) ) ));
  return index;
}

void BuddyAllocator::free(void* a) {
  //pointer of actual block
  BlockHeader* block = (BlockHeader*)((char *)a - sizeof(BlockHeader));
  //checking if the free block is not the one conaining all the memory size
  if(block->block_size == total_memory_size){
    block->free = true;
    FreeList[FreeList.size()-1].insert(block);
    return;
  }
  //repeating same process over and over
  while(1){
    int listIndex =  freeListIndex(block->block_size);
    //checking if block size is index range of FreeList
    if(listIndex >= FreeList.size() ){
      break;
    }
    BlockHeader* buddy = getbuddy(block);
    //checking if buddy is free
    if(buddy->free == true){
      FreeList[listIndex].remove(buddy);
      block = merge(block,buddy); 
    }
    //buddy is not free
    else{
      block->free = true;
      FreeList[listIndex].insert(block);
      return;
    }
  }
  //inserting the last merged block into freelist so it can be available to use
  FreeList[freeListIndex(block->block_size)].insert(block);
  block->free = true;
}

void BuddyAllocator::printlist (){
  cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
  int64_t total_free_memory = 0;
  for (int i=0; i<FreeList.size(); i++){
    int blocksize = ((1<<i) * basic_block_size); // all blocks at this level are this size
    cout << "[" << i <<"] (" << blocksize << ") : ";  // block size at index should always be 2^i * bbs
    int count = 0;
    BlockHeader* b = FreeList[i].head;
    // go through the list from head to tail and count
    while (b){
      total_free_memory += blocksize;
      count ++;
      // block size at index should always be 2^i * bbs
      // checking to make sure that the block is not out of place
      if (b->block_size != blocksize){
        cerr << "ERROR:: Block is in a wrong list" << endl;
        exit (-1);
      }
      b = b->next;
    }
    cout << count << endl;
    cout << "Amount of available free memory: " << total_free_memory << " bytes" << endl;  
  }
}
//returning buddy 
BlockHeader* BuddyAllocator::getbuddy(BlockHeader * addr){
  int offset =  (char *)addr-start;
  int buddyOffset = offset^(addr->block_size);
  char* buddyAddress = start + buddyOffset;
  return (BlockHeader*) buddyAddress;
}
//checking if  two BlockHeaders are buddies
bool BuddyAllocator::arebuddies(BlockHeader* block1, BlockHeader* block2){
  return getbuddy(block1) == block2;
}

BlockHeader* BuddyAllocator::split (BlockHeader* block){
  //indexes of free list
  int currIndex = freeListIndex(block->block_size);
  int prevIndex =  currIndex -1;
  int splitOffset = block->block_size / 2;
  //removing  splitting block from its  current list
  FreeList[currIndex].remove(block);
  //creating the Block Header at split point of the block
  BlockHeader* halfBlock = (BlockHeader*)((char *)block + (splitOffset));
  //changing data members of the blocks gotten from split
  block->block_size = splitOffset;
  halfBlock->block_size = splitOffset;
  halfBlock->free = true;
  block->free = true;
  halfBlock->next = nullptr;
  block->next = nullptr;
  //inserting new blocks to its corresponding list
  FreeList[prevIndex].insert(halfBlock);
  FreeList[prevIndex].insert(block);
  return halfBlock;
}

BlockHeader* BuddyAllocator::merge (BlockHeader* block1, BlockHeader* block2){
  BlockHeader* mergedBlock;
  int blockSizeMerged = block1->block_size*2;
  //block1 is the lower address
  if(block1 < block2){  
    //pointing block2 to block1 the lower address
    block2 = (BlockHeader*)((char*)block2 - block2->block_size);
    mergedBlock =  block1;
  }
  //block2 is the lower address
  else{
    //pointing block1 to block2 the lower address
    block1 = (BlockHeader*)((char*)block1 - block1->block_size);
    mergedBlock = block2;
  }
  //doubling size of merged block
  mergedBlock->block_size = blockSizeMerged;
  //freeing mem of merged block
  mergedBlock->free = true;
  return mergedBlock;
}
