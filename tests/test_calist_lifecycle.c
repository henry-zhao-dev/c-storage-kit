#include <assert.h>
#include <stdlib.h>

#include "calist.h"
#include "cvalue.h"

#include "test_helpers.h"

static void test_creation_and_access(void) {
  calist *list = calist_create_size(cvalue_int(), 2);
  assert(calist_type(list) == cvalue_int());
  assert(calist_size(list) == 0);
  assert(calist_empty(list));
  assert(calist_capacity(list) == 2);

  calist_reserve(list, 8);
  assert(calist_capacity(list) == 8);
  calist_reserve(list, 4);
  assert(calist_capacity(list) == 8);

  int values[] = {3, 1, 3, 2, 5};
  for (size_t i = 0; i < sizeof(values) / sizeof(*values); ++i) {
    calist_append(list, &values[i]);
  }
  assert_int_list(list, (int[]){3, 1, 3, 2, 5}, 5);

  values[0] = 99;
  assert(*(const int *)calist_get(list, 0) == 3);
  *(int *)calist_get_mutable(list, 0) = 6;
  calist_set(list, 1, WRAP_INT(4));
  calist_swap(list, 0, 4);
  calist_insert_front(list, WRAP_INT(7));
  calist_insert(list, 3, WRAP_INT(9));
  assert_int_list(list, (int[]){7, 5, 4, 9, 3, 2, 6}, 7);

  calist *copy = calist_dup(list);
  assert(calist_equals(list, copy));
  *(int *)calist_get_mutable(copy, 0) = 100;
  assert(!calist_equals(list, copy));

  calist_print(list);
  calist_destroy(copy);
  calist_destroy(list);
}

static void test_clear_and_reclaim(void) {
  calist *list = calist_create_size(cvalue_int(), 8);
  calist_append(list, WRAP_INT(1));
  calist_append(list, WRAP_INT(2));
  size_t capacity = calist_capacity(list);

  calist_clear(list);
  assert(calist_empty(list));
  assert(calist_capacity(list) == capacity);
  calist_reclaim(list);
  assert(calist_capacity(list) == 0);

  calist_append(list, WRAP_INT(3));
  assert(calist_capacity(list) >= 1);
  calist_destroy(list);
  calist_destroy(NULL);
}

static void test_aliases(void) {
  calist *list = calist_create(cvalue_int());
  calist_add(list, WRAP_INT(1));
  calist_insert_back(list, WRAP_INT(2));
  assert(calist_length(list) == 2);
  assert(calist_find(list, WRAP_INT(1)) == 0);
  assert(calist_find_last(list, WRAP_INT(2)) == 1);

  calist *source = calist_create(cvalue_int());
  calist_add(source, WRAP_INT(3));
  calist_add_all(list, source);
  assert_int_list(list, (int[]){1, 2, 3}, 3);

  calist_destroy(source);
  calist_destroy(list);
}

int main(void) {
  test_creation_and_access();
  test_clear_and_reclaim();
  test_aliases();
  return EXIT_SUCCESS;
}
