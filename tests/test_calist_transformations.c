#include <stdlib.h>

#include "calist.h"
#include "ctype.h"

#include "test_helpers.h"

static bool is_even(const calist *list, const void *item, const void *args) {
  (void)list;
  (void)args;
  return *(const int *)item % 2 == 0;
}

static void increment(const calist *list, void *item, const void *args) {
  (void)list;
  *(int *)item += *(const int *)args;
}

static void test_replacement_and_sorting(void) {
  calist *list = calist_create(ctype_int());
  int values[] = {4, 2, 4, 6, 4, 1};
  for (size_t i = 0; i < sizeof(values) / sizeof(*values); ++i) {
    calist_append(list, &values[i]);
  }

  assert(calist_replace(list, WRAP_INT(4), WRAP_INT(10)) == 0);
  assert(calist_replace_last(list, WRAP_INT(4), WRAP_INT(11)) == 4);
  assert(calist_replace_all(list, WRAP_INT(2), WRAP_INT(20)) == 1);
  assert(calist_replace_all(list, WRAP_INT(99), WRAP_INT(0)) == 0);
  assert(calist_replace_if(list, WRAP_INT(0), is_even, NULL) == 4);
  assert_int_list(list, (int[]){0, 0, 0, 0, 11, 1}, 6);

  calist_qsort(list);
  assert_int_list(list, (int[]){0, 0, 0, 0, 1, 11}, 6);
  assert(calist_bsearch(list, WRAP_INT(11)) != CALIST_INDEX_NOT_FOUND);
  assert(calist_bsearch(list, WRAP_INT(99)) == CALIST_INDEX_NOT_FOUND);

  calist_reverse(list);
  assert_int_list(list, (int[]){11, 1, 0, 0, 0, 0}, 6);

  int increment_by = 1;
  calist_foreach(list, increment, &increment_by);
  assert_int_list(list, (int[]){12, 2, 1, 1, 1, 1}, 6);

  calist_destroy(list);
}

static void test_unique_and_removal(void) {
  calist *list = calist_create(ctype_int());
  int values[] = {12, 2, 1, 1, 1};
  for (size_t i = 0; i < sizeof(values) / sizeof(*values); ++i) {
    calist_append(list, &values[i]);
  }

  calist *unique = calist_unique(list);
  assert_int_list(unique, (int[]){12, 2, 1}, 3);
  assert(calist_remove_dup(list) == 2);
  assert_int_list(list, (int[]){12, 2, 1}, 3);
  calist_destroy(unique);

  calist_remove_range(list, 1, 3);
  assert_int_list(list, (int[]){12}, 1);
  calist_pop(list, 0);
  assert(calist_empty(list));

  calist_append(list, WRAP_INT(12));
  calist_append(list, WRAP_INT(1));
  assert(calist_remove(list, WRAP_INT(1)) == 1);
  assert(calist_remove_last(list, WRAP_INT(12)) == 0);
  assert(calist_empty(list));
  calist_destroy(list);
}

int main(void) {
  test_replacement_and_sorting();
  test_unique_and_removal();
  return EXIT_SUCCESS;
}
