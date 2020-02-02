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
	::free(start);
}
void* BuddyAllocator::alloc(int length) {
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */
  int rightLength = length + sizeof(BlockHeader);
  int listIndex = freeListIndex(rightLength);
  if(rightLength > total_memory_size){
    cout << "Size cannot be greater than total memory allocated" << endl;
    return nullptr;
  }
  if(FreeList[listIndex].head){
    BlockHeader* b = FreeList[listIndex].head;
    //removing head from the list where it fits
    FreeList[listIndex].remove(FreeList[listIndex].head);
    //block header allocated
    b->free = false;
    return (char *)(b+1);
  }else{
    int backtrack = listIndex;;
    while(FreeList[backtrack].head == nullptr){
      //checking if went to bigger size than memory size and could not find a not nullptr
      if(backtrack >= FreeList.size()){
        return nullptr;
      }
      backtrack++;
    }
    //bactracking up to listIndex
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
    
    //returning the pointer blockheard plus the size of Block Header
    return (char*)(mem+1);
  }
}
int BuddyAllocator::freeListIndex(int length){
  int index = log2(ceil( ((double)length) / ((double) basic_block_size) ) );
  return index;
}

void BuddyAllocator::free(void* a) {
    /* Same here! */
  //address of actual block
  BlockHeader* block = (BlockHeader*)((char *)a - sizeof(BlockHeader));
  if(block->block_size == total_memory_size){
    return;
  }
  while(1){
    //address out of index of list
    int listIndex =  freeListIndex(block->block_size);
    //index out of bounds
    if(listIndex >= FreeList.size() ){
      break;
    }
    BlockHeader* buddy = getbuddy(block);
    //removing block of +current list
    FreeList[listIndex].remove(block);
    //buddy is free
    if(buddy->free){
      FreeList[listIndex].remove(buddy);
      block = merge(block,buddy); 
      FreeList[listIndex+1].insert(block);
      buddy = nullptr;  
    }
    //buddy is not free
    else{
      block->free = true;
      FreeList[listIndex].insert(block);
      buddy = nullptr;
      return;
    }
  }
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

bool BuddyAllocator::arebuddies(BlockHeader* block1, BlockHeader* block2){
  return getbuddy(block1) == block2;
}

BlockHeader* BuddyAllocator::split (BlockHeader* block){
  //indexes of free list
  int currIndex = freeListIndex(block->block_size);
  int prevIndex =  currIndex -1;
  //setting block to allocated and removing block from its current list
  int splitOffset = block->block_size / 2;
  FreeList[currIndex].remove(block);
  //creating the Block Header at split point of the block
  BlockHeader* halfBlock = (BlockHeader*)((char *)block + (splitOffset));
  //updating size of block created and current block
  block->block_size = splitOffset;
  halfBlock->block_size = splitOffset;
  //changing data members of the blocks
  halfBlock->free = true;
  halfBlock->next = nullptr;
  block->next = nullptr;
  block->free = true;

  //deleting split block from free list
  //adding split blocks to freelist at prevIndex
  //adding halfblock
  FreeList[prevIndex].insert(halfBlock);
  FreeList[prevIndex].insert(block);
  //removing split block
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



  //removes block from FreeList
	void BuddyAllocator::removeBlock(BlockHeader* block){
    int index = freeListIndex(block->block_size);
    if(index < FreeList.size() && index >= 0){
      FreeList[index].remove(block);
    }

  }
	//inserts a block to FreeList
	void BuddyAllocator::insertBlock(BlockHeader* block){
    int index = freeListIndex(block->block_size);
    if(index < FreeList.size() && index >= 0){
      FreeList[index].insert(block )
    }
  }