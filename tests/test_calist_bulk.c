#include <stdlib.h>

#include "calist.h"
#include "ctype.h"

#include "test_helpers.h"

static void test_append_and_insert_all(void) {
  calist *source = calist_create(ctype_int());
  calist_append(source, WRAP_INT(8));
  calist_append(source, WRAP_INT(9));

  calist *list = calist_create(ctype_int());
  calist_append(list, WRAP_INT(1));
  calist_append(list, WRAP_INT(4));
  calist_append_all(list, source);
  assert_int_list(list, (int[]){1, 4, 8, 9}, 4);

  calist_insert_all(list, 1, source);
  assert_int_list(list, (int[]){1, 8, 9, 4, 8, 9}, 6);

  calist_destroy(list);
  calist_destroy(source);
}

static void test_self_append_and_insert(void) {
  calist *list = calist_create(ctype_int());
  calist_append(list, WRAP_INT(5));
  calist_append(list, WRAP_INT(6));

  calist_append_all(list, list);
  assert_int_list(list, (int[]){5, 6, 5, 6}, 4);
  calist_insert_all(list, 2, list);
  assert_int_list(list, (int[]){5, 6, 5, 6, 5, 6, 5, 6}, 8);

  calist_destroy(list);
}

int main(void) {
  test_append_and_insert_all();
  test_self_append_and_insert();
  return EXIT_SUCCESS;
}
