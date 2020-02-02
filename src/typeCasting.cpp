#include <iostream>
#include "BuddyAllocator.h"

using namespace std;



int main(){

    /*
    //allocating the address of malloc
    char * start = (char *)malloc(sizeof(int));
    // displaying address pointed by pointer
    cout << static_cast<void*>(start) << endl;
    */
    LinkedList myList = LinkedList();
    BlockHeader* node = new BlockHeader();
    node->block_size = 100;
    myList.insert(node);
    myList.print();
    BlockHeader* node1 = new BlockHeader();
    myList.insert(node1);
    myList.print();
    myList.remove(node);
    myList.print();
}