// The ctype module provides the ctype ADT, which is essential for 
//   implementing generic storage ADTs. The module also provides 
//   predefined ctype instances for common built-in data types 
//   (int, double, char, etc.)

#ifndef CTYPE_H
#define CTYPE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// A ctype describes a type for use in generic storage ADTs.
// attributes:
//   - size:    the size in bytes of the associated data
// methods:
//   - dup:     creates a deep copy of the given value (heap-allocated),
//              returns NULL if allocation fails
//   - destroy: frees a value previously created by dup
//   - print:   displays a human-readable representation of the value
//   - cmp:     compares two items, item1 and item2
//              returns 0 if equal, <0 if item1 < item2, >0 if item1 > item2
// ownership:
//   - A ctype does not own the values passed to its callbacks.
//   - ctype_create returns an owned descriptor; destroy it with
//     ctype_destroy. The built-in singleton descriptors must not be freed.
//   - dup must return a fresh heap allocation which destroy can release.
typedef struct ctype ctype;

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

// ctype_create(size, dup, destroy, print, cmp) creates a ctype 
//   with the given attributes (see ctype documentation above).
// requires: dup, destroy, print, cmp are not NULL
// effects: allocates heap memory [caller must free with ctype_destroy]
ctype *ctype_create(size_t size,
                    void *(*dup)(const void *),
                    void (*destroy)(void *),
                    void (*print)(const void *),
                    int (*cmp)(const void *, const void *));

// ctype_destroy(type) frees type from the heap memory.
// effects: frees heap memory [type becomes invalid]
void ctype_destroy(ctype *type);

// ctype_equals(t1, t2) produces true if t1 and t2 represent the same type
//   and false otherwise.
// requires: t1 and t2 are not NULL
// note: ctype_equals only compares addresses of t1 and t2
bool ctype_equals(const ctype *t1, const ctype *t2);

// data_size(type) produces the data size of type in bytes.
// requires: type is not NULL
size_t data_size(const ctype *type);

// data_dup(value, type) creates a deep copy of value in the heap memory
//   using the dup method of type.
// requires: type is not NULL
// effects: allocates heap memory [caller must free with data_destroy]
// note: the callback may return NULL on allocation failure; callers in this
//       library treat that as a fatal allocation error.
void *data_dup(const void *value, const ctype *type);

// data_destroy(value, type) frees value from the heap memory using
//   the destroy method of type.
// requires: type is not NULL
// effects: frees heap memory [value becomes invalid]
void data_destroy(void *value, const ctype *type);

// data_print(value, type) displays value using the print method of type.
// requires: type is not NULL
// effects: produces output [if value is not NULL]
void data_print(const void *value, const ctype *type);

// data_cmp(item1, item2, type) produces 0 if item1 and item2 are equal,
//   a negative integer if item1 < item2, and a positive integer if 
//   item1 > item2.
// requires: item1, item2, and type are not NULL
int data_cmp(const void *item1, const void *item2, const ctype *type);

// The following methods each produce a shared singleton ctype ADT
//   for common built-in C types.
// note: the returned pointer must not be freed
// === Integral types ===
const ctype *ctype_int(void);
const ctype *ctype_long(void);
const ctype *ctype_char(void);
const ctype *ctype_bool(void);
const ctype *ctype_size_t(void);
// === Floating-point types ===
const ctype *ctype_float(void);
const ctype *ctype_double(void);
// === String type ===
const ctype *ctype_string(void);

#ifdef __cplusplus
}
#endif

#endif
