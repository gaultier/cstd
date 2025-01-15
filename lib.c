#ifndef CSTD_LIB_C
#define CSTD_LIB_C
#define _POSIX_C_SOURCE 200809L
#define __XSI_VISIBLE 600
#define __BSD_VISIBLE 1
#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE 1
#include "sha1.c"
#include <errno.h>
#include <fcntl.h> // TODO: Windows.
#include <stdarg.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> // TODO: Windows.
#include <sys/stat.h> // TODO: Windows.
#include <time.h>
#include <unistd.h> // TODO: Windows.

#ifndef PG_MIN
#define PG_MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define PG_KiB (1024ULL)
#define PG_MiB (1024ULL * PG_Ki)
#define PG_GiB (1024ULL * PG_Mi)
#define PG_TiB (1024ULL * PG_Gi)

#define PG_Microseconds (1000ULL)
#define PG_Milliseconds (1000ULL * PG_Microseconds)
#define PG_Seconds (1000ULL * PG_Milliseconds)

#define PG_DYN(T)                                                              \
  typedef struct {                                                             \
    T *data;                                                                   \
    u64 len, cap;                                                              \
  }

#define PG_SLICE(T)                                                            \
  typedef struct {                                                             \
    T *data;                                                                   \
    u64 len;                                                                   \
  }

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t u128;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define PG_RESULT(T)                                                           \
  typedef struct {                                                             \
    PgError err;                                                               \
    T res;                                                                     \
  }

#define PG_OK(T)                                                               \
  typedef struct {                                                             \
    T res;                                                                     \
    bool ok;                                                                   \
  }

typedef u32 PgError;
#define PG_ERR_OK ((u32)0)
#define PG_ERR_INVALID_VALUE ((u32)0x701)
#define PG_ERR_OUT_OF_MEMORY ((u32)0x702)

PG_RESULT(u64) Pgu64Result;

PG_DYN(u8) Pgu8Dyn;
PG_SLICE(u8) Pgu8Slice;
typedef Pgu8Slice PgString;

#define PG_STATIC_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define PG_CLAMP(min, n, max) ((n) < (min) ? (min) : (n) > (max) ? (max) : n)

#define PG_SUB_SAT(a, b) ((a) > (b) ? ((a) - (b)) : 0)

[[maybe_unused]] static void pg_stacktrace_print(const char *file, int line,
                                                 const char *function) {
  fprintf(stderr, "%s:%d:%s\n", file, line, function);
  // TODO
}

#define PG_ASSERT(x)                                                           \
  (x) ? (0)                                                                    \
      : (pg_stacktrace_print(__FILE__, __LINE__, __FUNCTION__),                \
         __builtin_trap(), 0)

#define PG_C_ARRAY_AT_PTR(arr, len, idx)                                       \
  (((i64)(idx) >= (i64)(len)) ? (__builtin_trap(), &(arr)[0])                  \
                              : (PG_ASSERT(nullptr != (arr)), (&(arr)[idx])))

#define PG_C_ARRAY_AT(arr, len, idx) (*PG_C_ARRAY_AT_PTR(arr, len, idx))

#define PG_SLICE_AT(s, idx) (PG_C_ARRAY_AT((s).data, (s).len, idx))

#define PG_SLICE_AT_PTR(s, idx) (PG_C_ARRAY_AT_PTR((s)->data, (s)->len, idx))

#define PG_SLICE_MAKE(T, l, arena)                                             \
  ((T##Slice){.data = pg_arena_new(arena, T, l), .len = l})

#define PG_SLICE_SWAP_REMOVE(s, idx)                                           \
  do {                                                                         \
    if ((i64)(idx) >= (i64)((s)->len)) {                                       \
      __builtin_trap();                                                        \
    }                                                                          \
    *(PG_C_ARRAY_AT_PTR((s)->data, (s)->len, idx)) =                           \
        PG_C_ARRAY_AT((s)->data, (s)->len, (s)->len - 1);                      \
    (s)->len -= 1;                                                             \
  } while (0)

[[maybe_unused]] [[nodiscard]] static bool pg_character_is_hex_digit(u8 c) {
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') ||
         ('a' <= c && c <= 'f');
}

[[maybe_unused]] [[nodiscard]] static bool pg_character_is_alphabetical(u8 c) {
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

[[maybe_unused]] [[nodiscard]] static bool pg_character_is_numeric(u8 c) {
  return ('0' <= c && c <= '9');
}

[[maybe_unused]] [[nodiscard]] static bool pg_character_is_alphanumeric(u8 c) {
  return pg_character_is_numeric(c) || pg_character_is_alphabetical(c);
}

[[maybe_unused]] [[nodiscard]] static u8 pg_character_from_hex(u8 c) {
  PG_ASSERT(pg_character_is_hex_digit(c));

  if ('0' <= c && c <= '9') {
    return c - '0';
  }

  if ('A' <= c && c <= 'F') {
    return 10 + c - 'A';
  }

  if ('a' <= c && c <= 'f') {
    return 10 + c - 'a';
  }

  PG_ASSERT(false);
}

PG_RESULT(PgString) PgStringResult;
PG_OK(PgString) PgStringOk;
PG_SLICE(PgString) PgStringSlice;

PG_RESULT(PgStringSlice) PgStringSliceResult;

#define PG_SLICE_IS_EMPTY(s)                                                   \
  (((s).len == 0) ? true : (PG_ASSERT(nullptr != (s).data), false))

#define PG_S(s) ((PgString){.data = (u8 *)s, .len = sizeof(s) - 1})

[[maybe_unused]] [[nodiscard]] static bool pg_string_is_empty(PgString s) {
  return PG_SLICE_IS_EMPTY(s);
}

[[maybe_unused]] [[nodiscard]] static bool
pg_string_is_alphabetical(PgString s) {
  for (u64 i = 0; i < s.len; i++) {
    u8 c = PG_SLICE_AT(s, i);
    if (!pg_character_is_alphabetical(c)) {
      return false;
    }
  }
  return true;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_trim_left(PgString s,
                                                                   u8 c) {
  PgString res = s;

  for (u64 s_i = 0; s_i < s.len; s_i++) {
    PG_ASSERT(s.data != nullptr);
    if (PG_C_ARRAY_AT(s.data, s.len, s_i) != c) {
      return res;
    }

    res.data += 1;
    res.len -= 1;
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_trim_right(PgString s,
                                                                    u8 c) {
  PgString res = s;

  for (i64 s_i = (i64)s.len - 1; s_i >= 0; s_i--) {
    PG_ASSERT(s.data != nullptr);
    if (PG_C_ARRAY_AT(s.data, s.len, s_i) != c) {
      return res;
    }

    res.len -= 1;
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_trim(PgString s,
                                                              u8 c) {
  PgString res = pg_string_trim_left(s, c);
  res = pg_string_trim_right(res, c);

  return res;
}

typedef struct {
  PgString s;
  PgString sep;
} PgSplitIterator;

[[maybe_unused]] [[nodiscard]] static PgSplitIterator
pg_string_split_string(PgString s, PgString sep) {
  return (PgSplitIterator){.s = s, .sep = sep};
}

[[maybe_unused]] [[nodiscard]] static i64
pg_string_indexof_byte(PgString haystack, u8 needle) {
  if (PG_SLICE_IS_EMPTY(haystack)) {
    return -1;
  }

  const u8 *res = memchr(haystack.data, needle, haystack.len);
  if (res == nullptr) {
    return -1;
  }

  return res - haystack.data;
}

#define PG_SLICE_RANGE(s, start, end)                                          \
  ((typeof((s))){                                                              \
      .data = (s).len == PG_CLAMP(0, start, (s).len)                           \
                  ? nullptr                                                    \
                  : PG_C_ARRAY_AT_PTR((s).data, (s).len,                       \
                                      PG_CLAMP(0, start, (s).len)),            \
      .len =                                                                   \
          PG_SUB_SAT(PG_CLAMP(0, end, (s).len), PG_CLAMP(0, start, (s).len)),  \
  })

#define PG_SLICE_RANGE_START(s, start) PG_SLICE_RANGE(s, start, (s).len)

[[maybe_unused]] [[nodiscard]] static bool pg_string_eq(PgString a,
                                                        PgString b) {
  if (a.len == 0 && b.len == 0) {
    return true;
  }

  if (a.data == nullptr && b.data == nullptr && a.len == b.len) {
    return true;
  }
  if (a.data == nullptr) {
    return false;
  }
  if (b.data == nullptr) {
    return false;
  }

  if (a.len != b.len) {
    return false;
  }

  PG_ASSERT(a.data != nullptr);
  PG_ASSERT(b.data != nullptr);
  PG_ASSERT(a.len == b.len);

  return memcmp(a.data, b.data, a.len) == 0;
}

[[maybe_unused]] [[nodiscard]] static i64
pg_string_indexof_string(PgString haystack, PgString needle) {
  if (haystack.data == nullptr) {
    return -1;
  }

  if (haystack.len == 0) {
    return -1;
  }

  if (needle.data == nullptr) {
    return -1;
  }

  if (needle.len == 0) {
    return -1;
  }

  if (needle.len > haystack.len) {
    return -1;
  }

  PG_ASSERT(nullptr != haystack.data);
  PG_ASSERT(nullptr != needle.data);
  void *ptr = memmem(haystack.data, haystack.len, needle.data, needle.len);
  if (nullptr == ptr) {
    return -1;
  }

  u64 res = (u64)((u8 *)ptr - haystack.data);
  PG_ASSERT(res < haystack.len);
  return (i64)res;
}

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_string_split_next(PgSplitIterator *it) {
  if (PG_SLICE_IS_EMPTY(it->s)) {
    return (PgStringOk){0};
  }

  for (u64 _i = 0; _i < it->s.len; _i++) {
    i64 idx = pg_string_indexof_string(it->s, it->sep);
    if (-1 == idx) {
      // Last element.
      PgStringOk res = {.res = it->s, .ok = true};
      it->s = (PgString){0};
      return res;
    }

    if (idx == 0) { // Multiple contiguous separators.
      it->s = PG_SLICE_RANGE_START(it->s, (u64)idx + it->sep.len);
      continue;
    } else {
      PgStringOk res = {.res = PG_SLICE_RANGE(it->s, 0, (u64)idx), .ok = true};
      it->s = PG_SLICE_RANGE_START(it->s, (u64)idx + it->sep.len);

      return res;
    }
  }
  return (PgStringOk){0};
}

typedef struct {
  PgString left, right;
  bool consumed;
} PgStringPairConsume;

[[maybe_unused]] [[nodiscard]] static PgStringPairConsume
pg_string_consume_until_byte_excl(PgString haystack, u8 needle) {
  PgStringPairConsume res = {0};

  i64 idx = pg_string_indexof_byte(haystack, needle);
  if (-1 == idx) {
    res.left = haystack;
    res.right = haystack;
    return res;
  }

  res.left = PG_SLICE_RANGE(haystack, 0, (u64)idx);
  res.right = PG_SLICE_RANGE_START(haystack, (u64)idx);
  res.consumed = true;

  PG_ASSERT(needle == PG_SLICE_AT(res.right, 0));
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringPairConsume
pg_string_consume_until_byte_incl(PgString haystack, u8 needle) {
  PgStringPairConsume res = {0};

  i64 idx = pg_string_indexof_byte(haystack, needle);
  if (-1 == idx) {
    res.left = haystack;
    res.right = haystack;
    return res;
  }

  res.left = PG_SLICE_RANGE(haystack, 0, (u64)idx);
  res.right = PG_SLICE_RANGE_START(haystack, (u64)idx + 1);
  res.consumed = true;

  return res;
}

typedef struct {
  PgString left, right;
  bool consumed;
  u8 matched;
} PgStringPairConsumeAny;

[[maybe_unused]] [[nodiscard]] static PgStringPairConsumeAny
pg_string_consume_until_any_byte_incl(PgString haystack, PgString needles) {
  PgStringPairConsumeAny res = {0};

  for (u64 i = 0; i < needles.len; i++) {
    u8 needle = PG_SLICE_AT(needles, i);
    PgStringPairConsume res_consume =
        pg_string_consume_until_byte_incl(haystack, needle);
    if (res_consume.consumed) {
      res.left = res_consume.left;
      res.right = res_consume.right;
      res.consumed = res_consume.consumed;
      res.matched = needle;
      return res;
    }
  }
  // Not found.
  res.left = haystack;
  res.right = haystack;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringPairConsumeAny
pg_string_consume_until_any_byte_excl(PgString haystack, PgString needles) {
  PgStringPairConsumeAny res = {0};

  for (u64 i = 0; i < needles.len; i++) {
    u8 needle = PG_SLICE_AT(needles, i);
    PgStringPairConsume res_consume =
        pg_string_consume_until_byte_excl(haystack, needle);
    if (res_consume.consumed) {
      res.left = res_consume.left;
      res.right = res_consume.right;
      res.consumed = res_consume.consumed;
      res.matched = needle;
      return res;
    }
  }
  // Not found.
  res.left = haystack;
  res.right = haystack;
  return res;
}

[[maybe_unused]] [[nodiscard]] static i64
pg_string_indexof_any_byte(PgString haystack, PgString needle) {
  for (i64 i = 0; i < (i64)haystack.len; i++) {
    u8 c_h = PG_SLICE_AT(haystack, i);

    for (i64 j = 0; j < (i64)needle.len; j++) {
      u8 c_n = PG_SLICE_AT(needle, j);
      if (c_h == c_n) {
        return i;
      }
    }
  }
  return -1;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_string_starts_with(PgString haystack, PgString needle) {
  if (haystack.len == 0 || haystack.len < needle.len) {
    return false;
  }
  PG_ASSERT(nullptr != haystack.data);
  PG_ASSERT(nullptr != needle.data);

  PgString start = PG_SLICE_RANGE(haystack, 0, needle.len);

  return pg_string_eq(needle, start);
}

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_string_consume_byte(PgString haystack, u8 needle) {
  PgStringOk res = {0};

  if (haystack.len == 0) {
    return res;
  }
  if (haystack.data[0] != needle) {
    return res;
  }

  res.ok = true;
  res.res = PG_SLICE_RANGE_START(haystack, 1UL); // Remaining.
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_string_consume_string(PgString haystack, PgString needle) {
  PgStringOk res = {0};
  res.res = haystack;

  for (u64 i = 0; i < needle.len; i++) {
    res = pg_string_consume_byte(res.res, PG_SLICE_AT(needle, i));
    if (!res.ok) {
      return res;
    }
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_string_consume_any_string(PgString haystack, PgStringSlice needles) {
  PgStringOk res = {0};
  res.res = haystack;

  for (u64 i = 0; i < needles.len; i++) {
    res = pg_string_consume_string(res.res, PG_SLICE_AT(needles, i));
    if (res.ok) {
      return res;
    }
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_string_ends_with(PgString haystack, PgString needle) {
  if (haystack.len == 0 || haystack.len < needle.len) {
    return false;
  }
  PG_ASSERT(nullptr != haystack.data);
  PG_ASSERT(nullptr != needle.data);

  PgString end = PG_SLICE_RANGE_START(haystack, haystack.len - needle.len);

  return pg_string_eq(needle, end);
}

typedef struct {
  u64 n;
  bool present;
  PgString remaining;
} PgParseNumberResult;

[[maybe_unused]] [[nodiscard]] static PgParseNumberResult
pg_string_parse_u64(PgString s) {
  PgParseNumberResult res = {0};
  res.remaining = s;

  // Forbid leading zero(es) if there is more than one digit.
  if (pg_string_starts_with(s, PG_S("0")) && s.len >= 2 &&
      pg_character_is_numeric(PG_SLICE_AT(s, 1))) {
    return res;
  }

  u64 i = 0;
  for (; i < s.len; i++) {
    u8 c = PG_SLICE_AT(s, i);

    if (!pg_character_is_numeric(c)) { // End of numbers sequence.
      break;
    }

    res.n *= 10;
    res.n += (u8)PG_SLICE_AT(s, i) - '0';
    res.present = true;
  }
  res.remaining = PG_SLICE_RANGE_START(s, i);
  return res;
}

typedef struct {
  u8 *start;
  u8 *end;
} PgArena;

__attribute((malloc, alloc_size(2, 4), alloc_align(3)))
[[maybe_unused]] [[nodiscard]] static void *
pg_arena_alloc(PgArena *a, u64 size, u64 align, u64 count) {
  PG_ASSERT(a->start != nullptr);

  const u64 padding = (-(u64)a->start & (align - 1));
  PG_ASSERT(padding <= align);

  const i64 available = (i64)a->end - (i64)a->start - (i64)padding;
  PG_ASSERT(available >= 0);
  PG_ASSERT(count <= (u64)available / size);

  void *res = a->start + padding;
  PG_ASSERT(res != nullptr);
  PG_ASSERT(res <= (void *)a->end);

  a->start += padding + count * size;
  PG_ASSERT(a->start <= a->end);
  PG_ASSERT((u64)a->start % align == 0); // Aligned.

  return memset(res, 0, count * size);
}

#define pg_arena_new(a, t, n) (t *)pg_arena_alloc(a, sizeof(t), _Alignof(t), n)

[[maybe_unused]] [[nodiscard]] static PgString pg_string_make(u64 len,
                                                              PgArena *arena) {
  PgString res = {0};
  res.len = len;
  res.data = pg_arena_new(arena, u8, len);
  return res;
}

[[maybe_unused]] [[nodiscard]] static char *pg_string_to_cstr(PgString s,
                                                              PgArena *arena) {
  char *res = (char *)pg_arena_new(arena, u8, s.len + 1);
  if (NULL != s.data) {
    memcpy(res, s.data, s.len);
  }

  PG_ASSERT(0 == PG_C_ARRAY_AT(res, s.len + 1, s.len));

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString cstr_to_string(char *s) {
  return (PgString){
      .data = (u8 *)s,
      .len = strlen(s),
  };
}

typedef enum {
  STRING_CMP_LESS = -1,
  STRING_CMP_EQ = 0,
  STRING_CMP_GREATER = 1,
} PgStringCompare;

[[maybe_unused]] [[nodiscard]] static PgStringCompare
pg_string_cmp(PgString a, PgString b) {
  int cmp = memcmp(a.data, b.data, PG_MIN(a.len, b.len));
  if (cmp < 0) {
    return STRING_CMP_LESS;
  } else if (cmp > 0) {
    return STRING_CMP_GREATER;
  } else if (a.len == b.len) {
    return STRING_CMP_EQ;
  }

  PG_ASSERT(0 == cmp);
  PG_ASSERT(a.len != b.len);

  if (a.len < b.len) {
    return STRING_CMP_LESS;
  }
  if (a.len > b.len) {
    return STRING_CMP_GREATER;
  }
  PG_ASSERT(0);
}

[[maybe_unused]] static void PG_DYN_GROW(void *slice, u64 size, u64 align,
                                         u64 count, PgArena *a) {
  PG_ASSERT(nullptr != slice);

  struct {
    void *data;
    u64 len;
    u64 cap;
  } PgReplica;

  memcpy(&PgReplica, slice, sizeof(PgReplica));
  PG_ASSERT(PgReplica.cap < count);

  u64 new_cap = PgReplica.cap == 0 ? 2 : PgReplica.cap;
  for (u64 i = 0; i < 64; i++) {
    if (new_cap < count) {
      PG_ASSERT(new_cap < UINT64_MAX / 2);
      PG_ASSERT(false == ckd_mul(&new_cap, new_cap, 2));
    } else {
      break;
    }
  }
  PG_ASSERT(new_cap >= 2);
  PG_ASSERT(new_cap >= count);
  PG_ASSERT(new_cap > PgReplica.cap);

  u64 array_end = 0;
  u64 array_bytes_count = 0;
  PG_ASSERT(false == ckd_mul(&array_bytes_count, size, PgReplica.cap));
  PG_ASSERT(false ==
            ckd_add(&array_end, (u64)PgReplica.data, array_bytes_count));
  PG_ASSERT((u64)PgReplica.data <= array_end);
  PG_ASSERT(array_end < (u64)a->end);

  if (nullptr ==
      PgReplica.data) { // First allocation ever for this dynamic array.
    PgReplica.data = pg_arena_alloc(a, size, align, new_cap);
  } else if ((u64)a->start == array_end) { // Optimization.
    // This is the case of growing the array which is at the end of the arena.
    // In that case we can simply bump the arena pointer and avoid any copies.
    (void)pg_arena_alloc(a, size, 1 /* Force no padding */,
                         new_cap - PgReplica.cap);
  } else { // General case.
    void *data = pg_arena_alloc(a, size, align, new_cap);

    // Import check to avoid overlapping memory ranges in memcpy.
    PG_ASSERT(data != PgReplica.data);

    memcpy(data, PgReplica.data, array_bytes_count);
    PgReplica.data = data;
  }
  PgReplica.cap = new_cap;

  PG_ASSERT(nullptr != slice);
  memcpy(slice, &PgReplica, sizeof(PgReplica));
}

#define PG_DYN_ENSURE_CAP(dyn, new_cap, arena)                                 \
  ((dyn)->cap < (new_cap))                                                     \
      ? PG_DYN_GROW(dyn, sizeof(*(dyn)->data), _Alignof((dyn)->data[0]),       \
                    new_cap, arena),                                           \
      0 : 0

#define PG_DYN_SPACE(T, dyn)                                                   \
  ((T){.data = (dyn)->data + (dyn)->len, .len = (dyn)->cap - (dyn)->len})

PG_DYN(PgString) PgStringDyn;
PG_RESULT(PgStringDyn) PgStringDynResult;

#define PG_DYN_PUSH(s, arena)                                                  \
  (PG_DYN_ENSURE_CAP(s, (s)->len + 1, arena), (s)->data + (s)->len++)

#define PG_DYN_POP(s)                                                          \
  do {                                                                         \
    PG_ASSERT((s)->len > 0);                                                   \
    memset(PG_DYN_LAST_PTR(s), 0, sizeof((s)->data[(s)->len - 1]));            \
    (s)->len -= 1;                                                             \
  } while (0)

#define PG_DYN_LAST_PTR(s) PG_C_ARRAY_AT_PTR((s)->data, (s)->len, (s)->len - 1)

#define PG_DYN_LAST(s) PG_C_ARRAY_AT((s).data, (s).len, (s).len - 1)

#define PG_DYN_AT_PTR(s, idx) PG_C_ARRAY_AT_PTR((s)->data, (s)->len, idx)

#define PG_DYN_AT(s, idx) PG_C_ARRAY_AT((s).data, (s).len, idx)

#define PG_DYN_APPEND_SLICE(dst, src, arena)                                   \
  do {                                                                         \
    PG_DYN_ENSURE_CAP(dst, (dst)->len + (src).len, arena);                     \
    for (u64 _iii = 0; _iii < src.len; _iii++) {                               \
      *PG_DYN_PUSH(dst, arena) = PG_SLICE_AT(src, _iii);                       \
    }                                                                          \
  } while (0)

#define PG_DYN_SLICE(T, dyn) ((T){.data = dyn.data, .len = dyn.len})

[[maybe_unused]] static void
pg_string_builder_append_u64_to_string(Pgu8Dyn *dyn, u64 n, PgArena *arena) {
  u8 tmp[30] = {0};
  // TODO: Not use snprintf.
  const int written_count = snprintf((char *)tmp, sizeof(tmp), "%lu", n);

  PG_ASSERT(written_count > 0);

  PgString s = {.data = tmp, .len = (u64)written_count};
  PG_DYN_APPEND_SLICE(dyn, s, arena);
}

[[maybe_unused]] static void pg_u32_to_u8x4_be(u32 n, PgString *dst) {
  PG_ASSERT(sizeof(n) == dst->len);

  *(PG_SLICE_AT_PTR(dst, 0)) = (u8)(n >> 24);
  *(PG_SLICE_AT_PTR(dst, 1)) = (u8)(n >> 16);
  *(PG_SLICE_AT_PTR(dst, 2)) = (u8)(n >> 8);
  *(PG_SLICE_AT_PTR(dst, 3)) = (u8)(n >> 0);
}

[[maybe_unused]] static void pg_string_builder_append_u32(Pgu8Dyn *dyn, u32 n,
                                                          PgArena *arena) {

  u8 data[sizeof(n)] = {0};
  PgString s = {.data = data, .len = sizeof(n)};
  pg_u32_to_u8x4_be(n, &s);
  PG_DYN_APPEND_SLICE(dyn, s, arena);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_u64_to_string(u64 n, PgArena *arena) {
  Pgu8Dyn sb = {0};
  pg_string_builder_append_u64_to_string(&sb, n, arena);
  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static u8 pg_u8_to_character_hex(u8 n) {
  PG_ASSERT(n < 16);

  if (n <= 9) {
    return n + '0';
  } else if (10 == n) {
    return 'a';
  } else if (11 == n) {
    return 'b';
  } else if (12 == n) {
    return 'c';
  } else if (13 == n) {
    return 'd';
  } else if (14 == n) {
    return 'e';
  } else if (15 == n) {
    return 'f';
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static u8 pg_u8_to_character_hex_upper(u8 n) {
  PG_ASSERT(n < 16);

  if (n <= 9) {
    return n + '0';
  } else if (10 == n) {
    return 'A';
  } else if (11 == n) {
    return 'B';
  } else if (12 == n) {
    return 'C';
  } else if (13 == n) {
    return 'D';
  } else if (14 == n) {
    return 'E';
  } else if (15 == n) {
    return 'F';
  }
  PG_ASSERT(0);
}

static void pg_string_builder_append_u8_hex_upper(Pgu8Dyn *dyn, u8 n,
                                                  PgArena *arena) {
  PG_DYN_ENSURE_CAP(dyn, dyn->len + 2, arena);
  u64 PG_DYN_ORIGINAL_LEN = dyn->len;

  u8 c1 = n % 16;
  u8 c2 = n / 16;
  *PG_DYN_PUSH(dyn, arena) = pg_u8_to_character_hex_upper(c2);
  *PG_DYN_PUSH(dyn, arena) = pg_u8_to_character_hex_upper(c1);
  PG_ASSERT(2 == (dyn->len - PG_DYN_ORIGINAL_LEN));
}

[[maybe_unused]] static void
pg_string_builder_append_u128_hex(Pgu8Dyn *dyn, u128 n, PgArena *arena) {
  PG_DYN_ENSURE_CAP(dyn, dyn->len + 32, arena);
  u64 PG_DYN_ORIGINAL_LEN = dyn->len;

  u8 it[16] = {0};
  PG_ASSERT(sizeof(it) == sizeof(n));
  memcpy(it, (u8 *)&n, sizeof(n));

  for (u64 i = 0; i < sizeof(it); i++) {
    u8 c1 = it[i] % 16;
    u8 c2 = it[i] / 16;
    *PG_DYN_PUSH(dyn, arena) = pg_u8_to_character_hex(c2);
    *PG_DYN_PUSH(dyn, arena) = pg_u8_to_character_hex(c1);
  }
  PG_ASSERT(32 == (dyn->len - PG_DYN_ORIGINAL_LEN));
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_dup(PgString src,
                                                             PgArena *arena) {
  PgString dst = pg_string_make(src.len, arena);
  memcpy(dst.data, src.data, src.len);

  return dst;
}

[[maybe_unused]] [[nodiscard]] static u64
pg_round_up_multiple_of(u64 n, u64 multiple) {
  PG_ASSERT(0 != multiple);

  if (0 == n % multiple) {
    return n; // No-op.
  }

  u64 factor = n / multiple;
  return (factor + 1) * n;
}

// FIXME: Windows.
[[maybe_unused]] [[nodiscard]] static PgArena
pg_arena_make_from_virtual_mem(u64 size) {
  u64 page_size = (u64)sysconf(_SC_PAGE_SIZE);
  u64 alloc_real_size = pg_round_up_multiple_of(size, page_size);
  PG_ASSERT(0 == alloc_real_size % page_size);

  u64 mmap_size = alloc_real_size;
  // Page guard before.
  PG_ASSERT(false == ckd_add(&mmap_size, mmap_size, page_size));
  // Page guard after.
  PG_ASSERT(false == ckd_add(&mmap_size, mmap_size, page_size));

  u8 *alloc = mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE,
                   MAP_ANON | MAP_PRIVATE, -1, 0);
  PG_ASSERT(nullptr != alloc);

  u64 page_guard_before = (u64)alloc;

  PG_ASSERT(false == ckd_add((u64 *)&alloc, (u64)alloc, page_size));
  PG_ASSERT(page_guard_before + page_size == (u64)alloc);

  u64 page_guard_after = (u64)0;
  PG_ASSERT(false == ckd_add(&page_guard_after, (u64)alloc, alloc_real_size));
  PG_ASSERT((u64)alloc + alloc_real_size == page_guard_after);
  PG_ASSERT(page_guard_before + page_size + alloc_real_size ==
            page_guard_after);

  PG_ASSERT(0 == mprotect((void *)page_guard_before, page_size, PROT_NONE));
  PG_ASSERT(0 == mprotect((void *)page_guard_after, page_size, PROT_NONE));

  // Trigger a page fault preemptively to detect invalid virtual memory
  // mappings.
  *(u8 *)alloc = 0;

  return (PgArena){.start = alloc, .end = (u8 *)alloc + size};
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_os_sendfile(int fd_in, int fd_out, u64 n_bytes) {
#if defined(__linux__)
#include <sys/sendfile.h>
  ssize_t res = sendfile(fd_out, fd_in, nullptr, n_bytes);
  if (res == -1) {
    return (PgError)errno;
  }
  if (res != (ssize_t)n_bytes) {
    return (PgError)EAGAIN;
  }
  return 0;
#elif defined(__FreeBSD__)
  int res = sendfile(fd_in, fd_out, 0, n_bytes, nullptr, nullptr, 0);
  if (res == -1) {
    return (PgError)errno;
  }
  return 0;
#else
#error "sendfile(2) not implemented on other OSes than Linux/FreeBSD."
#endif
}

[[maybe_unused]] [[nodiscard]] static i64
pg_string_indexof_unescaped_byte(PgString haystack, u8 needle) {
  for (u64 i = 0; i < haystack.len; i++) {
    u8 c = PG_SLICE_AT(haystack, i);

    if (c != needle) {
      continue;
    }

    if (i == 0) {
      return (i64)i;
    }

    u8 previous = PG_SLICE_AT(haystack, i - 1);
    if ('\\' != previous) {
      return (i64)i;
    }
  }

  return -1;
}

[[maybe_unused]] [[nodiscard]] static u64
pg_skip_over_whitespace(PgString s, u64 idx_start) {
  PG_ASSERT(idx_start < s.len);

  u64 idx = idx_start;
  for (; idx < s.len; idx++) {
    u8 c = PG_SLICE_AT(s, idx);
    if (' ' != c) {
      return idx;
    }
  }

  return idx;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_clone(PgString s,
                                                               PgArena *arena) {
  PgString res = pg_string_make(s.len, arena);
  if (res.data != nullptr) {
    memcpy(res.data, s.data, s.len);
  }

  return res;
}

[[maybe_unused]] static void pg_string_lowercase_ascii_mut(PgString s) {
  for (u64 i = 0; i < s.len; i++) {
    u8 *c = PG_C_ARRAY_AT_PTR(s.data, s.len, i);
    if ('A' <= *c && *c <= 'Z') {
      *c += 32;
    }
  }
}

[[maybe_unused]] [[nodiscard]] static bool
pg_string_ieq_ascii(PgString a, PgString b, PgArena *arena) {
  if (a.data == nullptr && b.data == nullptr && a.len == b.len) {
    return true;
  }
  if (a.data == nullptr) {
    return false;
  }
  if (b.data == nullptr) {
    return false;
  }

  if (a.len != b.len) {
    return false;
  }

  PG_ASSERT(a.data != nullptr);
  PG_ASSERT(b.data != nullptr);
  PG_ASSERT(a.len == b.len);

  PgArena tmp = *arena;
  PgString a_clone = pg_string_clone(a, &tmp);
  PgString b_clone = pg_string_clone(b, &tmp);

  pg_string_lowercase_ascii_mut(a_clone);
  pg_string_lowercase_ascii_mut(b_clone);

  return pg_string_eq(a_clone, b_clone);
}

[[maybe_unused]] static void pg_sha1(PgString s, u8 hash[20]) {
  pg_SHA1_CTX ctx = {0};
  pg_SHA1Init(&ctx);
  pg_SHA1Update(&ctx, s.data, s.len);
  pg_SHA1Final(hash, &ctx);
}

// TODO: Windows.
[[maybe_unused]] static PgStringResult file_read_full(PgString path,
                                                      PgArena *arena) {

  PgStringResult res = {0};
  char *path_c = pg_string_to_cstr(path, arena);

  int fd = open(path_c, O_RDONLY);
  if (fd < 0) {
    res.err = (PgError)errno;
    return res;
  }

  struct stat st = {0};
  if (-1 == stat(path_c, &st)) {
    res.err = (PgError)errno;
    goto end;
  }

  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, (u64)st.st_size, arena);

  for (u64 lim = 0; lim < (u64)st.st_size; lim++) {
    if ((u64)st.st_size == sb.len) {
      break;
    }

    PgString space = {.data = sb.data + sb.len, .len = sb.cap - sb.len};
    ssize_t read_n = read(fd, space.data, space.len);
    if (-1 == read_n) {
      res.err = (PgError)errno;
      goto end;
    }

    if (0 == read_n) {
      res.err = (PgError)PG_ERR_INVALID_VALUE;
      goto end;
    }

    PG_ASSERT((u64)read_n <= space.len);

    sb.len += (u64)read_n;
  }

end:
  close(fd);
  res.res = PG_DYN_SLICE(PgString, sb);
  return res;
}

typedef struct {
  u32 ip;   // Host order.
  u16 port; // Host order.
} PgIpv4Address;

PG_DYN(PgIpv4Address) PgIpv4AddressDyn;

[[maybe_unused]] [[nodiscard]] static PgString
pg_make_unique_id_u128_string(PgArena *arena) {
  u128 id = 0;
  arc4random_buf(&id, sizeof(id));

  Pgu8Dyn dyn = {0};
  pg_string_builder_append_u128_hex(&dyn, id, arena);

  return PG_DYN_SLICE(PgString, dyn);
}

[[maybe_unused]] static void pg_url_encode_string(Pgu8Dyn *sb, PgString key,
                                                  PgString value,
                                                  PgArena *arena) {
  for (u64 i = 0; i < key.len; i++) {
    u8 c = PG_SLICE_AT(key, i);
    if (pg_character_is_alphanumeric(c)) {
      *PG_DYN_PUSH(sb, arena) = c;
    } else {
      *PG_DYN_PUSH(sb, arena) = '%';
      pg_string_builder_append_u8_hex_upper(sb, c, arena);
    }
  }

  *PG_DYN_PUSH(sb, arena) = '=';

  for (u64 i = 0; i < value.len; i++) {
    u8 c = PG_SLICE_AT(value, i);
    if (pg_character_is_alphanumeric(c)) {
      *PG_DYN_PUSH(sb, arena) = c;
    } else {
      *PG_DYN_PUSH(sb, arena) = '%';
      pg_string_builder_append_u8_hex_upper(sb, c, arena);
    }
  }
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_net_ipv4_address_to_string(PgIpv4Address address, PgArena *arena) {
  Pgu8Dyn sb = {0};
  pg_string_builder_append_u64_to_string(&sb, (address.ip >> 24) & 0xFF, arena);
  *PG_DYN_PUSH(&sb, arena) = '.';
  pg_string_builder_append_u64_to_string(&sb, (address.ip >> 16) & 0xFF, arena);
  *PG_DYN_PUSH(&sb, arena) = '.';
  pg_string_builder_append_u64_to_string(&sb, (address.ip >> 8) & 0xFF, arena);
  *PG_DYN_PUSH(&sb, arena) = '.';
  pg_string_builder_append_u64_to_string(&sb, (address.ip >> 0) & 0xFF, arena);
  *PG_DYN_PUSH(&sb, arena) = ':';
  pg_string_builder_append_u64_to_string(&sb, address.port, arena);

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static u32 pg_u8x4_be_to_u32(PgString s) {
  PG_ASSERT(4 == s.len);
  return (u32)(PG_SLICE_AT(s, 0) << 24) | (u32)(PG_SLICE_AT(s, 1) << 16) |
         (u32)(PG_SLICE_AT(s, 2) << 8) | (u32)(PG_SLICE_AT(s, 3) << 0);
}

[[maybe_unused]] [[nodiscard]] static bool pg_bitfield_get(PgString bitfield,
                                                           u64 idx_bit) {
  PG_ASSERT(idx_bit < bitfield.len * 8);

  u64 idx_byte = idx_bit / 8;

  return PG_SLICE_AT(bitfield, idx_byte) & (1 << (idx_bit % 8));
}

// FIXME: Windows.
typedef int PgSocket;
typedef int PgFile;
typedef int PgTimer;

typedef enum {
  PG_CLOCK_KIND_MONOTONIC,
  // TODO: More?
} PgClockKind;

PG_RESULT(PgTimer) PgTimerResult;

[[maybe_unused]] [[nodiscard]] static PgTimerResult
pg_timer_create(PgClockKind clock_kind, u64 ns);

[[maybe_unused]] [[nodiscard]] static PgError pg_timer_release(PgTimer timer);

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_time_ns_now(PgClockKind clock_kind);

PG_RESULT(PgSocket) PgCreateSocketResult;
[[maybe_unused]] [[nodiscard]] static PgCreateSocketResult
pg_net_create_tcp_socket();
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_close(PgSocket sock);
[[maybe_unused]] [[nodiscard]] static PgError pg_net_set_nodelay(PgSocket sock,
                                                                 bool enabled);
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_connect_ipv4(PgSocket sock, PgIpv4Address address);
typedef struct {
  PgIpv4Address address;
  PgSocket socket;
} PgIpv4AddressSocket;
PG_RESULT(PgIpv4AddressSocket) PgDnsResolveIpv4AddressSocketResult;
[[maybe_unused]] [[nodiscard]] static PgDnsResolveIpv4AddressSocketResult
pg_net_dns_resolve_ipv4_tcp(PgString host, u16 port, PgArena arena);

[[maybe_unused]] [[nodiscard]] static PgError pg_net_tcp_listen(PgSocket sock);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_tcp_bind_ipv4(PgSocket sock, PgIpv4Address addr);
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_enable_reuse(PgSocket sock);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_set_blocking(PgSocket sock, bool blocking);

typedef struct {
  PgIpv4Address addr;
  PgSocket socket;
  PgError err;
} PgIpv4AddressAcceptResult;
[[maybe_unused]] [[nodiscard]] static PgIpv4AddressAcceptResult
pg_net_tcp_accept(PgSocket sock);

typedef u64 PgAioQueue;
PG_RESULT(PgAioQueue) PgAioQueueCreateResult;
[[maybe_unused]] [[nodiscard]] static PgAioQueueCreateResult
pg_aio_queue_create();

typedef enum {
  PG_AIO_EVENT_KIND_NONE = 0,
  PG_AIO_EVENT_KIND_IN = 1,
  PG_AIO_EVENT_KIND_OUT = 2,
  PG_AIO_EVENT_KIND_ERR = 4,
} PgAioEventKind;

typedef enum {
  PG_AIO_EVENT_ACTION_NONE,
  PG_AIO_EVENT_ACTION_ADD,
  PG_AIO_EVENT_ACTION_MOD,
  PG_AIO_EVENT_ACTION_DEL,
} PgAioEventAction;

typedef struct {
  PgSocket socket;
  PgTimer timer;
  PgAioEventKind kind;
  PgAioEventAction action;
} PgAioEvent;

PG_SLICE(PgAioEvent) PgAioEventSlice;
PG_DYN(PgAioEvent) PgAioEventDyn;

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_queue_ctl(PgAioQueue queue, PgAioEventSlice events);

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_queue_ctl_one(PgAioQueue queue, PgAioEvent event) {
  PgAioEventSlice events = {.data = &event, .len = 1};
  return pg_aio_queue_ctl(queue, events);
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_aio_queue_wait(PgAioQueue queue, PgAioEventSlice events, i64 timeout_ms,
                  PgArena arena);

#if defined(__linux__) || defined(__FreeBSD__) // TODO: More Unices.
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

static PgCreateSocketResult pg_net_create_tcp_socket() {
  PgCreateSocketResult res = {0};

  PgSocket sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock_fd) {
    res.err = (PgError)errno;
    return res;
  }

  res.res = sock_fd;

  return res;
}

static PgError pg_net_socket_close(PgSocket sock) {
  return (PgError)close(sock);
}

static PgError pg_net_set_nodelay(PgSocket sock, bool enabled) {
  int opt = enabled;
  if (-1 == setsockopt(sock, SOL_TCP, TCP_NODELAY, &opt, sizeof(opt))) {
    return (PgError)errno;
  }

  return 0;
}

static PgError pg_net_connect_ipv4(PgSocket sock, PgIpv4Address address) {
  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(address.port),
      .sin_addr = {htonl(address.ip)},
  };

  if (-1 == connect(sock, (struct sockaddr *)&addr, sizeof(addr))) {
    return (PgError)errno;
  }

  return 0;
}

static PgDnsResolveIpv4AddressSocketResult
pg_net_dns_resolve_ipv4_tcp(PgString host, u16 port, PgArena arena) {
  PgDnsResolveIpv4AddressSocketResult res = {0};

  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *addr_info = nullptr;
  int res_getaddrinfo =
      getaddrinfo(pg_string_to_cstr(host, &arena),
                  pg_string_to_cstr(pg_u64_to_string(port, &arena), &arena),
                  &hints, &addr_info);
  if (res_getaddrinfo != 0) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  struct addrinfo *rp = nullptr;
  for (rp = addr_info; rp != nullptr; rp = rp->ai_next) {
    PgCreateSocketResult res_create_socket = pg_net_create_tcp_socket();
    if (res_create_socket.err) {
      res.err = res_create_socket.err;
      continue;
    }

    // TODO: Use pg_net_connect_ipv4?
    if (-1 == connect(res_create_socket.res, rp->ai_addr, rp->ai_addrlen)) {
      // TODO: EINPROGRESS in case of non-blocking.
      (void)pg_net_socket_close(res_create_socket.res);
      continue;
    }

    res.res.socket = res_create_socket.res;

    res.res.address.ip =
        inet_netof(((struct sockaddr_in *)(void *)rp->ai_addr)->sin_addr);
    res.res.address.port = port;
    break;
  }

  freeaddrinfo(addr_info);

  if (nullptr == rp) { // No address succeeded.
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_set_blocking(PgSocket sock, bool blocking) {
  int flags = fcntl(sock, F_GETFL);
  if (-1 == flags) {
    return (PgError)errno;
  }

  if (blocking) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }
  if (-1 == fcntl(sock, F_SETFL, flags)) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError pg_net_tcp_listen(PgSocket sock) {
  if (-1 == listen(sock, 1024)) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_tcp_bind_ipv4(PgSocket sock, PgIpv4Address addr) {
  struct sockaddr_in addrin = {0};
  addrin.sin_family = AF_INET;
  addrin.sin_port = htons(addr.port);
  addrin.sin_addr.s_addr = htonl(addr.ip);

  if (-1 == bind(sock, (struct sockaddr *)&addrin, sizeof(addrin))) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_enable_reuse(PgSocket sock) {
  int val = 1;
  if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))) {
    return (PgError)errno;
  }

  if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val))) {
    return (PgError)errno;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgIpv4AddressAcceptResult
pg_net_tcp_accept(PgSocket sock) {
  PG_ASSERT(0 != sock);

  PgIpv4AddressAcceptResult res = {0};

  struct sockaddr_in sockaddrin = {0};
  socklen_t sockaddrin_len = sizeof(sockaddrin);
  int sock_client =
      accept(sock, (struct sockaddr *)&sockaddrin, &sockaddrin_len);
  if (-1 == sock_client) {
    res.err = (PgError)errno;
    return res;
  }

  res.socket = (PgSocket)sock_client;
  res.addr.port = ntohs(sockaddrin.sin_port);
  res.addr.ip = ntohl(sockaddrin.sin_addr.s_addr);

  return res;
}

#else
#error "TODO"
#endif

#if defined(__linux__)
#include <sys/epoll.h>
#include <sys/timerfd.h>

[[maybe_unused]] [[nodiscard]] static PgAioQueueCreateResult
pg_aio_queue_create() {
  PgAioQueueCreateResult res = {0};
  int queue = epoll_create(1 /* Ignored */);
  if (-1 == queue) {
    res.err = (PgError)errno;
  }
  res.res = (PgAioQueue)queue;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_queue_ctl(PgAioQueue queue, PgAioEventSlice events) {
  for (u64 i = 0; i < events.len; i++) {
    PgAioEvent event = PG_SLICE_AT(events, i);
    PG_ASSERT(event.socket ^ event.timer);

    int op = 0;
    switch (event.action) {
    case PG_AIO_EVENT_ACTION_ADD:
      op = EPOLL_CTL_ADD;
      break;
    case PG_AIO_EVENT_ACTION_MOD:
      op = EPOLL_CTL_MOD;
      break;
    case PG_AIO_EVENT_ACTION_DEL:
      op = EPOLL_CTL_DEL;
      break;
    case PG_AIO_EVENT_ACTION_NONE:
    default:
      PG_ASSERT(0);
    }

    struct epoll_event epoll_event = {0};
    if (event.kind & PG_AIO_EVENT_KIND_IN) {
      epoll_event.events |= EPOLLIN;
    }
    if (event.kind & PG_AIO_EVENT_KIND_OUT) {
      epoll_event.events |= EPOLLOUT;
    }
    if (event.kind & PG_AIO_EVENT_KIND_ERR) {
      epoll_event.events |= EPOLLERR;
    }

    if (event.socket) {
      epoll_event.data.fd = event.socket;
    } else if (event.timer) {
      epoll_event.data.fd = event.timer;
    }

    int res_epoll =
        epoll_ctl((int)queue, op, epoll_event.data.fd, &epoll_event);
    if (-1 == res_epoll) {
      return (PgError)errno;
    }
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_aio_queue_wait(PgAioQueue queue, PgAioEventSlice events, i64 timeout_ms,
                  PgArena arena) {
  Pgu64Result res = {0};
  if (PG_SLICE_IS_EMPTY(events)) {
    return res;
  }

  struct epoll_event *epoll_events =
      pg_arena_new(&arena, struct epoll_event, events.len);

  int res_epoll =
      epoll_wait((int)queue, epoll_events, (int)events.len, (int)timeout_ms);
  if (-1 == res_epoll) {
    res.err = (PgError)errno;
    return res;
  }
  res.res = (u64)res_epoll;

  for (u64 i = 0; i < res.res; i++) {
    PgAioEvent *event = PG_SLICE_AT_PTR(&events, i);
    *event = (PgAioEvent){0};

    struct epoll_event epoll_event = epoll_events[i];
    if (epoll_event.events & EPOLLIN) {
      event->kind |= PG_AIO_EVENT_KIND_IN;
    }
    if (epoll_event.events & EPOLLOUT) {
      event->kind |= PG_AIO_EVENT_KIND_OUT;
    }
    if (epoll_event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
      event->kind |= PG_AIO_EVENT_KIND_ERR;
    }
    event->socket = epoll_event.data.fd;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static int
pg_linux_clock(PgClockKind clock_kind) {
  switch (clock_kind) {
  case PG_CLOCK_KIND_MONOTONIC:
    return CLOCK_MONOTONIC;
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static PgTimerResult
pg_timer_create(PgClockKind clock, u64 ns) {
  PgTimerResult res = {0};

  int ret = timerfd_create(pg_linux_clock(clock), TFD_NONBLOCK);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.res = (PgTimer)ret;

  struct itimerspec ts = {0};
  ts.it_value.tv_sec = ns / PG_Seconds;
  ts.it_value.tv_nsec = ns % PG_Seconds;
  ret = timerfd_settime((int)res.res, 0, &ts, nullptr);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError pg_timer_release(PgTimer timer) {
  if (-1 == close((int)timer)) {
    return (PgError)errno;
  }
  return (PgError)0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_time_ns_now(PgClockKind clock) {
  Pgu64Result res = {0};

  struct timespec ts = {0};
  int ret = clock_gettime(pg_linux_clock(clock), &ts);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.res = (u64)ts.tv_sec * PG_Seconds + (u64)ts.tv_nsec;

  return res;
}
#endif

PG_RESULT(PgString) PgIoResult;

typedef Pgu64Result (*ReadFn)(void *self, u8 *buf, size_t buf_len);
typedef Pgu64Result (*WriteFn)(void *self, u8 *buf, size_t buf_len);

// TODO: Guard with `ifdef`?
// TODO: Windows?
[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_unix_read(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);

  int fd = (int)(u64)self;
  ssize_t n = read(fd, buf, buf_len);

  Pgu64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

// TODO: Guard with `ifdef`?
// TODO: Windows?
[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_unix_write(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);

  int fd = (int)(u64)self;
  ssize_t n = write(fd, buf, buf_len);

  Pgu64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

typedef struct {
  Pgu8Dyn sb;
  PgArena *arena;
} StringBuilder;

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_string_builder_write(void *self, u8 *buf, size_t buf_len) {
  StringBuilder *sb = self;
  PgString s = {.data = buf, .len = buf_len};
  PG_DYN_APPEND_SLICE(&sb->sb, s, sb->arena);

  return (Pgu64Result){.res = buf_len};
}

typedef struct {
  u64 idx_read, idx_write;
  PgString data;
} PgRing;

[[maybe_unused]] [[nodiscard]] static u64 pg_ring_write_space(PgRing rg) {
  if (rg.idx_write == rg.idx_read) { // Empty.
    return rg.data.len - 1;
  } else if (rg.idx_write < rg.idx_read) { // Easy case.
    u64 res = rg.idx_read - rg.idx_write - 1;
    PG_ASSERT(res < rg.data.len);
    return res;
  } else if (rg.idx_write > rg.idx_read) { // Hard case.
    u64 can_write1 = rg.data.len - rg.idx_write;
    u64 can_write2 = rg.idx_read;
    if (can_write1 >= 1 && rg.idx_read == 0) {
      can_write1 -= 1; // Reserve empty slot.
    } else if (can_write2 >= 1) {
      PG_ASSERT(rg.idx_read > 0);
      can_write2 -= 1;
    }
    PG_ASSERT(can_write1 <= rg.data.len - 1);
    PG_ASSERT(can_write2 <= rg.data.len - 1);
    return can_write1 + can_write2;
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static u64 pg_ring_read_space(PgRing rg) {
  if (rg.idx_write == rg.idx_read) { // Empty.
    return 0;
  } else if (rg.idx_read < rg.idx_write) { // Easy case.
    u64 res = rg.idx_write - rg.idx_read;
    PG_ASSERT(res < rg.data.len);
    return res;
  } else if (rg.idx_read > rg.idx_write) { // Hard case.
    u64 can_read1 = rg.data.len - rg.idx_read;
    u64 can_read2 = rg.idx_write;
    return can_read1 + can_read2;
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_write_slice(PgRing *rg,
                                                               PgString data) {
  PG_ASSERT(nullptr != rg->data.data);
  PG_ASSERT(rg->idx_read <= rg->data.len);
  PG_ASSERT(rg->idx_write <= rg->data.len);
  PG_ASSERT(rg->data.len > 0);

  if (rg->idx_write < rg->idx_read) { // Easy case.
    u64 space = rg->idx_read - rg->idx_write - 1;
    PG_ASSERT(space <= rg->data.len);

    if (data.len > space) {
      return false;
    }
    memcpy(rg->data.data + rg->idx_write, data.data, data.len);
    rg->idx_write += data.len;
    PG_ASSERT(rg->idx_write <= rg->data.len);
    PG_ASSERT(rg->idx_write < rg->idx_read);
  } else { // Hard case: need potentially two writes.
    PG_ASSERT(rg->idx_write >= rg->idx_read);

    u64 can_write1 = rg->data.len - rg->idx_write;

    u64 can_write2 = rg->idx_read;
    if (can_write1 >= 1 && rg->idx_read == 0) {
      can_write1 -= 1; // Reserve empty slot.
    } else if (can_write2 >= 1) {
      PG_ASSERT(rg->idx_read > 0);
      can_write2 -= 1;
    }
    PG_ASSERT(can_write1 <= rg->data.len - 1);
    PG_ASSERT(can_write2 <= rg->data.len - 1);

    u64 can_write = can_write1 + can_write2;
    if (can_write < data.len) {
      return false;
    }

    u64 write_len1 = PG_MIN(can_write1, data.len);
    PG_ASSERT(rg->idx_write + write_len1 <= rg->data.len);
    PG_ASSERT(write_len1 <= data.len);
    memcpy(rg->data.data + rg->idx_write, data.data, write_len1);
    rg->idx_write += write_len1;
    if (rg->idx_write == rg->data.len) {
      rg->idx_write = 0;
    }
    PG_ASSERT(rg->idx_write < rg->data.len);

    u64 write_len2 = data.len - write_len1;
    if (write_len2 > 0) {
      PG_ASSERT(rg->idx_write = rg->data.len - 1);

      PG_ASSERT(write_len2 + 1 <= rg->idx_read);
      PG_ASSERT(write_len1 + write_len2 <= data.len);
      memcpy(rg->data.data, data.data + write_len1, write_len2);
      rg->idx_write = write_len2;
      PG_ASSERT(rg->idx_write + 1 <= rg->idx_read);
    }
  }

  return true;
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_read_slice(PgRing *rg,
                                                              PgString data) {
  PG_ASSERT(nullptr != rg->data.data);
  PG_ASSERT(rg->idx_read <= rg->data.len);
  PG_ASSERT(rg->idx_write <= rg->data.len);
  PG_ASSERT(rg->data.len > 0);

  if (0 == data.len) {
    return true;
  }
  PG_ASSERT(nullptr != data.data);

  if (rg->idx_write == rg->idx_read) { // Empty.
    return false;
  } else if (rg->idx_read < rg->idx_write) { // Easy case.
    u64 can_read = rg->idx_write - rg->idx_read;
    if (data.len > can_read) {
      return false;
    }
    u64 n_read = PG_MIN(data.len, can_read);

    memcpy(data.data, rg->data.data + rg->idx_read, n_read);
    rg->idx_read += n_read;
    PG_ASSERT(rg->idx_read < rg->data.len);
  } else { // Hard case: potentially 2 reads.
    PG_ASSERT(rg->idx_read > rg->idx_write);
    u64 can_read1 = rg->data.len - rg->idx_read;
    u64 can_read2 = rg->idx_write;
    u64 can_read = can_read1 + can_read2;
    if (can_read < data.len) {
      // TODO: Should we do short reads like `read(2)`?
      return false;
    }

    u64 read_len1 = PG_MIN(can_read1, data.len);
    PG_ASSERT(read_len1 <= data.len);
    PG_ASSERT(read_len1 <= rg->data.len);

    memcpy(data.data, rg->data.data + rg->idx_read, read_len1);
    rg->idx_read += read_len1;
    if (rg->idx_read == rg->data.len) {
      rg->idx_read = 0;
    }
    PG_ASSERT(rg->idx_read < rg->data.len);
    PG_ASSERT(rg->idx_write < rg->data.len);

    u64 read_len2 = data.len - read_len1;
    if (read_len2 > 0) {
      PG_ASSERT(0 == rg->idx_read);

      memcpy(data.data + read_len1, rg->data.data, read_len2);
      rg->idx_read += read_len2;
      PG_ASSERT(rg->idx_read <= data.len);
      PG_ASSERT(rg->idx_read <= rg->data.len);
      PG_ASSERT(rg->idx_read <= rg->idx_write);
    }
  }

  return true;
}

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_ring_read_until_excl(PgRing *rg, PgString needle, PgArena *arena) {
  PgStringOk res = {0};
  i64 idx = -1;

  {
    PgRing cpy_rg = *rg;
    PgArena cpy_arena = *arena;

    PgString dst = pg_string_make(pg_ring_read_space(*rg), arena);
    PG_ASSERT(pg_ring_read_slice(rg, dst));
    *rg = cpy_rg;       // Reset.
    *arena = cpy_arena; // Reset.

    idx = pg_string_indexof_string(dst, needle);
    if (-1 == idx) {
      return res;
    }
  }

  res.ok = true;
  res.res = pg_string_make((u64)idx, arena);
  PG_ASSERT(pg_ring_read_slice(rg, res.res));

  // Read and throw away the needle.
  {
    PgArena pg_arena_tmp = *arena;
    PgString dst_needle = pg_string_make(needle.len, &pg_arena_tmp);
    PG_ASSERT(pg_ring_read_slice(rg, dst_needle));
    PG_ASSERT(pg_string_eq(needle, dst_needle));
  }

  return res;
}

typedef struct {
  void *ctx;
  ReadFn read_fn;
} PgReader;

typedef struct {
  void *ctx;
  WriteFn write_fn;
} PgWriter;

[[maybe_unused]] [[nodiscard]] static PgSocket pg_reader_socket(PgReader *r) {
  PG_ASSERT(r->ctx);
  return (PgSocket)(u64)r->ctx;
}

[[maybe_unused]] [[nodiscard]] static PgSocket pg_writer_socket(PgWriter *w) {
  PG_ASSERT(w->ctx);
  return (PgSocket)(u64)w->ctx;
}

typedef enum {
  HTTP_METHOD_UNKNOWN,
  HTTP_METHOD_GET,
  HTTP_METHOD_POST
} PgHttpMethod;

[[maybe_unused]]
PgString static pg_http_method_to_string(PgHttpMethod m) {
  switch (m) {
  case HTTP_METHOD_UNKNOWN:
    return PG_S("unknown");
  case HTTP_METHOD_GET:
    return PG_S("GET");
  case HTTP_METHOD_POST:
    return PG_S("POST");
  default:
    PG_ASSERT(0);
  }
}

typedef struct {
  PgString key, value;
} PgKeyValue;

PG_RESULT(PgKeyValue) PgKeyValueResult;

PG_DYN(PgKeyValue) PgKeyValueDyn;

PG_RESULT(PgKeyValueDyn) PgDynKeyValueResult;

typedef struct {
  PgString scheme;
  PgString username, password;
  PgString host; // Including subdomains.
  PgStringDyn path_components;
  PgKeyValueDyn query_parameters;
  u16 port;
  // TODO: fragment.
} PgUrl;

typedef struct {
  PgString id;
  PgUrl url; // Does not have a scheme, domain, port.
  PgHttpMethod method;
  PgKeyValueDyn headers;
  u8 version_minor;
  u8 version_major;
} PgHttpRequest;

// `GET /en-US/docs/Web/HTTP/Messages HTTP/1.1`.
typedef struct {
  PgHttpMethod method;
  u8 version_minor;
  u8 version_major;
  PgUrl url; // Does not have a scheme, domain, port.
} PgHttpRequestStatusLine;

PG_RESULT(PgHttpRequestStatusLine) PgHttpRequestStatusLineResult;

// `HTTP/1.1 201 Created`.
typedef struct {
  u8 version_minor;
  u8 version_major;
  u16 status;
} PgHttpResponseStatusLine;

PG_RESULT(PgHttpResponseStatusLine) PgHttpResponseStatusLineResult;

typedef struct {
  u8 version_major;
  u8 version_minor;
  u16 status;
  PgKeyValueDyn headers;
} PgHttpResponse;

[[maybe_unused]] [[nodiscard]] static PgHttpResponseStatusLineResult
pg_http_parse_response_status_line(PgString status_line) {
  PgHttpResponseStatusLineResult res = {0};

  PgString remaining = status_line;
  {
    PgStringOk consume = pg_string_consume_string(remaining, PG_S("HTTP/"));
    if (!consume.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume.res;
  }

  {
    PgParseNumberResult res_major = pg_string_parse_u64(remaining);
    if (!res_major.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_major.n > 3) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.res.version_major = (u8)res_major.n;
    remaining = res_major.remaining;
  }

  {
    PgStringOk consume = pg_string_consume_byte(remaining, '.');
    if (!consume.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume.res;
  }

  {
    PgParseNumberResult res_minor = pg_string_parse_u64(remaining);
    if (!res_minor.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_minor.n > 9) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.res.version_minor = (u8)res_minor.n;
    remaining = res_minor.remaining;
  }

  {
    PgStringOk consume = pg_string_consume_byte(remaining, ' ');
    if (!consume.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume.res;
  }

  {
    PgParseNumberResult res_status_code = pg_string_parse_u64(remaining);
    if (!res_status_code.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_status_code.n < 100 || res_status_code.n > 599) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.res.status = (u16)res_status_code.n;
    remaining = res_status_code.remaining;
  }

  // TODO: Should we keep the human-readable status code around or validate it?

  return res;
}

[[maybe_unused]]
static void pg_http_push_header(PgKeyValueDyn *headers, PgString key,
                                PgString value, PgArena *arena) {
  *PG_DYN_PUSH(headers, arena) = (PgKeyValue){.key = key, .value = value};
}

[[maybe_unused]] [[nodiscard]] static bool
pg_http_request_write_status_line(PgRing *rg, PgHttpRequest req,
                                  PgArena arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 128, &arena);
  PG_DYN_APPEND_SLICE(&sb, pg_http_method_to_string(req.method), &arena);
  PG_DYN_APPEND_SLICE(&sb, PG_S(" /"), &arena);

  for (u64 i = 0; i < req.url.path_components.len; i++) {
    PgString path_component = PG_DYN_AT(req.url.path_components, i);
    PG_DYN_APPEND_SLICE(&sb, path_component, &arena);

    if (i < req.url.path_components.len - 1) {
      *PG_DYN_PUSH(&sb, &arena) = '/';
    }
  }

  if (req.url.query_parameters.len > 0) {
    *PG_DYN_PUSH(&sb, &arena) = '?';
    for (u64 i = 0; i < req.url.query_parameters.len; i++) {
      PgKeyValue param = PG_DYN_AT(req.url.query_parameters, i);
      pg_url_encode_string(&sb, param.key, param.value, &arena);

      if (i < req.url.query_parameters.len - 1) {
        *PG_DYN_PUSH(&sb, &arena) = '&';
      }
    }
  }

  PG_DYN_APPEND_SLICE(&sb, PG_S(" HTTP/1.1"), &arena);
  PG_DYN_APPEND_SLICE(&sb, PG_S("\r\n"), &arena);

  PgString s = PG_DYN_SLICE(PgString, sb);
  return pg_ring_write_slice(rg, s);
}

[[maybe_unused]] [[nodiscard]] static bool
pg_http_response_write_status_line(PgRing *rg, PgHttpResponse res,
                                   PgArena arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 128, &arena);
  PG_DYN_APPEND_SLICE(&sb, PG_S("HTTP/"), &arena);
  pg_string_builder_append_u64_to_string(&sb, res.version_major, &arena);
  PG_DYN_APPEND_SLICE(&sb, PG_S("."), &arena);
  pg_string_builder_append_u64_to_string(&sb, res.version_minor, &arena);
  PG_DYN_APPEND_SLICE(&sb, PG_S(" "), &arena);

  pg_string_builder_append_u64_to_string(&sb, res.status, &arena);

  PG_DYN_APPEND_SLICE(&sb, PG_S(" \r\n"), &arena);

  PgString s = PG_DYN_SLICE(PgString, sb);
  return pg_ring_write_slice(rg, s);
}

[[maybe_unused]] [[nodiscard]] static bool
pg_http_write_header(PgRing *rg, PgKeyValue header, PgArena arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 128, &arena);
  PG_DYN_APPEND_SLICE(&sb, header.key, &arena);
  PG_DYN_APPEND_SLICE(&sb, PG_S(": "), &arena);
  PG_DYN_APPEND_SLICE(&sb, header.value, &arena);
  PG_DYN_APPEND_SLICE(&sb, PG_S("\r\n"), &arena);

  PgString s = PG_DYN_SLICE(PgString, sb);
  return pg_ring_write_slice(rg, s);
}

// NOTE: Only sanitation for including the string inside an HTML tag e.g.:
// `<div>...ESCAPED_STRING..</div>`.
// To include the string inside other context (e.g. JS, CSS, HTML attributes,
// etc), more advance sanitation is required.
[[maybe_unused]] [[nodiscard]] static PgString
pg_html_sanitize(PgString s, PgArena *arena) {
  Pgu8Dyn res = {0};
  PG_DYN_ENSURE_CAP(&res, s.len, arena);
  for (u64 i = 0; i < s.len; i++) {
    u8 c = PG_SLICE_AT(s, i);

    if ('&' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&amp"), arena);
    } else if ('<' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&lt"), arena);
    } else if ('>' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&gt"), arena);
    } else if ('"' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&quot"), arena);
    } else if ('\'' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&#x27"), arena);
    } else {
      *PG_DYN_PUSH(&res, arena) = c;
    }
  }

  return PG_DYN_SLICE(PgString, res);
}

typedef struct {
  PgString username, password;
} PgUrlUserInfo;

PG_RESULT(PgUrlUserInfo) PgUrlUserInfoResult;

typedef struct {
  PgUrlUserInfo user_info;
  PgString host;
  u16 port;
} PgUrlAuthority;

PG_RESULT(PgUrlAuthority) PgUrlAuthorityResult;

PG_RESULT(PgUrl) PgUrlResult;

[[maybe_unused]] [[nodiscard]] static PgStringDynResult
pg_url_parse_path_components(PgString s, PgArena *arena) {
  PgStringDynResult res = {0};

  if (-1 != pg_string_indexof_any_byte(s, PG_S("?#:"))) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  if (PG_SLICE_IS_EMPTY(s)) {
    return res;
  }

  if (!pg_string_starts_with(s, PG_S("/"))) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  PgStringDyn components = {0};

  PgSplitIterator split_it_slash = pg_string_split_string(s, PG_S("/"));
  for (u64 i = 0; i < s.len; i++) { // Bound.
    PgStringOk split = pg_string_split_next(&split_it_slash);
    if (!split.ok) {
      break;
    }

    if (PG_SLICE_IS_EMPTY(split.res)) {
      continue;
    }

    *PG_DYN_PUSH(&components, arena) = split.res;
  }

  res.res = components;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgDynKeyValueResult
pg_url_parse_query_parameters(PgString s, PgArena *arena) {
  PgDynKeyValueResult res = {0};

  PgString remaining = s;
  {
    PgStringOk res_consume_question = pg_string_consume_byte(s, '?');
    if (!res_consume_question.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = res_consume_question.res;
  }

  for (u64 _i = 0; _i < s.len; _i++) {
    PgStringPairConsume res_consume_and =
        pg_string_consume_until_byte_incl(remaining, '&');
    remaining = res_consume_and.right;

    PgString kv = res_consume_and.left;
    PgStringPairConsume res_consume_eq =
        pg_string_consume_until_byte_incl(kv, '=');
    PgString k = res_consume_eq.left;
    PgString v = res_consume_eq.consumed ? res_consume_eq.right : PG_S("");

    if (!PG_SLICE_IS_EMPTY(k)) {
      *PG_DYN_PUSH(&res.res, arena) = (PgKeyValue){.key = k, .value = v};
    }

    if (!res_consume_and.consumed) {
      break;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgUrlUserInfoResult
pg_url_parse_user_info(PgString s) {
  PgUrlUserInfoResult res = {0};
  // https://www.rfc-editor.org/rfc/rfc3986#section-3.2.1:
  // Use of the format "user:password" in the userinfo field is
  // deprecated.  Applications should not render as clear text any data
  // after the first colon (":") character found within a userinfo
  // subcomponent unless the data after the colon is the empty string
  // (indicating no password).  Applications may choose to ignore or
  // reject such data when it is received.

  if (PG_SLICE_IS_EMPTY(s)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  return res;
}

PG_RESULT(u16) Pgu16Result;

[[maybe_unused]] [[nodiscard]] static Pgu16Result
pg_url_parse_port(PgString s) {
  Pgu16Result res = {0};

  // Allowed.
  if (PG_SLICE_IS_EMPTY(s)) {
    return res;
  }

  PgParseNumberResult port_parse = pg_string_parse_u64(s);
  if (!PG_SLICE_IS_EMPTY(port_parse.remaining)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  if (port_parse.n > UINT16_MAX) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  res.res = (u16)port_parse.n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgUrlAuthorityResult
pg_url_parse_authority(PgString s) {
  PgUrlAuthorityResult res = {0};

  PgString remaining = s;
  // User info, optional.
  {
    PgStringPairConsume user_info_and_rem =
        pg_string_consume_until_byte_incl(remaining, '@');
    remaining = user_info_and_rem.right;

    if (user_info_and_rem.consumed) {
      PgUrlUserInfoResult res_user_info =
          pg_url_parse_user_info(user_info_and_rem.left);
      if (res_user_info.err) {
        res.err = res_user_info.err;
        return res;
      }
      res.res.user_info = res_user_info.res;
    }
  }

  // Host, mandatory.
  PgStringPairConsume host_and_rem =
      pg_string_consume_until_byte_incl(remaining, ':');
  {
    remaining = host_and_rem.right;
    res.res.host = host_and_rem.left;
    if (PG_SLICE_IS_EMPTY(res.res.host)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
  }

  // Port, optional.
  if (host_and_rem.consumed) {
    Pgu16Result res_port = pg_url_parse_port(host_and_rem.right);
    if (res_port.err) {
      res.err = res_port.err;
      return res;
    }
    res.res.port = res_port.res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_url_is_scheme_valid(PgString scheme) {
  if (PG_SLICE_IS_EMPTY(scheme)) {
    return false;
  }

  u8 first = PG_SLICE_AT(scheme, 0);
  if (!pg_character_is_alphabetical(first)) {
    return false;
  }

  for (u64 i = 0; i < scheme.len; i++) {
    u8 c = PG_SLICE_AT(scheme, i);
    if (!(pg_character_is_alphanumeric(c) || c == '+' || c == '-' ||
          c == '.')) {
      return false;
    }
  }

  return true;
}

[[maybe_unused]] [[nodiscard]] static PgUrlResult
pg_url_parse_after_authority(PgString s, PgArena *arena) {
  PgUrlResult res = {0};
  PgString remaining = s;

  PgStringPairConsumeAny path_components_and_rem =
      pg_string_consume_until_any_byte_excl(remaining, PG_S("?#"));
  remaining = path_components_and_rem.right;

  // Path, optional.
  if (pg_string_starts_with(s, PG_S("/"))) {
    PG_ASSERT(!PG_SLICE_IS_EMPTY(path_components_and_rem.left));

    PgStringDynResult res_path_components =
        pg_url_parse_path_components(path_components_and_rem.left, arena);
    if (res_path_components.err) {
      res.err = res_path_components.err;
      return res;
    }
    res.res.path_components = res_path_components.res;
  }

  // Query parameters, optional.
  if (path_components_and_rem.consumed &&
      path_components_and_rem.matched == '?') {
    PgDynKeyValueResult res_query =
        pg_url_parse_query_parameters(path_components_and_rem.right, arena);
    if (res_query.err) {
      res.err = res_query.err;
      return res;
    }
    res.res.query_parameters = res_query.res;
  }

  // TODO: fragments.

  PG_ASSERT(PG_SLICE_IS_EMPTY(res.res.scheme));
  PG_ASSERT(PG_SLICE_IS_EMPTY(res.res.username));
  PG_ASSERT(PG_SLICE_IS_EMPTY(res.res.password));
  PG_ASSERT(PG_SLICE_IS_EMPTY(res.res.host));
  PG_ASSERT(0 == res.res.port);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgUrlResult pg_url_parse(PgString s,
                                                               PgArena *arena) {
  PgUrlResult res = {0};

  PgString remaining = s;

  // Scheme, mandatory.
  {
    PgStringPairConsume scheme_and_rem =
        pg_string_consume_until_byte_incl(remaining, ':');
    remaining = scheme_and_rem.right;

    if (!scheme_and_rem.consumed) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    if (!pg_url_is_scheme_valid(scheme_and_rem.left)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.res.scheme = scheme_and_rem.left;
  }

  // Assume `://` as separator between the scheme and authority.
  // TODO: Be less strict hier.
  {

    PgStringOk res_consume = pg_string_consume_string(remaining, PG_S("//"));
    if (!res_consume.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = res_consume.res;
  }

  // Authority, mandatory.
  PgStringPairConsumeAny authority_and_rem =
      pg_string_consume_until_any_byte_excl(remaining, PG_S("/?#"));
  remaining = authority_and_rem.right;
  {
    if (PG_SLICE_IS_EMPTY(authority_and_rem.left)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    PgUrlAuthorityResult res_authority =
        pg_url_parse_authority(authority_and_rem.left);
    if (res_authority.err) {
      res.err = res_authority.err;
      return res;
    }
    res.res.host = res_authority.res.host;
    res.res.port = res_authority.res.port;
    res.res.username = res_authority.res.user_info.username;
    res.res.password = res_authority.res.user_info.password;
  }

  PgUrlResult res_after_authority =
      pg_url_parse_after_authority(remaining, arena);
  if (res_after_authority.err) {
    res.err = res_after_authority.err;
    return res;
  }
  res.res.path_components = res_after_authority.res.path_components;
  res.res.query_parameters = res_after_authority.res.query_parameters;

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool pg_http_url_is_valid(PgUrl u) {
  // TODO: Support https.
  if (!pg_string_eq(u.scheme, PG_S("http"))) {
    return false;
  }

  return true;
}

[[maybe_unused]] [[nodiscard]] static PgHttpRequestStatusLineResult
pg_http_parse_request_status_line(PgString status_line, PgArena *arena) {
  PgHttpRequestStatusLineResult res = {0};

  PgString remaining = status_line;
  {
    if (pg_string_starts_with(remaining, PG_S("GET"))) {
      PgStringOk consume = pg_string_consume_string(remaining, PG_S("GET"));
      PG_ASSERT(consume.ok);
      remaining = consume.res;
      res.res.method = HTTP_METHOD_GET;
    } else if (pg_string_starts_with(remaining, PG_S("POST"))) {
      PgStringOk consume = pg_string_consume_string(remaining, PG_S("POST"));
      PG_ASSERT(consume.ok);
      remaining = consume.res;
      res.res.method = HTTP_METHOD_POST;
    } else {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
  }

  {
    PgStringOk consume = pg_string_consume_byte(remaining, ' ');
    if (!consume.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume.res;
  }

  i64 idx_space = pg_string_indexof_byte(remaining, ' ');
  if (-1 == idx_space) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  PgString path = PG_SLICE_RANGE(remaining, 0, (u64)idx_space);
  remaining = PG_SLICE_RANGE_START(remaining, (u64)idx_space + 1);
  {
    PgUrlResult res_url = pg_url_parse_after_authority(path, arena);
    if (res_url.err) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    res.res.url = res_url.res;
  }

  {
    PgStringOk consume = pg_string_consume_string(remaining, PG_S("HTTP/"));
    if (!consume.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume.res;
  }

  {
    PgParseNumberResult res_major = pg_string_parse_u64(remaining);
    if (!res_major.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_major.n > 3) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.res.version_major = (u8)res_major.n;
    remaining = res_major.remaining;
  }

  {
    PgStringOk consume = pg_string_consume_byte(remaining, '.');
    if (!consume.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume.res;
  }

  {
    PgParseNumberResult res_minor = pg_string_parse_u64(remaining);
    if (!res_minor.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_minor.n > 9) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.res.version_minor = (u8)res_minor.n;
    remaining = res_minor.remaining;
  }

  if (!PG_SLICE_IS_EMPTY(remaining)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgKeyValueResult
pg_http_parse_header(PgString s) {
  PgKeyValueResult res = {0};

  i64 idx = pg_string_indexof_byte(s, ':');
  if (-1 == idx) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.res.key = PG_SLICE_RANGE(s, 0, (u64)idx);
  if (PG_SLICE_IS_EMPTY(res.res.key)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.res.value =
      pg_string_trim_left(PG_SLICE_RANGE_START(s, (u64)idx + 1), ' ');
  if (PG_SLICE_IS_EMPTY(res.res.value)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  return res;
}

typedef struct {
  bool done;
  PgHttpResponse res;
  PgError err;
} PgHttpResponseReadResult;

typedef struct {
  bool done;
  PgHttpRequest res;
  PgError err;
} PgHttpRequestReadResult;

[[maybe_unused]] [[nodiscard]] static PgHttpResponseReadResult
pg_http_read_response(PgRing *rg, u64 max_http_headers, PgArena *arena) {
  PgHttpResponseReadResult res = {0};
  PgString sep = PG_S("\r\n\r\n");

  PgStringOk s = pg_ring_read_until_excl(rg, sep, arena);
  if (!s.ok) { // In progress.
    return res;
  }

  PgSplitIterator it = pg_string_split_string(s.res, PG_S("\r\n"));
  PgStringOk res_split = pg_string_split_next(&it);
  if (!res_split.ok) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  PgHttpResponseStatusLineResult res_status_line =
      pg_http_parse_response_status_line(res_split.res);
  if (res_status_line.err) {
    res.err = res_status_line.err;
    return res;
  }

  res.res.status = res_status_line.res.status;
  res.res.version_major = res_status_line.res.version_major;
  res.res.version_minor = res_status_line.res.version_minor;

  for (u64 _i = 0; _i < max_http_headers; _i++) {
    res_split = pg_string_split_next(&it);
    if (!res_split.ok) {
      break;
    }
    PgKeyValueResult res_kv = pg_http_parse_header(res_split.res);
    if (res_kv.err) {
      res.err = res_kv.err;
      return res;
    }

    *PG_DYN_PUSH(&res.res.headers, arena) = res_kv.res;
  }
  if (!PG_SLICE_IS_EMPTY(it.s)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.done = true;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgHttpRequestReadResult
pg_http_read_request(PgRing *rg, u64 max_http_headers, PgArena *arena) {
  PgHttpRequestReadResult res = {0};
  PgString sep = PG_S("\r\n\r\n");

  PgStringOk s = pg_ring_read_until_excl(rg, sep, arena);
  if (!s.ok) { // In progress.
    return res;
  }

  PgSplitIterator it = pg_string_split_string(s.res, PG_S("\r\n"));
  PgStringOk res_split = pg_string_split_next(&it);
  if (!res_split.ok) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  PgHttpRequestStatusLineResult res_status_line =
      pg_http_parse_request_status_line(res_split.res, arena);
  if (res_status_line.err) {
    res.err = res_status_line.err;
    return res;
  }

  res.res.method = res_status_line.res.method;
  res.res.url = res_status_line.res.url;
  res.res.version_major = res_status_line.res.version_major;
  res.res.version_minor = res_status_line.res.version_minor;

  for (u64 _i = 0; _i < max_http_headers; _i++) {
    res_split = pg_string_split_next(&it);
    if (!res_split.ok) {
      break;
    }
    PgKeyValueResult res_kv = pg_http_parse_header(res_split.res);
    if (res_kv.err) {
      res.err = res_kv.err;
      return res;
    }

    *PG_DYN_PUSH(&res.res.headers, arena) = res_kv.res;
  }
  if (!PG_SLICE_IS_EMPTY(it.s)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.done = true;
  return res;
}

[[maybe_unused]] static PgError
pg_http_write_request(PgRing *rg, PgHttpRequest res, PgArena arena) {
  if (!pg_http_request_write_status_line(rg, res, arena)) {
    return (PgError)PG_ERR_OUT_OF_MEMORY;
  }

  for (u64 i = 0; i < res.headers.len; i++) {
    PgKeyValue header = PG_DYN_AT(res.headers, i);
    if (!pg_http_write_header(rg, header, arena)) {
      return (PgError)PG_ERR_OUT_OF_MEMORY;
    }
  }
  if (!pg_ring_write_slice(rg, PG_S("\r\n"))) {
    return (PgError)PG_ERR_OUT_OF_MEMORY;
  }

  return (PgError)0;
}

[[maybe_unused]] static PgError
pg_http_write_response(PgRing *rg, PgHttpResponse res, PgArena arena) {
  if (!pg_http_response_write_status_line(rg, res, arena)) {
    return (PgError)PG_ERR_OUT_OF_MEMORY;
  }
  for (u64 i = 0; i < res.headers.len; i++) {
    PgKeyValue header = PG_DYN_AT(res.headers, i);
    if (!pg_http_write_header(rg, header, arena)) {
      return (PgError)PG_ERR_OUT_OF_MEMORY;
    }
  }
  if (!pg_ring_write_slice(rg, PG_S("\r\n"))) {

    return (PgError)PG_ERR_OUT_OF_MEMORY;
  }
  return (PgError)0;
}

[[maybe_unused]] [[nodiscard]] static PgReader
pg_reader_make_from_socket(PgSocket socket) {
  // TODO: Windows.
  return (PgReader){.read_fn = pg_unix_read, .ctx = (void *)(u64)socket};
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_socket(PgSocket socket) {
  // TODO: Windows.
  return (PgWriter){.write_fn = pg_unix_write, .ctx = (void *)(u64)socket};
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_file(PgFile *file) {
  // TODO: Windows.
  return (PgWriter){.write_fn = pg_unix_write, .ctx = (void *)file};
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_string_builder(StringBuilder *sb) {
  return (PgWriter){.write_fn = pg_string_builder_write, .ctx = (void *)sb};
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_reader_read(PgReader *r, PgRing *rg, PgArena arena) {
  PG_ASSERT(nullptr != r->read_fn);

  Pgu64Result res = {0};

  PgString dst = pg_string_make(pg_ring_write_space(*rg), &arena);
  res = r->read_fn(r->ctx, dst.data, dst.len);

  if (res.err) {
    return res;
  }
  dst.len = res.res;
  PG_ASSERT(true == pg_ring_write_slice(rg, dst));

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_write(PgWriter *w, PgRing *rg, PgArena arena) {
  PG_ASSERT(nullptr != w->write_fn);

  PgString dst = pg_string_make(pg_ring_read_space(*rg), &arena);
  PG_ASSERT(true == pg_ring_read_slice(rg, dst));

  return w->write_fn(w->ctx, dst.data, dst.len);
}

#if 0
[[nodiscard]] static PgParseNumberResult
request_parse_content_length_maybe(PgHttpRequest req, PgArena *arena) {
  PG_ASSERT(!req.err);

  for (u64 i = 0; i < req.headers.len; i++) {
    PgKeyValue h = req.headers.data[i];

    if (!pg_string_ieq_ascii(PG_S("Content-Length"), h.key, arena)) {
      continue;
    }

    return pg_string_parse_u64(h.value);
  }
  return (PgParseNumberResult){0};
}

[[maybe_unused]] [[nodiscard]] static PgHttpResponse
pg_http_client_request(PgIpv4AddressSocket sock, PgHttpRequest req, PgArena *arena) {
  PgHttpResponse res = {0};

  if (!PG_SLICE_IS_EMPTY(req.path_raw)) {
    // Should use `req.path_components`, not `path.raw`.
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  if (HM_UNKNOWN == req.method) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  PgString pg_http_request_serialized = pg_http_request_serialize(req, arena);
  log(PG_LOG_LEVEL_DEBUG, "http request", arena, PG_L("ip", sock.address.ip),
      PG_L("port", sock.address.port), PG_L("serialized", pg_http_request_serialized));

  // TODO: should not be an assert but a returned error.
  PG_ASSERT(send(sock.socket, pg_http_request_serialized.data,
              pg_http_request_serialized.len,
              0) == (i64)pg_http_request_serialized.len);

  BufferedReader reader = buffered_reader_make(sock.socket, arena);

  {
    IoResult io_result =
        buffered_reader_read_until_slice(&reader, PG_S("\r\n"), arena);
    if (io_result.err) {
      res.err = io_result.err;
      goto end;
    }
    if (PG_SLICE_IS_EMPTY(io_result.res)) {
      res.err = HS_ERR_INVALID_HTTP_RESPONSE;
      goto end;
    }

    PgString http1_1_version_needle = PG_S("HTTP/1.1 ");
    PgString http1_0_version_needle = PG_S("HTTP/1.0 ");
    PG_ASSERT(http1_0_version_needle.len == http1_1_version_needle.len);

    if (!(pg_string_starts_with(io_result.res, http1_0_version_needle) ||
          pg_string_starts_with(io_result.res, http1_1_version_needle))) {
      res.err = HS_ERR_INVALID_HTTP_RESPONSE;
      goto end;
    }

    PgString status_str =
        PG_SLICE_RANGE_START(io_result.res, http1_1_version_needle.len);
    PgParseNumberResult status_parsed = pg_string_parse_u64(status_str);
    if (!status_parsed.present) {
      res.err = HS_ERR_INVALID_HTTP_RESPONSE;
      goto end;
    }
    if (!status_parsed.present) {
      res.err = PG_ERR_INVALID_VALUE;
      goto end;
    }
    if (!(200 <= status_parsed.n && status_parsed.n <= 599)) {
      res.err = PG_ERR_INVALID_VALUE;
      goto end;
    }

    res.status = (u16)status_parsed.n;
  }

  res.err = pg_http_read_headers(&reader, &res.headers, arena);
  if (res.err) {
    log(PG_LOG_LEVEL_ERROR, "http request failed to read headers", arena,
        PG_L("req.method", req.method), PG_L("req.path_raw", req.path_raw),
        PG_L("err", res.err));
    goto end;
  }

  PgParseNumberResult content_length =
      request_parse_content_length_maybe(req, arena);
  if (!content_length.present) {
    res.err = HS_ERR_INVALID_HTTP_REQUEST;
    return res;
  }

  if (content_length.present) {
    IoResult res_io =
        pg_reader_read_exactly((PgReader *)&reader, content_length.n, arena);
    if (res_io.err) {
      res.err = res_io.err;
      return res;
    }
    res.body = res_io.res;
  }

end:
  (void)pg_net_socket_close(sock.socket);
  return res;
}
#endif

typedef struct {
  PgString key, value;
} PgFormDataKV;

PG_DYN(PgFormDataKV) PgFormDataKVDyn;

typedef struct {
  // NOTE: Repeated keys are allowed, that's how 'arrays' are encoded.
  PgFormDataKVDyn form;
  PgError err;
} PgFormDataParseResult;

typedef struct {
  PgFormDataKV kv;
  PgError err;
  PgString remaining;
} PgFormDataKVParseResult;

typedef struct {
  PgString data;
  PgError err;
  PgString remaining;
} PgFormDataKVElementParseResult;

[[nodiscard]] static PgFormDataKVElementParseResult
pg_form_data_kv_parse_element(PgString in, u8 pg_character_terminator,
                              PgArena *arena) {
  PgFormDataKVElementParseResult res = {0};
  Pgu8Dyn data = {0};

  u64 i = 0;
  for (; i < in.len; i++) {
    u8 c = in.data[i];

    if ('+' == c) {
      *PG_DYN_PUSH(&data, arena) = ' ';
    } else if ('%' == c) {
      if ((in.len - i) < 2) {
        res.err = PG_ERR_INVALID_VALUE;
        return res;
      }
      u8 c1 = in.data[i + 1];
      u8 c2 = in.data[i + 2];

      if (!(pg_character_is_hex_digit(c1) && pg_character_is_hex_digit(c2))) {
        res.err = PG_ERR_INVALID_VALUE;
        return res;
      }

      u8 utf8_character =
          pg_character_from_hex(c1) * 16 + pg_character_from_hex(c2);
      *PG_DYN_PUSH(&data, arena) = utf8_character;
      i += 2; // Consume 2 characters.
    } else if (pg_character_terminator == c) {
      i += 1; // Consume.
      break;
    } else {
      *PG_DYN_PUSH(&data, arena) = c;
    }
  }

  res.data = PG_DYN_SLICE(PgString, data);
  res.remaining = PG_SLICE_RANGE_START(in, i);
  return res;
}

[[nodiscard]] static PgFormDataKVParseResult
pg_form_data_kv_parse(PgString in, PgArena *arena) {
  PgFormDataKVParseResult res = {0};

  PgString remaining = in;

  PgFormDataKVElementParseResult key_parsed =
      pg_form_data_kv_parse_element(remaining, '=', arena);
  if (key_parsed.err) {
    res.err = key_parsed.err;
    return res;
  }
  res.kv.key = key_parsed.data;

  remaining = key_parsed.remaining;

  PgFormDataKVElementParseResult value_parsed =
      pg_form_data_kv_parse_element(remaining, '&', arena);
  if (value_parsed.err) {
    res.err = value_parsed.err;
    return res;
  }
  res.kv.value = value_parsed.data;
  res.remaining = value_parsed.remaining;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFormDataParseResult
pg_form_data_parse(PgString in, PgArena *arena) {
  PgFormDataParseResult res = {0};

  PgString remaining = in;

  for (u64 i = 0; i < in.len; i++) { // Bound.
    if (PG_SLICE_IS_EMPTY(remaining)) {
      break;
    }

    PgFormDataKVParseResult kv = pg_form_data_kv_parse(remaining, arena);
    if (kv.err) {
      res.err = kv.err;
      return res;
    }

    *PG_DYN_PUSH(&res.form, arena) = kv.kv;

    remaining = kv.remaining;
  }
  return res;
}

typedef enum {
  PG_HTML_NONE,
  PG_HTML_TITLE,
  PG_HTML_SPAN,
  PG_HTML_INPUT,
  PG_HTML_BUTTON,
  PG_HTML_LINK,
  PG_HTML_META,
  PG_HTML_HEAD,
  PG_HTML_BODY,
  PG_HTML_DIV,
  PG_HTML_OL,
  PG_HTML_LI,
  PG_HTML_TEXT,
  PG_HTML_FORM,
  PG_HTML_FIELDSET,
  PG_HTML_LABEL,
  PG_HTML_SCRIPT,
  PG_HTML_STYLE,
  PG_HTML_LEGEND,
  PG_HTML_MAX, // Pseudo.
} PgHtmlKind;

typedef struct PgHtmlElement PgHtmlElement;
PG_DYN(PgHtmlElement) PgHtmlElementDyn;

struct PgHtmlElement {
  PgHtmlKind kind;
  PgKeyValueDyn attributes;
  union {
    PgHtmlElementDyn children;
    PgString
        text; // Only for `PG_HTML_TEXT`, `PG_HTML_LEGEND`, `PG_HTML_TITLE`,
              // `PG_HTML_SCRIPT`, `PG_HTML_STYLE`, `PG_HTML_BUTTON`.
  };
};

typedef struct {
  PgHtmlElement body;
  PgHtmlElement head;
} PgHtmlDocument;

[[maybe_unused]] [[nodiscard]] static PgHtmlDocument
pg_html_make(PgString title, PgArena *arena) {
  PgHtmlDocument res = {0};

  {

    PgHtmlElement tag_head = {.kind = PG_HTML_HEAD};
    {
      PgHtmlElement tag_meta = {.kind = PG_HTML_META};
      {
        *PG_DYN_PUSH(&tag_meta.attributes, arena) =
            (PgKeyValue){.key = PG_S("charset"), .value = PG_S("utf-8")};
      }
      *PG_DYN_PUSH(&tag_head.children, arena) = tag_meta;
    }
    {
      PgHtmlElement tag_title = {.kind = PG_HTML_TITLE, .text = title};
      *PG_DYN_PUSH(&tag_head.children, arena) = tag_title;
    }

    res.head = tag_head;

    PgHtmlElement tag_body = {.kind = PG_HTML_BODY};
    res.body = tag_body;
  }

  return res;
}

static void pg_html_attributes_to_string(PgKeyValueDyn attributes, Pgu8Dyn *sb,
                                         PgArena *arena) {
  for (u64 i = 0; i < attributes.len; i++) {
    PgKeyValue attr = PG_DYN_AT(attributes, i);
    PG_ASSERT(-1 == pg_string_indexof_string(attr.key, PG_S("\"")));

    *PG_DYN_PUSH(sb, arena) = ' ';
    PG_DYN_APPEND_SLICE(sb, attr.key, arena);
    *PG_DYN_PUSH(sb, arena) = '=';
    *PG_DYN_PUSH(sb, arena) = '"';
    // TODO: escape string.
    PG_DYN_APPEND_SLICE(sb, attr.value, arena);
    *PG_DYN_PUSH(sb, arena) = '"';
  }
}

static void pg_html_tags_to_string(PgHtmlElementDyn elements, Pgu8Dyn *sb,
                                   PgArena *arena);
static void pg_html_tag_to_string(PgHtmlElement e, Pgu8Dyn *sb, PgArena *arena);

static void pg_html_tags_to_string(PgHtmlElementDyn elements, Pgu8Dyn *sb,
                                   PgArena *arena) {
  for (u64 i = 0; i < elements.len; i++) {
    PgHtmlElement e = PG_DYN_AT(elements, i);
    pg_html_tag_to_string(e, sb, arena);
  }
}

[[maybe_unused]]
static void pg_html_document_to_string(PgHtmlDocument doc, Pgu8Dyn *sb,
                                       PgArena *arena) {
  PG_DYN_APPEND_SLICE(sb, PG_S("<!DOCTYPE html>"), arena);

  PG_DYN_APPEND_SLICE(sb, PG_S("<html>"), arena);
  pg_html_tag_to_string(doc.head, sb, arena);
  pg_html_tag_to_string(doc.body, sb, arena);
  PG_DYN_APPEND_SLICE(sb, PG_S("</html>"), arena);
}

static void pg_html_tag_to_string(PgHtmlElement e, Pgu8Dyn *sb,
                                  PgArena *arena) {
  static const PgString tag_to_string[PG_HTML_MAX] = {
      [PG_HTML_NONE] = PG_S("FIXME"),
      [PG_HTML_TITLE] = PG_S("title"),
      [PG_HTML_SPAN] = PG_S("span"),
      [PG_HTML_INPUT] = PG_S("input"),
      [PG_HTML_BUTTON] = PG_S("button"),
      [PG_HTML_LINK] = PG_S("link"),
      [PG_HTML_META] = PG_S("meta"),
      [PG_HTML_HEAD] = PG_S("head"),
      [PG_HTML_BODY] = PG_S("body"),
      [PG_HTML_DIV] = PG_S("div"),
      [PG_HTML_TEXT] = PG_S("span"),
      [PG_HTML_FORM] = PG_S("form"),
      [PG_HTML_FIELDSET] = PG_S("fieldset"),
      [PG_HTML_LABEL] = PG_S("label"),
      [PG_HTML_SCRIPT] = PG_S("script"),
      [PG_HTML_STYLE] = PG_S("style"),
      [PG_HTML_LEGEND] = PG_S("legend"),
      [PG_HTML_OL] = PG_S("ol"),
      [PG_HTML_LI] = PG_S("li"),
  };

  PG_ASSERT(!(PG_HTML_NONE == e.kind || PG_HTML_MAX == e.kind));

  *PG_DYN_PUSH(sb, arena) = '<';
  PG_DYN_APPEND_SLICE(sb, tag_to_string[e.kind], arena);
  pg_html_attributes_to_string(e.attributes, sb, arena);
  *PG_DYN_PUSH(sb, arena) = '>';

  switch (e.kind) {
  // Cases of tag without any children and no closing tag.
  case PG_HTML_LINK:
    [[fallthrough]];
  case PG_HTML_META:
    PG_ASSERT(0 == e.children.len);
    return;

  // 'Normal' tags.
  case PG_HTML_OL:
    [[fallthrough]];
  case PG_HTML_LI:
    [[fallthrough]];
  case PG_HTML_HEAD:
    [[fallthrough]];
  case PG_HTML_DIV:
    [[fallthrough]];
  case PG_HTML_FORM:
    [[fallthrough]];
  case PG_HTML_FIELDSET:
    [[fallthrough]];
  case PG_HTML_LABEL:
    [[fallthrough]];
  case PG_HTML_INPUT:
    [[fallthrough]];
  case PG_HTML_SPAN:
    [[fallthrough]];
  case PG_HTML_BODY:
    pg_html_tags_to_string(e.children, sb, arena);
    break;

  // Only cases where `.text` is valid.
  case PG_HTML_BUTTON:
    [[fallthrough]];
  case PG_HTML_SCRIPT:
    [[fallthrough]];
  case PG_HTML_STYLE:
    [[fallthrough]];
  case PG_HTML_LEGEND:
    [[fallthrough]];
  case PG_HTML_TITLE:
    [[fallthrough]];
  case PG_HTML_TEXT:
    PG_DYN_APPEND_SLICE(sb, e.text, arena);
    break;

  // Invalid cases.
  case PG_HTML_NONE:
    [[fallthrough]];
  case PG_HTML_MAX:
    [[fallthrough]];
  default:
    PG_ASSERT(0);
  }

  PG_DYN_APPEND_SLICE(sb, PG_S("</"), arena);
  PG_DYN_APPEND_SLICE(sb, tag_to_string[e.kind], arena);
  *PG_DYN_PUSH(sb, arena) = '>';
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_http_req_extract_cookie_with_name(PgHttpRequest req, PgString cookie_name,
                                     PgArena *arena) {
  PgString res = {0};
  {
    for (u64 i = 0; i < req.headers.len; i++) {
      PgKeyValue h = PG_SLICE_AT(req.headers, i);

      if (!pg_string_ieq_ascii(h.key, PG_S("Cookie"), arena)) {
        continue;
      }
      if (PG_SLICE_IS_EMPTY(h.value)) {
        continue;
      }

      PgSplitIterator it_semicolon = pg_string_split_string(h.value, PG_S(";"));
      for (u64 j = 0; j < h.value.len; j++) {
        PgStringOk split_semicolon = pg_string_split_next(&it_semicolon);
        if (!split_semicolon.ok) {
          break;
        }

        PgSplitIterator it_equals =
            pg_string_split_string(split_semicolon.res, PG_S("="));
        PgStringOk split_equals_left = pg_string_split_next(&it_equals);
        if (!split_equals_left.ok) {
          break;
        }
        if (!pg_string_eq(split_equals_left.res, cookie_name)) {
          // Could be: `; Secure;`
          continue;
        }
        PgStringOk split_equals_right = pg_string_split_next(&it_equals);
        if (!PG_SLICE_IS_EMPTY(split_equals_right.res)) {
          return split_equals_right.res;
        }
      }
    }
  }
  return res;
}

typedef enum {
  PG_LOG_VALUE_STRING,
  PG_LOG_VALUE_U64,
} PgLogValueKind;

typedef enum {
  PG_LOG_LEVEL_DEBUG,
  PG_LOG_LEVEL_INFO,
  PG_LOG_LEVEL_ERROR,
  PG_LOG_LEVEL_FATAL,
} PgLogLevel;

typedef struct {
  PgLogLevel level;
  PgWriter writer;
} PgLogger;

typedef struct {
  PgLogValueKind kind;
  union {
    PgString s;
    u64 n64;
    u128 n128;
  };
} PgLogValue;

typedef struct {
  PgString key;
  PgLogValue value;
} PgLogEntry;

[[maybe_unused]] [[nodiscard]] static PgLogger
pg_log_logger_make_stdout_json(PgLogLevel level) {
  PgLogger logger = {
      .level = level,
      .writer = pg_writer_make_from_file(
          (PgFile *)(u64)STDOUT_FILENO), // TODO: Windows
  };

  return logger;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_log_level_to_string(PgLogLevel level) {
  switch (level) {
  case PG_LOG_LEVEL_DEBUG:
    return PG_S("debug");
  case PG_LOG_LEVEL_INFO:
    return PG_S("info");
  case PG_LOG_LEVEL_ERROR:
    return PG_S("error");
  case PG_LOG_LEVEL_FATAL:
    return PG_S("fatal");
  default:
    PG_ASSERT(false);
  }
}

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_entry_int(PgString k,
                                                                  int v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_entry_u16(PgString k,
                                                                  u16 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_entry_u32(PgString k,
                                                                  u32 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_entry_u64(PgString k,
                                                                  u64 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = v,
  };
}

[[maybe_unused]] [[nodiscard]] static PgLogEntry
pg_log_entry_slice(PgString k, PgString v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_STRING,
      .value.s = v,
  };
}

#define PG_L(k, v)                                                             \
  (_Generic((v),                                                               \
       int: pg_log_entry_int,                                                  \
       u16: pg_log_entry_u16,                                                  \
       u32: pg_log_entry_u32,                                                  \
       u64: pg_log_entry_u64,                                                  \
       PgString: pg_log_entry_slice)((PG_S(k)), (v)))

#define PG_LOG_ARGS_COUNT(...)                                                 \
  (sizeof((PgLogEntry[]){__VA_ARGS__}) / sizeof(PgLogEntry))
#define pg_log(logger, lvl, msg, arena, ...)                                   \
  do {                                                                         \
    if ((logger)->level > (lvl)) {                                             \
      break;                                                                   \
    };                                                                         \
    PgArena xxx_tmp_arena = (arena);                                           \
    PgString xxx_log_line =                                                    \
        pg_log_make_log_line(lvl, PG_S(msg), &xxx_tmp_arena,                   \
                             PG_LOG_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__);     \
    (logger)->writer.write_fn((logger)->writer.ctx, xxx_log_line.data,         \
                              xxx_log_line.len);                               \
  } while (0)

[[maybe_unused]] [[nodiscard]] static PgString
pg_json_escape_string(PgString entry, PgArena *arena) {
  Pgu8Dyn sb = {0};
  *PG_DYN_PUSH(&sb, arena) = '"';

  for (u64 i = 0; i < entry.len; i++) {
    u8 c = PG_SLICE_AT(entry, i);
    if ('"' == c) {
      *PG_DYN_PUSH(&sb, arena) = '\\';
      *PG_DYN_PUSH(&sb, arena) = '"';
    } else if ('\\' == c) {
      *PG_DYN_PUSH(&sb, arena) = '\\';
      *PG_DYN_PUSH(&sb, arena) = '\\';
    } else if ('\b' == c) {
      *PG_DYN_PUSH(&sb, arena) = '\\';
      *PG_DYN_PUSH(&sb, arena) = 'b';
    } else if ('\f' == c) {
      *PG_DYN_PUSH(&sb, arena) = '\\';
      *PG_DYN_PUSH(&sb, arena) = 'f';
    } else if ('\n' == c) {
      *PG_DYN_PUSH(&sb, arena) = '\\';
      *PG_DYN_PUSH(&sb, arena) = 'n';
    } else if ('\r' == c) {
      *PG_DYN_PUSH(&sb, arena) = '\\';
      *PG_DYN_PUSH(&sb, arena) = 'r';
    } else if ('\t' == c) {
      *PG_DYN_PUSH(&sb, arena) = '\\';
      *PG_DYN_PUSH(&sb, arena) = 't';
    } else {
      *PG_DYN_PUSH(&sb, arena) = c;
    }
  }
  *PG_DYN_PUSH(&sb, arena) = '"';

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_json_unescape_string(PgString entry, PgArena *arena) {
  Pgu8Dyn sb = {0};

  for (u64 i = 0; i < entry.len; i++) {
    u8 c = PG_SLICE_AT(entry, i);
    u8 next = i + 1 < entry.len ? PG_SLICE_AT(entry, i + 1) : 0;

    if ('\\' == c) {
      if ('"' == next) {
        *PG_DYN_PUSH(&sb, arena) = '"';
        i += 1;
      } else if ('\\' == next) {
        *PG_DYN_PUSH(&sb, arena) = '\\';
        i += 1;
      } else if ('b' == next) {
        *PG_DYN_PUSH(&sb, arena) = '\b';
        i += 1;
      } else if ('f' == next) {
        *PG_DYN_PUSH(&sb, arena) = '\f';
        i += 1;
      } else if ('n' == next) {
        *PG_DYN_PUSH(&sb, arena) = '\n';
        i += 1;
      } else if ('r' == next) {
        *PG_DYN_PUSH(&sb, arena) = '\r';
        i += 1;
      } else if ('t' == next) {
        *PG_DYN_PUSH(&sb, arena) = '\t';
        i += 1;
      } else {
        *PG_DYN_PUSH(&sb, arena) = c;
      }
    } else {
      *PG_DYN_PUSH(&sb, arena) = c;
    }
  }

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] static void
pg_string_builder_append_json_object_key_string_value_string(Pgu8Dyn *sb,
                                                             PgString key,
                                                             PgString value,
                                                             PgArena *arena) {
  PgString pg_json_key = pg_json_escape_string(key, arena);
  PG_DYN_APPEND_SLICE(sb, pg_json_key, arena);

  PG_DYN_APPEND_SLICE(sb, PG_S(":"), arena);

  PgString pg_json_value = pg_json_escape_string(value, arena);
  PG_DYN_APPEND_SLICE(sb, pg_json_value, arena);

  PG_DYN_APPEND_SLICE(sb, PG_S(","), arena);
}

[[maybe_unused]] static void
pg_string_builder_append_json_object_key_string_value_u64(Pgu8Dyn *sb,
                                                          PgString key,
                                                          u64 value,
                                                          PgArena *arena) {
  PgString pg_json_key = pg_json_escape_string(key, arena);
  PG_DYN_APPEND_SLICE(sb, pg_json_key, arena);

  PG_DYN_APPEND_SLICE(sb, PG_S(":"), arena);

  pg_string_builder_append_u64_to_string(sb, value, arena);

  PG_DYN_APPEND_SLICE(sb, PG_S(","), arena);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_log_make_log_line(PgLogLevel level, PgString msg, PgArena *arena,
                     i32 args_count, ...) {
  struct timespec monotonic = {0};
  clock_gettime(CLOCK_MONOTONIC, &monotonic);
  u64 monotonic_ns =
      (u64)monotonic.tv_sec * 1000'000'000 + (u64)monotonic.tv_nsec;

  struct timespec now = {0};
  clock_gettime(CLOCK_REALTIME, &now);
  u64 timestamp_ns = (u64)now.tv_sec * 1000'000'000 + (u64)now.tv_nsec;

  Pgu8Dyn sb = {0};
  *PG_DYN_PUSH(&sb, arena) = '{';

  pg_string_builder_append_json_object_key_string_value_string(
      &sb, PG_S("level"), pg_log_level_to_string(level), arena);
  pg_string_builder_append_json_object_key_string_value_u64(
      &sb, PG_S("timestamp_ns"), timestamp_ns, arena);
  pg_string_builder_append_json_object_key_string_value_u64(
      &sb, PG_S("monotonic_ns"), monotonic_ns, arena);
  pg_string_builder_append_json_object_key_string_value_string(
      &sb, PG_S("message"), msg, arena);

  va_list argp = {0};
  va_start(argp, args_count);
  for (i32 i = 0; i < args_count; i++) {
    PgLogEntry entry = va_arg(argp, PgLogEntry);

    switch (entry.value.kind) {
    case PG_LOG_VALUE_STRING: {
      pg_string_builder_append_json_object_key_string_value_string(
          &sb, entry.key, entry.value.s, arena);
      break;
    }
    case PG_LOG_VALUE_U64:
      pg_string_builder_append_json_object_key_string_value_u64(
          &sb, entry.key, entry.value.n64, arena);
      break;
    default:
      PG_ASSERT(0 && "invalid PgLogValueKind");
    }
  }
  va_end(argp);

  PG_ASSERT(pg_string_ends_with(PG_DYN_SLICE(PgString, sb), PG_S(",")));
  PG_DYN_POP(&sb);
  PG_DYN_APPEND_SLICE(&sb, PG_S("}\n"), arena);

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_json_encode_string_slice(PgStringSlice strings, PgArena *arena) {
  Pgu8Dyn sb = {0};
  *PG_DYN_PUSH(&sb, arena) = '[';

  for (u64 i = 0; i < strings.len; i++) {
    PgString s = PG_DYN_AT(strings, i);
    PgString encoded = pg_json_escape_string(s, arena);
    PG_DYN_APPEND_SLICE(&sb, encoded, arena);

    if (i + 1 < strings.len) {
      *PG_DYN_PUSH(&sb, arena) = ',';
    }
  }

  *PG_DYN_PUSH(&sb, arena) = ']';

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static PgStringSliceResult
pg_json_decode_string_slice(PgString s, PgArena *arena) {
  PgStringSliceResult res = {0};
  if (s.len < 2) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  if ('[' != PG_SLICE_AT(s, 0)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  PgStringDyn dyn = {0};
  for (u64 i = 1; i < s.len - 2;) {
    i = pg_skip_over_whitespace(s, i);

    u8 c = PG_SLICE_AT(s, i);
    if ('"' != c) { // Opening quote.
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    i += 1;

    PgString remaining = PG_SLICE_RANGE_START(s, i);
    i64 end_quote_idx = pg_string_indexof_unescaped_byte(remaining, '"');
    if (-1 == end_quote_idx) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    PG_ASSERT(0 <= end_quote_idx);

    PgString str = PG_SLICE_RANGE(s, i, i + (u64)end_quote_idx);
    PgString unescaped = pg_json_unescape_string(str, arena);
    *PG_DYN_PUSH(&dyn, arena) = unescaped;

    i += (u64)end_quote_idx;

    if ('"' != c) { // Closing quote.
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    i += 1;

    i = pg_skip_over_whitespace(s, i);
    if (i + 1 == s.len) {
      break;
    }

    c = PG_SLICE_AT(s, i);
    if (',' != c) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    i += 1;
  }

  if (']' != PG_SLICE_AT(s, s.len - 1)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.res = PG_DYN_SLICE(PgStringSlice, dyn);
  return res;
}

#endif
