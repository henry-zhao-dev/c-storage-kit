#include <assert.h>
#include <stdlib.h>

#include "calist.h"
#include "cvalue.h"

static size_t duplicate_calls;
static size_t destroy_calls;
static size_t print_calls;

static void *duplicate_int(const void *value) {
  int *copy = malloc(sizeof(*copy));
  if (!copy) {
    return NULL;
  }
  ++duplicate_calls;
  *copy = *(const int *)value;
  return copy;
}

static void destroy_int(void *value) {
  ++destroy_calls;
  free(value);
}

static void print_int_without_output(const void *value) {
  (void)value;
  ++print_calls;
}

static int compare_int(const void *value1, const void *value2) {
  const int value1_int = *(const int *)value1;
  const int value2_int = *(const int *)value2;
  return (value1_int > value2_int) - (value1_int < value2_int);
}

int main(void) {
  duplicate_calls = 0;
  destroy_calls = 0;
  print_calls = 0;

  cvalue *custom = cvalue_create(sizeof(int), duplicate_int, destroy_int,
                                 print_int_without_output, compare_int);
  int value = 42;
  int *value_copy = data_dup(&value, custom);

  assert(value_copy != NULL);
  assert(value_copy != &value);
  assert(*value_copy == value);
  data_print(value_copy, custom);
  assert(print_calls == 1);
  data_destroy(value_copy, custom);
  assert(destroy_calls == 1);

  calist *list = calist_create(custom);
  calist_append(list, &value);
  calist_set(list, 0, WRAP_INT(7));
  assert(*(const int *)calist_get(list, 0) == 7);
  calist_print(list);
  assert(duplicate_calls == 3);
  assert(print_calls == 2);

  calist_destroy(list);
  assert(destroy_calls == 3);
  cvalue_destroy(custom);
  return EXIT_SUCCESS;
}
