// tools.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"

// clear DRAM data blocks by filling zeroes
void MyBzero(char *p, int size) {
   int i;
   //loop for 'size' number of times:
   for(i=0; i<size; i++){
	  
	// set where p points to to (char *)0
	*p++=(char)0;
   }
}

// dequeue, return 1st element in array, and move all forward
// if queue empty, return -1
int DeQ(q_t *p) {         // return -1 if q[] contains no valid elements
   int i,  element = -1;

   //if the size of the queue that p poins to is 0, return element
   if(p->size==0){
	return element;
   }
   //copy the 1st in the array that p points to to element
   //decrement the size of the queue that p points to
   //move all elements in the array to the front by one position
   element = p->q[0];
   p->size--;
   for(i=0;i<p->size;i++){
	p->q[i]=p->q[i+1];
   }
   return element;
}

// enqueue element to next available position in array, 'size' is array index
void EnQ(int element, q_t *p) {
   //if the size of the queue that p points to is already Q_SIZE {
   if(p->size==Q_SIZE){
  
	 cons_printf("Kernel Panic: queue is full, cannot EnQ!\n");
      //return;       // alternative: breakpoint() into GDB
   }
   //copy element to the location in the array indexed by 'size'
   p->q[p->size]=element;
   //increment 'size' of the queue
   p->size++;
}

