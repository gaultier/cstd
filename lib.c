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
  } Dyn##T

#define SLICE(T)                                                               \
  typedef struct {                                                             \
    T *data;                                                                   \
    u64 len;                                                                   \
  } T##Slice

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t u128;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define RESULT(T)                                                              \
  typedef struct {                                                             \
    Error err;                                                                 \
    T res;                                                                     \
  }

#define OK(T)                                                                  \
  typedef struct {                                                             \
    T res;                                                                     \
    bool ok;                                                                   \
  } T##Ok

typedef u32 Error;
RESULT(u64) IoCountResult;
RESULT(u64) u64Result;

#define static_array_len(a) (sizeof(a) / sizeof((a)[0]))

#define CLAMP(min, n, max) ((n) < (min) ? (min) : (n) > (max) ? (max) : n)

#define SUB_SAT(a, b) ((a) > (b) ? ((a) - (b)) : 0)

[[maybe_unused]] static void print_stacktrace(const char *file, int line,
                                              const char *function) {
  fprintf(stderr, "%s:%d:%s\n", file, line, function);
  // TODO
}

#define ASSERT(x)                                                              \
  (x) ? (0)                                                                    \
      : (print_stacktrace(__FILE__, __LINE__, __FUNCTION__), __builtin_trap(), \
         0)

#define AT_PTR(arr, len, idx)                                                  \
  (((i64)(idx) >= (i64)(len)) ? (__builtin_trap(), &(arr)[0])                  \
                              : (ASSERT(nullptr != (arr)), (&(arr)[idx])))

#define AT(arr, len, idx) (*AT_PTR(arr, len, idx))

#define slice_at(s, idx) (AT((s).data, (s).len, idx))

#define slice_at_ptr(s, idx) (AT_PTR((s)->data, (s)->len, idx))

#define slice_make(T, l, arena)                                                \
  ((T##Slice){.data = arena_new(arena, T, l), .len = l})

#define slice_swap_remove(s, idx)                                              \
  do {                                                                         \
    if ((i64)(idx) >= (i64)((s)->len)) {                                       \
      __builtin_trap();                                                        \
    }                                                                          \
    *(AT_PTR((s)->data, (s)->len, idx)) =                                      \
        AT((s)->data, (s)->len, (s)->len - 1);                                 \
    (s)->len -= 1;                                                             \
  } while (0)

[[maybe_unused]] [[nodiscard]] static bool ch_is_hex_digit(u8 c) {
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') ||
         ('a' <= c && c <= 'f');
}

[[maybe_unused]] [[nodiscard]] static bool ch_is_alphabetical(u8 c) {
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

[[maybe_unused]] [[nodiscard]] static bool ch_is_numeric(u8 c) {
  return ('0' <= c && c <= '9');
}

[[maybe_unused]] [[nodiscard]] static bool ch_is_alphanumeric(u8 c) {
  return ch_is_numeric(c) || ch_is_alphabetical(c);
}

[[maybe_unused]] [[nodiscard]] static u8 ch_from_hex(u8 c) {
  ASSERT(ch_is_hex_digit(c));

  if ('0' <= c && c <= '9') {
    return c - '0';
  }

  if ('A' <= c && c <= 'F') {
    return 10 + c - 'A';
  }

  if ('a' <= c && c <= 'f') {
    return 10 + c - 'a';
  }

  ASSERT(false);
}

typedef struct {
  u8 *data;
  u64 len;
} String;

RESULT(String) StringResult;
OK(String);
SLICE(String);

RESULT(StringSlice) StringSliceResult;

#define slice_is_empty(s)                                                      \
  (((s).len == 0) ? true : (ASSERT(nullptr != (s).data), false))

#define S(s) ((String){.data = (u8 *)s, .len = sizeof(s) - 1})

[[maybe_unused]] [[nodiscard]] static bool string_is_empty(String s) {
  return slice_is_empty(s);
}

[[maybe_unused]] [[nodiscard]] static bool string_is_alphabetical(String s) {
  for (u64 i = 0; i < s.len; i++) {
    u8 c = slice_at(s, i);
    if (!ch_is_alphabetical(c)) {
      return false;
    }
  }
  return true;
}

[[maybe_unused]] [[nodiscard]] static String string_trim_left(String s, u8 c) {
  String res = s;

  for (u64 s_i = 0; s_i < s.len; s_i++) {
    ASSERT(s.data != nullptr);
    if (AT(s.data, s.len, s_i) != c) {
      return res;
    }

    res.data += 1;
    res.len -= 1;
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static String string_trim_right(String s, u8 c) {
  String res = s;

  for (i64 s_i = (i64)s.len - 1; s_i >= 0; s_i--) {
    ASSERT(s.data != nullptr);
    if (AT(s.data, s.len, s_i) != c) {
      return res;
    }

    res.len -= 1;
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static String string_trim(String s, u8 c) {
  String res = string_trim_left(s, c);
  res = string_trim_right(res, c);

  return res;
}

typedef struct {
  String s;
  String sep;
} SplitIterator;

typedef struct {
  String s;
  bool ok;
} SplitResult;

[[maybe_unused]] [[nodiscard]] static SplitIterator
string_split_string(String s, String sep) {
  return (SplitIterator){.s = s, .sep = sep};
}

[[maybe_unused]] [[nodiscard]] static i64 string_indexof_byte(String haystack,
                                                              u8 needle) {
  if (slice_is_empty(haystack)) {
    return -1;
  }

  const u8 *res = memchr(haystack.data, needle, haystack.len);
  if (res == nullptr) {
    return -1;
  }

  return res - haystack.data;
}

#define slice_range(s, start, end)                                             \
  ((typeof((s))){                                                              \
      .data = (s).len == CLAMP(0, start, (s).len)                              \
                  ? nullptr                                                    \
                  : AT_PTR((s).data, (s).len, CLAMP(0, start, (s).len)),       \
      .len = SUB_SAT(CLAMP(0, end, (s).len), CLAMP(0, start, (s).len)),        \
  })

#define slice_range_start(s, start) slice_range(s, start, (s).len)

[[maybe_unused]] [[nodiscard]] static bool string_eq(String a, String b) {
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

  ASSERT(a.data != nullptr);
  ASSERT(b.data != nullptr);
  ASSERT(a.len == b.len);

  return memcmp(a.data, b.data, a.len) == 0;
}

[[maybe_unused]] [[nodiscard]] static i64 string_indexof_string(String haystack,
                                                                String needle) {
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

  ASSERT(nullptr != haystack.data);
  ASSERT(nullptr != needle.data);
  void *ptr = memmem(haystack.data, haystack.len, needle.data, needle.len);
  if (nullptr == ptr) {
    return -1;
  }

  u64 res = (u64)((u8 *)ptr - haystack.data);
  ASSERT(res < haystack.len);
  return (i64)res;
}

[[maybe_unused]] [[nodiscard]] static SplitResult
string_split_next(SplitIterator *it) {
  if (slice_is_empty(it->s)) {
    return (SplitResult){0};
  }

  for (u64 _i = 0; _i < it->s.len; _i++) {
    i64 idx = string_indexof_string(it->s, it->sep);
    if (-1 == idx) {
      // Last element.
      SplitResult res = {.s = it->s, .ok = true};
      it->s = (String){0};
      return res;
    }

    if (idx == 0) { // Multiple contiguous separators.
      it->s = slice_range_start(it->s, (u64)idx + it->sep.len);
      continue;
    } else {
      SplitResult res = {.s = slice_range(it->s, 0, (u64)idx), .ok = true};
      it->s = slice_range_start(it->s, (u64)idx + it->sep.len);

      return res;
    }
  }
  return (SplitResult){0};
}

typedef struct {
  String left, right;
  bool consumed;
} StringPairConsume;

[[maybe_unused]] [[nodiscard]] static StringPairConsume
string_consume_until_byte_excl(String haystack, u8 needle) {
  StringPairConsume res = {0};

  i64 idx = string_indexof_byte(haystack, needle);
  if (-1 == idx) {
    res.left = haystack;
    res.right = haystack;
    return res;
  }

  res.left = slice_range(haystack, 0, (u64)idx);
  res.right = slice_range_start(haystack, (u64)idx);
  res.consumed = true;

  ASSERT(needle == slice_at(res.right, 0));
  return res;
}

[[maybe_unused]] [[nodiscard]] static StringPairConsume
string_consume_until_byte_incl(String haystack, u8 needle) {
  StringPairConsume res = {0};

  i64 idx = string_indexof_byte(haystack, needle);
  if (-1 == idx) {
    res.left = haystack;
    res.right = haystack;
    return res;
  }

  res.left = slice_range(haystack, 0, (u64)idx);
  res.right = slice_range_start(haystack, (u64)idx + 1);
  res.consumed = true;

  return res;
}

typedef struct {
  String left, right;
  bool consumed;
  u8 matched;
} StringPairConsumeAny;

[[maybe_unused]] [[nodiscard]] static StringPairConsumeAny
string_consume_until_any_byte_incl(String haystack, String needles) {
  StringPairConsumeAny res = {0};

  for (u64 i = 0; i < needles.len; i++) {
    u8 needle = slice_at(needles, i);
    StringPairConsume res_consume =
        string_consume_until_byte_incl(haystack, needle);
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

[[maybe_unused]] [[nodiscard]] static StringPairConsumeAny
string_consume_until_any_byte_excl(String haystack, String needles) {
  StringPairConsumeAny res = {0};

  for (u64 i = 0; i < needles.len; i++) {
    u8 needle = slice_at(needles, i);
    StringPairConsume res_consume =
        string_consume_until_byte_excl(haystack, needle);
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
string_indexof_any_byte(String haystack, String needle) {
  for (i64 i = 0; i < (i64)haystack.len; i++) {
    u8 c_h = slice_at(haystack, i);

    for (i64 j = 0; j < (i64)needle.len; j++) {
      u8 c_n = slice_at(needle, j);
      if (c_h == c_n) {
        return i;
      }
    }
  }
  return -1;
}

[[maybe_unused]] [[nodiscard]] static bool string_starts_with(String haystack,
                                                              String needle) {
  if (haystack.len == 0 || haystack.len < needle.len) {
    return false;
  }
  ASSERT(nullptr != haystack.data);
  ASSERT(nullptr != needle.data);

  String start = slice_range(haystack, 0, needle.len);

  return string_eq(needle, start);
}

typedef struct {
  bool consumed;
  String remaining;
} StringConsumeResult;

[[maybe_unused]] [[nodiscard]] static StringConsumeResult
string_consume_byte(String haystack, u8 needle) {
  StringConsumeResult res = {0};

  if (haystack.len == 0) {
    return res;
  }
  if (haystack.data[0] != needle) {
    return res;
  }

  res.consumed = true;
  res.remaining = slice_range_start(haystack, 1UL);
  return res;
}

[[maybe_unused]] [[nodiscard]] static StringConsumeResult
string_consume_string(String haystack, String needle) {
  StringConsumeResult res = {0};
  res.remaining = haystack;

  for (u64 i = 0; i < needle.len; i++) {
    res = string_consume_byte(res.remaining, slice_at(needle, i));
    if (!res.consumed) {
      return res;
    }
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static StringConsumeResult
string_consume_any_string(String haystack, StringSlice needles) {
  StringConsumeResult res = {0};
  res.remaining = haystack;

  for (u64 i = 0; i < needles.len; i++) {
    res = string_consume_string(res.remaining, slice_at(needles, i));
    if (res.consumed) {
      return res;
    }
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool string_ends_with(String haystack,
                                                            String needle) {
  if (haystack.len == 0 || haystack.len < needle.len) {
    return false;
  }
  ASSERT(nullptr != haystack.data);
  ASSERT(nullptr != needle.data);

  String end = slice_range_start(haystack, haystack.len - needle.len);

  return string_eq(needle, end);
}

typedef struct {
  u64 n;
  bool present;
  String remaining;
} ParseNumberResult;

[[maybe_unused]] [[nodiscard]] static ParseNumberResult
string_parse_u64(String s) {
  ParseNumberResult res = {0};
  res.remaining = s;

  // Forbid leading zero(es) if there is more than one digit.
  if (string_starts_with(s, S("0")) && s.len >= 2 &&
      ch_is_numeric(slice_at(s, 1))) {
    return res;
  }

  u64 i = 0;
  for (; i < s.len; i++) {
    u8 c = slice_at(s, i);

    if (!ch_is_numeric(c)) { // End of numbers sequence.
      break;
    }

    res.n *= 10;
    res.n += (u8)slice_at(s, i) - '0';
    res.present = true;
  }
  res.remaining = slice_range_start(s, i);
  return res;
}

typedef struct {
  u8 *start;
  u8 *end;
} Arena;

__attribute((malloc, alloc_size(2, 4), alloc_align(3)))
[[maybe_unused]] [[nodiscard]] static void *
arena_alloc(Arena *a, u64 size, u64 align, u64 count) {
  ASSERT(a->start != nullptr);

  const u64 padding = (-(u64)a->start & (align - 1));
  ASSERT(padding <= align);

  const i64 available = (i64)a->end - (i64)a->start - (i64)padding;
  ASSERT(available >= 0);
  ASSERT(count <= (u64)available / size);

  void *res = a->start + padding;
  ASSERT(res != nullptr);
  ASSERT(res <= (void *)a->end);

  a->start += padding + count * size;
  ASSERT(a->start <= a->end);
  ASSERT((u64)a->start % align == 0); // Aligned.

  return memset(res, 0, count * size);
}

#define arena_new(a, t, n) (t *)arena_alloc(a, sizeof(t), _Alignof(t), n)

[[maybe_unused]] [[nodiscard]] static String string_make(u64 len,
                                                         Arena *arena) {
  String res = {0};
  res.len = len;
  res.data = arena_new(arena, u8, len);
  return res;
}

[[maybe_unused]] [[nodiscard]] static char *string_to_cstr(String s,
                                                           Arena *arena) {
  char *res = (char *)arena_new(arena, u8, s.len + 1);
  if (NULL != s.data) {
    memcpy(res, s.data, s.len);
  }

  ASSERT(0 == AT(res, s.len + 1, s.len));

  return res;
}

[[maybe_unused]] [[nodiscard]] static String cstr_to_string(char *s) {
  return (String){
      .data = (u8 *)s,
      .len = strlen(s),
  };
}

typedef enum {
  STRING_CMP_LESS = -1,
  STRING_CMP_EQ = 0,
  STRING_CMP_GREATER = 1,
} StringCompare;

[[maybe_unused]] [[nodiscard]] static StringCompare string_cmp(String a,
                                                               String b) {
  int cmp = memcmp(a.data, b.data, PG_MIN(a.len, b.len));
  if (cmp < 0) {
    return STRING_CMP_LESS;
  } else if (cmp > 0) {
    return STRING_CMP_GREATER;
  } else if (a.len == b.len) {
    return STRING_CMP_EQ;
  }

  ASSERT(0 == cmp);
  ASSERT(a.len != b.len);

  if (a.len < b.len) {
    return STRING_CMP_LESS;
  }
  if (a.len > b.len) {
    return STRING_CMP_GREATER;
  }
  __builtin_unreachable();
}

[[maybe_unused]] static void dyn_grow(void *slice, u64 size, u64 align,
                                      u64 count, Arena *a) {
  ASSERT(nullptr != slice);

  struct {
    void *data;
    u64 len;
    u64 cap;
  } replica;

  memcpy(&replica, slice, sizeof(replica));
  ASSERT(replica.cap < count);

  u64 new_cap = replica.cap == 0 ? 2 : replica.cap;
  for (u64 i = 0; i < 64; i++) {
    if (new_cap < count) {
      ASSERT(new_cap < UINT64_MAX / 2);
      ASSERT(false == ckd_mul(&new_cap, new_cap, 2));
    } else {
      break;
    }
  }
  ASSERT(new_cap >= 2);
  ASSERT(new_cap >= count);
  ASSERT(new_cap > replica.cap);

  u64 array_end = 0;
  u64 array_bytes_count = 0;
  ASSERT(false == ckd_mul(&array_bytes_count, size, replica.cap));
  ASSERT(false == ckd_add(&array_end, (u64)replica.data, array_bytes_count));
  ASSERT((u64)replica.data <= array_end);
  ASSERT(array_end < (u64)a->end);

  if (nullptr ==
      replica.data) { // First allocation ever for this dynamic array.
    replica.data = arena_alloc(a, size, align, new_cap);
  } else if ((u64)a->start == array_end) { // Optimization.
    // This is the case of growing the array which is at the end of the arena.
    // In that case we can simply bump the arena pointer and avoid any copies.
    (void)arena_alloc(a, size, 1 /* Force no padding */, new_cap - replica.cap);
  } else { // General case.
    void *data = arena_alloc(a, size, align, new_cap);

    // Import check to avoid overlapping memory ranges in memcpy.
    ASSERT(data != replica.data);

    memcpy(data, replica.data, array_bytes_count);
    replica.data = data;
  }
  replica.cap = new_cap;

  ASSERT(nullptr != slice);
  memcpy(slice, &replica, sizeof(replica));
}

#define dyn_ensure_cap(dyn, new_cap, arena)                                    \
  ((dyn)->cap < (new_cap))                                                     \
      ? dyn_grow(dyn, sizeof(*(dyn)->data), _Alignof((dyn)->data[0]), new_cap, \
                 arena),                                                       \
      0 : 0

#define dyn_space(T, dyn)                                                      \
  ((T){.data = (dyn)->data + (dyn)->len, .len = (dyn)->cap - (dyn)->len})

typedef struct {
  u8 *data;
  u64 len, cap;
} DynU8;

PG_DYN(String);
RESULT(DynString) DynStringResult;

#define dyn_push(s, arena)                                                     \
  (dyn_ensure_cap(s, (s)->len + 1, arena), (s)->data + (s)->len++)

#define dyn_pop(s)                                                             \
  do {                                                                         \
    ASSERT((s)->len > 0);                                                      \
    memset(dyn_last_ptr(s), 0, sizeof((s)->data[(s)->len - 1]));               \
    (s)->len -= 1;                                                             \
  } while (0)

#define dyn_last_ptr(s) AT_PTR((s)->data, (s)->len, (s)->len - 1)

#define dyn_last(s) AT((s).data, (s).len, (s).len - 1)

#define dyn_at_ptr(s, idx) AT_PTR((s)->data, (s)->len, idx)

#define dyn_at(s, idx) AT((s).data, (s).len, idx)

#define dyn_append_slice(dst, src, arena)                                      \
  do {                                                                         \
    dyn_ensure_cap(dst, (dst)->len + (src).len, arena);                        \
    for (u64 _iii = 0; _iii < src.len; _iii++) {                               \
      *dyn_push(dst, arena) = slice_at(src, _iii);                             \
    }                                                                          \
  } while (0)

#define dyn_slice(T, dyn) ((T){.data = dyn.data, .len = dyn.len})

[[maybe_unused]] static void dynu8_append_u64_to_string(DynU8 *dyn, u64 n,
                                                        Arena *arena) {
  u8 tmp[30] = {0};
  const int written_count = snprintf((char *)tmp, sizeof(tmp), "%lu", n);

  ASSERT(written_count > 0);

  String s = {.data = tmp, .len = (u64)written_count};
  dyn_append_slice(dyn, s, arena);
}

[[maybe_unused]] static void u32_to_u8x4_be(u32 n, String *dst) {
  ASSERT(sizeof(n) == dst->len);

  *(slice_at_ptr(dst, 0)) = (u8)(n >> 24);
  *(slice_at_ptr(dst, 1)) = (u8)(n >> 16);
  *(slice_at_ptr(dst, 2)) = (u8)(n >> 8);
  *(slice_at_ptr(dst, 3)) = (u8)(n >> 0);
}

[[maybe_unused]] static void dynu8_append_u32(DynU8 *dyn, u32 n, Arena *arena) {

  u8 data[sizeof(n)] = {0};
  String s = {.data = data, .len = sizeof(n)};
  u32_to_u8x4_be(n, &s);
  dyn_append_slice(dyn, s, arena);
}

[[maybe_unused]] [[nodiscard]] static String u64_to_string(u64 n,
                                                           Arena *arena) {
  DynU8 sb = {0};
  dynu8_append_u64_to_string(&sb, n, arena);
  return dyn_slice(String, sb);
}

[[maybe_unused]] [[nodiscard]] static u8 u8_to_ch_hex(u8 n) {
  ASSERT(n < 16);

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
  ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static u8 u8_to_ch_hex_upper(u8 n) {
  ASSERT(n < 16);

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
  ASSERT(0);
}

static void dynu8_append_u8_hex_upper(DynU8 *dyn, u8 n, Arena *arena) {
  dyn_ensure_cap(dyn, dyn->len + 2, arena);
  u64 dyn_original_len = dyn->len;

  u8 c1 = n % 16;
  u8 c2 = n / 16;
  *dyn_push(dyn, arena) = u8_to_ch_hex_upper(c2);
  *dyn_push(dyn, arena) = u8_to_ch_hex_upper(c1);
  ASSERT(2 == (dyn->len - dyn_original_len));
}

[[maybe_unused]] static void dynu8_append_u128_hex(DynU8 *dyn, u128 n,
                                                   Arena *arena) {
  dyn_ensure_cap(dyn, dyn->len + 32, arena);
  u64 dyn_original_len = dyn->len;

  u8 it[16] = {0};
  ASSERT(sizeof(it) == sizeof(n));
  memcpy(it, (u8 *)&n, sizeof(n));

  for (u64 i = 0; i < sizeof(it); i++) {
    u8 c1 = it[i] % 16;
    u8 c2 = it[i] / 16;
    *dyn_push(dyn, arena) = u8_to_ch_hex(c2);
    *dyn_push(dyn, arena) = u8_to_ch_hex(c1);
  }
  ASSERT(32 == (dyn->len - dyn_original_len));
}

[[maybe_unused]] [[nodiscard]] static String string_dup(String src,
                                                        Arena *arena) {
  String dst = string_make(src.len, arena);
  memcpy(dst.data, src.data, src.len);

  return dst;
}

[[maybe_unused]] [[nodiscard]] static u64 round_up_multiple_of(u64 n,
                                                               u64 multiple) {
  ASSERT(0 != multiple);

  if (0 == n % multiple) {
    return n; // No-op.
  }

  u64 factor = n / multiple;
  return (factor + 1) * n;
}

[[maybe_unused]] [[nodiscard]] static Arena
arena_make_from_virtual_mem(u64 size) {
  u64 page_size = (u64)sysconf(_SC_PAGE_SIZE); // FIXME
  u64 alloc_real_size = round_up_multiple_of(size, page_size);
  ASSERT(0 == alloc_real_size % page_size);

  u64 mmap_size = alloc_real_size;
  // Page guard before.
  ASSERT(false == ckd_add(&mmap_size, mmap_size, page_size));
  // Page guard after.
  ASSERT(false == ckd_add(&mmap_size, mmap_size, page_size));

  u8 *alloc = mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE,
                   MAP_ANON | MAP_PRIVATE, -1, 0);
  ASSERT(nullptr != alloc);

  u64 page_guard_before = (u64)alloc;

  ASSERT(false == ckd_add((u64 *)&alloc, (u64)alloc, page_size));
  ASSERT(page_guard_before + page_size == (u64)alloc);

  u64 page_guard_after = (u64)0;
  ASSERT(false == ckd_add(&page_guard_after, (u64)alloc, alloc_real_size));
  ASSERT((u64)alloc + alloc_real_size == page_guard_after);
  ASSERT(page_guard_before + page_size + alloc_real_size == page_guard_after);

  ASSERT(0 == mprotect((void *)page_guard_before, page_size, PROT_NONE));
  ASSERT(0 == mprotect((void *)page_guard_after, page_size, PROT_NONE));

  // Trigger a page fault preemptively to detect invalid virtual memory
  // mappings.
  *(u8 *)alloc = 0;

  return (Arena){.start = alloc, .end = (u8 *)alloc + size};
}

[[maybe_unused]] [[nodiscard]] static Error os_sendfile(int fd_in, int fd_out,
                                                        u64 n_bytes) {
#if defined(__linux__)
#include <sys/sendfile.h>
  ssize_t res = sendfile(fd_out, fd_in, nullptr, n_bytes);
  if (res == -1) {
    return (Error)errno;
  }
  if (res != (ssize_t)n_bytes) {
    return (Error)EAGAIN;
  }
  return 0;
#elif defined(__FreeBSD__)
  int res = sendfile(fd_in, fd_out, 0, n_bytes, nullptr, nullptr, 0);
  if (res == -1) {
    return (Error)errno;
  }
  return 0;
#else
#error "sendfile(2) not implemented on other OSes than Linux/FreeBSD."
#endif
}

[[maybe_unused]] [[nodiscard]] static i64
string_indexof_unescaped_byte(String haystack, u8 needle) {
  for (u64 i = 0; i < haystack.len; i++) {
    u8 c = slice_at(haystack, i);

    if (c != needle) {
      continue;
    }

    if (i == 0) {
      return (i64)i;
    }

    u8 previous = slice_at(haystack, i - 1);
    if ('\\' != previous) {
      return (i64)i;
    }
  }

  return -1;
}

[[maybe_unused]] [[nodiscard]] static u64 skip_over_whitespace(String s,
                                                               u64 idx_start) {
  ASSERT(idx_start < s.len);

  u64 idx = idx_start;
  for (; idx < s.len; idx++) {
    u8 c = slice_at(s, idx);
    if (' ' != c) {
      return idx;
    }
  }

  return idx;
}

[[maybe_unused]] [[nodiscard]] static String string_clone(String s,
                                                          Arena *arena) {
  String res = string_make(s.len, arena);
  if (res.data != nullptr) {
    memcpy(res.data, s.data, s.len);
  }

  return res;
}

[[maybe_unused]] static void string_lowercase_ascii_mut(String s) {
  for (u64 i = 0; i < s.len; i++) {
    u8 *c = AT_PTR(s.data, s.len, i);
    if ('A' <= *c && *c <= 'Z') {
      *c += 32;
    }
  }
}

[[maybe_unused]] [[nodiscard]] static bool string_ieq_ascii(String a, String b,
                                                            Arena *arena) {
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

  ASSERT(a.data != nullptr);
  ASSERT(b.data != nullptr);
  ASSERT(a.len == b.len);

  Arena tmp = *arena;
  String a_clone = string_clone(a, &tmp);
  String b_clone = string_clone(b, &tmp);

  string_lowercase_ascii_mut(a_clone);
  string_lowercase_ascii_mut(b_clone);

  return string_eq(a_clone, b_clone);
}

[[maybe_unused]] static void sha1(String s, u8 hash[20]) {
  SHA1_CTX ctx = {0};
  SHA1Init(&ctx);
  SHA1Update(&ctx, s.data, s.len);
  SHA1Final(hash, &ctx);
}

// TODO: Windows.
[[maybe_unused]] static StringResult file_read_full(String path, Arena *arena) {

  StringResult res = {0};
  char *path_c = string_to_cstr(path, arena);

  int fd = open(path_c, O_RDONLY);
  if (fd < 0) {
    res.err = (Error)errno;
    return res;
  }

  struct stat st = {0};
  if (-1 == stat(path_c, &st)) {
    res.err = (Error)errno;
    goto end;
  }

  DynU8 sb = {0};
  dyn_ensure_cap(&sb, (u64)st.st_size, arena);

  for (u64 lim = 0; lim < (u64)st.st_size; lim++) {
    if ((u64)st.st_size == sb.len) {
      break;
    }

    String space = {.data = sb.data + sb.len, .len = sb.cap - sb.len};
    ssize_t read_n = read(fd, space.data, space.len);
    if (-1 == read_n) {
      res.err = (Error)errno;
      goto end;
    }

    if (0 == read_n) {
      res.err = (Error)EINVAL;
      goto end;
    }

    ASSERT((u64)read_n <= space.len);

    sb.len += (u64)read_n;
  }

end:
  close(fd);
  res.res = dyn_slice(String, sb);
  return res;
}

typedef struct {
  u32 ip;   // Host order.
  u16 port; // Host order.
} Ipv4Address;

PG_DYN(Ipv4Address);

[[maybe_unused]] [[nodiscard]] static String
make_unique_id_u128_string(Arena *arena) {
  u128 id = 0;
  arc4random_buf(&id, sizeof(id));

  DynU8 dyn = {0};
  dynu8_append_u128_hex(&dyn, id, arena);

  return dyn_slice(String, dyn);
}

[[maybe_unused]] static void url_encode_string(DynU8 *sb, String key,
                                               String value, Arena *arena) {
  for (u64 i = 0; i < key.len; i++) {
    u8 c = slice_at(key, i);
    if (ch_is_alphanumeric(c)) {
      *dyn_push(sb, arena) = c;
    } else {
      *dyn_push(sb, arena) = '%';
      dynu8_append_u8_hex_upper(sb, c, arena);
    }
  }

  *dyn_push(sb, arena) = '=';

  for (u64 i = 0; i < value.len; i++) {
    u8 c = slice_at(value, i);
    if (ch_is_alphanumeric(c)) {
      *dyn_push(sb, arena) = c;
    } else {
      *dyn_push(sb, arena) = '%';
      dynu8_append_u8_hex_upper(sb, c, arena);
    }
  }
}

[[maybe_unused]] [[nodiscard]] static String
ipv4_address_to_string(Ipv4Address address, Arena *arena) {
  DynU8 sb = {0};
  dynu8_append_u64_to_string(&sb, (address.ip >> 24) & 0xFF, arena);
  *dyn_push(&sb, arena) = '.';
  dynu8_append_u64_to_string(&sb, (address.ip >> 16) & 0xFF, arena);
  *dyn_push(&sb, arena) = '.';
  dynu8_append_u64_to_string(&sb, (address.ip >> 8) & 0xFF, arena);
  *dyn_push(&sb, arena) = '.';
  dynu8_append_u64_to_string(&sb, (address.ip >> 0) & 0xFF, arena);
  *dyn_push(&sb, arena) = ':';
  dynu8_append_u64_to_string(&sb, address.port, arena);

  return dyn_slice(String, sb);
}

[[maybe_unused]] [[nodiscard]] static u64 monotonic_now_ns() {
  struct timespec now = {0};
  if (-1 == clock_gettime(
#ifdef CLOCK_MONOTONIC_COARSE
                CLOCK_MONOTONIC_COARSE
#else
                CLOCK_MONOTONIC
#endif
                ,
                &now)) {
    exit(errno);
  }
  u64 now_ns = (u64)now.tv_sec * 1000'000'000 + (u64)now.tv_nsec;
  return now_ns;
}

[[maybe_unused]] [[nodiscard]] static u32 u8x4_be_to_u32(String s) {
  ASSERT(4 == s.len);
  return (u32)(slice_at(s, 0) << 24) | (u32)(slice_at(s, 1) << 16) |
         (u32)(slice_at(s, 2) << 8) | (u32)(slice_at(s, 3) << 0);
}

[[maybe_unused]] [[nodiscard]] static bool bitfield_get(String bitfield,
                                                        u64 idx_bit) {
  ASSERT(idx_bit < bitfield.len * 8);

  u64 idx_byte = idx_bit / 8;

  return slice_at(bitfield, idx_byte) & (1 << (idx_bit % 8));
}

// FIXME: Windows.
typedef int Socket;
typedef int File;
typedef int Timer;

typedef enum {
  CLOCK_KIND_MONOTONIC,
  // TODO: More?
} ClockKind;

RESULT(Timer) TimerResult;

[[maybe_unused]] [[nodiscard]] static TimerResult
pg_timer_create(ClockKind clock_kind, u64 ns);

[[maybe_unused]] [[nodiscard]] static Error pg_timer_release(Timer timer);

[[maybe_unused]] [[nodiscard]] static u64Result
pg_time_ns_now(ClockKind clock_kind);

RESULT(Socket) CreateSocketResult;
[[maybe_unused]] [[nodiscard]] static CreateSocketResult
net_create_tcp_socket();
[[maybe_unused]] [[nodiscard]] static Error net_socket_close(Socket sock);
[[maybe_unused]] [[nodiscard]] static Error net_set_nodelay(Socket sock,
                                                            bool enabled);
[[maybe_unused]] [[nodiscard]] static Error
net_connect_ipv4(Socket sock, Ipv4Address address);
typedef struct {
  Ipv4Address address;
  Socket socket;
} Ipv4AddressSocket;
RESULT(Ipv4AddressSocket) DnsResolveIpv4AddressSocketResult;
[[maybe_unused]] [[nodiscard]] static DnsResolveIpv4AddressSocketResult
net_dns_resolve_ipv4_tcp(String host, u16 port, Arena arena);

[[maybe_unused]] [[nodiscard]] static Error net_tcp_listen(Socket sock);

[[maybe_unused]] [[nodiscard]] static Error net_tcp_bind_ipv4(Socket sock,
                                                              Ipv4Address addr);
[[maybe_unused]] [[nodiscard]] static Error
net_socket_enable_reuse(Socket sock);

[[maybe_unused]] [[nodiscard]] static Error
net_socket_set_blocking(Socket sock, bool blocking);

typedef struct {
  Ipv4Address addr;
  Socket socket;
  Error err;
} Ipv4AddressAcceptResult;
[[maybe_unused]] [[nodiscard]] static Ipv4AddressAcceptResult
net_tcp_accept(Socket sock);

typedef u64 AioQueue;
RESULT(AioQueue) AioQueueCreateResult;
[[maybe_unused]] [[nodiscard]] static AioQueueCreateResult aio_queue_create();

typedef enum {
  AIO_EVENT_KIND_NONE = 0,
  AIO_EVENT_KIND_IN = 1,
  AIO_EVENT_KIND_OUT = 2,
  AIO_EVENT_KIND_ERR = 4,
} AioEventKind;

typedef enum {
  AIO_EVENT_ACTION_KIND_NONE,
  AIO_EVENT_ACTION_KIND_ADD,
  AIO_EVENT_ACTION_KIND_MOD,
  AIO_EVENT_ACTION_KIND_DEL,
} AioEventActionKind;

typedef struct {
  Socket socket;
  Timer timer;
  AioEventKind kind;
  AioEventActionKind action;
} AioEvent;

SLICE(AioEvent);
PG_DYN(AioEvent);

[[maybe_unused]] [[nodiscard]] static Error aio_queue_ctl(AioQueue queue,
                                                          AioEventSlice events);

[[maybe_unused]] [[nodiscard]] static Error aio_queue_ctl_one(AioQueue queue,
                                                              AioEvent event) {
  AioEventSlice events = {.data = &event, .len = 1};
  return aio_queue_ctl(queue, events);
}

[[maybe_unused]] [[nodiscard]] static IoCountResult
aio_queue_wait(AioQueue queue, AioEventSlice events, i64 timeout_ms,
               Arena arena);

#if defined(__linux__) || defined(__FreeBSD__) // TODO: More Unices.
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

static CreateSocketResult net_create_tcp_socket() {
  CreateSocketResult res = {0};

  Socket sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock_fd) {
    res.err = (Error)errno;
    return res;
  }

  res.res = sock_fd;

  return res;
}

static Error net_socket_close(Socket sock) { return (Error)close(sock); }

static Error net_set_nodelay(Socket sock, bool enabled) {
  int opt = enabled;
  if (-1 == setsockopt(sock, SOL_TCP, TCP_NODELAY, &opt, sizeof(opt))) {
    return (Error)errno;
  }

  return 0;
}

static Error net_connect_ipv4(Socket sock, Ipv4Address address) {
  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(address.port),
      .sin_addr = {htonl(address.ip)},
  };

  if (-1 == connect(sock, (struct sockaddr *)&addr, sizeof(addr))) {
    return (Error)errno;
  }

  return 0;
}

static DnsResolveIpv4AddressSocketResult
net_dns_resolve_ipv4_tcp(String host, u16 port, Arena arena) {
  DnsResolveIpv4AddressSocketResult res = {0};

  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *addr_info = nullptr;
  int res_getaddrinfo = getaddrinfo(
      string_to_cstr(host, &arena),
      string_to_cstr(u64_to_string(port, &arena), &arena), &hints, &addr_info);
  if (res_getaddrinfo != 0) {
    res.err = EINVAL;
    return res;
  }

  struct addrinfo *rp = nullptr;
  for (rp = addr_info; rp != nullptr; rp = rp->ai_next) {
    CreateSocketResult res_create_socket = net_create_tcp_socket();
    if (res_create_socket.err) {
      res.err = res_create_socket.err;
      continue;
    }

    // TODO: Use net_connect_ipv4?
    if (-1 == connect(res_create_socket.res, rp->ai_addr, rp->ai_addrlen)) {
      // TODO: EINPROGRESS in case of non-blocking.
      (void)net_socket_close(res_create_socket.res);
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
    res.err = EINVAL;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Error
net_socket_set_blocking(Socket sock, bool blocking) {
  int flags = fcntl(sock, F_GETFL);
  if (-1 == flags) {
    return (Error)errno;
  }

  if (blocking) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }
  if (-1 == fcntl(sock, F_SETFL, flags)) {
    return (Error)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Error net_tcp_listen(Socket sock) {
  if (-1 == listen(sock, 1024)) {
    return (Error)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Error
net_tcp_bind_ipv4(Socket sock, Ipv4Address addr) {
  struct sockaddr_in addrin = {0};
  addrin.sin_family = AF_INET;
  addrin.sin_port = htons(addr.port);
  addrin.sin_addr.s_addr = htonl(addr.ip);

  if (-1 == bind(sock, (struct sockaddr *)&addrin, sizeof(addrin))) {
    return (Error)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Error
net_socket_enable_reuse(Socket sock) {
  int val = 1;
  if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))) {
    return (Error)errno;
  }

  if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val))) {
    return (Error)errno;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static Ipv4AddressAcceptResult
net_tcp_accept(Socket sock) {
  ASSERT(0 != sock);

  Ipv4AddressAcceptResult res = {0};

  struct sockaddr_in sockaddrin = {0};
  socklen_t sockaddrin_len = sizeof(sockaddrin);
  int sock_client =
      accept(sock, (struct sockaddr *)&sockaddrin, &sockaddrin_len);
  if (-1 == sock_client) {
    res.err = (Error)errno;
    return res;
  }

  res.socket = (Socket)sock_client;
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

[[maybe_unused]] [[nodiscard]] static AioQueueCreateResult aio_queue_create() {
  AioQueueCreateResult res = {0};
  int queue = epoll_create(1 /* Ignored */);
  if (-1 == queue) {
    res.err = (Error)errno;
  }
  res.res = (AioQueue)queue;
  return res;
}

[[maybe_unused]] [[nodiscard]] static Error
aio_queue_ctl(AioQueue queue, AioEventSlice events) {
  for (u64 i = 0; i < events.len; i++) {
    AioEvent event = slice_at(events, i);
    ASSERT(event.socket ^ event.timer);

    int op = 0;
    switch (event.action) {
    case AIO_EVENT_ACTION_KIND_ADD:
      op = EPOLL_CTL_ADD;
      break;
    case AIO_EVENT_ACTION_KIND_MOD:
      op = EPOLL_CTL_MOD;
      break;
    case AIO_EVENT_ACTION_KIND_DEL:
      op = EPOLL_CTL_DEL;
      break;
    case AIO_EVENT_ACTION_KIND_NONE:
    default:
      ASSERT(0);
    }

    struct epoll_event epoll_event = {0};
    if (event.kind & AIO_EVENT_KIND_IN) {
      epoll_event.events |= EPOLLIN;
    }
    if (event.kind & AIO_EVENT_KIND_OUT) {
      epoll_event.events |= EPOLLOUT;
    }
    if (event.kind & AIO_EVENT_KIND_ERR) {
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
      return (Error)errno;
    }
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static IoCountResult
aio_queue_wait(AioQueue queue, AioEventSlice events, i64 timeout_ms,
               Arena arena) {
  IoCountResult res = {0};
  if (slice_is_empty(events)) {
    return res;
  }

  struct epoll_event *epoll_events =
      arena_new(&arena, struct epoll_event, events.len);

  int res_epoll =
      epoll_wait((int)queue, epoll_events, (int)events.len, (int)timeout_ms);
  if (-1 == res_epoll) {
    res.err = (Error)errno;
    return res;
  }
  res.res = (u64)res_epoll;

  for (u64 i = 0; i < res.res; i++) {
    AioEvent *event = slice_at_ptr(&events, i);
    *event = (AioEvent){0};

    struct epoll_event epoll_event = epoll_events[i];
    if (epoll_event.events & EPOLLIN) {
      event->kind |= AIO_EVENT_KIND_IN;
    }
    if (epoll_event.events & EPOLLOUT) {
      event->kind |= AIO_EVENT_KIND_OUT;
    }
    if (epoll_event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
      event->kind |= AIO_EVENT_KIND_ERR;
    }
    event->socket = epoll_event.data.fd;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static int linux_clock(ClockKind clock_kind) {
  switch (clock_kind) {
  case CLOCK_KIND_MONOTONIC:
    return CLOCK_MONOTONIC;
  default:
    ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static TimerResult
pg_timer_create(ClockKind clock, u64 ns) {
  TimerResult res = {0};

  int ret = timerfd_create(linux_clock(clock), TFD_NONBLOCK);
  if (-1 == ret) {
    res.err = (Error)errno;
    return res;
  }

  res.res = (Timer)ret;

  struct itimerspec ts = {0};
  ts.it_value.tv_sec = ns / PG_Seconds;
  ts.it_value.tv_nsec = ns % PG_Seconds;
  ret = timerfd_settime((int)res.res, 0, &ts, nullptr);
  if (-1 == ret) {
    res.err = (Error)errno;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Error pg_timer_release(Timer timer) {
  if (-1 == close((int)timer)) {
    return (Error)errno;
  }
  return (Error)0;
}

[[maybe_unused]] [[nodiscard]] static u64Result
pg_time_ns_now(ClockKind clock) {
  u64Result res = {0};

  struct timespec ts = {0};
  int ret = clock_gettime(linux_clock(clock), &ts);
  if (-1 == ret) {
    res.err = (Error)errno;
    return res;
  }

  res.res = (u64)ts.tv_sec * PG_Seconds + (u64)ts.tv_nsec;

  return res;
}
#endif

RESULT(String) IoResult;

typedef IoCountResult (*ReadFn)(void *self, u8 *buf, size_t buf_len);
typedef IoCountResult (*WriteFn)(void *self, u8 *buf, size_t buf_len);

// TODO: Guard with `ifdef`?
// TODO: Windows?
[[maybe_unused]] [[nodiscard]] static IoCountResult
unix_read(void *self, u8 *buf, size_t buf_len) {
  ASSERT(nullptr != self);

  int fd = (int)(u64)self;
  ssize_t n = read(fd, buf, buf_len);

  IoCountResult res = {0};
  if (n < 0) {
    res.err = (Error)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

// TODO: Guard with `ifdef`?
// TODO: Windows?
[[maybe_unused]] [[nodiscard]] static IoCountResult
unix_write(void *self, u8 *buf, size_t buf_len) {
  ASSERT(nullptr != self);

  int fd = (int)(u64)self;
  ssize_t n = write(fd, buf, buf_len);

  IoCountResult res = {0};
  if (n < 0) {
    res.err = (Error)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

typedef struct {
  DynU8 sb;
  Arena *arena;
} StringBuilder;

[[maybe_unused]] [[nodiscard]] static IoCountResult
string_builder_write(void *self, u8 *buf, size_t buf_len) {
  StringBuilder *sb = self;
  String s = {.data = buf, .len = buf_len};
  dyn_append_slice(&sb->sb, s, sb->arena);

  return (IoCountResult){.res = buf_len};
}

typedef struct {
  u64 idx_read, idx_write;
  String data;
} RingBuffer;

[[maybe_unused]] [[nodiscard]] static u64
ring_buffer_write_space(RingBuffer rg) {
  if (rg.idx_write == rg.idx_read) { // Empty.
    return rg.data.len - 1;
  } else if (rg.idx_write < rg.idx_read) { // Easy case.
    u64 res = rg.idx_read - rg.idx_write - 1;
    ASSERT(res < rg.data.len);
    return res;
  } else if (rg.idx_write > rg.idx_read) { // Hard case.
    u64 can_write1 = rg.data.len - rg.idx_write;
    u64 can_write2 = rg.idx_read;
    if (can_write1 >= 1 && rg.idx_read == 0) {
      can_write1 -= 1; // Reserve empty slot.
    } else if (can_write2 >= 1) {
      ASSERT(rg.idx_read > 0);
      can_write2 -= 1;
    }
    ASSERT(can_write1 <= rg.data.len - 1);
    ASSERT(can_write2 <= rg.data.len - 1);
    return can_write1 + can_write2;
  }
  ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static u64
ring_buffer_read_space(RingBuffer rg) {
  if (rg.idx_write == rg.idx_read) { // Empty.
    return 0;
  } else if (rg.idx_read < rg.idx_write) { // Easy case.
    u64 res = rg.idx_write - rg.idx_read;
    ASSERT(res < rg.data.len);
    return res;
  } else if (rg.idx_read > rg.idx_write) { // Hard case.
    u64 can_read1 = rg.data.len - rg.idx_read;
    u64 can_read2 = rg.idx_write;
    return can_read1 + can_read2;
  }
  ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static bool
ring_buffer_write_slice(RingBuffer *rg, String data) {
  ASSERT(nullptr != rg->data.data);
  ASSERT(rg->idx_read <= rg->data.len);
  ASSERT(rg->idx_write <= rg->data.len);
  ASSERT(rg->data.len > 0);

  if (rg->idx_write < rg->idx_read) { // Easy case.
    u64 space = rg->idx_read - rg->idx_write - 1;
    ASSERT(space <= rg->data.len);

    if (data.len > space) {
      return false;
    }
    memcpy(rg->data.data + rg->idx_write, data.data, data.len);
    rg->idx_write += data.len;
    ASSERT(rg->idx_write <= rg->data.len);
    ASSERT(rg->idx_write < rg->idx_read);
  } else { // Hard case: need potentially two writes.
    ASSERT(rg->idx_write >= rg->idx_read);

    u64 can_write1 = rg->data.len - rg->idx_write;

    u64 can_write2 = rg->idx_read;
    if (can_write1 >= 1 && rg->idx_read == 0) {
      can_write1 -= 1; // Reserve empty slot.
    } else if (can_write2 >= 1) {
      ASSERT(rg->idx_read > 0);
      can_write2 -= 1;
    }
    ASSERT(can_write1 <= rg->data.len - 1);
    ASSERT(can_write2 <= rg->data.len - 1);

    u64 can_write = can_write1 + can_write2;
    if (can_write < data.len) {
      return false;
    }

    u64 write_len1 = PG_MIN(can_write1, data.len);
    ASSERT(rg->idx_write + write_len1 <= rg->data.len);
    ASSERT(write_len1 <= data.len);
    memcpy(rg->data.data + rg->idx_write, data.data, write_len1);
    rg->idx_write += write_len1;
    if (rg->idx_write == rg->data.len) {
      rg->idx_write = 0;
    }
    ASSERT(rg->idx_write < rg->data.len);

    u64 write_len2 = data.len - write_len1;
    if (write_len2 > 0) {
      ASSERT(rg->idx_write = rg->data.len - 1);

      ASSERT(write_len2 + 1 <= rg->idx_read);
      ASSERT(write_len1 + write_len2 <= data.len);
      memcpy(rg->data.data, data.data + write_len1, write_len2);
      rg->idx_write = write_len2;
      ASSERT(rg->idx_write + 1 <= rg->idx_read);
    }
  }

  return true;
}

[[maybe_unused]] [[nodiscard]] static bool
ring_buffer_read_slice(RingBuffer *rg, String data) {
  ASSERT(nullptr != rg->data.data);
  ASSERT(rg->idx_read <= rg->data.len);
  ASSERT(rg->idx_write <= rg->data.len);
  ASSERT(rg->data.len > 0);

  if (0 == data.len) {
    return true;
  }
  ASSERT(nullptr != data.data);

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
    ASSERT(rg->idx_read < rg->data.len);
  } else { // Hard case: potentially 2 reads.
    ASSERT(rg->idx_read > rg->idx_write);
    u64 can_read1 = rg->data.len - rg->idx_read;
    u64 can_read2 = rg->idx_write;
    u64 can_read = can_read1 + can_read2;
    if (can_read < data.len) {
      // TODO: Should we do short reads like `read(2)`?
      return false;
    }

    u64 read_len1 = PG_MIN(can_read1, data.len);
    ASSERT(read_len1 <= data.len);
    ASSERT(read_len1 <= rg->data.len);

    memcpy(data.data, rg->data.data + rg->idx_read, read_len1);
    rg->idx_read += read_len1;
    if (rg->idx_read == rg->data.len) {
      rg->idx_read = 0;
    }
    ASSERT(rg->idx_read < rg->data.len);
    ASSERT(rg->idx_write < rg->data.len);

    u64 read_len2 = data.len - read_len1;
    if (read_len2 > 0) {
      ASSERT(0 == rg->idx_read);

      memcpy(data.data + read_len1, rg->data.data, read_len2);
      rg->idx_read += read_len2;
      ASSERT(rg->idx_read <= data.len);
      ASSERT(rg->idx_read <= rg->data.len);
      ASSERT(rg->idx_read <= rg->idx_write);
    }
  }

  return true;
}

[[maybe_unused]] [[nodiscard]] static StringOk
ring_buffer_read_until_excl(RingBuffer *rg, String needle, Arena *arena) {
  StringOk res = {0};
  i64 idx = -1;

  {
    RingBuffer cpy_rg = *rg;
    Arena cpy_arena = *arena;

    String dst = string_make(ring_buffer_read_space(*rg), arena);
    ASSERT(ring_buffer_read_slice(rg, dst));
    *rg = cpy_rg;       // Reset.
    *arena = cpy_arena; // Reset.

    idx = string_indexof_string(dst, needle);
    if (-1 == idx) {
      return res;
    }
  }

  res.ok = true;
  res.res = string_make((u64)idx, arena);
  ASSERT(ring_buffer_read_slice(rg, res.res));

  // Read and throw away the needle.
  {
    Arena arena_tmp = *arena;
    String dst_needle = string_make(needle.len, &arena_tmp);
    ASSERT(ring_buffer_read_slice(rg, dst_needle));
    ASSERT(string_eq(needle, dst_needle));
  }

  return res;
}

typedef struct {
  void *ctx;
  ReadFn read_fn;
} Reader;

typedef struct {
  void *ctx;
  WriteFn write_fn;
} Writer;

[[maybe_unused]] [[nodiscard]] static Socket reader_socket(Reader *r) {
  ASSERT(r->ctx);
  return (Socket)(u64)r->ctx;
}

[[maybe_unused]] [[nodiscard]] static Socket writer_socket(Writer *w) {
  ASSERT(w->ctx);
  return (Socket)(u64)w->ctx;
}

typedef enum {
  HTTP_METHOD_UNKNOWN,
  HTTP_METHOD_GET,
  HTTP_METHOD_POST
} HttpMethod;

[[maybe_unused]]
String static http_method_to_s(HttpMethod m) {
  switch (m) {
  case HTTP_METHOD_UNKNOWN:
    return S("unknown");
  case HTTP_METHOD_GET:
    return S("GET");
  case HTTP_METHOD_POST:
    return S("POST");
  default:
    ASSERT(0);
  }
}

typedef struct {
  String key, value;
} KeyValue;

RESULT(KeyValue) KeyValueResult;

typedef struct {
  KeyValue *data;
  u64 len, cap;
} DynKeyValue;

RESULT(DynKeyValue) DynKeyValueResult;

typedef struct {
  String scheme;
  String username, password;
  String host; // Including subdomains.
  DynString path_components;
  DynKeyValue query_parameters;
  u16 port;
  // TODO: fragment.
} Url;

typedef struct {
  String id;
  Url url; // Does not have a scheme, domain, port.
  HttpMethod method;
  DynKeyValue headers;
  u8 version_minor;
  u8 version_major;
} HttpRequest;

// `GET /en-US/docs/Web/HTTP/Messages HTTP/1.1`.
typedef struct {
  HttpMethod method;
  u8 version_minor;
  u8 version_major;
  Url url; // Does not have a scheme, domain, port.
} HttpRequestStatusLine;

RESULT(HttpRequestStatusLine) HttpRequestStatusLineResult;

// `HTTP/1.1 201 Created`.
typedef struct {
  u8 version_minor;
  u8 version_major;
  u16 status;
} HttpResponseStatusLine;

RESULT(HttpResponseStatusLine) HttpResponseStatusLineResult;

#if 0
typedef struct {
  HttpRequest req;
  HttpParseState state;
} HttpRequestParse;
#endif

typedef struct {
  u8 version_major;
  u8 version_minor;
  u16 status;
  DynKeyValue headers;
} HttpResponse;

[[maybe_unused]] [[nodiscard]] static HttpResponseStatusLineResult
http_parse_response_status_line(String status_line) {
  HttpResponseStatusLineResult res = {0};

  String remaining = status_line;
  {
    StringConsumeResult consume = string_consume_string(remaining, S("HTTP/"));
    if (!consume.consumed) {
      res.err = EINVAL;
      return res;
    }
    remaining = consume.remaining;
  }

  {
    ParseNumberResult res_major = string_parse_u64(remaining);
    if (!res_major.present) {
      res.err = EINVAL;
      return res;
    }
    if (res_major.n > 3) {
      res.err = EINVAL;
      return res;
    }
    res.res.version_major = (u8)res_major.n;
    remaining = res_major.remaining;
  }

  {
    StringConsumeResult consume = string_consume_byte(remaining, '.');
    if (!consume.consumed) {
      res.err = EINVAL;
      return res;
    }
    remaining = consume.remaining;
  }

  {
    ParseNumberResult res_minor = string_parse_u64(remaining);
    if (!res_minor.present) {
      res.err = EINVAL;
      return res;
    }
    if (res_minor.n > 9) {
      res.err = EINVAL;
      return res;
    }
    res.res.version_minor = (u8)res_minor.n;
    remaining = res_minor.remaining;
  }

  {
    StringConsumeResult consume = string_consume_byte(remaining, ' ');
    if (!consume.consumed) {
      res.err = EINVAL;
      return res;
    }
    remaining = consume.remaining;
  }

  {
    ParseNumberResult res_status_code = string_parse_u64(remaining);
    if (!res_status_code.present) {
      res.err = EINVAL;
      return res;
    }
    if (res_status_code.n < 100 || res_status_code.n > 599) {
      res.err = EINVAL;
      return res;
    }
    res.res.status = (u16)res_status_code.n;
    remaining = res_status_code.remaining;
  }

  // TODO: Should we keep the human-readable status code around or validate it?

  return res;
}

[[maybe_unused]]
static void http_push_header(DynKeyValue *headers, String key, String value,
                             Arena *arena) {
  *dyn_push(headers, arena) = (KeyValue){.key = key, .value = value};
}

[[maybe_unused]] [[nodiscard]] static bool
http_request_write_status_line(RingBuffer *rg, HttpRequest req, Arena arena) {
  DynU8 sb = {0};
  dyn_ensure_cap(&sb, 128, &arena);
  dyn_append_slice(&sb, http_method_to_s(req.method), &arena);
  dyn_append_slice(&sb, S(" /"), &arena);

  for (u64 i = 0; i < req.url.path_components.len; i++) {
    String path_component = dyn_at(req.url.path_components, i);
    dyn_append_slice(&sb, path_component, &arena);

    if (i < req.url.path_components.len - 1) {
      *dyn_push(&sb, &arena) = '/';
    }
  }

  if (req.url.query_parameters.len > 0) {
    *dyn_push(&sb, &arena) = '?';
    for (u64 i = 0; i < req.url.query_parameters.len; i++) {
      KeyValue param = dyn_at(req.url.query_parameters, i);
      url_encode_string(&sb, param.key, param.value, &arena);

      if (i < req.url.query_parameters.len - 1) {
        *dyn_push(&sb, &arena) = '&';
      }
    }
  }

  dyn_append_slice(&sb, S(" HTTP/1.1"), &arena);
  dyn_append_slice(&sb, S("\r\n"), &arena);

  String s = dyn_slice(String, sb);
  return ring_buffer_write_slice(rg, s);
}

[[maybe_unused]] [[nodiscard]] static bool
http_response_write_status_line(RingBuffer *rg, HttpResponse res, Arena arena) {
  DynU8 sb = {0};
  dyn_ensure_cap(&sb, 128, &arena);
  dyn_append_slice(&sb, S("HTTP/"), &arena);
  dynu8_append_u64_to_string(&sb, res.version_major, &arena);
  dyn_append_slice(&sb, S("."), &arena);
  dynu8_append_u64_to_string(&sb, res.version_minor, &arena);
  dyn_append_slice(&sb, S(" "), &arena);

  dynu8_append_u64_to_string(&sb, res.status, &arena);

  dyn_append_slice(&sb, S(" \r\n"), &arena);

  String s = dyn_slice(String, sb);
  return ring_buffer_write_slice(rg, s);
}

[[maybe_unused]] [[nodiscard]] static bool
http_write_header(RingBuffer *rg, KeyValue header, Arena arena) {
  DynU8 sb = {0};
  dyn_ensure_cap(&sb, 128, &arena);
  dyn_append_slice(&sb, header.key, &arena);
  dyn_append_slice(&sb, S(": "), &arena);
  dyn_append_slice(&sb, header.value, &arena);
  dyn_append_slice(&sb, S("\r\n"), &arena);

  String s = dyn_slice(String, sb);
  return ring_buffer_write_slice(rg, s);
}

// NOTE: Only sanitation for including the string inside an HTML tag e.g.:
// `<div>...ESCAPED_STRING..</div>`.
// To include the string inside other context (e.g. JS, CSS, HTML attributes,
// etc), more advance sanitation is required.
[[maybe_unused]] [[nodiscard]] static String html_sanitize(String s,
                                                           Arena *arena) {
  DynU8 res = {0};
  dyn_ensure_cap(&res, s.len, arena);
  for (u64 i = 0; i < s.len; i++) {
    u8 c = slice_at(s, i);

    if ('&' == c) {
      dyn_append_slice(&res, S("&amp"), arena);
    } else if ('<' == c) {
      dyn_append_slice(&res, S("&lt"), arena);
    } else if ('>' == c) {
      dyn_append_slice(&res, S("&gt"), arena);
    } else if ('"' == c) {
      dyn_append_slice(&res, S("&quot"), arena);
    } else if ('\'' == c) {
      dyn_append_slice(&res, S("&#x27"), arena);
    } else {
      *dyn_push(&res, arena) = c;
    }
  }

  return dyn_slice(String, res);
}

typedef struct {
  String username, password;
} UrlUserInfo;

RESULT(UrlUserInfo) UrlUserInfoResult;

typedef struct {
  UrlUserInfo user_info;
  String host;
  u16 port;
} UrlAuthority;

RESULT(UrlAuthority) UrlAuthorityResult;

RESULT(Url) ParseUrlResult;

[[maybe_unused]] [[nodiscard]] static DynStringResult
url_parse_path_components(String s, Arena *arena) {
  DynStringResult res = {0};

  if (-1 != string_indexof_any_byte(s, S("?#:"))) {
    res.err = EINVAL;
    return res;
  }

  if (slice_is_empty(s)) {
    return res;
  }

  if (!string_starts_with(s, S("/"))) {
    res.err = EINVAL;
    return res;
  }

  DynString components = {0};

  SplitIterator split_it_slash = string_split_string(s, S("/"));
  for (u64 i = 0; i < s.len; i++) { // Bound.
    SplitResult split = string_split_next(&split_it_slash);
    if (!split.ok) {
      break;
    }

    if (slice_is_empty(split.s)) {
      continue;
    }

    *dyn_push(&components, arena) = split.s;
  }

  res.res = components;
  return res;
}

[[maybe_unused]] [[nodiscard]] static DynKeyValueResult
url_parse_query_parameters(String s, Arena *arena) {
  DynKeyValueResult res = {0};

  String remaining = s;
  {
    StringConsumeResult res_consume_question = string_consume_byte(s, '?');
    if (!res_consume_question.consumed) {
      res.err = EINVAL;
      return res;
    }
    remaining = res_consume_question.remaining;
  }

  for (u64 _i = 0; _i < s.len; _i++) {
    StringPairConsume res_consume_and =
        string_consume_until_byte_incl(remaining, '&');
    remaining = res_consume_and.right;

    String kv = res_consume_and.left;
    StringPairConsume res_consume_eq = string_consume_until_byte_incl(kv, '=');
    String k = res_consume_eq.left;
    String v = res_consume_eq.consumed ? res_consume_eq.right : S("");

    if (!slice_is_empty(k)) {
      *dyn_push(&res.res, arena) = (KeyValue){.key = k, .value = v};
    }

    if (!res_consume_and.consumed) {
      break;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static UrlUserInfoResult
url_parse_user_info(String s) {
  UrlUserInfoResult res = {0};
  // https://www.rfc-editor.org/rfc/rfc3986#section-3.2.1:
  // Use of the format "user:password" in the userinfo field is
  // deprecated.  Applications should not render as clear text any data
  // after the first colon (":") character found within a userinfo
  // subcomponent unless the data after the colon is the empty string
  // (indicating no password).  Applications may choose to ignore or
  // reject such data when it is received.

  if (slice_is_empty(s)) {
    res.err = EINVAL;
    return res;
  }

  return res;
}

RESULT(u16) PortResult;

[[maybe_unused]] [[nodiscard]] static PortResult url_parse_port(String s) {
  PortResult res = {0};

  // Allowed.
  if (slice_is_empty(s)) {
    return res;
  }

  ParseNumberResult port_parse = string_parse_u64(s);
  if (!slice_is_empty(port_parse.remaining)) {
    res.err = EINVAL;
    return res;
  }
  if (port_parse.n > UINT16_MAX) {
    res.err = EINVAL;
    return res;
  }
  res.res = (u16)port_parse.n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static UrlAuthorityResult
url_parse_authority(String s) {
  UrlAuthorityResult res = {0};

  String remaining = s;
  // User info, optional.
  {
    StringPairConsume user_info_and_rem =
        string_consume_until_byte_incl(remaining, '@');
    remaining = user_info_and_rem.right;

    if (user_info_and_rem.consumed) {
      UrlUserInfoResult res_user_info =
          url_parse_user_info(user_info_and_rem.left);
      if (res_user_info.err) {
        res.err = res_user_info.err;
        return res;
      }
      res.res.user_info = res_user_info.res;
    }
  }

  // Host, mandatory.
  StringPairConsume host_and_rem =
      string_consume_until_byte_incl(remaining, ':');
  {
    remaining = host_and_rem.right;
    res.res.host = host_and_rem.left;
    if (slice_is_empty(res.res.host)) {
      res.err = EINVAL;
      return res;
    }
  }

  // Port, optional.
  if (host_and_rem.consumed) {
    PortResult res_port = url_parse_port(host_and_rem.right);
    if (res_port.err) {
      res.err = res_port.err;
      return res;
    }
    res.res.port = res_port.res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool url_is_scheme_valid(String scheme) {
  if (slice_is_empty(scheme)) {
    return false;
  }

  u8 first = slice_at(scheme, 0);
  if (!ch_is_alphabetical(first)) {
    return false;
  }

  for (u64 i = 0; i < scheme.len; i++) {
    u8 c = slice_at(scheme, i);
    if (!(ch_is_alphanumeric(c) || c == '+' || c == '-' || c == '.')) {
      return false;
    }
  }

  return true;
}

[[maybe_unused]] [[nodiscard]] static ParseUrlResult
url_parse_after_authority(String s, Arena *arena) {
  ParseUrlResult res = {0};
  String remaining = s;

  StringPairConsumeAny path_components_and_rem =
      string_consume_until_any_byte_excl(remaining, S("?#"));
  remaining = path_components_and_rem.right;

  // Path, optional.
  if (string_starts_with(s, S("/"))) {
    ASSERT(!slice_is_empty(path_components_and_rem.left));

    DynStringResult res_path_components =
        url_parse_path_components(path_components_and_rem.left, arena);
    if (res_path_components.err) {
      res.err = res_path_components.err;
      return res;
    }
    res.res.path_components = res_path_components.res;
  }

  // Query parameters, optional.
  if (path_components_and_rem.consumed &&
      path_components_and_rem.matched == '?') {
    DynKeyValueResult res_query =
        url_parse_query_parameters(path_components_and_rem.right, arena);
    if (res_query.err) {
      res.err = res_query.err;
      return res;
    }
    res.res.query_parameters = res_query.res;
  }

  // TODO: fragments.

  ASSERT(slice_is_empty(res.res.scheme));
  ASSERT(slice_is_empty(res.res.username));
  ASSERT(slice_is_empty(res.res.password));
  ASSERT(slice_is_empty(res.res.host));
  ASSERT(0 == res.res.port);

  return res;
}

[[maybe_unused]] [[nodiscard]] static ParseUrlResult url_parse(String s,
                                                               Arena *arena) {
  ParseUrlResult res = {0};

  String remaining = s;

  // Scheme, mandatory.
  {
    StringPairConsume scheme_and_rem =
        string_consume_until_byte_incl(remaining, ':');
    remaining = scheme_and_rem.right;

    if (!scheme_and_rem.consumed) {
      res.err = EINVAL;
      return res;
    }

    if (!url_is_scheme_valid(scheme_and_rem.left)) {
      res.err = EINVAL;
      return res;
    }
    res.res.scheme = scheme_and_rem.left;
  }

  // Assume `://` as separator between the scheme and authority.
  // TODO: Be less strict hier.
  {

    StringConsumeResult res_consume = string_consume_string(remaining, S("//"));
    if (!res_consume.consumed) {
      res.err = EINVAL;
      return res;
    }
    remaining = res_consume.remaining;
  }

  // Authority, mandatory.
  StringPairConsumeAny authority_and_rem =
      string_consume_until_any_byte_excl(remaining, S("/?#"));
  remaining = authority_and_rem.right;
  {
    if (slice_is_empty(authority_and_rem.left)) {
      res.err = EINVAL;
      return res;
    }

    UrlAuthorityResult res_authority =
        url_parse_authority(authority_and_rem.left);
    if (res_authority.err) {
      res.err = res_authority.err;
      return res;
    }
    res.res.host = res_authority.res.host;
    res.res.port = res_authority.res.port;
    res.res.username = res_authority.res.user_info.username;
    res.res.password = res_authority.res.user_info.password;
  }

  ParseUrlResult res_after_authority =
      url_parse_after_authority(remaining, arena);
  if (res_after_authority.err) {
    res.err = res_after_authority.err;
    return res;
  }
  res.res.path_components = res_after_authority.res.path_components;
  res.res.query_parameters = res_after_authority.res.query_parameters;

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool http_url_is_valid(Url u) {
  // TODO: Support https.
  if (!string_eq(u.scheme, S("http"))) {
    return false;
  }

  return true;
}

[[maybe_unused]] [[nodiscard]] static HttpRequestStatusLineResult
http_parse_request_status_line(String status_line, Arena *arena) {
  HttpRequestStatusLineResult res = {0};

  String remaining = status_line;
  {
    if (string_starts_with(remaining, S("GET"))) {
      StringConsumeResult consume = string_consume_string(remaining, S("GET"));
      ASSERT(consume.consumed);
      remaining = consume.remaining;
      res.res.method = HTTP_METHOD_GET;
    } else if (string_starts_with(remaining, S("POST"))) {
      StringConsumeResult consume = string_consume_string(remaining, S("POST"));
      ASSERT(consume.consumed);
      remaining = consume.remaining;
      res.res.method = HTTP_METHOD_POST;
    } else {
      res.err = EINVAL;
      return res;
    }
  }

  {
    StringConsumeResult consume = string_consume_byte(remaining, ' ');
    if (!consume.consumed) {
      res.err = EINVAL;
      return res;
    }
    remaining = consume.remaining;
  }

  i64 idx_space = string_indexof_byte(remaining, ' ');
  if (-1 == idx_space) {
    res.err = EINVAL;
    return res;
  }
  String path = slice_range(remaining, 0, (u64)idx_space);
  remaining = slice_range_start(remaining, (u64)idx_space + 1);
  {
    ParseUrlResult res_url = url_parse_after_authority(path, arena);
    if (res_url.err) {
      res.err = EINVAL;
      return res;
    }

    res.res.url = res_url.res;
  }

  {
    StringConsumeResult consume = string_consume_string(remaining, S("HTTP/"));
    if (!consume.consumed) {
      res.err = EINVAL;
      return res;
    }
    remaining = consume.remaining;
  }

  {
    ParseNumberResult res_major = string_parse_u64(remaining);
    if (!res_major.present) {
      res.err = EINVAL;
      return res;
    }
    if (res_major.n > 3) {
      res.err = EINVAL;
      return res;
    }
    res.res.version_major = (u8)res_major.n;
    remaining = res_major.remaining;
  }

  {
    StringConsumeResult consume = string_consume_byte(remaining, '.');
    if (!consume.consumed) {
      res.err = EINVAL;
      return res;
    }
    remaining = consume.remaining;
  }

  {
    ParseNumberResult res_minor = string_parse_u64(remaining);
    if (!res_minor.present) {
      res.err = EINVAL;
      return res;
    }
    if (res_minor.n > 9) {
      res.err = EINVAL;
      return res;
    }
    res.res.version_minor = (u8)res_minor.n;
    remaining = res_minor.remaining;
  }

  if (!slice_is_empty(remaining)) {
    res.err = EINVAL;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static KeyValueResult
http_parse_header(String s) {
  KeyValueResult res = {0};

  i64 idx = string_indexof_byte(s, ':');
  if (-1 == idx) {
    res.err = EINVAL;
    return res;
  }

  res.res.key = slice_range(s, 0, (u64)idx);
  if (slice_is_empty(res.res.key)) {
    res.err = EINVAL;
    return res;
  }

  res.res.value = string_trim_left(slice_range_start(s, (u64)idx + 1), ' ');
  if (slice_is_empty(res.res.value)) {
    res.err = EINVAL;
    return res;
  }

  return res;
}

typedef struct {
  bool done;
  HttpResponse res;
  Error err;
} HttpResponseReadResult;

typedef struct {
  bool done;
  HttpRequest res;
  Error err;
} HttpRequestReadResult;

[[maybe_unused]] [[nodiscard]] static HttpResponseReadResult
http_read_response(RingBuffer *rg, u64 max_http_headers, Arena *arena) {
  HttpResponseReadResult res = {0};
  String sep = S("\r\n\r\n");

  StringOk s = ring_buffer_read_until_excl(rg, sep, arena);
  if (!s.ok) { // In progress.
    return res;
  }

  SplitIterator it = string_split_string(s.res, S("\r\n"));
  SplitResult res_split = string_split_next(&it);
  if (!res_split.ok) {
    res.err = EINVAL;
    return res;
  }

  HttpResponseStatusLineResult res_status_line =
      http_parse_response_status_line(res_split.s);
  if (res_status_line.err) {
    res.err = res_status_line.err;
    return res;
  }

  res.res.status = res_status_line.res.status;
  res.res.version_major = res_status_line.res.version_major;
  res.res.version_minor = res_status_line.res.version_minor;

  for (u64 _i = 0; _i < max_http_headers; _i++) {
    res_split = string_split_next(&it);
    if (!res_split.ok) {
      break;
    }
    KeyValueResult res_kv = http_parse_header(res_split.s);
    if (res_kv.err) {
      res.err = res_kv.err;
      return res;
    }

    *dyn_push(&res.res.headers, arena) = res_kv.res;
  }
  if (!slice_is_empty(it.s)) {
    res.err = EINVAL;
    return res;
  }

  res.done = true;
  return res;
}

[[maybe_unused]] [[nodiscard]] static HttpRequestReadResult
http_read_request(RingBuffer *rg, u64 max_http_headers, Arena *arena) {
  HttpRequestReadResult res = {0};
  String sep = S("\r\n\r\n");

  StringOk s = ring_buffer_read_until_excl(rg, sep, arena);
  if (!s.ok) { // In progress.
    return res;
  }

  SplitIterator it = string_split_string(s.res, S("\r\n"));
  SplitResult res_split = string_split_next(&it);
  if (!res_split.ok) {
    res.err = EINVAL;
    return res;
  }

  HttpRequestStatusLineResult res_status_line =
      http_parse_request_status_line(res_split.s, arena);
  if (res_status_line.err) {
    res.err = res_status_line.err;
    return res;
  }

  res.res.method = res_status_line.res.method;
  res.res.url = res_status_line.res.url;
  res.res.version_major = res_status_line.res.version_major;
  res.res.version_minor = res_status_line.res.version_minor;

  for (u64 _i = 0; _i < max_http_headers; _i++) {
    res_split = string_split_next(&it);
    if (!res_split.ok) {
      break;
    }
    KeyValueResult res_kv = http_parse_header(res_split.s);
    if (res_kv.err) {
      res.err = res_kv.err;
      return res;
    }

    *dyn_push(&res.res.headers, arena) = res_kv.res;
  }
  if (!slice_is_empty(it.s)) {
    res.err = EINVAL;
    return res;
  }

  res.done = true;
  return res;
}

[[maybe_unused]] static Error http_write_request(RingBuffer *rg,
                                                 HttpRequest res, Arena arena) {
  if (!http_request_write_status_line(rg, res, arena)) {
    return (Error)ENOMEM;
  }

  for (u64 i = 0; i < res.headers.len; i++) {
    KeyValue header = dyn_at(res.headers, i);
    if (!http_write_header(rg, header, arena)) {
      return (Error)ENOMEM;
    }
  }
  if (!ring_buffer_write_slice(rg, S("\r\n"))) {
    return (Error)ENOMEM;
  }

  return (Error)0;
}

[[maybe_unused]] static Error
http_write_response(RingBuffer *rg, HttpResponse res, Arena arena) {
  if (!http_response_write_status_line(rg, res, arena)) {
    return (Error)ENOMEM;
  }
  for (u64 i = 0; i < res.headers.len; i++) {
    KeyValue header = dyn_at(res.headers, i);
    if (!http_write_header(rg, header, arena)) {
      return (Error)ENOMEM;
    }
  }
  if (!ring_buffer_write_slice(rg, S("\r\n"))) {

    return (Error)ENOMEM;
  }
  return (Error)0;
}

[[maybe_unused]] [[nodiscard]] static Reader
reader_make_from_socket(Socket socket) {
  // TODO: Windows.
  return (Reader){.read_fn = unix_read, .ctx = (void *)(u64)socket};
}

[[maybe_unused]] [[nodiscard]] static Writer
writer_make_from_socket(Socket socket) {
  // TODO: Windows.
  return (Writer){.write_fn = unix_write, .ctx = (void *)(u64)socket};
}

[[maybe_unused]] [[nodiscard]] static Writer writer_make_from_file(File *file) {
  // TODO: Windows.
  return (Writer){.write_fn = unix_write, .ctx = (void *)file};
}

[[maybe_unused]] [[nodiscard]] static Writer
writer_make_from_string_builder(StringBuilder *sb) {
  return (Writer){.write_fn = string_builder_write, .ctx = (void *)sb};
}

[[maybe_unused]] [[nodiscard]] static IoCountResult
reader_read(Reader *r, RingBuffer *rg, Arena arena) {
  ASSERT(nullptr != r->read_fn);

  IoCountResult res = {0};

  String dst = string_make(ring_buffer_write_space(*rg), &arena);
  res = r->read_fn(r->ctx, dst.data, dst.len);

  if (res.err) {
    return res;
  }
  dst.len = res.res;
  ASSERT(true == ring_buffer_write_slice(rg, dst));

  return res;
}

[[maybe_unused]] [[nodiscard]] static IoCountResult
writer_write(Writer *w, RingBuffer *rg, Arena arena) {
  ASSERT(nullptr != w->write_fn);

  String dst = string_make(ring_buffer_read_space(*rg), &arena);
  ASSERT(true == ring_buffer_read_slice(rg, dst));

  return w->write_fn(w->ctx, dst.data, dst.len);
}

#if 0
[[nodiscard]] static ParseNumberResult
request_parse_content_length_maybe(HttpRequest req, Arena *arena) {
  ASSERT(!req.err);

  for (u64 i = 0; i < req.headers.len; i++) {
    KeyValue h = req.headers.data[i];

    if (!string_ieq_ascii(S("Content-Length"), h.key, arena)) {
      continue;
    }

    return string_parse_u64(h.value);
  }
  return (ParseNumberResult){0};
}

[[maybe_unused]] [[nodiscard]] static HttpResponse
http_client_request(Ipv4AddressSocket sock, HttpRequest req, Arena *arena) {
  HttpResponse res = {0};

  if (!slice_is_empty(req.path_raw)) {
    // Should use `req.path_components`, not `path.raw`.
    res.err = EINVAL;
    return res;
  }

  if (HM_UNKNOWN == req.method) {
    res.err = EINVAL;
    return res;
  }

  String http_request_serialized = http_request_serialize(req, arena);
  log(LOG_LEVEL_DEBUG, "http request", arena, L("ip", sock.address.ip),
      L("port", sock.address.port), L("serialized", http_request_serialized));

  // TODO: should not be an assert but a returned error.
  ASSERT(send(sock.socket, http_request_serialized.data,
              http_request_serialized.len,
              0) == (i64)http_request_serialized.len);

  BufferedReader reader = buffered_reader_make(sock.socket, arena);

  {
    IoResult io_result =
        buffered_reader_read_until_slice(&reader, S("\r\n"), arena);
    if (io_result.err) {
      res.err = io_result.err;
      goto end;
    }
    if (slice_is_empty(io_result.res)) {
      res.err = HS_ERR_INVALID_HTTP_RESPONSE;
      goto end;
    }

    String http1_1_version_needle = S("HTTP/1.1 ");
    String http1_0_version_needle = S("HTTP/1.0 ");
    ASSERT(http1_0_version_needle.len == http1_1_version_needle.len);

    if (!(string_starts_with(io_result.res, http1_0_version_needle) ||
          string_starts_with(io_result.res, http1_1_version_needle))) {
      res.err = HS_ERR_INVALID_HTTP_RESPONSE;
      goto end;
    }

    String status_str =
        slice_range_start(io_result.res, http1_1_version_needle.len);
    ParseNumberResult status_parsed = string_parse_u64(status_str);
    if (!status_parsed.present) {
      res.err = HS_ERR_INVALID_HTTP_RESPONSE;
      goto end;
    }
    if (!status_parsed.present) {
      res.err = EINVAL;
      goto end;
    }
    if (!(200 <= status_parsed.n && status_parsed.n <= 599)) {
      res.err = EINVAL;
      goto end;
    }

    res.status = (u16)status_parsed.n;
  }

  res.err = http_read_headers(&reader, &res.headers, arena);
  if (res.err) {
    log(LOG_LEVEL_ERROR, "http request failed to read headers", arena,
        L("req.method", req.method), L("req.path_raw", req.path_raw),
        L("err", res.err));
    goto end;
  }

  ParseNumberResult content_length =
      request_parse_content_length_maybe(req, arena);
  if (!content_length.present) {
    res.err = HS_ERR_INVALID_HTTP_REQUEST;
    return res;
  }

  if (content_length.present) {
    IoResult res_io =
        reader_read_exactly((Reader *)&reader, content_length.n, arena);
    if (res_io.err) {
      res.err = res_io.err;
      return res;
    }
    res.body = res_io.res;
  }

end:
  (void)net_socket_close(sock.socket);
  return res;
}
#endif

typedef struct {
  String key, value;
} FormDataKV;

typedef struct {
  FormDataKV *data;
  u64 len, cap;
} DynFormData;

typedef struct {
  // NOTE: Repeated keys are allowed, that's how 'arrays' are encoded.
  DynFormData form;
  Error err;
} FormDataParseResult;

typedef struct {
  FormDataKV kv;
  Error err;
  String remaining;
} FormDataKVParseResult;

typedef struct {
  String data;
  Error err;
  String remaining;
} FormDataKVElementParseResult;

[[nodiscard]] static FormDataKVElementParseResult
form_data_kv_parse_element(String in, u8 ch_terminator, Arena *arena) {
  FormDataKVElementParseResult res = {0};
  DynU8 data = {0};

  u64 i = 0;
  for (; i < in.len; i++) {
    u8 c = in.data[i];

    if ('+' == c) {
      *dyn_push(&data, arena) = ' ';
    } else if ('%' == c) {
      if ((in.len - i) < 2) {
        res.err = EINVAL;
        return res;
      }
      u8 c1 = in.data[i + 1];
      u8 c2 = in.data[i + 2];

      if (!(ch_is_hex_digit(c1) && ch_is_hex_digit(c2))) {
        res.err = EINVAL;
        return res;
      }

      u8 utf8_character = ch_from_hex(c1) * 16 + ch_from_hex(c2);
      *dyn_push(&data, arena) = utf8_character;
      i += 2; // Consume 2 characters.
    } else if (ch_terminator == c) {
      i += 1; // Consume.
      break;
    } else {
      *dyn_push(&data, arena) = c;
    }
  }

  res.data = dyn_slice(String, data);
  res.remaining = slice_range_start(in, i);
  return res;
}

[[nodiscard]] static FormDataKVParseResult form_data_kv_parse(String in,
                                                              Arena *arena) {
  FormDataKVParseResult res = {0};

  String remaining = in;

  FormDataKVElementParseResult key_parsed =
      form_data_kv_parse_element(remaining, '=', arena);
  if (key_parsed.err) {
    res.err = key_parsed.err;
    return res;
  }
  res.kv.key = key_parsed.data;

  remaining = key_parsed.remaining;

  FormDataKVElementParseResult value_parsed =
      form_data_kv_parse_element(remaining, '&', arena);
  if (value_parsed.err) {
    res.err = value_parsed.err;
    return res;
  }
  res.kv.value = value_parsed.data;
  res.remaining = value_parsed.remaining;

  return res;
}

[[maybe_unused]] [[nodiscard]] static FormDataParseResult
form_data_parse(String in, Arena *arena) {
  FormDataParseResult res = {0};

  String remaining = in;

  for (u64 i = 0; i < in.len; i++) { // Bound.
    if (slice_is_empty(remaining)) {
      break;
    }

    FormDataKVParseResult kv = form_data_kv_parse(remaining, arena);
    if (kv.err) {
      res.err = kv.err;
      return res;
    }

    *dyn_push(&res.form, arena) = kv.kv;

    remaining = kv.remaining;
  }
  return res;
}

typedef enum {
  HTML_NONE,
  HTML_TITLE,
  HTML_SPAN,
  HTML_INPUT,
  HTML_BUTTON,
  HTML_LINK,
  HTML_META,
  HTML_HEAD,
  HTML_BODY,
  HTML_DIV,
  HTML_OL,
  HTML_LI,
  HTML_TEXT,
  HTML_FORM,
  HTML_FIELDSET,
  HTML_LABEL,
  HTML_SCRIPT,
  HTML_STYLE,
  HTML_LEGEND,
  HTML_MAX, // Pseudo.
} HtmlKind;

typedef struct HtmlElement HtmlElement;
typedef struct {
  HtmlElement *data;
  u64 len, cap;
} DynHtmlElements;

struct HtmlElement {
  HtmlKind kind;
  DynKeyValue attributes;
  union {
    DynHtmlElements children;
    String text; // Only for `HTML_TEXT`, `HTML_LEGEND`, `HTML_TITLE`,
                 // `HTML_SCRIPT`, `HTML_STYLE`, `HTML_BUTTON`.
  };
};

typedef struct {
  HtmlElement body;
  HtmlElement head;
} HtmlDocument;

[[maybe_unused]] [[nodiscard]] static HtmlDocument html_make(String title,
                                                             Arena *arena) {
  HtmlDocument res = {0};

  {

    HtmlElement tag_head = {.kind = HTML_HEAD};
    {
      HtmlElement tag_meta = {.kind = HTML_META};
      {
        *dyn_push(&tag_meta.attributes, arena) =
            (KeyValue){.key = S("charset"), .value = S("utf-8")};
      }
      *dyn_push(&tag_head.children, arena) = tag_meta;
    }
    {
      HtmlElement tag_title = {.kind = HTML_TITLE, .text = title};
      *dyn_push(&tag_head.children, arena) = tag_title;
    }

    res.head = tag_head;

    HtmlElement tag_body = {.kind = HTML_BODY};
    res.body = tag_body;
  }

  return res;
}

static void html_attributes_to_string(DynKeyValue attributes, DynU8 *sb,
                                      Arena *arena) {
  for (u64 i = 0; i < attributes.len; i++) {
    KeyValue attr = dyn_at(attributes, i);
    ASSERT(-1 == string_indexof_string(attr.key, S("\"")));

    *dyn_push(sb, arena) = ' ';
    dyn_append_slice(sb, attr.key, arena);
    *dyn_push(sb, arena) = '=';
    *dyn_push(sb, arena) = '"';
    // TODO: escape string.
    dyn_append_slice(sb, attr.value, arena);
    *dyn_push(sb, arena) = '"';
  }
}

static void html_tags_to_string(DynHtmlElements elements, DynU8 *sb,
                                Arena *arena);
static void html_tag_to_string(HtmlElement e, DynU8 *sb, Arena *arena);

static void html_tags_to_string(DynHtmlElements elements, DynU8 *sb,
                                Arena *arena) {
  for (u64 i = 0; i < elements.len; i++) {
    HtmlElement e = dyn_at(elements, i);
    html_tag_to_string(e, sb, arena);
  }
}

[[maybe_unused]]
static void html_document_to_string(HtmlDocument doc, DynU8 *sb, Arena *arena) {
  dyn_append_slice(sb, S("<!DOCTYPE html>"), arena);

  dyn_append_slice(sb, S("<html>"), arena);
  html_tag_to_string(doc.head, sb, arena);
  html_tag_to_string(doc.body, sb, arena);
  dyn_append_slice(sb, S("</html>"), arena);
}

static void html_tag_to_string(HtmlElement e, DynU8 *sb, Arena *arena) {
  static const String tag_to_string[HTML_MAX] = {
      [HTML_NONE] = S("FIXME"),
      [HTML_TITLE] = S("title"),
      [HTML_SPAN] = S("span"),
      [HTML_INPUT] = S("input"),
      [HTML_BUTTON] = S("button"),
      [HTML_LINK] = S("link"),
      [HTML_META] = S("meta"),
      [HTML_HEAD] = S("head"),
      [HTML_BODY] = S("body"),
      [HTML_DIV] = S("div"),
      [HTML_TEXT] = S("span"),
      [HTML_FORM] = S("form"),
      [HTML_FIELDSET] = S("fieldset"),
      [HTML_LABEL] = S("label"),
      [HTML_SCRIPT] = S("script"),
      [HTML_STYLE] = S("style"),
      [HTML_LEGEND] = S("legend"),
      [HTML_OL] = S("ol"),
      [HTML_LI] = S("li"),
  };

  ASSERT(!(HTML_NONE == e.kind || HTML_MAX == e.kind));

  *dyn_push(sb, arena) = '<';
  dyn_append_slice(sb, tag_to_string[e.kind], arena);
  html_attributes_to_string(e.attributes, sb, arena);
  *dyn_push(sb, arena) = '>';

  switch (e.kind) {
  // Cases of tag without any children and no closing tag.
  case HTML_LINK:
    [[fallthrough]];
  case HTML_META:
    ASSERT(0 == e.children.len);
    return;

  // 'Normal' tags.
  case HTML_OL:
    [[fallthrough]];
  case HTML_LI:
    [[fallthrough]];
  case HTML_HEAD:
    [[fallthrough]];
  case HTML_DIV:
    [[fallthrough]];
  case HTML_FORM:
    [[fallthrough]];
  case HTML_FIELDSET:
    [[fallthrough]];
  case HTML_LABEL:
    [[fallthrough]];
  case HTML_INPUT:
    [[fallthrough]];
  case HTML_SPAN:
    [[fallthrough]];
  case HTML_BODY:
    html_tags_to_string(e.children, sb, arena);
    break;

  // Only cases where `.text` is valid.
  case HTML_BUTTON:
    [[fallthrough]];
  case HTML_SCRIPT:
    [[fallthrough]];
  case HTML_STYLE:
    [[fallthrough]];
  case HTML_LEGEND:
    [[fallthrough]];
  case HTML_TITLE:
    [[fallthrough]];
  case HTML_TEXT:
    dyn_append_slice(sb, e.text, arena);
    break;

  // Invalid cases.
  case HTML_NONE:
    [[fallthrough]];
  case HTML_MAX:
    [[fallthrough]];
  default:
    ASSERT(0);
  }

  dyn_append_slice(sb, S("</"), arena);
  dyn_append_slice(sb, tag_to_string[e.kind], arena);
  *dyn_push(sb, arena) = '>';
}

[[maybe_unused]] [[nodiscard]] static String
http_req_extract_cookie_with_name(HttpRequest req, String cookie_name,
                                  Arena *arena) {
  String res = {0};
  {
    for (u64 i = 0; i < req.headers.len; i++) {
      KeyValue h = slice_at(req.headers, i);

      if (!string_ieq_ascii(h.key, S("Cookie"), arena)) {
        continue;
      }
      if (slice_is_empty(h.value)) {
        continue;
      }

      SplitIterator it_semicolon = string_split_string(h.value, S(";"));
      for (u64 j = 0; j < h.value.len; j++) {
        SplitResult split_semicolon = string_split_next(&it_semicolon);
        if (!split_semicolon.ok) {
          break;
        }

        SplitIterator it_equals =
            string_split_string(split_semicolon.s, S("="));
        SplitResult split_equals_left = string_split_next(&it_equals);
        if (!split_equals_left.ok) {
          break;
        }
        if (!string_eq(split_equals_left.s, cookie_name)) {
          // Could be: `; Secure;`
          continue;
        }
        SplitResult split_equals_right = string_split_next(&it_equals);
        if (!slice_is_empty(split_equals_right.s)) {
          return split_equals_right.s;
        }
      }
    }
  }
  return res;
}

typedef enum {
  LV_STRING,
  LV_U64,
} LogValueKind;

typedef enum {
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_FATAL,
} LogLevel;

typedef struct {
  LogLevel level;
  Writer writer;
} Logger;

typedef struct {
  LogValueKind kind;
  union {
    String s;
    u64 n64;
    u128 n128;
  };
} LogValue;

typedef struct {
  String key;
  LogValue value;
} LogEntry;

[[maybe_unused]] [[nodiscard]] static Logger
log_logger_make_stdout_json(LogLevel level) {
  Logger logger = {
      .level = level,
      .writer =
          writer_make_from_file((File *)(u64)STDOUT_FILENO), // TODO: Windows
  };

  return logger;
}

[[maybe_unused]] [[nodiscard]] static String
log_level_to_string(LogLevel level) {
  switch (level) {
  case LOG_LEVEL_DEBUG:
    return S("debug");
  case LOG_LEVEL_INFO:
    return S("info");
  case LOG_LEVEL_ERROR:
    return S("error");
  case LOG_LEVEL_FATAL:
    return S("fatal");
  default:
    ASSERT(false);
  }
}

[[maybe_unused]] [[nodiscard]] static LogEntry log_entry_int(String k, int v) {
  return (LogEntry){
      .key = k,
      .value.kind = LV_U64,
      .value.n64 = (u64)v,
  };
}

[[maybe_unused]] [[nodiscard]] static LogEntry log_entry_u16(String k, u16 v) {
  return (LogEntry){
      .key = k,
      .value.kind = LV_U64,
      .value.n64 = (u64)v,
  };
}

[[maybe_unused]] [[nodiscard]] static LogEntry log_entry_u32(String k, u32 v) {
  return (LogEntry){
      .key = k,
      .value.kind = LV_U64,
      .value.n64 = (u64)v,
  };
}

[[maybe_unused]] [[nodiscard]] static LogEntry log_entry_u64(String k, u64 v) {
  return (LogEntry){
      .key = k,
      .value.kind = LV_U64,
      .value.n64 = v,
  };
}

[[maybe_unused]] [[nodiscard]] static LogEntry log_entry_slice(String k,
                                                               String v) {
  return (LogEntry){
      .key = k,
      .value.kind = LV_STRING,
      .value.s = v,
  };
}

#define L(k, v)                                                                \
  (_Generic((v),                                                               \
       int: log_entry_int,                                                     \
       u16: log_entry_u16,                                                     \
       u32: log_entry_u32,                                                     \
       u64: log_entry_u64,                                                     \
       String: log_entry_slice)((S(k)), (v)))

#define LOG_ARGS_COUNT(...)                                                    \
  (sizeof((LogEntry[]){__VA_ARGS__}) / sizeof(LogEntry))
#define logger_log(logger, lvl, msg, arena, ...)                               \
  do {                                                                         \
    if ((logger)->level > (lvl)) {                                             \
      break;                                                                   \
    };                                                                         \
    Arena xxx_tmp_arena = (arena);                                             \
    String xxx_log_line =                                                      \
        log_make_log_line(lvl, S(msg), &xxx_tmp_arena,                         \
                          LOG_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__);           \
    (logger)->writer.write_fn((logger)->writer.ctx, xxx_log_line.data,         \
                              xxx_log_line.len);                               \
  } while (0)

[[maybe_unused]] [[nodiscard]] static String json_escape_string(String entry,
                                                                Arena *arena) {
  DynU8 sb = {0};
  *dyn_push(&sb, arena) = '"';

  for (u64 i = 0; i < entry.len; i++) {
    u8 c = slice_at(entry, i);
    if ('"' == c) {
      *dyn_push(&sb, arena) = '\\';
      *dyn_push(&sb, arena) = '"';
    } else if ('\\' == c) {
      *dyn_push(&sb, arena) = '\\';
      *dyn_push(&sb, arena) = '\\';
    } else if ('\b' == c) {
      *dyn_push(&sb, arena) = '\\';
      *dyn_push(&sb, arena) = 'b';
    } else if ('\f' == c) {
      *dyn_push(&sb, arena) = '\\';
      *dyn_push(&sb, arena) = 'f';
    } else if ('\n' == c) {
      *dyn_push(&sb, arena) = '\\';
      *dyn_push(&sb, arena) = 'n';
    } else if ('\r' == c) {
      *dyn_push(&sb, arena) = '\\';
      *dyn_push(&sb, arena) = 'r';
    } else if ('\t' == c) {
      *dyn_push(&sb, arena) = '\\';
      *dyn_push(&sb, arena) = 't';
    } else {
      *dyn_push(&sb, arena) = c;
    }
  }
  *dyn_push(&sb, arena) = '"';

  return dyn_slice(String, sb);
}

[[maybe_unused]] [[nodiscard]] static String
json_unescape_string(String entry, Arena *arena) {
  DynU8 sb = {0};

  for (u64 i = 0; i < entry.len; i++) {
    u8 c = slice_at(entry, i);
    u8 next = i + 1 < entry.len ? slice_at(entry, i + 1) : 0;

    if ('\\' == c) {
      if ('"' == next) {
        *dyn_push(&sb, arena) = '"';
        i += 1;
      } else if ('\\' == next) {
        *dyn_push(&sb, arena) = '\\';
        i += 1;
      } else if ('b' == next) {
        *dyn_push(&sb, arena) = '\b';
        i += 1;
      } else if ('f' == next) {
        *dyn_push(&sb, arena) = '\f';
        i += 1;
      } else if ('n' == next) {
        *dyn_push(&sb, arena) = '\n';
        i += 1;
      } else if ('r' == next) {
        *dyn_push(&sb, arena) = '\r';
        i += 1;
      } else if ('t' == next) {
        *dyn_push(&sb, arena) = '\t';
        i += 1;
      } else {
        *dyn_push(&sb, arena) = c;
      }
    } else {
      *dyn_push(&sb, arena) = c;
    }
  }

  return dyn_slice(String, sb);
}

[[maybe_unused]] static void
dynu8_append_json_object_key_string_value_string(DynU8 *sb, String key,
                                                 String value, Arena *arena) {
  String json_key = json_escape_string(key, arena);
  dyn_append_slice(sb, json_key, arena);

  dyn_append_slice(sb, S(":"), arena);

  String json_value = json_escape_string(value, arena);
  dyn_append_slice(sb, json_value, arena);

  dyn_append_slice(sb, S(","), arena);
}

[[maybe_unused]] static void
dynu8_append_json_object_key_string_value_u64(DynU8 *sb, String key, u64 value,
                                              Arena *arena) {
  String json_key = json_escape_string(key, arena);
  dyn_append_slice(sb, json_key, arena);

  dyn_append_slice(sb, S(":"), arena);

  dynu8_append_u64_to_string(sb, value, arena);

  dyn_append_slice(sb, S(","), arena);
}

[[maybe_unused]] [[nodiscard]] static String
log_make_log_line(LogLevel level, String msg, Arena *arena, i32 args_count,
                  ...) {
  struct timespec monotonic = {0};
  clock_gettime(CLOCK_MONOTONIC, &monotonic);
  u64 monotonic_ns =
      (u64)monotonic.tv_sec * 1000'000'000 + (u64)monotonic.tv_nsec;

  struct timespec now = {0};
  clock_gettime(CLOCK_REALTIME, &now);
  u64 timestamp_ns = (u64)now.tv_sec * 1000'000'000 + (u64)now.tv_nsec;

  DynU8 sb = {0};
  *dyn_push(&sb, arena) = '{';

  dynu8_append_json_object_key_string_value_string(
      &sb, S("level"), log_level_to_string(level), arena);
  dynu8_append_json_object_key_string_value_u64(&sb, S("timestamp_ns"),
                                                timestamp_ns, arena);
  dynu8_append_json_object_key_string_value_u64(&sb, S("monotonic_ns"),
                                                monotonic_ns, arena);
  dynu8_append_json_object_key_string_value_string(&sb, S("message"), msg,
                                                   arena);

  va_list argp = {0};
  va_start(argp, args_count);
  for (i32 i = 0; i < args_count; i++) {
    LogEntry entry = va_arg(argp, LogEntry);

    switch (entry.value.kind) {
    case LV_STRING: {
      dynu8_append_json_object_key_string_value_string(&sb, entry.key,
                                                       entry.value.s, arena);
      break;
    }
    case LV_U64:
      dynu8_append_json_object_key_string_value_u64(&sb, entry.key,
                                                    entry.value.n64, arena);
      break;
    default:
      ASSERT(0 && "invalid LogValueKind");
    }
  }
  va_end(argp);

  ASSERT(string_ends_with(dyn_slice(String, sb), S(",")));
  dyn_pop(&sb);
  dyn_append_slice(&sb, S("}\n"), arena);

  return dyn_slice(String, sb);
}

[[maybe_unused]] [[nodiscard]] static String
json_encode_string_slice(StringSlice strings, Arena *arena) {
  DynU8 sb = {0};
  *dyn_push(&sb, arena) = '[';

  for (u64 i = 0; i < strings.len; i++) {
    String s = dyn_at(strings, i);
    String encoded = json_escape_string(s, arena);
    dyn_append_slice(&sb, encoded, arena);

    if (i + 1 < strings.len) {
      *dyn_push(&sb, arena) = ',';
    }
  }

  *dyn_push(&sb, arena) = ']';

  return dyn_slice(String, sb);
}

[[maybe_unused]] [[nodiscard]] static StringSliceResult
json_decode_string_slice(String s, Arena *arena) {
  StringSliceResult res = {0};
  if (s.len < 2) {
    res.err = EINVAL;
    return res;
  }
  if ('[' != slice_at(s, 0)) {
    res.err = EINVAL;
    return res;
  }

  DynString dyn = {0};
  for (u64 i = 1; i < s.len - 2;) {
    i = skip_over_whitespace(s, i);

    u8 c = slice_at(s, i);
    if ('"' != c) { // Opening quote.
      res.err = EINVAL;
      return res;
    }
    i += 1;

    String remaining = slice_range_start(s, i);
    i64 end_quote_idx = string_indexof_unescaped_byte(remaining, '"');
    if (-1 == end_quote_idx) {
      res.err = EINVAL;
      return res;
    }

    ASSERT(0 <= end_quote_idx);

    String str = slice_range(s, i, i + (u64)end_quote_idx);
    String unescaped = json_unescape_string(str, arena);
    *dyn_push(&dyn, arena) = unescaped;

    i += (u64)end_quote_idx;

    if ('"' != c) { // Closing quote.
      res.err = EINVAL;
      return res;
    }
    i += 1;

    i = skip_over_whitespace(s, i);
    if (i + 1 == s.len) {
      break;
    }

    c = slice_at(s, i);
    if (',' != c) {
      res.err = EINVAL;
      return res;
    }
    i += 1;
  }

  if (']' != slice_at(s, s.len - 1)) {
    res.err = EINVAL;
    return res;
  }

  res.res = dyn_slice(StringSlice, dyn);
  return res;
}

#endif
