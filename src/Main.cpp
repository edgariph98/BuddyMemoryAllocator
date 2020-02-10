#include "Ackerman.h"
#include "BuddyAllocator.h"
#include <unistd.h>
#include <cstdlib>
#include <math.h>
void easytest(BuddyAllocator* ba){
  // be creative here
  // know what to expect after every allocation/deallocation cycle

  // here are a few examples
  ba->printlist();
  // allocating a byte
  char * mem = (char *) ba->alloc (1);
  // now print again, how should the list look now
  ba->printlist ();

  ba->free (mem); // give back the memory you just allocated
  ba->printlist(); // shouldn't the list now look like as in the beginning

}
bool isPowerOfTwo(int n) 
{ 
   if(n==0) 
   return false; 
  
   return (ceil(log2(n)) == floor(log2(n))); 
} 

int main(int argc, char ** argv) {

  int basic_block_size = 128, memory_length = 512*1024;
  // create memory manager
  int comm;
  while((comm = getopt(argc, argv, "b:s:")) != -1){
    switch (comm)
    {
      case 'b':
        basic_block_size = (int)atoi(optarg);
        break;
      case 's':
        memory_length = (int)atoi(optarg);
      default:
        break;
    }
  }
  if(!isPowerOfTwo(basic_block_size)){
    cout << "Basic Block SIze is not a power of 2" << endl;
    exit(-1);
  }else if(!isPowerOfTwo(memory_length)){
    cout << "Memory Length is not a power of 2 " << endl;
    exit(-1);
  }
  cout << "BBS: " << basic_block_size << endl;
  cout << "Memory Size: " << memory_length << endl;
  BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);
  /*cout << allocator->freeListIndex(1900) << endl;
  vector<char *> blocks;
  for(int i = 1; i < 1000; i*=2){
    char* mem = (char *)allocator->alloc(i);
    blocks.push_back(mem);
  }
  allocator->printlist();
  for(int i =0; i <blocks.size();++i){
    allocator->free(blocks[i]);
  }
  allocator->printlist();
  delete allocator;*/

  /*
  cout << "EMPTY lIST" << endl;
  allocator->printlist();
  //char* bytes400 = (char*)allocator->alloc(400);
  char * bytes100 = (char *)allocator->alloc(100);
  cout << "List with Allocated memory" << endl;
  allocator->printlist();
  allocator->free(bytes100);
  //  allocator->free(bytes400);
  cout <<"List after de allocating 100 bytes" << endl;
  allocator->printlist();*/

  

  
  // the following won't print anything until you start using FreeList and replace the "new" with your own implementation
  //easytest (allocator);

  
  // stress-test the memory manager, do this only after you are done with small test cases
  Ackerman* am = new Ackerman ();
  am->test(allocator); // this is the full-fledged test. 
  
  // destroy memory manager
  delete allocator;
  //allocator->~BuddyAllocator();

  return 0;
}
