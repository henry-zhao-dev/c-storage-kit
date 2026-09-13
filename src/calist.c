#include "calist.h"
#include "cerror.h"

#include <stdlib.h>

struct calist {
  void **data;
  const cvalue *type;
  size_t size;
  size_t capacity;
};

const size_t CALIST_INDEX_NOT_FOUND = SIZE_MAX;

static const size_t DEFAULT_INIT_CAPACITY = 1;

static const char *ASSERT_CALIST_NOT_EMPTY = "calist cannot be empty!";
static const char *ASSERT_CALIST_SAME_TYPE = "calist must have the same type!";
static const char *ASSERT_INDEX_BOUNDED =
    "index must be less than the size of calist!";
static const char *ASSERT_INDEX_BOUNDED_INCLUSIVE =
    "index must not exceed the size of calist!";
static const char *ASSERT_INDEX_END_AFTER_START =
    "The ending index cannot be less than the starting index!";

static const char *ERROR_ITEM_DUP = "Failed to duplicate item!";
static const char *ERROR_SIZE_OVERFLOW = "Requested storage size is too large!";

// Helper function declaration
static void qsort_range(calist *al, size_t first, size_t last);

static void check_pointer_array_size(const size_t count) {
  if (count > SIZE_MAX / sizeof(void *)) {
    FATAL_ERROR(ERROR_SIZE_OVERFLOW);
  }
}

static size_t next_capacity(const size_t capacity) {
  if (capacity == 0) {
    return 1;
  }
  if (capacity > SIZE_MAX / 2) {
    FATAL_ERROR(ERROR_SIZE_OVERFLOW);
  }
  return capacity * 2;
}

calist *calist_create(const cvalue *type) {
  ASSERT_NOT_NULL(type, NULL);
  return calist_create_size(type, DEFAULT_INIT_CAPACITY);
}

calist *calist_create_size(const cvalue *type, const size_t init_cap) {
  ASSERT_NOT_NULL(type, NULL);
  ASSERT_MSG(init_cap, "The initial capacity of calist cannot be zero!");
  check_pointer_array_size(init_cap);

  calist *al = malloc(sizeof(*al));
  if (!al) {
    ALLOC_ERROR("calist");
  }

  al->data = malloc(sizeof(*al->data) * init_cap);
  if (!al->data) {
    free(al);
    ALLOC_ERROR("calist with the given capacity");
  }

  al->type = type;
  al->size = 0;
  al->capacity = init_cap;
  return al;
}

void calist_destroy(calist *al) {
  if (!al)
    return;

  for (size_t i = 0; i < al->size; ++i) {
    data_destroy(al->data[i], al->type);
  }
  free(al->data);
  free(al);
}

void calist_clear(calist *al) {
  ASSERT_NOT_NULL(al, NULL);

  for (size_t i = 0; i < al->size; ++i) {
    data_destroy(al->data[i], al->type);
  }
  al->size = 0;
}

calist *calist_dup(const calist *al) {
  ASSERT_NOT_NULL(al, NULL);

  calist *al_copy = malloc(sizeof(*al_copy));
  if (!al_copy) {
    ALLOC_ERROR("calist");
  }

  al_copy->data = NULL;
  if (al->capacity > 0) {
    check_pointer_array_size(al->capacity);
    al_copy->data = malloc(sizeof(*al_copy->data) * al->capacity);
    if (!al_copy->data) {
      free(al_copy);
      ALLOC_ERROR("calist with the given capacity");
    }
  }

  for (size_t i = 0; i < al->size; ++i) {
    void *item = data_dup(al->data[i], al->type);
    if (!item) {
      FATAL_ERROR(ERROR_ITEM_DUP);
    }
    al_copy->data[i] = item;
  }

  al_copy->type = al->type;
  al_copy->size = al->size;
  al_copy->capacity = al->capacity;
  return al_copy;
}

void calist_print(const calist *al) {
  ASSERT_NOT_NULL(al, NULL);

  printf("[");
  for (size_t i = 0; i < al->size; ++i) {
    if (i != 0) {
      printf(", ");
    }
    data_print(al->data[i], al->type);
  }
  printf("]\n");
}

bool calist_equals(const calist *l1, const calist *l2) {
  ASSERT_NOT_NULL(l1, "The first calist");
  ASSERT_NOT_NULL(l2, "The second calist");

  if (l1->size != l2->size || !cvalue_equals(l1->type, l2->type)) {
    return false;
  }

  for (size_t i = 0; i < l1->size; ++i) {
    if (data_cmp(l1->data[i], l2->data[i], l1->type)) {
      return false;
    }
  }
  return true;
}

const cvalue *calist_type(const calist *al) {
  ASSERT_NOT_NULL(al, NULL);
  return al->type;
}

size_t calist_size(const calist *al) {
  ASSERT_NOT_NULL(al, NULL);
  return al->size;
}

bool calist_empty(const calist *al) {
  ASSERT_NOT_NULL(al, NULL);
  return (al->size == 0);
}

size_t calist_capacity(const calist *al) {
  ASSERT_NOT_NULL(al, NULL);
  return al->capacity;
}

void calist_reserve(calist *al, const size_t n) {
  ASSERT_NOT_NULL(al, NULL);

  if (n <= al->capacity)
    return;
  check_pointer_array_size(n);

  void **new_data = realloc(al->data, sizeof(void *) * n);
  if (!new_data) {
    FATAL_ERROR("Failed to reserve the given capacity!");
  }

  al->data = new_data;
  al->capacity = n;
}

void calist_reclaim(calist *al) {
  ASSERT_NOT_NULL(al, NULL);

  if (al->size == al->capacity)
    return;

  if (al->size == 0) {
    free(al->data);
    al->data = NULL;
    al->capacity = 0;
    return;
  }

  void **new_data = realloc(al->data, sizeof(void *) * al->size);
  if (!new_data) {
    FATAL_ERROR("Failed to reclaim the unused storage!");
  }

  al->data = new_data;
  al->capacity = al->size;
}

const void *calist_get(const calist *al, const size_t index) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_MSG(index < al->size, ASSERT_INDEX_BOUNDED);
  return al->data[index];
}

void *calist_get_mutable(const calist *al, const size_t index) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_MSG(index < al->size, ASSERT_INDEX_BOUNDED);
  return al->data[index];
}

void calist_set(const calist *al, const size_t index, const void *new_item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_MSG(index < al->size, ASSERT_INDEX_BOUNDED);
  ASSERT_NOT_NULL(new_item, "The new item");

  void *item_copy = data_dup(new_item, al->type);
  if (!item_copy) {
    FATAL_ERROR(ERROR_ITEM_DUP);
  }

  data_destroy(al->data[index], al->type);
  al->data[index] = item_copy;
}

void calist_swap(const calist *al, const size_t i, const size_t j) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_MSG(i < al->size, ASSERT_INDEX_BOUNDED);
  ASSERT_MSG(j < al->size, ASSERT_INDEX_BOUNDED);

  void *temp = al->data[i];
  al->data[i] = al->data[j];
  al->data[j] = temp;
}

void calist_append(calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);

  void *item_copy = data_dup(item, al->type);
  if (!item_copy) {
    FATAL_ERROR(ERROR_ITEM_DUP);
  }

  if (al->size == al->capacity) {
    calist_reserve(al, next_capacity(al->capacity));
  }
  al->data[al->size] = item_copy;
  ++al->size;
}

void calist_append_all(calist *al, const calist *src) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(src, NULL);
  ASSERT_MSG(cvalue_equals(src->type, al->type), ASSERT_CALIST_SAME_TYPE);

  const calist *source = src;
  calist *snapshot = NULL;
  if (src == al) {
    snapshot = calist_dup(src);
    source = snapshot;
  }

  if (source->size > SIZE_MAX - al->size) {
    FATAL_ERROR(ERROR_SIZE_OVERFLOW);
  }
  for (size_t i = 0; i < source->size; ++i) {
    calist_append(al, source->data[i]);
  }
  calist_destroy(snapshot);
}

void calist_insert(calist *al, const size_t index, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);
  ASSERT_MSG(index <= al->size, ASSERT_INDEX_BOUNDED_INCLUSIVE);

  void *item_copy = data_dup(item, al->type);
  if (!item_copy) {
    FATAL_ERROR(ERROR_ITEM_DUP);
  }

  if (al->size == al->capacity) {
    calist_reserve(al, next_capacity(al->capacity));
  }
  for (size_t i = al->size; i > index; --i) {
    al->data[i] = al->data[i - 1];
  }
  al->data[index] = item_copy;
  ++al->size;
}

void calist_insert_front(calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);
  calist_insert(al, 0, item);
}

void calist_insert_all(calist *al, const size_t index, const calist *src) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(src, NULL);
  ASSERT_MSG(cvalue_equals(src->type, al->type), ASSERT_CALIST_SAME_TYPE);
  ASSERT_MSG(index <= al->size, ASSERT_INDEX_BOUNDED_INCLUSIVE);

  if (src->size > SIZE_MAX - al->size) {
    FATAL_ERROR(ERROR_SIZE_OVERFLOW);
  }

  const calist *source = src;
  calist *snapshot = NULL;
  if (src == al) {
    snapshot = calist_dup(src);
    source = snapshot;
  }

  calist_reserve(al, al->size + source->size);
  // Shift elements backwards to make room
  for (size_t i = al->size; i-- > index;) {
    al->data[i + source->size] = al->data[i];
  }

  for (size_t i = 0; i < source->size; ++i) {
    al->data[index + i] = data_dup(source->data[i], al->type);
    if (!al->data[index + i]) {
      FATAL_ERROR(ERROR_ITEM_DUP);
    }
  }
  al->size += source->size;
  calist_destroy(snapshot);
}

void calist_pop(calist *al, const size_t index) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_MSG(index < al->size, ASSERT_INDEX_BOUNDED);

  data_destroy(al->data[index], al->type);
  for (size_t i = index; i < al->size - 1; ++i) {
    al->data[i] = al->data[i + 1];
  }
  --al->size;
}

size_t calist_remove(calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_NOT_NULL(item, NULL);

  for (size_t i = 0; i < al->size; ++i) {
    if (!data_cmp(item, al->data[i], al->type)) {
      calist_pop(al, i);
      return i;
    }
  }
  return CALIST_INDEX_NOT_FOUND;
}

size_t calist_remove_last(calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_NOT_NULL(item, NULL);

  for (size_t i = al->size; i-- > 0;) {
    if (!data_cmp(item, al->data[i], al->type)) {
      calist_pop(al, i);
      return i;
    }
  }
  return CALIST_INDEX_NOT_FOUND;
}

size_t calist_remove_all(calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_NOT_NULL(item, NULL);

  size_t total = 0;
  for (size_t i = al->size; i-- > 0;) {
    if (!data_cmp(item, al->data[i], al->type)) {
      calist_pop(al, i);
      ++total;
    }
  }
  return total;
}

size_t calist_remove_if(calist *al, calist_pred pred, const void *args) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_NOT_NULL(pred, NULL);

  size_t total = 0;
  for (size_t i = al->size; i-- > 0;) {
    if (pred(al, al->data[i], args)) {
      calist_pop(al, i);
      ++total;
    }
  }
  return total;
}

void calist_remove_range(calist *al, const size_t from_index,
                         const size_t to_index) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_MSG(from_index < al->size, ASSERT_INDEX_BOUNDED);
  ASSERT_MSG(to_index >= from_index, ASSERT_INDEX_END_AFTER_START);
  ASSERT_MSG(to_index <= al->size, ASSERT_INDEX_BOUNDED_INCLUSIVE);

  const size_t range = to_index - from_index;
  for (size_t i = from_index; i < to_index; ++i) {
    data_destroy(al->data[i], al->type);
  }
  for (size_t i = to_index; i < al->size; ++i) {
    al->data[i - range] = al->data[i];
  }
  al->size -= range;
}

bool calist_contains(const calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);
  return (calist_index(al, item) != CALIST_INDEX_NOT_FOUND);
}

size_t calist_index(const calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);

  for (size_t i = 0; i < al->size; ++i) {
    if (!data_cmp(item, al->data[i], al->type)) {
      return i;
    }
  }
  return CALIST_INDEX_NOT_FOUND;
}

size_t calist_index_last(const calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);

  if (al->size == 0) {
    return CALIST_INDEX_NOT_FOUND;
  }

  for (size_t i = al->size; i-- > 0;) {
    if (!data_cmp(item, al->data[i], al->type)) {
      return i;
    }
  }
  return CALIST_INDEX_NOT_FOUND;
}

calist *calist_index_all(const calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);

  calist *indices = calist_create(cvalue_size_t());
  for (size_t i = 0; i < al->size; ++i) {
    if (!data_cmp(item, al->data[i], al->type)) {
      calist_append(indices, &i);
    }
  }
  return indices;
}

calist *calist_index_all_if(const calist *al, calist_pred pred,
                            const void *args) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(pred, NULL);

  calist *indices = calist_create(cvalue_size_t());
  for (size_t i = 0; i < al->size; ++i) {
    if (pred(al, al->data[i], args)) {
      calist_append(indices, &i);
    }
  }
  return indices;
}

size_t calist_count(const calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);

  size_t total = 0;
  for (size_t i = 0; i < al->size; ++i) {
    if (!data_cmp(item, al->data[i], al->type)) {
      ++total;
    }
  }
  return total;
}

size_t calist_replace(const calist *al, const void *old_item,
                      const void *new_item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(old_item, "The original item");
  ASSERT_NOT_NULL(new_item, "The new item");

  const size_t index = calist_index(al, old_item);
  if (index != CALIST_INDEX_NOT_FOUND) {
    calist_set(al, index, new_item);
  }
  return index;
}

size_t calist_replace_last(const calist *al, const void *old_item,
                           const void *new_item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(old_item, "The original item");
  ASSERT_NOT_NULL(new_item, "The new item");

  const size_t index = calist_index_last(al, old_item);
  if (index != CALIST_INDEX_NOT_FOUND) {
    calist_set(al, index, new_item);
  }
  return index;
}

size_t calist_replace_all(const calist *al, const void *old_item,
                          const void *new_item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(old_item, "The original item");
  ASSERT_NOT_NULL(new_item, "The new item");

  size_t total = 0;
  for (size_t i = 0; i < al->size; ++i) {
    if (!data_cmp(old_item, al->data[i], al->type)) {
      calist_set(al, i, new_item);
      ++total;
    }
  }
  return total;
}

size_t calist_replace_if(const calist *al, const void *new_item,
                         const calist_pred pred, const void *args) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(new_item, "The replacement item");
  ASSERT_NOT_NULL(pred, NULL);

  size_t total = 0;
  for (size_t i = 0; i < al->size; ++i) {
    if (pred(al, al->data[i], args)) {
      calist_set(al, i, new_item);
      ++total;
    }
  }
  return total;
}

void calist_qsort(calist *al) {
  ASSERT_NOT_NULL(al, NULL);

  if (al->size > 0) {
    qsort_range(al, 0, al->size - 1);
  }
}

size_t calist_bsearch(const calist *al, const void *item) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(item, NULL);

  if (al->size == 0) {
    return CALIST_INDEX_NOT_FOUND;
  }

  size_t low = 0;
  size_t high = al->size;

  while (low < high) {
    const size_t mid = low + (high - low) / 2;
    const int cmp = data_cmp(al->data[mid], item, al->type);
    if (!cmp) {
      return mid;
    }
    if (cmp < 0) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return CALIST_INDEX_NOT_FOUND;
}

void calist_reverse(const calist *al) {
  ASSERT_NOT_NULL(al, NULL);
  for (size_t i = 0; i < al->size / 2; ++i) {
    calist_swap(al, i, al->size - i - 1);
  }
}

calist *calist_slice(const calist *al, const size_t from_index,
                     const size_t to_index) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_MSG(al->size > 0, ASSERT_CALIST_NOT_EMPTY);
  ASSERT_MSG(from_index < al->size, ASSERT_INDEX_BOUNDED);
  ASSERT_MSG(to_index >= from_index, ASSERT_INDEX_END_AFTER_START);
  ASSERT_MSG(to_index <= al->size, ASSERT_INDEX_BOUNDED_INCLUSIVE);

  calist *sub = calist_create(al->type);
  calist_reserve(sub, to_index - from_index);
  for (size_t i = from_index; i < to_index; ++i) {
    calist_append(sub, al->data[i]);
  }
  return sub;
}

calist *calist_filter(const calist *al, calist_pred pred, const void *args) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(pred, NULL);

  calist *filtered = calist_create(al->type);
  for (size_t i = 0; i < al->size; ++i) {
    if (pred(al, al->data[i], args)) {
      calist_append(filtered, al->data[i]);
    }
  }
  return filtered;
}

void calist_foreach(const calist *al, calist_map map, const void *args) {
  ASSERT_NOT_NULL(al, NULL);
  ASSERT_NOT_NULL(map, NULL);

  for (size_t i = 0; i < al->size; ++i) {
    map(al, al->data[i], args);
  }
}

calist *calist_unique(const calist *al) {
  ASSERT_NOT_NULL(al, NULL);

  calist *unique = calist_create(al->type);
  for (size_t i = 0; i < al->size; ++i) {
    if (!calist_contains(unique, al->data[i])) {
      calist_append(unique, al->data[i]);
    }
  }
  return unique;
}

size_t calist_remove_dup(calist *al) {
  ASSERT_NOT_NULL(al, NULL);

  size_t total = 0;
  for (size_t i = 0; i < al->size; ++i) {
    for (size_t j = al->size; j-- > i + 1;) {
      if (!data_cmp(al->data[j], al->data[i], al->type)) {
        calist_pop(al, j);
        ++total;
      }
    }
  }
  return total;
}

// Helper function implementation
static void qsort_range(calist *al, const size_t first, const size_t last) {
  ASSERT_NOT_NULL(al, NULL);

  if (last <= first) {
    return;
  }

  const void *pivot = al->data[first];
  size_t pos = last;

  for (size_t i = last; i > first; --i) {
    if (data_cmp(al->data[i], pivot, al->type) > 0) {
      calist_swap(al, i, pos);
      --pos;
    }
  }
  calist_swap(al, first, pos);

  // Prevent underflow when pos == 0
  if (pos > 0) {
    qsort_range(al, first, pos - 1);
  }
  qsort_range(al, pos + 1, last);
}
