// The cvalue module provides the cvalue ADT, which is essential for 
//   implementing generic storage ADTs. The module also provides 
//   predefined cvalue instances for common built-in data types 
//   (int, double, char, etc.)

#ifndef CVALUE_H
#define CVALUE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// A cvalue describes a type for use in generic storage ADTs.
// attributes:
//   - size:    the size in bytes of the associated data
// methods:
//   - dup:     creates a deep copy of the given value (heap-allocated),
//              returns NULL if allocation fails
//   - destroy: frees a value previously created by dup
//   - print:   displays a human-readable representation of the value
//   - cmp:     compares two values, value1 and value2
//              returns 0 if equal, <0 if value1 < value2, >0 if value1 > value2
// ownership:
//   - A cvalue does not own the values passed to its callbacks.
//   - cvalue_create returns an owned descriptor; destroy it with
//     cvalue_destroy. The built-in singleton descriptors must not be freed.
//   - dup must return a fresh heap allocation which destroy can release.
typedef struct cvalue cvalue;

// Wrap stack variables to temporary pointers for generic storage ADTs
// === Integral types ===
#define WRAP_INT(i) (&(int) {i})
#define WRAP_LONG(l) (&(long) {l})
#define WRAP_CHAR(c) (&(char) {c})
#define WRAP_BOOL(b) (&(bool) {b})
#define WRAP_SIZE_T(st) (&(size_t) {st})
// === Floating-point types ===
#define WRAP_FLOAT(f) (&(float) {f})
#define WRAP_DOUBLE(d) (&(double) {d})

// cvalue_create(size, dup, destroy, print, cmp) creates a cvalue 
//   with the given attributes (see cvalue documentation above).
// requires: dup, destroy, print, cmp are not NULL
// effects: allocates heap memory [caller must free with cvalue_destroy]
cvalue *cvalue_create(size_t size,
                      void *(*dup)(const void *),
                      void (*destroy)(void *),
                      void (*print)(const void *),
                      int (*cmp)(const void *, const void *));

// cvalue_destroy(type) frees type from the heap memory.
// effects: frees heap memory [type becomes invalid]
void cvalue_destroy(cvalue *type);

// cvalue_equals(t1, t2) produces true if t1 and t2 represent the same type
//   and false otherwise.
// requires: t1 and t2 are not NULL
// note: cvalue_equals only compares addresses of t1 and t2
bool cvalue_equals(const cvalue *t1, const cvalue *t2);

// data_size(type) produces the data size of type in bytes.
// requires: type is not NULL
size_t data_size(const cvalue *type);

// data_dup(value, type) creates a deep copy of value in the heap memory
//   using the dup method of type.
// requires: type is not NULL
// effects: allocates heap memory [caller must free with data_destroy]
// note: the callback may return NULL on allocation failure; callers in this
//       library treat that as a fatal allocation error.
void *data_dup(const void *value, const cvalue *type);

// data_destroy(value, type) frees value from the heap memory using
//   the destroy method of type.
// requires: type is not NULL
// effects: frees heap memory [value becomes invalid]
void data_destroy(void *value, const cvalue *type);

// data_print(value, type) displays value using the print method of type.
// requires: type is not NULL
// effects: produces output [if value is not NULL]
void data_print(const void *value, const cvalue *type);

// data_cmp(value1, value2, type) produces 0 if value1 and value2 are equal,
//   a negative integer if value1 < value2, and a positive integer if 
//   value1 > value2.
// requires: value1, value2, and type are not NULL
int data_cmp(const void *value1, const void *value2, const cvalue *type);

// The following methods each produce a shared singleton cvalue ADT
//   for common built-in C types.
// note: the returned pointer must not be freed
// === Integral types ===
const cvalue *cvalue_int(void);
const cvalue *cvalue_long(void);
const cvalue *cvalue_char(void);
const cvalue *cvalue_bool(void);
const cvalue *cvalue_size_t(void);
// === Floating-point types ===
const cvalue *cvalue_float(void);
const cvalue *cvalue_double(void);
// === String type ===
const cvalue *cvalue_string(void);

#ifdef __cplusplus
}
#endif

#endif
