#include "cerror.h"
#include "cvalue.h"

#include <stdio.h>
#include <string.h>

struct cvalue {
  size_t size;
  void *(*dup)(const void *);
  void (*destroy)(void *);
  void (*print)(const void *);
  int (*cmp)(const void *, const void *);
};

// Helper function declaration
#define DEFINE_DUP(type)                       \
  static void *dup_##type(const void *value) { \
    ASSERT_NOT_NULL(value, NULL);              \
    void *copy = malloc(sizeof(type));         \
    if (!copy) {                               \
      return NULL;                             \
    }                                          \
    memcpy(copy, value, sizeof(type));         \
    return copy;                               \
  }

#define DEFINE_PRINT(type, format)              \
  static void print_##type(const void *value) { \
    ASSERT_NOT_NULL(value, NULL);               \
    const type *value_ptr = value;              \
    printf(format, *value_ptr);                 \
  }

#define DEFINE_CMP(type)                                              \
  static int cmp_##type(const void *value1, const void *value2) {     \
    ASSERT_NOT_NULL(value1, NULL);                                    \
    ASSERT_NOT_NULL(value2, NULL);                                    \
    const type *value1_ptr = value1;                                  \
    const type *value2_ptr = value2;                                  \
    return (*value1_ptr > *value2_ptr) - (*value1_ptr < *value2_ptr); \
  }

// === Integral types ===
DEFINE_DUP(int)
DEFINE_PRINT(int, "%d")
DEFINE_CMP(int)

DEFINE_DUP(long)
DEFINE_PRINT(long, "%ld")
DEFINE_CMP(long)

DEFINE_DUP(char)
DEFINE_PRINT(char, "%c")
DEFINE_CMP(char)

DEFINE_DUP(bool)
static void print_bool(const void *value); // Print string "true" or "false"
DEFINE_CMP(bool)

DEFINE_DUP(size_t)
DEFINE_PRINT(size_t, "%zu")
DEFINE_CMP(size_t)

// === Floating-point types ===
DEFINE_DUP(float)
DEFINE_PRINT(float, "%g")
DEFINE_CMP(float)

DEFINE_DUP(double)
DEFINE_PRINT(double, "%g")
DEFINE_CMP(double)

// === String type ===
static void *dup_string(const void *value);
static int cmp_string(const void *value1, const void *value2);
static void print_string(const void *value);

cvalue *cvalue_create(const size_t size, void *(*dup)(const void *),
                      void (*destroy)(void *), void (*print)(const void *),
                      int (*cmp)(const void *, const void *)) {
  ASSERT_NOT_NULL(dup, NULL);
  ASSERT_NOT_NULL(destroy, NULL);
  ASSERT_NOT_NULL(print, NULL);
  ASSERT_NOT_NULL(cmp, NULL);

  cvalue *type = malloc(sizeof(*type));
  if (!type) {
    ALLOC_ERROR("cvalue");
  }

  type->size = size;
  type->dup = dup;
  type->destroy = destroy;
  type->print = print;
  type->cmp = cmp;
  return type;
}

void cvalue_destroy(cvalue *type) {
  if (type) {
    free(type);
  }
}

bool cvalue_equals(const cvalue *t1, const cvalue *t2) {
  ASSERT_NOT_NULL(t1, "The first cvalue");
  ASSERT_NOT_NULL(t2, "The second cvalue");
  return (t1 == t2);
}

size_t data_size(const cvalue *type) {
  ASSERT_NOT_NULL(type, NULL);
  return type->size;
}

void *data_dup(const void *value, const cvalue *type) {
  ASSERT_NOT_NULL(value, NULL);
  ASSERT_NOT_NULL(type, NULL);
  return (value ? type->dup(value) : NULL);
}

void data_destroy(void *value, const cvalue *type) {
  ASSERT_NOT_NULL(type, NULL);
  if (value) {
    type->destroy(value);
  }
}

void data_print(const void *value, const cvalue *type) {
  ASSERT_NOT_NULL(value, NULL);
  ASSERT_NOT_NULL(type, NULL);
  if (value) {
    type->print(value);
  }
}

int data_cmp(const void *value1, const void *value2, const cvalue *type) {
  ASSERT_NOT_NULL(value1, NULL);
  ASSERT_NOT_NULL(value2, NULL);
  ASSERT_NOT_NULL(type, NULL);
  return type->cmp(value1, value2);
}

// === Integral types ===
const cvalue *cvalue_int(void) {
  static const cvalue int_type = {
      .size = sizeof(int),
      .dup = dup_int,
      .destroy = free,
      .print = print_int,
      .cmp = cmp_int,
  };
  return &int_type;
}

const cvalue *cvalue_long(void) {
  static const cvalue long_type = {
      .size = sizeof(long),
      .dup = dup_long,
      .destroy = free,
      .print = print_long,
      .cmp = cmp_long,
  };
  return &long_type;
}

const cvalue *cvalue_char(void) {
  static const cvalue char_type = {
      .size = sizeof(char),
      .dup = dup_char,
      .destroy = free,
      .print = print_char,
      .cmp = cmp_char,
  };
  return &char_type;
}

const cvalue *cvalue_bool(void) {
  static const cvalue bool_type = {
      .size = sizeof(bool),
      .dup = dup_bool,
      .destroy = free,
      .print = print_bool,
      .cmp = cmp_bool,
  };
  return &bool_type;
}

const cvalue *cvalue_size_t(void) {
  static const cvalue size_t_type = {
      .size = sizeof(size_t),
      .dup = dup_size_t,
      .destroy = free,
      .print = print_size_t,
      .cmp = cmp_size_t,
  };
  return &size_t_type;
}

// === Floating-point types ===
const cvalue *cvalue_float(void) {
  static const cvalue float_type = {
      .size = sizeof(float),
      .dup = dup_float,
      .destroy = free,
      .print = print_float,
      .cmp = cmp_float,
  };
  return &float_type;
}

const cvalue *cvalue_double(void) {
  static const cvalue double_type = {
      .size = sizeof(double),
      .dup = dup_double,
      .destroy = free,
      .print = print_double,
      .cmp = cmp_double,
  };
  return &double_type;
}

// === String type ===
const cvalue *cvalue_string(void) {
  static const cvalue string_type = {
      .size = sizeof(char *),
      .dup = dup_string,
      .destroy = free,
      .print = print_string,
      .cmp = cmp_string,
  };
  return &string_type;
}

// Helper function implementation
static void print_bool(const void *value) {
  ASSERT_NOT_NULL(value, NULL);
  const bool *value_ptr = value;
  printf(*value_ptr ? "true" : "false");
}

static void *dup_string(const void *value) {
  ASSERT_NOT_NULL(value, NULL);

  const char *value_ptr = value;
  const size_t value_len = strlen(value_ptr);

  char *dup_str = malloc(sizeof(char) * (value_len + 1));
  if (!dup_str) {
    return NULL;
  }
  strcpy(dup_str, value_ptr);
  return dup_str;
}

static int cmp_string(const void *value1, const void *value2) {
  ASSERT_NOT_NULL(value1, "The first string");
  ASSERT_NOT_NULL(value2, "The second string");
  const char *value1_ptr = value1;
  const char *value2_ptr = value2;
  return strcmp(value1_ptr, value2_ptr);
}

static void print_string(const void *value) {
  ASSERT_NOT_NULL(value, NULL);
  const char *value_ptr = value;
  printf("%s", value_ptr);
}
