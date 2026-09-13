#include <stdbool.h>
#include <stdlib.h>

#include "calist.h"
#include "cvalue.h"

#include "test_helpers.h"

static bool is_even(const calist *list, const void *item, const void *args) {
  (void)list;
  (void)args;
  return *(const int *)item % 2 == 0;
}

static void test_search_and_count(void) {
  calist *list = calist_create(cvalue_int());
  const int values[] = {4, 2, 4, 6, 4, 1};
  for (size_t i = 0; i < sizeof(values) / sizeof(*values); ++i) {
    calist_append(list, &values[i]);
  }

  assert(calist_contains(list, WRAP_INT(2)));
  assert(!calist_contains(list, WRAP_INT(99)));
  assert(calist_index(list, WRAP_INT(4)) == 0);
  assert(calist_index_last(list, WRAP_INT(4)) == 4);
  assert(calist_index(list, WRAP_INT(99)) == CALIST_INDEX_NOT_FOUND);
  assert(calist_index_last(list, WRAP_INT(99)) == CALIST_INDEX_NOT_FOUND);
  assert(calist_count(list, WRAP_INT(4)) == 3);

  calist *indices = calist_index_all(list, WRAP_INT(4));
  assert_size_t_list(indices, (size_t[]){0, 2, 4}, 3);
  calist_destroy(indices);

  indices = calist_index_all_if(list, is_even, NULL);
  assert_size_t_list(indices, (size_t[]){0, 1, 2, 3, 4}, 5);
  calist_destroy(indices);

  calist_destroy(list);
}

static void test_filter_and_remove(void) {
  calist *list = calist_create(cvalue_int());
  const int values[] = {4, 2, 4, 6, 4, 1};
  for (size_t i = 0; i < sizeof(values) / sizeof(*values); ++i) {
    calist_append(list, &values[i]);
  }

  calist *filtered = calist_filter(list, is_even, NULL);
  assert_int_list(filtered, (int[]){4, 2, 4, 6, 4}, 5);
  calist_destroy(filtered);

  calist *evens_removed = calist_dup(list);
  assert(calist_remove_if(evens_removed, is_even, NULL) == 5);
  assert_int_list(evens_removed, (int[]){1}, 1);
  calist_destroy(evens_removed);

  assert(calist_remove_all(list, WRAP_INT(4)) == 3);
  assert_int_list(list, (int[]){2, 6, 1}, 3);

  calist *slice = calist_slice(list, 1, 3);
  assert_int_list(slice, (int[]){6, 1}, 2);
  calist_destroy(slice);
  calist_destroy(list);
}

int main(void) {
  test_search_and_count();
  test_filter_and_remove();
  return EXIT_SUCCESS;
}
