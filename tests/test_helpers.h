#ifndef CSTORAGE_KIT_TEST_HELPERS_H
#define CSTORAGE_KIT_TEST_HELPERS_H

#include <assert.h>

#include "calist.h"
#include "ctype.h"

static void assert_int_list(const calist *list,
                            const int *expected,
                            size_t size) {
  assert(calist_size(list) == size);
  for (size_t i = 0; i < size; ++i) {
    assert(*(const int *)calist_get(list, i) == expected[i]);
  }
}

static void assert_size_t_list(const calist *list,
                               const size_t *expected,
                               size_t size) {
  assert(calist_size(list) == size);
  assert(ctype_equals(calist_type(list), ctype_size_t()));
  for (size_t i = 0; i < size; ++i) {
    assert(*(const size_t *)calist_get(list, i) == expected[i]);
  }
}

#endif
