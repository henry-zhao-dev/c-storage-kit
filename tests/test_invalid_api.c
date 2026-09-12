#include <stdlib.h>
#include <string.h>

#include "calist.h"

int main(int argc, char **argv) {
  calist *list = calist_create(ctype_int());

  if (argc != 2) {
    calist_destroy(list);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "bounds") == 0) {
    (void)calist_get(list, 0);
  } else if (strcmp(argv[1], "empty-remove") == 0) {
    (void)calist_remove(list, WRAP_INT(1));
  } else if (strcmp(argv[1], "insert-bounds") == 0) {
    calist_insert(list, 1, WRAP_INT(1));
  } else if (strcmp(argv[1], "type-mismatch") == 0) {
    calist *other = calist_create(ctype_long());
    calist_append_all(list, other);
  }

  calist_destroy(list);
  return EXIT_FAILURE;
}
