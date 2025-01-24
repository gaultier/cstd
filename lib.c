#ifndef CSTD_LIB_C
#define CSTD_LIB_C

// TODO: Windows.
// TODO: [Unix] Retry syscalls on EINTR.

#if defined(__linux__) || defined(__FreeBSD__) // TODO: More Unices.
#define PG_OS_UNIX
#endif

#ifdef PG_OS_UNIX
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 1
#endif

#include "sha1.c"
#include <stdarg.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef PG_MIN
#define PG_MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define PG_KiB (1024ULL)
#define PG_MiB (1024ULL * PG_KiB)
#define PG_GiB (1024ULL * PG_MiB)
#define PG_TiB (1024ULL * PG_GiB)

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
#define PG_ERR_INVALID_VALUE ((u32)0x701)
#define PG_ERR_OUT_OF_MEMORY ((u32)0x702)
#define PG_ERR_IO ((u32)0x703)

PG_RESULT(u64) Pgu64Result;
PG_RESULT(bool) PgBoolResult;

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

[[maybe_unused]] [[nodiscard]] static u64 pg_div_ceil(u64 a, u64 b) {
  PG_ASSERT(b > 0);
  return a / b + (a % b != 0);
}

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
    *(PG_C_ARRAY_AT_PTR((s)->data, (s)->len, (idx))) =                         \
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
  u64 j = 0;
  u8 needle_first = PG_SLICE_AT(needle, 0);

  for (u64 _i = 0; _i < haystack.len - needle.len - 1; _i++) {
    PgString remaining = PG_SLICE_RANGE_START(haystack, j);
    i64 idx = pg_string_indexof_byte(remaining, needle_first);
    if (-1 == idx) {
      return -1;
    }

    PgString found_maybe =
        PG_SLICE_RANGE(remaining, (u64)idx, (u64)idx + needle.len);
    if (pg_string_eq(needle, found_maybe)) {
      return (i64)j + idx;
    }
    j += (u64)idx + needle.len;
  }

  return -1;
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

  void *os_start;
  u64 os_alloc_size;
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
    memset(PG_SLICE_LAST_PTR(s), 0, sizeof((s)->data[(s)->len - 1]));          \
    (s)->len -= 1;                                                             \
  } while (0)

#define PG_SLICE_LAST_PTR(s)                                                   \
  PG_C_ARRAY_AT_PTR((s)->data, (s)->len, (s)->len - 1)

#define PG_SLICE_LAST(s) PG_C_ARRAY_AT((s).data, (s).len, (s).len - 1)

#define PG_DYN_APPEND_SLICE(dst, src, arena)                                   \
  do {                                                                         \
    PG_DYN_ENSURE_CAP(dst, (dst)->len + (src).len, arena);                     \
    for (u64 _iii = 0; _iii < src.len; _iii++) {                               \
      *PG_DYN_PUSH(dst, arena) = PG_SLICE_AT(src, _iii);                       \
    }                                                                          \
  } while (0)

#define PG_DYN_SLICE(T, dyn) ((T){.data = dyn.data, .len = dyn.len})

typedef Pgu64Result (*ReadFn)(void *self, u8 *buf, size_t buf_len);
typedef Pgu64Result (*WriteFn)(void *self, u8 *buf, size_t buf_len);

typedef struct {
  void *ctx;
  ReadFn read_fn;
} PgReader;

typedef struct {
  void *ctx;
  WriteFn write_fn;
  // Only useful for writing to a string builder (aka `Pgu8Dyn`).
  PgArena *arena; // TODO: Should it be instead in DYN_ structs?
} PgWriter;

typedef struct {
  u64 idx_read, idx_write;
  PgString data;
} PgRing;

[[maybe_unused]] static PgRing pg_ring_make(u64 cap, PgArena *arena) {
  return (PgRing){.data = pg_string_make(cap, arena)};
}

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

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_string_builder_write(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgWriter *w = self;
  Pgu8Dyn *sb = w->ctx;

  PgString s = {.data = buf, .len = buf_len};
  PG_DYN_APPEND_SLICE(sb, s, w->arena);

  return (Pgu64Result){.res = buf_len};
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_reader_ring_read(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgReader *r = self;
  PgRing *ring = r->ctx;

  PgString s = {.data = buf, .len = PG_MIN(buf_len, pg_ring_read_space(*ring))};
  PG_ASSERT(true == pg_ring_read_slice(ring, s));

  return (Pgu64Result){.res = s.len};
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_ring_write(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgWriter *w = self;
  PgRing *ring = w->ctx;

  PgString s = {.data = buf,
                .len = PG_MIN(buf_len, pg_ring_write_space(*ring))};
  PG_ASSERT(true == pg_ring_write_slice(ring, s));

  return (Pgu64Result){.res = s.len};
}

[[nodiscard]] [[maybe_unused]] static PgWriter
pg_writer_make_from_string_builder(Pgu8Dyn *sb, PgArena *arena) {
  PgWriter w = {0};
  w.ctx = sb;
  w.arena = arena;
  w.write_fn = pg_writer_string_builder_write;
  return w;
}

[[nodiscard]] [[maybe_unused]] static PgWriter
pg_writer_make_from_ring(PgRing *ring) {
  PgWriter w = {0};
  w.ctx = ring;
  w.write_fn = pg_writer_ring_write;
  return w;
}

[[maybe_unused]] [[nodiscard]] static PgError pg_writer_write_u8(PgWriter *w,
                                                                 u8 c) {
  PG_ASSERT(nullptr != w->write_fn);

  Pgu64Result res = w->write_fn(w, &c, 1);
  if (res.err) {
    return res.err;
  }

  return res.res == 1 ? 0 : PG_ERR_IO;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_write_all_string(PgWriter *w, PgString s) {
  PG_ASSERT(nullptr != w->write_fn);

  PgString remaining = s;
  for (u64 _i = 0; _i < s.len; _i++) {
    if (pg_string_is_empty(remaining)) {
      break;
    }

    Pgu64Result res = w->write_fn(w, remaining.data, remaining.len);
    if (res.err) {
      return res.err;
    }

    if (0 == res.res) {
      return PG_ERR_IO;
    }

    remaining = PG_SLICE_RANGE_START(remaining, res.res);
  }
  return pg_string_is_empty(remaining) ? 0 : PG_ERR_IO;
}

[[nodiscard]] [[maybe_unused]] static PgReader
pg_reader_make_from_ring(PgRing *ring) {
  PgReader r = {0};
  r.ctx = ring;
  r.read_fn = pg_reader_ring_read;
  return r;
}

[[nodiscard]] [[maybe_unused]] static Pgu64Result
pg_writer_write_from_reader(PgWriter *w, PgReader *r) {
  Pgu64Result res = {0};

  // TODO: Get a hint from the reader?
  u8 tmp[4096] = {0};
  PgString s = {.data = tmp, .len = PG_STATIC_ARRAY_LEN(tmp)};

  res = r->read_fn(r, s.data, s.len);
  if (res.err) {
    return res;
  }
  s.len = res.res;

  res = w->write_fn(w, s.data, s.len);
  if (res.err) {
    return res;
  }

  // WARN: In that case, there is data loss.
  // Not all readers support putting back data that could not be written out.
  if (res.res != s.len) {
    res.err = PG_ERR_IO;
    return res;
  }

  return res;
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_writer_write_u64_as_string(PgWriter *w, u64 n) {
  u8 tmp[30] = {0};
  // TODO: Not use snprintf?
  const int written_count = snprintf((char *)tmp, sizeof(tmp), "%lu", n);

  PG_ASSERT(written_count > 0);

  PgString s = {.data = tmp, .len = (u64)written_count};

  return pg_writer_write_all_string(w, s);
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
  PG_DYN_ENSURE_CAP(&sb, 25, arena);
  PgWriter w = pg_writer_make_from_string_builder(&sb, arena);

  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, n));

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

[[maybe_unused]] [[nodiscard]]
static PgError pg_writer_write_u8_hex_upper(PgWriter *w, u8 n) {

  u8 c1 = n % 16;
  u8 c2 = n / 16;

  PgError err = 0;
  err = pg_writer_write_u8(w, pg_u8_to_character_hex_upper(c2));
  if (err) {
    return err;
  }
  err = pg_writer_write_u8(w, pg_u8_to_character_hex_upper(c1));
  if (err) {
    return err;
  }
  return 0;
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

[[maybe_unused]] [[nodiscard]] static i64
pg_string_indexof_unescaped_byte(PgString haystack, u8 needle, u8 escape) {
  for (u64 i = 0; i < haystack.len; i++) {
    u8 c = PG_SLICE_AT(haystack, i);

    if (c != needle) {
      continue;
    }

    if (i == 0) {
      return (i64)i;
    }

    u8 previous = PG_SLICE_AT(haystack, i - 1);
    if (escape != previous) {
      return (i64)i;
    }
  }

  return -1;
}

[[maybe_unused]] [[nodiscard]] static i64
pg_string_indexof_any_unescaped_byte(PgString haystack, PgString needles,
                                     u8 escape) {
  for (u64 i = 0; i < needles.len; i++) {
    i64 idx = pg_string_indexof_unescaped_byte(haystack,
                                               PG_SLICE_AT(needles, i), escape);
    if (-1 != idx) {
      return idx;
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
pg_string_ieq_ascii(PgString a, PgString b, PgArena arena) {
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

  PgString a_clone = pg_string_clone(a, &arena);
  PgString b_clone = pg_string_clone(b, &arena);

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

typedef struct {
  u32 ip;   // Host order.
  u16 port; // Host order.
} PgIpv4Address;

PG_DYN(PgIpv4Address) PgIpv4AddressDyn;
PG_SLICE(PgIpv4Address) PgIpv4AddressSlice;

[[maybe_unused]] [[nodiscard]] static PgString
pg_net_ipv4_address_to_string(PgIpv4Address address, PgArena *arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 3 * 4 + 4 + 5, arena);
  PgWriter w = pg_writer_make_from_string_builder(&sb, arena);

  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, (address.ip >> 24) & 0xFF));
  PG_ASSERT(0 == pg_writer_write_u8(&w, '.'));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, (address.ip >> 16) & 0xFF));
  PG_ASSERT(0 == pg_writer_write_u8(&w, '.'));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, (address.ip >> 8) & 0xFF));
  PG_ASSERT(0 == pg_writer_write_u8(&w, '.'));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, (address.ip >> 0) & 0xFF));
  PG_ASSERT(0 == pg_writer_write_u8(&w, ':'));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, address.port));

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

[[maybe_unused]] static void pg_bitfield_set(PgString bitfield, u64 idx_bit,
                                             bool val) {
  PG_ASSERT(idx_bit < bitfield.len * 8);

  u64 idx_byte = idx_bit / 8;

  u8 *ptr = PG_SLICE_AT_PTR(&bitfield, idx_byte);
  if (val) {
    *ptr |= 1 << (idx_bit % 8);
  } else {
    *ptr &= ~(1 << (idx_bit % 8));
  }

  PG_ASSERT(val == pg_bitfield_get(bitfield, idx_bit));
}

[[maybe_unused]] [[nodiscard]] static u64 pg_bitfield_count(PgString bitfield) {
  u64 res = 0;
  for (u64 i = 0; i < bitfield.len; i++) {
    u8 c = PG_SLICE_AT(bitfield, i);
    res += (u8)__builtin_popcount((u8)c);
  }
  return res;
}

// FIXME: Windows.
typedef int PgSocket;
typedef int PgFile;
typedef int PgTimer;

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_file_write(void *self, u8 *buf, size_t buf_len);

[[nodiscard]] [[maybe_unused]] static PgWriter
pg_writer_make_from_file_handle(PgFile file) {
  PgWriter w = {0};
  w.ctx = (void *)(u64)file;
  w.write_fn = pg_writer_file_write;
  return w;
}

typedef enum {
  PG_CLOCK_KIND_MONOTONIC,
  PG_CLOCK_KIND_REALTIME,
  // TODO: More?
} PgClockKind;

PG_RESULT(PgTimer) PgTimerResult;

[[maybe_unused]] [[nodiscard]] static PgTimerResult
pg_timer_create(PgClockKind clock_kind, u64 ns);

[[maybe_unused]] [[nodiscard]] static PgError pg_timer_release(PgTimer timer);

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_time_ns_now(PgClockKind clock_kind);

[[nodiscard]] [[maybe_unused]] static PgStringResult
pg_file_read_full(PgString path, PgArena *arena);

typedef PgError (*PgFileReadOnChunk)(PgString chunk, void *ctx);
[[nodiscard]] [[maybe_unused]] static PgError
pg_file_read_chunks(PgString path, u64 chunk_size, PgFileReadOnChunk on_chunk,
                    void *ctx, PgArena arena);

// TODO: Async.
[[nodiscard]] [[maybe_unused]] static PgError
pg_file_write_data_at_offset_from_start(PgFile file, u64 offset, PgString data);

typedef enum [[clang::flag_enum]] : u8 {
  PG_FILE_FLAGS_READ = 1,
  PG_FILE_FLAGS_WRITE = 2,
  PG_FILE_FLAGS_CREATE = 4,
} PgFileFlags;

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_create(PgString path, PgFileFlags flags, PgArena arena);

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_set_size(PgString path, u64 size, PgArena arena);

[[nodiscard]] [[maybe_unused]] static u32 pg_rand_u32(u32 min_incl,
                                                      u32 max_excl);
[[maybe_unused]] static void pg_rand_string_mut(PgString s);
[[nodiscard]] [[maybe_unused]] static u128 pg_rand_u128();

[[nodiscard]] [[maybe_unused]] static u64 pg_os_get_page_size();
[[maybe_unused]] [[nodiscard]] static PgArena
pg_arena_make_from_virtual_mem(u64 size);

[[maybe_unused]] [[nodiscard]] static PgError pg_arena_release(PgArena *arena);

PG_RESULT(PgSocket) PgSocketResult;
[[maybe_unused]] [[nodiscard]] static PgSocketResult pg_net_create_tcp_socket();
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

[[maybe_unused]] [[nodiscard]] static PgError pg_net_tcp_listen(PgSocket sock,
                                                                u64 backlog);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_tcp_bind_ipv4(PgSocket sock, PgIpv4Address addr);
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_enable_reuse(PgSocket sock);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_set_blocking(PgSocket sock, bool blocking);

[[maybe_unused]] [[nodiscard]] static PgError
pg_os_sendfile(PgFile fd_in, PgFile fd_out, u64 n_bytes);

typedef struct {
  PgIpv4Address addr;
  PgSocket socket;
  PgError err;
} PgIpv4AddressAcceptResult;
[[maybe_unused]] [[nodiscard]] static PgIpv4AddressAcceptResult
pg_net_tcp_accept(PgSocket sock);

typedef u64 PgAioQueue;
PG_RESULT(PgAioQueue) PgAioQueueResult;
[[maybe_unused]] [[nodiscard]] static PgAioQueueResult pg_aio_queue_create();

typedef enum : u8 {
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
  u64 os_handle;
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

#if 0
typedef enum {
  PG_PATH_COMPONENT_KIND_ROOT_DIR,
  PG_PATH_COMPONENT_KIND_CURRENT_DIR,
  PG_PATH_COMPONENT_KIND_PARENT_DIR,
  PG_PATH_COMPONENT_KIND_PREFIX, // Only on Windows, e.g. `C:`.
  PG_PATH_COMPONENT_KIND_NORMAL,
} PgPathComponentKind;

typedef struct {
  PgPathComponentKind kind;
  PgString s; // If `kind == PG_PATH_COMPONENT_KIND_NORMAL`.
} PgPathComponent;

PG_DYN(PgPathComponent) PgPathComponentDyn;
PG_SLICE(PgPathComponent) PgPathComponentSlice;

typedef struct {
  PgPathComponentSlice
      components; // E.g. `[foo, bar, baz, song]` for `/foo/bar/baz/song.mp3`.
  PgString file_name; // E.g. `song.mp3`.
  PgString extension; // E.g. `mp3`.
} PgPath;
#endif

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_to_filename(PgString s);

#ifdef PG_OS_UNIX
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_reader_unix_read(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgReader *r = self;
  PG_ASSERT(0 != (u64)r->ctx);

  int fd = (int)(u64)r->ctx;
  ssize_t n = read(fd, buf, buf_len);

  Pgu64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_reader_unix_recv(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgReader *r = self;
  PG_ASSERT(0 != (u64)r->ctx);

  int fd = (int)(u64)r->ctx;
  ssize_t n = recv(fd, buf, buf_len, 0);

  Pgu64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_unix_write(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgWriter *w = self;
  PG_ASSERT(0 != (u64)w->ctx);

  int fd = (int)(u64)w->ctx;
  ssize_t n = write(fd, buf, buf_len);

  Pgu64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_file_write(void *self, u8 *buf, size_t buf_len) {
  return pg_writer_unix_write(self, buf, buf_len);
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_unix_send(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgWriter *w = self;
  PG_ASSERT(0 != (u64)w->ctx);

  int fd = (int)(u64)w->ctx;
  ssize_t n = send(fd, buf, buf_len, MSG_NOSIGNAL);

  Pgu64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_write_data_at_offset_from_start(PgFile file, u64 offset,
                                        PgString data) {
  if (-1 == lseek(file, (off_t)offset, SEEK_SET)) {
    return (PgError)errno;
  }

  PgWriter w = pg_writer_make_from_file_handle(file);
  return pg_writer_write_all_string(&w, data);
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_create(PgString path, PgFileFlags flags, PgArena arena) {
  PgError res = 0;

  int os_flags = 0;
  if (PG_FILE_FLAGS_READ & flags) {
    os_flags |= O_RDONLY;
  }
  if (PG_FILE_FLAGS_WRITE & flags) {
    os_flags |= O_WRONLY;
  }
  if (PG_FILE_FLAGS_CREATE & flags) {
    os_flags |= O_CREAT;
  }

  int mode = 0666; // TODO

  char *path_c = pg_string_to_cstr(path, &arena);
  int ret = open(path_c, os_flags, mode);
  if (-1 == ret) {
    res = (PgError)errno;
  } else {
    close(ret);
  }

  return res;
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_set_size(PgString path, u64 size, PgArena arena) {
  PgError res = 0;

  char *path_c = pg_string_to_cstr(path, &arena);
  int ret = truncate(path_c, (i64)size);
  if (-1 == ret) {
    res = (PgError)errno;
    return res;
  }
  return 0;
}

typedef PgError (*PgFileReadOnChunk)(PgString chunk, void *ctx);

// TODO: Async.
[[nodiscard]] [[maybe_unused]] static PgError
pg_file_read_chunks(PgString path, u64 chunk_size, PgFileReadOnChunk on_chunk,
                    void *ctx, PgArena arena) {

  char *path_c = pg_string_to_cstr(path, &arena);

  int fd = open(path_c, O_RDONLY);
  if (fd < 0) {
    return (PgError)errno;
  }

  Pgu8Dyn chunk = {0};
  PG_DYN_ENSURE_CAP(&chunk, chunk_size, &arena);
  PgError err = 0;

  for (;;) {
    PgString space = PG_DYN_SPACE(PgString, &chunk);
    if (pg_string_is_empty(space)) {
      PG_ASSERT(chunk_size == chunk.len);
      err = on_chunk(PG_DYN_SLICE(PgString, chunk), ctx);
      if (err) {
        goto end;
      }

      chunk.len = 0;
      space = PG_DYN_SPACE(PgString, &chunk);
    }

    PG_ASSERT(space.len > 0);
    PG_ASSERT(space.len <= chunk_size);

    ssize_t ret = read(fd, space.data, space.len);
    if (-1 == ret) {
      err = (PgError)errno;
      goto end;
    }

    if (0 == ret) { // EOF.
      err = on_chunk(PG_DYN_SLICE(PgString, chunk), ctx);
      goto end;
    }

    chunk.len += (u64)ret;
  }

end:
  close(fd);

  return err;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_to_filename(PgString s) {
  for (i64 i = (i64)s.len - 1; i >= 0; i--) {
    u8 cur = PG_SLICE_AT(s, i);
    u8 prev = i > 0 ? PG_SLICE_AT(s, i - 1) : 0;
    if ('/' == cur && '\\' != prev) {
      return PG_SLICE_RANGE_START(s, (u64)i + 1);
    }
  }
  return s;
}

[[nodiscard]] [[maybe_unused]] static u32 pg_rand_u32(u32 min_incl,
                                                      u32 max_excl) {
  PG_ASSERT(min_incl < max_excl);

  u32 res = min_incl + arc4random_uniform(max_excl - min_incl);
  res = PG_CLAMP(min_incl, res, max_excl);
  return res;
}

[[nodiscard]] [[maybe_unused]] static u128 pg_rand_u128() {
  u128 res = 0;
  arc4random_buf(&res, sizeof(res));
  return res;
}

[[maybe_unused]] static void pg_rand_string_mut(PgString s) {
  arc4random_buf(s.data, s.len);
}

[[nodiscard]] [[maybe_unused]] static u64 pg_os_get_page_size() {
  i64 ret = sysconf(_SC_PAGE_SIZE);
  PG_ASSERT(ret >= 0);

  return (u64)ret;
}

[[maybe_unused]] [[nodiscard]] static PgArena
pg_arena_make_from_virtual_mem(u64 size) {
  u64 page_size = pg_os_get_page_size();
  u64 os_alloc_size = pg_round_up_multiple_of(size, page_size);
  PG_ASSERT(0 == os_alloc_size % page_size);

  u64 mmap_size = os_alloc_size;
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
  PG_ASSERT(false == ckd_add(&page_guard_after, (u64)alloc, os_alloc_size));
  PG_ASSERT((u64)alloc + os_alloc_size == page_guard_after);
  PG_ASSERT(page_guard_before + page_size + os_alloc_size == page_guard_after);

  PG_ASSERT(0 == mprotect((void *)page_guard_before, page_size, PROT_NONE));
  PG_ASSERT(0 == mprotect((void *)page_guard_after, page_size, PROT_NONE));

  // Trigger a page fault preemptively to detect invalid virtual memory
  // mappings.
  *(u8 *)alloc = 0;

  return (PgArena){
      .start = alloc,
      .end = (u8 *)alloc + size,
      .os_start = (void *)page_guard_before,
      .os_alloc_size = os_alloc_size,
  };
}

[[maybe_unused]] [[nodiscard]] static PgError pg_arena_release(PgArena *arena) {
  if (nullptr == arena->start) {
    return 0;
  }

  PG_ASSERT(nullptr != arena->end);
  PG_ASSERT(nullptr != arena->os_start);

  int ret = munmap(arena->os_start, arena->os_alloc_size);
  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] static PgStringResult pg_file_read_full(PgString path,
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

// TODO: Windows.
static PgSocketResult pg_net_create_tcp_socket() {
  PgSocketResult res = {0};

  PgSocket sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock_fd) {
    res.err = (PgError)errno;
    return res;
  }

#if defined(SO_NOSIGPIPE)
  {
    int on = 1;
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on))) {
      res.err = (PgError)errno;
      return res;
    }
  }
#endif

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
    if (EINPROGRESS != errno) {
      return (PgError)errno;
    }
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
    PgSocketResult res_create_socket = pg_net_create_tcp_socket();
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
        ntohl(((struct sockaddr_in *)(void *)rp->ai_addr)->sin_addr.s_addr);
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

[[maybe_unused]] [[nodiscard]] static PgError pg_net_tcp_listen(PgSocket sock,
                                                                u64 backlog) {
  PG_ASSERT(backlog <= INT32_MAX);

  if (-1 == listen(sock, (int)backlog)) {
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

#define PG_PATH_SEP '\\'
#define PG_PATH_SEP_S "\\"

#error "TODO"
#endif

#if defined(__linux__)
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/timerfd.h>

[[maybe_unused]] [[nodiscard]] static PgError
pg_os_sendfile(int fd_in, int fd_out, u64 n_bytes) {
  ssize_t res = sendfile(fd_out, fd_in, nullptr, n_bytes);
  if (res == -1) {
    return (PgError)errno;
  }
  if (res != (ssize_t)n_bytes) {
    return (PgError)EAGAIN;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgAioQueueResult pg_aio_queue_create() {
  PgAioQueueResult res = {0};
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

    epoll_event.data.u64 = event.os_handle;

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
    event->os_handle = (u64)epoll_event.data.fd;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static int
pg_linux_clock(PgClockKind clock_kind) {
  switch (clock_kind) {
  case PG_CLOCK_KIND_MONOTONIC:
    return CLOCK_MONOTONIC;
  case PG_CLOCK_KIND_REALTIME:
    return CLOCK_REALTIME;
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

#if defined(__FreeBSD__)
[[maybe_unused]] [[nodiscard]] static PgError
pg_os_sendfile(int fd_in, int fd_out, u64 n_bytes) {
  int res = sendfile(fd_in, fd_out, 0, n_bytes, nullptr, nullptr, 0);
  if (res == -1) {
    return (PgError)errno;
  }
  return 0;
}
#endif

PG_RESULT(PgString) PgIoResult;

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
PG_SLICE(PgKeyValue) PgKeyValueSlice;
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

  // TODO: Should we keep the human-readable status code around or validate
  // it?

  return res;
}

[[maybe_unused]]
static void pg_http_push_header(PgKeyValueDyn *headers, PgString key,
                                PgString value, PgArena *arena) {
  *PG_DYN_PUSH(headers, arena) = (PgKeyValue){.key = key, .value = value};
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_writer_url_encode(PgWriter *w, PgString key, PgString value) {
  PgError err = 0;

  for (u64 i = 0; i < key.len; i++) {
    u8 c = PG_SLICE_AT(key, i);
    if (pg_character_is_alphanumeric(c)) {
      err = pg_writer_write_u8(w, c);
      if (err) {
        return err;
      }
    } else {
      err = pg_writer_write_u8(w, '%');
      if (err) {
        return err;
      }

      err = pg_writer_write_u8_hex_upper(w, c);
      if (err) {
        return err;
      }
    }
  }

  err = pg_writer_write_u8(w, '=');
  if (err) {
    return err;
  }

  for (u64 i = 0; i < value.len; i++) {
    u8 c = PG_SLICE_AT(value, i);
    if (pg_character_is_alphanumeric(c)) {
      err = pg_writer_write_u8(w, c);
      if (err) {
        return err;
      }
    } else {
      err = pg_writer_write_u8(w, '%');
      if (err) {
        return err;
      }
      err = pg_writer_write_u8_hex_upper(w, c);
      if (err) {
        return err;
      }
    }
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_http_request_write_status_line(PgWriter *w, PgHttpRequest req) {
  PgError err = 0;

  err = pg_writer_write_all_string(w, pg_http_method_to_string(req.method));
  if (err) {
    return err;
  }

  err = pg_writer_write_all_string(w, PG_S(" /"));
  if (err) {
    return err;
  }

  for (u64 i = 0; i < req.url.path_components.len; i++) {
    PgString path_component = PG_SLICE_AT(req.url.path_components, i);
    err = pg_writer_write_all_string(w, path_component);
    if (err) {
      return err;
    }

    if (i < req.url.path_components.len - 1) {
      err = pg_writer_write_all_string(w, PG_S("/"));
      if (err) {
        return err;
      }
    }
  }

  if (req.url.query_parameters.len > 0) {
    err = pg_writer_write_all_string(w, PG_S("?"));
    if (err) {
      return err;
    }

    for (u64 i = 0; i < req.url.query_parameters.len; i++) {
      PgKeyValue param = PG_SLICE_AT(req.url.query_parameters, i);
      err = pg_writer_url_encode(w, param.key, param.value);
      if (err) {
        return err;
      }

      if (i < req.url.query_parameters.len - 1) {
        err = pg_writer_write_all_string(w, PG_S("&"));
        if (err) {
          return err;
        }
      }
    }
  }

  err = pg_writer_write_all_string(w, PG_S(" HTTP/1.1\r\n"));
  if (err) {
    return err;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_http_response_write_status_line(PgWriter *w, PgHttpResponse res) {
  PgError err = 0;

  err = pg_writer_write_all_string(w, PG_S("HTTP/"));
  if (err) {
    return err;
  }

  err = pg_writer_write_u64_as_string(w, res.version_major);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, '.');
  if (err) {
    return err;
  }

  err = pg_writer_write_u64_as_string(w, res.version_minor);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, ' ');
  if (err) {
    return err;
  }

  err = pg_writer_write_u64_as_string(w, res.status);
  if (err) {
    return err;
  }

  err = pg_writer_write_all_string(w, PG_S("\r\n"));
  if (err) {
    return err;
  }

  return 0;
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_http_write_header(PgWriter *w, PgKeyValue header) {
  PgError err = 0;

  err = pg_writer_write_all_string(w, header.key);
  if (err) {
    return err;
  }

  err = pg_writer_write_all_string(w, PG_S(": "));
  if (err) {
    return err;
  }

  err = pg_writer_write_all_string(w, header.value);
  if (err) {
    return err;
  }

  err = pg_writer_write_all_string(w, PG_S("\r\n"));
  if (err) {
    return err;
  }

  return 0;
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

[[nodiscard]] [[maybe_unused]] static PgError
pg_http_write_request(PgWriter *w, PgHttpRequest req) {
  PgError err = 0;

  err = pg_http_request_write_status_line(w, req);
  if (err) {
    return err;
  }

  for (u64 i = 0; i < req.headers.len; i++) {
    PgKeyValue header = PG_SLICE_AT(req.headers, i);
    err = pg_http_write_header(w, header);
    if (err) {
      return err;
    }
  }
  err = pg_writer_write_all_string(w, PG_S("\r\n"));
  if (err) {
    return err;
  }

  return 0;
}

[[maybe_unused]] static PgString pg_http_request_to_string(PgHttpRequest req,
                                                           PgArena *arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb,
                    // TODO: Tweak this number?
                    128 + req.url.path_components.len * 64 +
                        req.url.query_parameters.len * 64 +
                        req.headers.len * 128,
                    arena);
  PgWriter w = pg_writer_make_from_string_builder(&sb, arena);

  PG_ASSERT(0 == pg_http_write_request(&w, req));

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] static PgError pg_http_write_response(PgWriter *w,
                                                       PgHttpResponse res) {
  PgError err = 0;

  err = pg_http_response_write_status_line(w, res);
  if (err) {
    return err;
  }

  for (u64 i = 0; i < res.headers.len; i++) {
    PgKeyValue header = PG_SLICE_AT(res.headers, i);
    err = pg_http_write_header(w, header);
    if (err) {
      return err;
    }
  }
  err = pg_writer_write_all_string(w, PG_S("\r\n"));
  if (err) {
    return err;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgReader
pg_reader_make_from_socket(PgSocket socket) {
  // TODO: Windows.
  // TODO: recv?
  return (PgReader){.read_fn = pg_reader_unix_recv, .ctx = (void *)(u64)socket};
}

[[maybe_unused]] [[nodiscard]] static PgReader
pg_reader_make_from_file(PgFile file) {
  // TODO: Windows.
  return (PgReader){.read_fn = pg_reader_unix_read, .ctx = (void *)(u64)file};
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_socket(PgSocket socket) {
  // TODO: Windows.
  return (PgWriter){.write_fn = pg_writer_unix_send,
                    .ctx = (void *)(u64)socket};
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_file(PgFile *file) {
  // TODO: Windows.
  return (PgWriter){.write_fn = pg_writer_unix_write, .ctx = (void *)file};
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_http_headers_parse_content_length(PgKeyValueSlice headers, PgArena arena) {
  Pgu64Result res = {0};

  for (u64 i = 0; i < headers.len; i++) {
    PgKeyValue h = PG_SLICE_AT(headers, i);

    if (!pg_string_ieq_ascii(PG_S("Content-Length"), h.key, arena)) {
      continue;
    }

    PgParseNumberResult res_parse = pg_string_parse_u64(h.value);
    if (!res_parse.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (!pg_string_is_empty(res_parse.remaining)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    res.res = res_parse.n;
    return res;
  }
  return res;
}

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
  PG_DYN_ENSURE_CAP(&data, in.len * 2, arena);

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
    PgKeyValue attr = PG_SLICE_AT(attributes, i);
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
    PgHtmlElement e = PG_SLICE_AT(elements, i);
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
                                     PgArena arena) {
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
  PG_LOG_VALUE_IPV4_ADDRESS,
} PgLogValueKind;

typedef enum {
  PG_LOG_LEVEL_DEBUG,
  PG_LOG_LEVEL_INFO,
  PG_LOG_LEVEL_ERROR,
  PG_LOG_LEVEL_FATAL,
} PgLogLevel;

typedef PgString (*PgMakeLogLineFn)(PgLogLevel level, PgString msg,
                                    PgArena *arena, i32 args_count, ...);
typedef struct {
  PgLogLevel level;
  PgWriter writer;
  PgArena arena;
  PgMakeLogLineFn make_log_line;
} PgLogger;

typedef struct {
  PgLogValueKind kind;
  union {
    PgString s;
    u64 n64;
    u128 n128;
    PgIpv4Address ipv4_address;
  };
} PgLogValue;

typedef struct {
  PgString key;
  PgLogValue value;
} PgLogEntry;

[[maybe_unused]] [[nodiscard]] static PgString
pg_log_make_log_line_json(PgLogLevel level, PgString msg, PgArena *arena,
                          i32 args_count, ...);
[[maybe_unused]] [[nodiscard]] static PgString
pg_log_make_log_line_logfmt(PgLogLevel level, PgString msg, PgArena *arena,
                            i32 args_count, ...);

[[maybe_unused]] [[nodiscard]] static PgLogger
pg_log_make_logger_stdout_json(PgLogLevel level) {
  PgLogger logger = {
      .level = level,
      .writer = pg_writer_make_from_file(
          (PgFile *)(u64)STDOUT_FILENO), // TODO: Windows
      .arena = pg_arena_make_from_virtual_mem(8 * PG_KiB),
      .make_log_line = pg_log_make_log_line_json,
  };

  return logger;
}

[[maybe_unused]] [[nodiscard]] static PgLogger
pg_log_make_logger_stdout_logfmt(PgLogLevel level) {
  PgLogger logger = {
      .level = level,
      .writer = pg_writer_make_from_file(
          (PgFile *)(u64)STDOUT_FILENO), // TODO: Windows
      .arena = pg_arena_make_from_virtual_mem(128 * PG_KiB),
      .make_log_line = pg_log_make_log_line_logfmt,
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

[[maybe_unused]] [[nodiscard]] static PgLogEntry
pg_log_entry_ipv4_address(PgString k, PgIpv4Address v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_IPV4_ADDRESS,
      .value.ipv4_address = v,
  };
}

#define PG_L(k, v)                                                             \
  (_Generic((v),                                                               \
       int: pg_log_entry_int,                                                  \
       u16: pg_log_entry_u16,                                                  \
       u32: pg_log_entry_u32,                                                  \
       u64: pg_log_entry_u64,                                                  \
       PgIpv4Address: pg_log_entry_ipv4_address,                               \
       PgString: pg_log_entry_slice)((PG_S(k)), (v)))

#define PG_LOG_ARGS_COUNT(...)                                                 \
  (sizeof((PgLogEntry[]){__VA_ARGS__}) / sizeof(PgLogEntry))
#define pg_log(logger, lvl, msg, ...)                                          \
  do {                                                                         \
    PG_ASSERT(nullptr != (logger)->make_log_line);                             \
    if ((logger)->level > (lvl)) {                                             \
      break;                                                                   \
    };                                                                         \
    PgArena xxx_tmp_arena = (logger)->arena;                                   \
    PgString xxx_log_line =                                                    \
        (logger)->make_log_line(lvl, PG_S(msg), &xxx_tmp_arena,                \
                                PG_LOG_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__);  \
    (logger)->writer.write_fn(&(logger)->writer, xxx_log_line.data,            \
                              xxx_log_line.len);                               \
  } while (0)

[[maybe_unused]] [[nodiscard]] static PgString
pg_json_escape_string(PgString entry, PgArena *arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 2 + entry.len * 2, arena);
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
pg_logfmt_escape_string(PgString entry, PgArena *arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 2 + entry.len * 2, arena);
  *PG_DYN_PUSH(&sb, arena) = '"';

  for (u64 i = 0; i < entry.len; i++) {
    u8 c = PG_SLICE_AT(entry, i);
    if (' ' == c || c == '-' || c == '_' || c == ':' ||
        pg_character_is_alphanumeric(c)) {
      *PG_DYN_PUSH(&sb, arena) = c;
    } else {
      u8 c1 = c % 16;
      u8 c2 = c / 16;
      PG_DYN_APPEND_SLICE(&sb, PG_S("\\x"), arena);
      *PG_DYN_PUSH(&sb, arena) = pg_u8_to_character_hex(c2);
      *PG_DYN_PUSH(&sb, arena) = pg_u8_to_character_hex(c1);
    }
  }
  *PG_DYN_PUSH(&sb, arena) = '"';

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_json_unescape_string(PgString entry, PgArena *arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, entry.len, arena);

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

[[nodiscard]] [[maybe_unused]] static PgError
pg_writer_write_json_object_key_string_value_string(PgWriter *w, PgString key,
                                                    PgString value,
                                                    PgArena *arena) {
  PgError err = 0;

  PgString pg_json_key = pg_json_escape_string(key, arena);

  err = pg_writer_write_all_string(w, pg_json_key);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, ':');
  if (err) {
    return err;
  }

  PgString pg_json_value = pg_json_escape_string(value, arena);
  err = pg_writer_write_all_string(w, pg_json_value);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, ',');
  if (err) {
    return err;
  }

  return 0;
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_writer_write_json_object_key_string_value_u64(PgWriter *w, PgString key,
                                                 u64 value, PgArena *arena) {
  PgError err = 0;

  PgString pg_json_key = pg_json_escape_string(key, arena);

  err = pg_writer_write_all_string(w, pg_json_key);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, ':');
  if (err) {
    return err;
  }

  err = pg_writer_write_u64_as_string(w, value);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, ',');
  if (err) {
    return err;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_log_make_log_line_json(PgLogLevel level, PgString msg, PgArena *arena,
                          i32 args_count, ...) {
  Pgu64Result res_monotonic_ns = pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC);
  Pgu64Result res_timestamp_ns = pg_time_ns_now(PG_CLOCK_KIND_REALTIME);
  // Ignore clock errors.

  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 256, arena);
  PgWriter w = pg_writer_make_from_string_builder(&sb, arena);

  PG_ASSERT(0 == pg_writer_write_u8(&w, '{'));

  PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_string(
                     &w, PG_S("level"), pg_log_level_to_string(level), arena));
  PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_u64(
                     &w, PG_S("timestamp_ns"), res_timestamp_ns.res, arena));
  PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_u64(
                     &w, PG_S("monotonic_ns"), res_monotonic_ns.res, arena));
  PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_string(
                     &w, PG_S("message"), msg, arena));

  va_list argp = {0};
  va_start(argp, args_count);
  for (i32 i = 0; i < args_count; i++) {
    PgLogEntry entry = va_arg(argp, PgLogEntry);

    switch (entry.value.kind) {
    case PG_LOG_VALUE_STRING: {
      PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_string(
                         &w, entry.key, entry.value.s, arena));
      break;
    }
    case PG_LOG_VALUE_U64:
      PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_u64(
                         &w, entry.key, entry.value.n64, arena));
      break;
    case PG_LOG_VALUE_IPV4_ADDRESS: {
      PgString ipv4_addr_str =
          pg_net_ipv4_address_to_string(entry.value.ipv4_address, arena);
      PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_string(
                         &w, entry.key, ipv4_addr_str, arena));
    } break;
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
pg_log_make_log_line_logfmt(PgLogLevel level, PgString msg, PgArena *arena,
                            i32 args_count, ...) {
  Pgu64Result res_monotonic_ns = pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC);
  Pgu64Result res_timestamp_ns = pg_time_ns_now(PG_CLOCK_KIND_REALTIME);
  // Ignore clock errors.

  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 256, arena);
  PgWriter w = pg_writer_make_from_string_builder(&sb, arena);

  PG_ASSERT(0 == pg_writer_write_all_string(&w, PG_S("level=")));
  PG_ASSERT(0 == pg_writer_write_all_string(&w, pg_log_level_to_string(level)));

  PG_ASSERT(0 == pg_writer_write_all_string(&w, PG_S(" timestamp_ns=")));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, res_timestamp_ns.res));

  PG_ASSERT(0 == pg_writer_write_all_string(&w, PG_S(" monotonic_ns=")));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, res_monotonic_ns.res));

  PG_ASSERT(0 == pg_writer_write_all_string(&w, PG_S(" message=")));
  PG_ASSERT(
      0 == pg_writer_write_all_string(&w, pg_logfmt_escape_string(msg, arena)));

  va_list argp = {0};
  va_start(argp, args_count);
  for (i32 i = 0; i < args_count; i++) {
    PgLogEntry entry = va_arg(argp, PgLogEntry);
    PG_ASSERT(0 == pg_writer_write_u8(&w, ' '));
    PG_ASSERT(0 == pg_writer_write_all_string(&w, entry.key));
    PG_ASSERT(0 == pg_writer_write_u8(&w, '='));

    switch (entry.value.kind) {
    case PG_LOG_VALUE_STRING: {
      PG_ASSERT(0 == pg_writer_write_all_string(
                         &w, pg_logfmt_escape_string(entry.value.s, arena)));
      break;
    }
    case PG_LOG_VALUE_U64:
      PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, entry.value.n64));
      break;
    case PG_LOG_VALUE_IPV4_ADDRESS: {
      PgString ipv4_addr_str =
          pg_net_ipv4_address_to_string(entry.value.ipv4_address, arena);
      PG_ASSERT(0 == pg_writer_write_all_string(&w, ipv4_addr_str));
    } break;
    default:
      PG_ASSERT(0 && "invalid PgLogValueKind");
    }
  }
  va_end(argp);

  PG_ASSERT(0 == pg_writer_write_u8(&w, '\n'));

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_json_encode_string_slice(PgStringSlice strings, PgArena *arena) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, strings.len * 128, arena);
  *PG_DYN_PUSH(&sb, arena) = '[';

  for (u64 i = 0; i < strings.len; i++) {
    PgString s = PG_SLICE_AT(strings, i);
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
    i64 end_quote_idx = pg_string_indexof_unescaped_byte(remaining, '"', '\\');
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

[[maybe_unused]] [[nodiscard]] static PgString
pg_make_unique_id_u128_string(PgArena *arena) {
  u128 id = pg_rand_u128();

  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 32, arena);
  pg_string_builder_append_u128_hex(&sb, id, arena);

  return PG_DYN_SLICE(PgString, sb);
}

typedef enum : u8 {
  PG_EVENT_LOOP_HANDLE_KIND_NONE,
  PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET,
  PG_EVENT_LOOP_HANDLE_KIND_TIMER,
  PG_EVENT_LOOP_HANDLE_KIND_DNS,
  // TODO: More.
} PgEventLoopHandleKind;

typedef struct PgEventLoop PgEventLoop;

typedef void (*PgEventLoopOnTcpConnect)(PgEventLoop *loop, u64 os_handle,
                                        void *ctx, PgError err);

typedef struct PgEventLoopHandle PgEventLoopHandle;

typedef void (*PgEventLoopOnClose)(PgEventLoopHandle *handle);

typedef void (*PgEventLoopOnRead)(PgEventLoop *loop, u64 os_handle, void *ctx,
                                  PgError err, PgString data);

typedef void (*PgEventLoopOnWrite)(PgEventLoop *loop, u64 os_handle, void *ctx,
                                   PgError err);

typedef void (*PgEventLoopOnTimer)(PgEventLoop *loop, u64 os_handle, void *ctx);

typedef void (*PgEventLoopOnDnsResolve)(PgEventLoop *loop, u64 os_handle,
                                        void *ctx, PgError err,
                                        PgIpv4Address address);

typedef enum : u8 {
  PG_EVENT_LOOP_HANDLE_STATE_NONE,
  PG_EVENT_LOOP_HANDLE_STATE_CONNECTING,
  PG_EVENT_LOOP_HANDLE_STATE_CONNECTED,
  PG_EVENT_LOOP_HANDLE_STATE_LISTENING,
  PG_EVENT_LOOP_HANDLE_STATE_CLOSING,
} PgEventLoopHandleState;

struct PgEventLoopHandle {
  PgEventLoopHandleKind kind;
  PgEventLoopHandleState state;
  bool event_in_os_queue;
  PgAioEventKind event_kind;
  u64 os_handle;

  PgRing ring_read;

  PgReader reader;
  PgWriter writer;

  PgArena arena;

  PgEventLoopOnTcpConnect on_connect;
  PgEventLoopOnClose on_close;
  PgEventLoopOnRead on_read;
  PgEventLoopOnWrite on_write;
  PgEventLoopOnTimer on_timer;
  void *ctx;
};

PG_DYN(PgEventLoopHandle) PgEventLoopHandleDyn;
PG_SLICE(PgEventLoopHandle) PgEventLoopHandleSlice;

struct PgEventLoop {
  PgEventLoopHandleDyn handles; // TODO: Smarter?
  PgAioQueue queue;
  PgArena arena;
  bool running;
};

PG_RESULT(PgEventLoop) PgEventLoopResult;

[[nodiscard]] [[maybe_unused]]
static PgEventLoopResult pg_event_loop_make_loop(PgArena arena) {
  PgEventLoopResult res = {0};
  {
    PgAioQueueResult res_queue = pg_aio_queue_create();
    if (res_queue.err) {
      res.err = res_queue.err;
      return res;
    }

    res.res.queue = res_queue.res;
  }
  res.res.arena = arena;
  PG_DYN_ENSURE_CAP(&res.res.handles, 1024, &res.res.arena);

  return res;
}

[[nodiscard]] [[maybe_unused]]
static PgEventLoopHandle pg_event_loop_make_tcp_handle(PgSocket socket,
                                                       void *ctx) {
  PgEventLoopHandle res = {0};
  res.kind = PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET;
  res.reader = pg_reader_make_from_socket(socket);
  res.writer = pg_writer_make_from_socket(socket);
  res.os_handle = (u64)socket;
  res.ctx = ctx;
  // TODO: Could lazily allocate memory for the rings in
  // `pg_event_loop_read_start`, `pg_event_loop_write` only if needed.
  // TODO: Get socket read/write size from OS for useful ring buffer sizes.
  u64 ring_read_size = 4096;
  u64 ring_write_size = 4096;

  res.arena = pg_arena_make_from_virtual_mem(ring_read_size + ring_write_size);
  res.ring_read = pg_ring_make(ring_read_size, &res.arena);

  return res;
}

[[nodiscard]] [[maybe_unused]]
static PgEventLoopHandle pg_event_loop_make_timer_handle(PgTimer timer,
                                                         void *ctx) {
  PgEventLoopHandle res = {0};
  res.kind = PG_EVENT_LOOP_HANDLE_KIND_TIMER;
  res.os_handle = (u64)timer;
  res.ctx = ctx;

  return res;
}

[[nodiscard]] [[maybe_unused]]
static i64 pg_event_loop_find_handle_idx(PgEventLoop *loop, u64 os_handle) {
  for (u64 i = 0; i < loop->handles.len; i++) {
    PgEventLoopHandle *handle = PG_SLICE_AT_PTR(&loop->handles, i);
    if (handle->os_handle == os_handle) {
      return (i64)i;
    }
  }
  return -1;
}

[[nodiscard]] [[maybe_unused]]
static PgEventLoopHandle *pg_event_loop_find_handle(PgEventLoop *loop,
                                                    u64 os_handle) {
  i64 idx = pg_event_loop_find_handle_idx(loop, os_handle);
  if (-1 == idx) {
    return nullptr;
  }
  return PG_SLICE_AT_PTR(&loop->handles, (u64)idx);
}

[[nodiscard]] [[maybe_unused]]
static Pgu64Result pg_event_loop_tcp_init(PgEventLoop *loop, void *ctx) {
  Pgu64Result res = {0};

  PgSocketResult res_socket = pg_net_create_tcp_socket();
  if (res_socket.err) {
    res.err = res_socket.err;
    return res;
  }

  {
    PgError err = pg_net_socket_set_blocking(res_socket.res, false);
    if (err) {
      res.err = err;
      return res;
    }
  }

  *PG_DYN_PUSH(&loop->handles, &loop->arena) =
      pg_event_loop_make_tcp_handle(res_socket.res, ctx);

  res.res = (u64)res_socket.res;
  return res;
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_tcp_connect(PgEventLoop *loop, u64 os_handle,
                                         PgIpv4Address addr,
                                         PgEventLoopOnTcpConnect on_connect) {
  PgEventLoopHandle *handle = pg_event_loop_find_handle(loop, os_handle);
  if (!handle) {
    return PG_ERR_INVALID_VALUE;
  }
  if (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET != handle->kind) {
    return PG_ERR_INVALID_VALUE;
  }

  handle->on_connect = on_connect;

  PgSocket socket = (PgSocket)os_handle;
  PgError err = pg_net_connect_ipv4(socket, addr);
  if (err) {
    return err;
  }

  handle->state = PG_EVENT_LOOP_HANDLE_STATE_CONNECTING;
  // Needed to be notified on a successful `connect`.
  handle->event_kind |= PG_AIO_EVENT_KIND_OUT;
  bool event_in_os_queue_before = handle->event_in_os_queue;
  handle->event_in_os_queue = true;

  PgAioEvent event = {
      .os_handle = os_handle,
      .kind = handle->event_kind,
      .action = event_in_os_queue_before ? PG_AIO_EVENT_ACTION_MOD
                                         : PG_AIO_EVENT_ACTION_ADD,
  };
  return pg_aio_queue_ctl_one(loop->queue, event);
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_tcp_bind(PgEventLoop *loop, u64 os_handle,
                                      PgIpv4Address addr) {
  PgEventLoopHandle *handle = pg_event_loop_find_handle(loop, os_handle);
  if (!handle) {
    return PG_ERR_INVALID_VALUE;
  }
  if (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET != handle->kind) {
    return PG_ERR_INVALID_VALUE;
  }
  return pg_net_tcp_bind_ipv4((PgSocket)os_handle, addr);
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_tcp_listen(PgEventLoop *loop, u64 os_handle,
                                        u64 backlog,
                                        PgEventLoopOnTcpConnect on_connect) {
  PgEventLoopHandle *handle = pg_event_loop_find_handle(loop, os_handle);
  if (!handle) {
    return PG_ERR_INVALID_VALUE;
  }
  if (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET != handle->kind) {
    return PG_ERR_INVALID_VALUE;
  }

  handle->on_connect = on_connect;

  PgError err = pg_net_tcp_listen((PgSocket)os_handle, backlog);
  if (err) {
    return err;
  }

  handle->state = PG_EVENT_LOOP_HANDLE_STATE_LISTENING;
  // Needed to be notified on a client connection.
  handle->event_kind |= PG_AIO_EVENT_KIND_IN;
  bool event_in_os_queue_before = handle->event_in_os_queue;
  handle->event_in_os_queue = true;

  PgAioEvent event = {
      .os_handle = os_handle,
      .kind = handle->event_kind,
      .action = event_in_os_queue_before ? PG_AIO_EVENT_ACTION_MOD
                                         : PG_AIO_EVENT_ACTION_ADD,
  };
  return pg_aio_queue_ctl_one(loop->queue, event);
}

[[nodiscard]] [[maybe_unused]]
static Pgu64Result pg_event_loop_tcp_accept(PgEventLoop *loop, u64 os_handle) {
  Pgu64Result res = {0};

  PgEventLoopHandle *handle = pg_event_loop_find_handle(loop, os_handle);
  if (!handle) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  if (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET != handle->kind) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  PgIpv4AddressAcceptResult res_accept = pg_net_tcp_accept((PgSocket)os_handle);
  if (res_accept.err) {
    res.err = res_accept.err;
    return res;
  }

  PgSocket client_socket = res_accept.socket;
  PgError err = pg_net_socket_set_blocking(client_socket, false);
  if (err) {
    res.err = err;
    return res;
  }

  PgEventLoopHandle client_handle =
      pg_event_loop_make_tcp_handle(res_accept.socket, nullptr);
  client_handle.state = PG_EVENT_LOOP_HANDLE_STATE_CONNECTED;
  client_handle.ctx = handle->ctx;

  *PG_DYN_PUSH(&loop->handles, &loop->arena) = client_handle;

  res.res = (u64)client_socket;
  return res;
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_handle_close(PgEventLoop *loop, u64 os_handle) {
  i64 idx = pg_event_loop_find_handle_idx(loop, os_handle);
  if (-1 == idx) {
    return PG_ERR_INVALID_VALUE;
  }

  PgEventLoopHandle *handle = PG_SLICE_AT_PTR(&loop->handles, (u64)idx);
  handle->state = PG_EVENT_LOOP_HANDLE_STATE_CLOSING;

  return 0;
}

[[maybe_unused]]
static void pg_event_loop_close_all_closing_handles(PgEventLoop *loop) {
  for (u64 i = 0; i < loop->handles.len; i++) {
    PgEventLoopHandle *handle = PG_SLICE_AT_PTR(&loop->handles, i);
    if (PG_EVENT_LOOP_HANDLE_STATE_CLOSING != handle->state) {
      continue;
    }

    if (handle->on_close) {
      handle->on_close(handle->ctx);
    }

    switch (handle->kind) {
    case PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET: {
      (void)pg_net_socket_close((PgSocket)handle->os_handle);
    } break;
    case PG_EVENT_LOOP_HANDLE_KIND_TIMER: {
      (void)pg_timer_release((PgTimer)handle->os_handle);
    } break;
    case PG_EVENT_LOOP_HANDLE_KIND_DNS: {
    } break;
    case PG_EVENT_LOOP_HANDLE_KIND_NONE:
    default:
      PG_ASSERT(0);
    }

    (void)pg_arena_release(&handle->arena);

    PG_SLICE_SWAP_REMOVE(&loop->handles, i);
    i -= 1;
  }
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_timer_stop(PgEventLoop *loop, u64 os_handle) {

  return pg_event_loop_handle_close(loop, os_handle);
}

[[maybe_unused]]
static void pg_event_loop_stop(PgEventLoop *loop) {
  loop->running = false;
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_run(PgEventLoop *loop, i64 timeout_ms) {
  PG_ASSERT(loop->queue != 0);
  PG_ASSERT(!loop->running);

  loop->running = true;

  PgAioEventSlice events_watch =
      PG_SLICE_MAKE(PgAioEvent, loop->handles.len, &loop->arena);

  while (loop->running) {
    Pgu64Result res_wait =
        pg_aio_queue_wait(loop->queue, events_watch, timeout_ms, loop->arena);
    if (res_wait.err) {
      return res_wait.err;
    }

    for (u64 i = 0; i < res_wait.res; i++) {
      PgAioEvent event_watch = PG_SLICE_AT(events_watch, i);
      PgEventLoopHandle *handle =
          pg_event_loop_find_handle(loop, event_watch.os_handle);
      PG_ASSERT(handle);

      PgError err_event_watch = 0;
      if (PG_AIO_EVENT_KIND_ERR & event_watch.kind) {
        // TODO: Ask os to give a precise error.
        err_event_watch = PG_ERR_INVALID_VALUE; // FIXME.
      }

      // Socket connect.
      if ((PG_AIO_EVENT_KIND_OUT & event_watch.kind) &&
          (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET == handle->kind) &&
          (PG_EVENT_LOOP_HANDLE_STATE_CONNECTING == handle->state) &&
          handle->on_connect) {
        handle->state = PG_EVENT_LOOP_HANDLE_STATE_CONNECTED;

        // Stop listening for 'connect'.
        handle->event_kind = handle->event_kind & ~PG_AIO_EVENT_KIND_OUT;
        bool event_in_os_queue_before = handle->event_in_os_queue;
        handle->event_in_os_queue = true;

        if (PG_EVENT_LOOP_HANDLE_STATE_CLOSING != handle->state) {
          PgAioEvent event_change = {
              .os_handle = handle->os_handle,
              .kind = handle->event_kind,
              .action = event_in_os_queue_before ? PG_AIO_EVENT_ACTION_MOD
                                                 : PG_AIO_EVENT_ACTION_ADD,
          };
          PG_ASSERT(0 == pg_aio_queue_ctl_one(loop->queue, event_change));
        }
        handle->on_connect(loop, handle->os_handle, handle->ctx,
                           err_event_watch);
      }

      // Socket listen.
      if ((PG_AIO_EVENT_KIND_IN & event_watch.kind) &&
          (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET == handle->kind) &&
          (PG_EVENT_LOOP_HANDLE_STATE_LISTENING == handle->state) &&
          handle->on_connect) {
        handle->on_connect(loop, handle->os_handle, handle->ctx,
                           err_event_watch);
      }

      // Socket read.
      if ((PG_AIO_EVENT_KIND_IN & event_watch.kind) &&
          (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET == handle->kind) &&
          (PG_EVENT_LOOP_HANDLE_STATE_CONNECTED == handle->state)) {
        {
          if (err_event_watch && handle->on_read) {
            handle->on_read(loop, handle->os_handle, handle->ctx,
                            err_event_watch, (PgString){0});
          } else {
            PgArena arena_tmp = loop->arena;

            // TODO: Ask OS for a hint.
            u64 read_try_len = 4096;

            PgString data = pg_string_make(read_try_len, &arena_tmp);
            Pgu64Result res_read =
                handle->reader.read_fn(&handle->reader, data.data, data.len);
            data.len = res_read.res;

            if (handle->on_read) {
              handle->on_read(loop, handle->os_handle, handle->ctx,
                              res_read.err, data);
            }
          }
        }
      }

#if 0
      // Socket write.
      if ((PG_AIO_EVENT_KIND_OUT & event_watch.kind) &&
          (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET == handle->kind) &&
          (PG_EVENT_LOOP_HANDLE_STATE_CONNECTED == handle->state) &&
          pg_ring_read_space(handle->ring_write) > 0) {
        {
          PgReader reader = pg_reader_make_from_ring(&handle->ring_write);
          Pgu64Result res_write =
              pg_writer_write_from_reader(&handle->writer, &reader);

          if (handle->on_write) {
            handle->on_write(loop, handle->os_handle, handle->ctx,
                             res_write.err);
          }

          // Stop listening for 'OUT' if all of the data was written.
          if (0 == pg_ring_read_space(handle->ring_write)) {
            handle->event_kind = handle->event_kind & ~PG_AIO_EVENT_KIND_OUT;
            bool event_in_os_queue_before = handle->event_in_os_queue;
            handle->event_in_os_queue = true;

            if (PG_EVENT_LOOP_HANDLE_STATE_CLOSING != handle->state) {
              PgAioEvent event_change = {
                  .os_handle = handle->os_handle,
                  .kind = handle->event_kind,
                  .action = event_in_os_queue_before ? PG_AIO_EVENT_ACTION_MOD
                                                     : PG_AIO_EVENT_ACTION_ADD,
              };
              PG_ASSERT(0 == pg_aio_queue_ctl_one(loop->queue, event_change));
            }
          }
        }
      }
#endif

      // Timer.
      // TODO: Repeating timer?
      if ((PG_AIO_EVENT_KIND_IN & event_watch.kind) &&
          (PG_EVENT_LOOP_HANDLE_KIND_TIMER == handle->kind) &&
          (PG_EVENT_LOOP_HANDLE_STATE_CLOSING != handle->state)) {
        if (handle->on_timer) {
          handle->on_timer(loop, handle->os_handle, handle->ctx);
        }
        (void)pg_event_loop_timer_stop(loop, handle->os_handle);
      }
    }

    pg_event_loop_close_all_closing_handles(loop);
  }
  return 0;
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_read_start(PgEventLoop *loop, u64 os_handle,
                                        PgEventLoopOnRead on_read) {

  PgEventLoopHandle *handle = pg_event_loop_find_handle(loop, os_handle);
  if (!handle) {
    return PG_ERR_INVALID_VALUE;
  }
  if (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET != handle->kind) {
    return PG_ERR_INVALID_VALUE;
  }
  if (PG_EVENT_LOOP_HANDLE_STATE_NONE == handle->state ||
      PG_EVENT_LOOP_HANDLE_STATE_CONNECTING == handle->state ||
      PG_EVENT_LOOP_HANDLE_STATE_CLOSING == handle->state) {
    return PG_ERR_INVALID_VALUE;
  }

  handle->on_read = on_read;
  handle->event_kind |= PG_AIO_EVENT_KIND_IN;
  bool event_in_os_queue_before = handle->event_in_os_queue;
  handle->event_in_os_queue = true;

  PgAioEvent event_change = {
      .os_handle = handle->os_handle,
      .kind = handle->event_kind,
      .action = event_in_os_queue_before ? PG_AIO_EVENT_ACTION_MOD
                                         : PG_AIO_EVENT_ACTION_ADD,
  };
  return pg_aio_queue_ctl_one(loop->queue, event_change);
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_read_stop(PgEventLoop *loop, u64 os_handle) {

  PgEventLoopHandle *handle = pg_event_loop_find_handle(loop, os_handle);
  if (!handle) {
    return PG_ERR_INVALID_VALUE;
  }
  if (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET != handle->kind) {
    return PG_ERR_INVALID_VALUE;
  }

  if (0 == handle->event_kind) {
    return 0;
  }

  handle->event_kind = handle->event_kind & ~PG_AIO_EVENT_KIND_IN;
  bool event_in_os_queue_before = handle->event_in_os_queue;
  handle->event_in_os_queue = true;

  PgAioEvent event_change = {
      .os_handle = handle->os_handle,
      .kind = handle->event_kind,
      .action = event_in_os_queue_before ? PG_AIO_EVENT_ACTION_MOD
                                         : PG_AIO_EVENT_ACTION_ADD,
  };
  return pg_aio_queue_ctl_one(loop->queue, event_change);
}

#if 0
struct PgEventLoopWriteRequest;
struct PgEventLoopWriteRequest {
  PgString data;
  PgEventLoopOnWrite on_write;
  u64 os_handle;
};
typedef struct PgEventLoopWriteRequest PgEventLoopWriteRequest;
#endif

[[maybe_unused]]
static void pg_event_loop_try_write(PgEventLoop *loop, u64 os_handle,
                                    PgEventLoopHandle *handle, PgString data,
                                    PgEventLoopOnWrite on_write) {

  Pgu64Result res =
      handle->writer.write_fn(&handle->writer, data.data, data.len);
  if (res.err) {
    if (on_write) {
      on_write(loop, os_handle, handle->ctx, res.err);
    }
    return;
  }

  if (res.res == data.len) { // Full write
    if (on_write) {
      on_write(loop, os_handle, handle->ctx, 0);
    }
    return;
  }

  // Partial write.
  // TODO: Enqueue and register interest in OUT.
  PG_ASSERT(0 && "TODO");
}

[[nodiscard]] [[maybe_unused]]
static PgError pg_event_loop_write(PgEventLoop *loop, u64 os_handle,
                                   PgString data, PgEventLoopOnWrite on_write) {

  PgEventLoopHandle *handle = pg_event_loop_find_handle(loop, os_handle);
  if (!handle) {
    return PG_ERR_INVALID_VALUE;
  }
  if (PG_EVENT_LOOP_HANDLE_KIND_TCP_SOCKET != handle->kind) {
    return PG_ERR_INVALID_VALUE;
  }

  pg_event_loop_try_write(loop, os_handle, handle, data, on_write);

  return 0;
}

[[nodiscard]] [[maybe_unused]]
static Pgu64Result
pg_event_loop_timer_start(PgEventLoop *loop, PgClockKind clock, u64 ns,
                          void *ctx, PgEventLoopOnTimer on_timer) {
  Pgu64Result res = {0};

  PgTimerResult res_timer = pg_timer_create(clock, ns);
  if (res_timer.err) {
    res.err = res_timer.err;
    return res;
  }
  PgEventLoopHandle handle =
      pg_event_loop_make_timer_handle(res_timer.res, ctx);
  handle.on_timer = on_timer;

  *PG_DYN_PUSH(&loop->handles, &loop->arena) = handle;

  PgAioEvent event_change = {
      .os_handle = handle.os_handle,
      .kind = PG_AIO_EVENT_KIND_IN,
      .action = PG_AIO_EVENT_ACTION_ADD,
  };
  PgError err = pg_aio_queue_ctl_one(loop->queue, event_change);
  if (err) {
    res.err = err;
    return res;
  }

  res.res = (u64)res_timer.res;
  return res;
}

[[nodiscard]] [[maybe_unused]]
static Pgu64Result pg_event_loop_dns_resolve_ipv4_tcp_start(
    PgEventLoop *loop, PgString host, u16 port,
    PgEventLoopOnDnsResolve on_dns_resolve, void *ctx) {
  Pgu64Result res = {0};

  // TODO: Run in a thread (pool).

  PgDnsResolveIpv4AddressSocketResult res_dns =
      pg_net_dns_resolve_ipv4_tcp(host, port, loop->arena);

  PgEventLoopHandle handle =
      pg_event_loop_make_tcp_handle(res_dns.res.socket, ctx);
  handle.state = PG_EVENT_LOOP_HANDLE_STATE_CONNECTED;

  *PG_DYN_PUSH(&loop->handles, &loop->arena) = handle;

  if (on_dns_resolve) {
    on_dns_resolve(loop, (u64)res_dns.res.socket, ctx, res_dns.err,
                   res_dns.res.address);
  }

  res.res = (u64)res_dns.res.socket;
  return res;
}

#endif
