// The cerror module provides macro functions that terminate the program
//   in case of unrecoverable errors or violated conditions, captures
//   the error locations and prints a custom error message.

#ifndef CERROR_H
#define CERROR_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Invalid API use is intentionally fatal: precondition violations call
// abort(), while allocation failures call exit(EXIT_FAILURE). The container
// APIs do not return an error code for these programmer/configuration errors.

// ASSERT_MSG(cond, msg) prints msg and terminates the program immediately
//   if cond is false. If msg is NULL, then the printed message will be
//   "Terminating program".
// requires: ASSERT_MSG(bool, const char *)
// effects: may produce output, may exit program
#define ASSERT_MSG(cond, msg) do { \
  if ((cond)) break; \
  const char *final_msg = (msg) ? (msg) : "Terminating program"; \
  fprintf(stderr, "[Assertion Failed] %s:%d in %s(): %s\n", \
          __FILE__, __LINE__, __func__, (final_msg)); \
  abort(); \
} while (0)

// ASSERT_NOT_NULL(nonnull, name) asserts that nonnull is true;
//   if nonnull is false, then "<name> cannot be NULL!" will be printed.
//   If name is NULL, then name = the variable name of nonnull.
// requires: ASSERT_NOT_NULL(bool, const char *)
// effects: may produce output, may exit program
#define ASSERT_NOT_NULL(nonnull, name) do { \
  if ((nonnull)) break; \
  const char *final_name = (name) ? (name) : #nonnull; \
  fprintf(stderr, "[Assertion Failed] %s:%d in %s(): %s cannot be NULL!\n", \
          __FILE__, __LINE__, __func__, (final_name)); \
  abort(); \
} while (0)

// FATAL_ERROR(msg) prints a formatted error message and terminates 
//   the program immediately. If msg is NULL, then the printed message 
//   will be "Terminating program".
// requires: FATAL_ERROR(const char *)
// effects: produces output, exits program
#define FATAL_ERROR(msg) do { \
  const char *final_msg = (msg) ? (msg) : "Terminating program"; \
  fprintf(stderr, "[Fatal Error] %s:%d in %s(): %s\n", \
          __FILE__, __LINE__, __func__, (final_msg)); \
  exit(EXIT_FAILURE); \
} while (0)

// ALLOC_ERROR(type) prints a memory allocation error message for type and 
//   terminates the program immediately;
//   if type is NULL, then type = "object".
// requires: ALLOC_ERROR(const char *)
// effects: produces output, exits program
#define ALLOC_ERROR(type) do { \
  const char *final_type = (type) ? (type) : "object"; \
  fprintf(stderr, "[Memory Error] %s:%d in %s(): Cannot create or reallocate %s\n", \
          __FILE__, __LINE__, __func__, (final_type)); \
  exit(EXIT_FAILURE); \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
