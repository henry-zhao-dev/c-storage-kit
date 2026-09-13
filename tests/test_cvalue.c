#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cvalue.h"

static size_t custom_dup_calls;
static size_t custom_destroy_calls;
static size_t custom_print_calls;

static void *duplicate_int(const void *value) {
  int *copy = malloc(sizeof(*copy));
  if (!copy) {
    return NULL;
  }
  ++custom_dup_calls;
  *copy = *(const int *)value;
  return copy;
}

static void destroy_int(void *value) {
  ++custom_destroy_calls;
  free(value);
}

static void print_int_without_output(const void *value) {
  (void)value;
  ++custom_print_calls;
}

static int compare_int(const void *value1, const void *value2) {
  const int value1_int = *(const int *)value1;
  const int value2_int = *(const int *)value2;
  return (value1_int > value2_int) - (value1_int < value2_int);
}

static void test_builtin_types(void) {
  const cvalue *integer = cvalue_int();

  assert(data_size(integer) == sizeof(int));
  assert(data_size(cvalue_long()) == sizeof(long));
  assert(data_size(cvalue_char()) == sizeof(char));
  assert(data_size(cvalue_bool()) == sizeof(bool));
  assert(data_size(cvalue_size_t()) == sizeof(size_t));
  assert(data_size(cvalue_float()) == sizeof(float));
  assert(data_size(cvalue_double()) == sizeof(double));
  assert(data_size(cvalue_string()) == sizeof(char *));

  assert(cvalue_equals(integer, cvalue_int()));
  assert(!cvalue_equals(integer, cvalue_long()));
  assert(data_cmp(WRAP_INT(1), WRAP_INT(2), integer) < 0);
  assert(data_cmp(WRAP_INT(2), WRAP_INT(2), integer) == 0);
  assert(data_cmp(WRAP_INT(3), WRAP_INT(2), integer) > 0);
}

static void test_string_data(void) {
  char word[] = "hello";
  char *word_copy = data_dup(word, cvalue_string());

  assert(word_copy != NULL);
  word[0] = 'j';
  assert(strcmp(word_copy, "hello") == 0);
  data_destroy(word_copy, cvalue_string());
}

static void test_custom_type_callbacks(void) {
  custom_dup_calls = 0;
  custom_destroy_calls = 0;
  custom_print_calls = 0;

  cvalue *custom = cvalue_create(sizeof(int), duplicate_int, destroy_int,
                               print_int_without_output, compare_int);
  const int value = 42;
  int *value_copy = data_dup(&value, custom);

  assert(value_copy != NULL);
  assert(value_copy != &value);
  assert(*value_copy == value);
  data_print(&value, custom);
  assert(custom_print_calls == 1);
  data_destroy(value_copy, custom);
  assert(custom_dup_calls == 1);
  assert(custom_destroy_calls == 1);

  cvalue_destroy(custom);
  cvalue_destroy(NULL);
}

int main(void) {
  test_builtin_types();
  test_string_data();
  test_custom_type_callbacks();
  return EXIT_SUCCESS;
}
