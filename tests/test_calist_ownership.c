#include <assert.h>
#include <stdlib.h>

#include "calist.h"
#include "ctype.h"

static size_t duplicate_calls;
static size_t destroy_calls;
static size_t print_calls;

static void *duplicate_int(const void *item) {
  int *copy = malloc(sizeof(*copy));
  if (!copy) {
    return NULL;
  }
  ++duplicate_calls;
  *copy = *(const int *)item;
  return copy;
}

static void destroy_int(void *item) {
  ++destroy_calls;
  free(item);
}

static void print_int_without_output(const void *item) {
  (void)item;
  ++print_calls;
}

static int compare_int(const void *first, const void *second) {
  const int first_value = *(const int *)first;
  const int second_value = *(const int *)second;
  return (first_value > second_value) - (first_value < second_value);
}

int main(void) {
  duplicate_calls = 0;
  destroy_calls = 0;
  print_calls = 0;

  ctype *custom = ctype_create(sizeof(int), duplicate_int, destroy_int,
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
  ctype_destroy(custom);
  return EXIT_SUCCESS;
}
