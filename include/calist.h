// The calist module provides the calist ADT.

#ifndef CALIST_H
#define CALIST_H

#include <stdbool.h>
#include <stddef.h>

#include "cvalue.h"

#ifdef __cplusplus
extern "C" {
#endif

// API policy:
//   - The calist owns every item returned by its cvalue's dup callback.
//   - Pointers returned by calist_get and calist_get_mutable are borrowed;
//     do not free them, and do not use them after a structural modification.
//   - Violating a documented precondition is a programming error and aborts.
//   - Allocation or duplication failure terminates the process with failure.

// A calist stores items in a dynamically resizable array,
// with all items deeply copied into heap memory.
//
// Each calist is associated with a specific cvalue, defining a common type
// for all stored items. Type-specific behaviors (duplication, comparison,
// printing, and deallocation) are handled through the cvalue interface.
//
// Memory model:
//   - All inserted items are deeply copied into separately allocated
//     heap memory.
//   - Stack-allocated or heap-allocated objects are both safe to insert;
//     the calist duplicates them internally.
//   - Stored items are automatically freed when individually removed or
//     when the calist is destroyed.
//   - Clients are responsible for freeing the original heap-allocated
//     objects after insertion to avoid memory leaks.
//
// Example:
//   int temp = 10;
//   calist_append(al, &temp);  // Deep-copied onto heap
//
//   int *ptr = malloc(sizeof(*ptr));
//   *ptr = 5;
//   calist_append(al, ptr);    // Deep-copied into calist
//   free(ptr);                 // Client must free original pointer
//
typedef struct calist calist;

// calist_pred is a predicate function that checks whether the given item
//   in al satisfies specific conditions.
// parameters:
//   - al: the calist containing the item (may be used for context or ignored)
//   - item: the item to be tested
//   - args: optional external data (may be NULL); typically a pointer to a
//           user-defined struct containing additional predicate parameters
// requires: al and item are not NULL
// note: returns true if item satisfies the conditions; false otherwise
typedef bool (*calist_pred)(const calist *al, const void *item,
                            const void *args);

// calist_map is a mapping function that optionally modifies the given item
//   in al. The function may choose to mutate item or simply read it,
//   depending on args or internal logic.
// parameters:
//   - al: the calist containing the item (may be used for context or ignored)
//   - item: a mutable pointer to the item to be optionally modified
//   - args: optional external data (may be NULL); typically a pointer to a
//           user-defined struct with transformation parameters
// requires: al and item are not NULL
// effects: may modify item
typedef void (*calist_map)(const calist *al, void *item, const void *args);

// The return value if an item is not in calist (SIZE_MAX)
extern const size_t CALIST_INDEX_NOT_FOUND;

// Method alias
#define calist_length calist_size
#define calist_add calist_append
#define calist_add_all calist_append_all
#define calist_insert_back calist_append
#define calist_find calist_index
#define calist_find_last calist_index_last

// calist_create(type) creates an empty calist which stores items of
//   the given type.
// requires: type is not NULL
// effects: allocates heap memory [caller must free with calist_destroy]
calist *calist_create(const cvalue *type);

// calist_create_size(type, init_cap) creates an empty calist of the
//   given type with an initial capacity of init_cap.
// requires: type is not NULL
//           init_cap > 0
// effects: allocates heap memory [caller must free with calist_destroy]
calist *calist_create_size(const cvalue *type, size_t init_cap);

// calist_destroy(al) frees al and its items from the heap memory.
// effects: frees heap memory [al becomes invalid]
void calist_destroy(calist *al);

// calist_clear(al) removes all items from al.
// requires: al is not NULL
// effects: modifies al, frees heap memory
// notes:
//   - calist_clear only frees the items in al
//   - call calist_destroy to completely destroy al
//   - call calist_reclaim to free the emptied storage of al
void calist_clear(calist *al);

// calist_dup(al) creates a deep copy of al.
// requires: al is not NULL
// effects: allocates heap memory [caller must free with calist_destroy]
calist *calist_dup(const calist *al);

// calist_print(al) displays al in the format [X, Y, ...], where
//   X, Y, ... are items of al.
// requires: al is not NULL
// effects: produces output
void calist_print(const calist *al);

// calist_equals(l1, l2) produces true if l1 and l2 have identical size,
//   type, and items; false otherwise.
// requires: l1 and l2 are not NULL
bool calist_equals(const calist *l1, const calist *l2);

// calist_type(al) produces the cvalue descriptor associated with al.
// requires: al is not NULL
const cvalue *calist_type(const calist *al);

// calist_size(al) produces the number of items in al.
// requires: al is not NULL
size_t calist_size(const calist *al);

// calist_empty(al) produces true if al is empty and false otherwise.
// requires: al is not NULL
bool calist_empty(const calist *al);

// calist_capacity(al) produces the maximum number of items
//   currently allocated for al.
// requires: al is not NULL
size_t calist_capacity(const calist *al);

// calist_reserve(al, n) ensures al can hold at least n items.
// requires: al is not NULL
// effects: may allocate heap memory
// note: if al has more than n items, calist_reserve has no effect
void calist_reserve(calist *al, size_t n);

// calist_reclaim(al) sets al's capacity to al's current size.
// requires: al is not NULL
// effects: may modify al, may reallocate heap memory
// note: if al's capacity is equal to al's size, calist_reclaim has no effect
void calist_reclaim(calist *al);

// calist_get(al, index) produces a constant pointer to the item
//   at the given index position in al.
// requires: al is not NULL and not empty
//           0 <= index < calist_size(al)
// notes:
//   - calist_get protects the contents of al by returning a
//     constant pointer, preventing direct modification
//   - call calist_get_mutable to modify an item in-place
const void *calist_get(const calist *al, size_t index);

// calist_get_mutable(al, index) produces a mutable pointer to the item
//   at the given index position in al.
// requires: al is not NULL and not empty
//           0 <= index < calist_size(al)
// warning: the returned pointer is borrowed. Do not free it; doing so will
//          result in a double free when removing the item or destroying al.
void *calist_get_mutable(const calist *al, size_t index);

// calist_set(al, index, new_item) replaces the old item at the given
//   index position in al with new_item.
// requires: al is not NULL and not empty
//           0 <= index < calist_size(al)
//           new_item is not NULL
// effects: modifies al [replaces the old item]
void calist_set(const calist *al, size_t index, const void *new_item);

// calist_swap(al, i, j) swaps the items at index positions i and j in al.
// requires: al is not NULL and not empty
//           0 <= i < calist_size(al)
//           0 <= j < calist_size(al)
// effects: modifies al
void calist_swap(const calist *al, size_t i, size_t j);

// calist_append(al, item) adds item to the back of al.
// requires: al and item are not NULL
// effects: modifies al, allocates heap memory
void calist_append(calist *al, const void *item);

// calist_append_all(al, src) adds all items in src to the back of al.
// requires: al and src are not NULL and have the same type
// effects: modifies al, allocates heap memory
void calist_append_all(calist *al, const calist *src);

// calist_insert(al, index, item) inserts item before the given index
//   position in al (shifting existing items to the right).
// requires: al and item are not NULL
//           0 <= index <= calist_size(al)
// effects: modifies al, allocates heap memory
void calist_insert(calist *al, size_t index, const void *item);

// calist_insert_front(al, item) inserts item to the beginning of al.
// requires: al and item are not NULL
// effects: modifies al, allocates heap memory
void calist_insert_front(calist *al, const void *item);

// calist_insert_all(al, index, src) inserts all items in src before the
//   given index position in al.
// requires: al and src are not NULL and have the same type
//           0 <= index <= calist_size(al)
// effects: modifies al, allocates heap memory
void calist_insert_all(calist *al, size_t index, const calist *src);

// calist_pop(al, index) removes the item at the given index position in al.
// requires: al is not NULL and not empty
//           0 <= index < calist_size(al)
// effects: modifies al, frees heap memory
void calist_pop(calist *al, size_t index);

// calist_remove(al, item) removes the first occurrence of item in al.
// requires: al is not NULL and not empty
//           item is not NULL
// effects: modifies al, frees heap memory
// note: returns the index position of item in al
//       and CALIST_INDEX_NOT_FOUND otherwise
size_t calist_remove(calist *al, const void *item);

// calist_remove_last(al, item) removes the last occurrence of item in al.
// requires: al is not NULL and not empty
//           item is not NULL
// effects: modifies al, frees heap memory
// note: returns the index position of item in al
//       and CALIST_INDEX_NOT_FOUND otherwise
size_t calist_remove_last(calist *al, const void *item);

// calist_remove_all(al, item) removes all occurrences of item in al.
// requires: al is not NULL and not empty
//           item is not NULL
// effects: modifies al, frees heap memory
// note: returns the number of items removed
size_t calist_remove_all(calist *al, const void *item);

// calist_remove_if(al, pred, args) removes all items in al that satisfy
//   pred, where args provides additional arguments to the pred function.
// requires: al is not NULL and not empty
//           pred is not NULL
// effects: modifies al, frees heap memory
// note: returns the number of items removed
size_t calist_remove_if(calist *al, calist_pred pred, const void *args);

// calist_remove_range(al, from_index, to_index) removes all items in al
//   from index position from_index to (to_index - 1), inclusive.
// requires: al is not NULL and not empty
//           0 <= from_index < calist_size(al)
//           from_index <= to_index <= calist_size(al)
// effects: modifies al, frees heap memory
void calist_remove_range(calist *al, size_t from_index, size_t to_index);

// calist_contains(al, item) produces true if al contains item
//   and false otherwise.
// requires: al and item are not NULL
bool calist_contains(const calist *al, const void *item);

// calist_index(al, item) produces the first index position of item in al
//   and CALIST_INDEX_NOT_FOUND if item is not in al.
// requires: al and item are not NULL
size_t calist_index(const calist *al, const void *item);

// calist_index_last(al, item) produces the last index position of item
//   in al and CALIST_INDEX_NOT_FOUND if item is not in al.
// requires: al and item are not NULL
size_t calist_index_last(const calist *al, const void *item);

// calist_index_all(al, item) produces a calist of cvalue_size_t() containing
//   the index positions of all occurrences of item in al.
// requires: al and item are not NULL
// effects: allocates heap memory [caller must free with calist_destroy]
calist *calist_index_all(const calist *al, const void *item);

// calist_index_all_if(al, pred, args) produces a calist of cvalue_size_t()
//   containing the index positions of all items in al that satisfy pred,
//   where args provides additional arguments to the pred function.
// requires: al and pred are not NULL
// effects: allocates heap memory [caller must free with calist_destroy]
calist *calist_index_all_if(const calist *al, calist_pred pred,
                            const void *args);

// calist_count(al, item) produces the total number of occurrences
//   of item in al.
// requires: al and item are not NULL
size_t calist_count(const calist *al, const void *item);

// calist_replace(al, old_item, new_item) replaces the first occurrence of
//   old_item in al with new_item.
// requires: al, old_item, and new_item are not NULL
// effects: may modify al [replaces old_item if found]
// note: returns the index position of old_item, or
//       CALIST_INDEX_NOT_FOUND if old_item is not in al
size_t calist_replace(const calist *al, const void *old_item,
                      const void *new_item);

// calist_replace_last(al, old_item, new_item) replaces the last occurrence
//   of old_item in al with new_item.
// requires: al, old_item, and new_item are not NULL
// effects: may modify al [replaces old_item if found]
// note: returns the index position of old_item, or
//       CALIST_INDEX_NOT_FOUND if old_item is not in al
size_t calist_replace_last(const calist *al, const void *old_item,
                           const void *new_item);

// calist_replace_all(al, old_item, new_item) replaces all occurrences of
//   old_item in al with new_item.
// requires: al, old_item, and new_item are not NULL
// effects: may modify al [replaces old_item if found]
// note: returns the number of items replaced
size_t calist_replace_all(const calist *al, const void *old_item,
                          const void *new_item);

// calist_replace_if(al, new_item, pred, args) replaces all items in al
//   that satisfy pred with new_item, where args provides additional
//   arguments to the pred function.
// requires: al, new_item, and pred are not NULL
// effects: may modify al [replaces items that satisfy pred]
// note: returns the number of items replaced
size_t calist_replace_if(const calist *al, const void *new_item,
                         calist_pred pred, const void *args);

// calist_qsort(al) sorts al using the quicksort algorithm.
// requires: al is not NULL
// effects: modifies al
void calist_qsort(calist *al);

// calist_bsearch(al, item) produces any index position of item in al
//   using binary search and CALIST_INDEX_NOT_FOUND if item is not in al.
// requires: al and item are not NULL
//           al must be sorted in ascending order [not asserted]
size_t calist_bsearch(const calist *al, const void *item);

// calist_reverse(al) reverses the order of items in al.
// requires: al is not NULL
// effects: modifies al
void calist_reverse(const calist *al);

// calist_slice(al, from_index, to_index) creates a sublist of al containing
//   items from index position from_index to (to_index - 1), inclusive.
// requires: al is not NULL and not empty
//           0 <= from_index < calist_size(al)
//           from_index <= to_index <= calist_size(al)
// effects: allocates heap memory [caller must free with calist_destroy]
calist *calist_slice(const calist *al, size_t from_index, size_t to_index);

// calist_filter(al, pred, args) produces a calist containing items in al
//   filtered by pred, where args provides additional arguments to the
//   pred function.
// requires: al and pred are not NULL
// effects: allocates heap memory [caller must free with calist_destroy]
calist *calist_filter(const calist *al, calist_pred pred, const void *args);

// calist_foreach(al, map, args) applies map to each item in al, where args
//   provides additional arguments to the map function.
// requires: al and map are not NULL
// effects: may modify item [see calist_map documentation]
void calist_foreach(const calist *al, calist_map map, const void *args);

// calist_unique(al) produces a calist containing unique items in al,
//   preserving the original order.
// requires: al is not NULL
// effects: allocates heap memory [caller must free with calist_destroy]
calist *calist_unique(const calist *al);

// calist_remove_dup(al) removes all duplicate items from al, keeping only
//   the first occurrence of each item.
// requires: al is not NULL
// effects: modifies al, frees heap memory
// note: returns the number of duplicate items removed
size_t calist_remove_dup(calist *al);

#ifdef __cplusplus
}
#endif

#endif
