// Demonstration of calist usage.

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "calist.h"

// Append stack-allocated, temporary values to al
static void calist_append_stack(calist *al) {
  int a = 5;
  const int b = 10;
  calist_append(al, &a);    // a is deep-copied into al
  calist_add(al, &b);       // 'calist_add' is an alias for 'calist_append'
  // a and b can now safely exit the stack frame

  // 'WRAP_INT' adds a stack-allocated integer without a variable
  calist_append(al, WRAP_INT(20));
}

// Append heap-allocated objects to al
static void calist_append_heap(calist *al) {
  int *ptr = malloc(sizeof(*ptr));
  *ptr = 30;
  calist_append(al, ptr);   // ptr is deep-copied into al
  free(ptr);                // Client must free original pointer
}

// Check if num is even
static bool is_even(const calist *al, const void *item, const void *args) {
  (void)al;
  (void)args;
  const int *int_item = item;
  return (*int_item % 2 == 0);
}

// Multiply the value of num by 3
static void multiply(const calist *al, void *item, const void *args) {
  (void)al;
  int *int_ptr = item;
  const int *factor = args;
  *int_ptr *= (*factor);
}

int main(void) {
  // Create an integer calist ['ctype_int' is included in ctype.h]
  calist *al = calist_create(ctype_int());
  
  // Append integers to the back of al
  calist_append_stack(al);
  calist_append_heap(al);
  
  // Print al to the console
  calist_print(al);         // Console: [5, 10, 20, 30]
  // Get the size of al
  assert(calist_size(al) == 4);

  // Inserting integers before index positions
  calist_insert(al, 2, WRAP_INT(5));      // al: [5, 10, 5, 20, 30]
  calist_insert(al, 3, WRAP_INT(25));     // al: [5, 10, 5, 25, 20, 30]
  calist_insert_front(al, WRAP_INT(35));  // al: [35, 5, 10, 5, 25, 20, 30]

  // Duplicate al
  calist *al_copy = calist_dup(al);
  calist_print(al_copy);    // Console: [35, 5, 10, 5, 25, 20, 30]
  // Comparing al and al_copy for equality
  assert(calist_equals(al, al_copy));

  // Remove the item at index position 2
  calist_pop(al, 2);                    // al: [35, 5, 5, 25, 20, 30]
  // Remove the first item with the value 25
  calist_remove(al, WRAP_INT(25));      // al: [35, 5, 5, 20, 30]
  // Remove all items with the value 5
  calist_remove_all(al, WRAP_INT(5));   // al: [35, 20, 30]
  // Remove all even integers from al
  calist_remove_if(al, is_even, NULL);  // al: [35]
  calist_print(al);         // Console: [35]
  
  // Remove all elements from al, but al is kept
  calist_clear(al);
  calist_print(al);         // Console: []

  // Add previous items from the copy
  calist_append_all(al, al_copy);  
  calist_print(al);         // Console: [35, 5, 10, 5, 25, 20, 30]
  calist_destroy(al_copy);  // Destroy al_copy and its content

  // Check if integers are in al
  assert(calist_contains(al, WRAP_INT(10)));
  assert(!calist_contains(al, WRAP_INT(99)));
  // First occurrence of 5 is at index position 1
  assert(calist_index(al, WRAP_INT(5)) == 1);
  // Last occurrence of 5 is at index position 3
  assert(calist_index_last(al, WRAP_INT(5)) == 3);
  // 'CALIST_INDEX_NOT_FOUND': 99 is not in al
  assert(calist_index(al, WRAP_INT(99)) == CALIST_INDEX_NOT_FOUND);
  // Count the number of occurrences
  assert(calist_count(al, WRAP_INT(5)) == 2);
  assert(calist_count(al, WRAP_INT(99)) == 0);
  
  // Access and mutate items at specific index positions
  // Get the item at index position 2
  const int *item = calist_get(al, 2);
  assert(*item == 10);
  // Modify the item at index position 0
  int *item_mutable = calist_get_mutable(al, 0);
  *item_mutable = 40;
  // Set the item at index position 5
  calist_set(al, 5, WRAP_INT(25));
  calist_print(al);         // Console: [40, 5, 10, 5, 25, 25, 30]

  // Quick sort algorithm
  calist_qsort(al);         
  calist_print(al);         // Console: [5, 5, 10, 25, 25, 30, 40]
  // Binary search on the sorted list
  assert(calist_bsearch(al, WRAP_INT(30)) != CALIST_INDEX_NOT_FOUND);
  assert(calist_bsearch(al, WRAP_INT(99)) == CALIST_INDEX_NOT_FOUND);
  // Reverse the list
  calist_reverse(al);
  calist_print(al);         // Console: [40, 30, 25, 25, 10, 5, 5]
  // Multiply all items by 3
  calist_foreach(al, multiply, WRAP_INT(3));
  calist_print(al);         // Console: [120, 90, 75, 75, 30, 15, 15]

  // Find unique elements and remove duplicates
  al_copy = calist_unique(al);
  calist_print(al_copy);    // Console: [120, 90, 75, 30, 15]
  assert(calist_remove_dup(al) == 2);
  calist_print(al);         // Console: [120, 90, 75, 30, 15]
  assert(calist_equals(al, al_copy));

  // IMPORTANT: free al and al_copy to prevent memory leak
  calist_destroy(al);
  calist_destroy(al_copy);
  calist_destroy(NULL);     // Also safe
}
