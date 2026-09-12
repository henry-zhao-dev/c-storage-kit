#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ctype.h"

static size_t custom_dup_calls;
static size_t custom_destroy_calls;
static size_t custom_print_calls;

static void *duplicate_int(const void *item) {
  int *copy = malloc(sizeof(*copy));
  if (!copy) {
    return NULL;
  }
  ++custom_dup_calls;
  *copy = *(const int *)item;
  return copy;
}

static void destroy_int(void *item) {
  ++custom_destroy_calls;
  free(item);
}

static void print_int_without_output(const void *item) {
  (void)item;
  ++custom_print_calls;
}

static int compare_int(const void *first, const void *second) {
  const int first_value = *(const int *)first;
  const int second_value = *(const int *)second;
  return (first_value > second_value) - (first_value < second_value);
}

static void test_builtin_types(void) {
  const ctype *integer = ctype_int();

  assert(data_size(integer) == sizeof(int));
  assert(data_size(ctype_long()) == sizeof(long));
  assert(data_size(ctype_char()) == sizeof(char));
  assert(data_size(ctype_bool()) == sizeof(bool));
  assert(data_size(ctype_size_t()) == sizeof(size_t));
  assert(data_size(ctype_float()) == sizeof(float));
  assert(data_size(ctype_double()) == sizeof(double));
  assert(data_size(ctype_string()) == sizeof(char *));

  assert(ctype_equals(integer, ctype_int()));
  assert(!ctype_equals(integer, ctype_long()));
  assert(data_cmp(WRAP_INT(1), WRAP_INT(2), integer) < 0);
  assert(data_cmp(WRAP_INT(2), WRAP_INT(2), integer) == 0);
  assert(data_cmp(WRAP_INT(3), WRAP_INT(2), integer) > 0);
}

static void test_string_data(void) {
  char word[] = "hello";
  char *word_copy = data_dup(word, ctype_string());

  assert(word_copy != NULL);
  word[0] = 'j';
  assert(strcmp(word_copy, "hello") == 0);
  data_destroy(word_copy, ctype_string());
}

static void test_custom_type_callbacks(void) {
  custom_dup_calls = 0;
  custom_destroy_calls = 0;
  custom_print_calls = 0;

  ctype *custom = ctype_create(sizeof(int), duplicate_int, destroy_int,
                               print_int_without_output, compare_int);
  int value = 42;
  int *value_copy = data_dup(&value, custom);

  assert(value_copy != NULL);
  assert(value_copy != &value);
  assert(*value_copy == value);
  data_print(&value, custom);
  assert(custom_print_calls == 1);
  data_destroy(value_copy, custom);
  assert(custom_dup_calls == 1);
  assert(custom_destroy_calls == 1);

  ctype_destroy(custom);
  ctype_destroy(NULL);
}

int main(void) {
  test_builtin_types();
  test_string_data();
  test_custom_type_callbacks();
  return EXIT_SUCCESS;
}
