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

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a) (((a) < 0) ? (-(a)) : (a))
#endif

#ifndef ABS_SUB
#define ABS_SUB(a, b) (((a) < (b)) ? ((b) - (a)) : ((a) - (b)))
#endif

#define KiB (1024ULL)
#define MiB (1024ULL * Ki)
#define GiB (1024ULL * Mi)
#define TiB (1024ULL * Gi)

#define DYN(T)                                                                 \
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

typedef u32 Error;

#define RESULT(T)                                                              \
  typedef struct {                                                             \
    Error err;                                                                 \
    T res;                                                                     \
  }

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

[[maybe_unused]] [[nodiscard]] static bool ch_is_alpha(u8 c) {
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

[[maybe_unused]] [[nodiscard]] static bool ch_is_numeric(u8 c) {
  return ('0' <= c && c <= '9');
}

[[maybe_unused]] [[nodiscard]] static bool ch_is_alphanumeric(u8 c) {
  return ch_is_numeric(c) || ch_is_alpha(c);
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

#define slice_is_empty(s)                                                      \
  (((s).len == 0) ? true : (ASSERT(nullptr != (s).data), false))

#define S(s) ((String){.data = (u8 *)s, .len = sizeof(s) - 1})

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
  u8 sep;
} SplitIterator;

typedef struct {
  String s;
  bool ok;
} SplitResult;

[[maybe_unused]] [[nodiscard]] static SplitIterator string_split(String s,
                                                                 u8 sep) {
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

[[maybe_unused]] [[nodiscard]] static SplitResult
string_split_next(SplitIterator *it) {
  if (slice_is_empty(it->s)) {
    return (SplitResult){0};
  }

  for (u64 _i = 0; _i < it->s.len; _i++) {
    const i64 idx = string_indexof_byte(it->s, it->sep);
    if (-1 == idx) {
      // Last element.
      SplitResult res = {.s = it->s, .ok = true};
      it->s = (String){0};
      return res;
    }

    if (idx == 0) { // Multiple contiguous separators.
      it->s = slice_range(it->s, (u64)idx + 1, 0);
      continue;
    } else {
      SplitResult res = {.s = slice_range(it->s, 0, (u64)idx), .ok = true};
      it->s = slice_range(it->s, (u64)idx + 1, 0);

      return res;
    }
  }
  return (SplitResult){0};
}

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

  void *ptr = memmem(haystack.data, haystack.len, needle.data, needle.len);
  if (nullptr == ptr) {
    return -1;
  }

  u64 res = (u64)((u8 *)ptr - haystack.data);
  ASSERT(res < haystack.len);
  return (i64)res;
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
string_consume(String haystack, u8 needle) {
  StringConsumeResult res = {0};

  if (haystack.len == 0) {
    return res;
  }
  if (haystack.data[0] != needle) {
    return res;
  }

  res.consumed = true;
  res.remaining = slice_range(haystack, 1UL, 0UL);
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool string_ends_with(String haystack,
                                                            String needle) {
  if (haystack.len == 0 || haystack.len < needle.len) {
    return false;
  }
  ASSERT(nullptr != haystack.data);
  ASSERT(nullptr != needle.data);

  String end = slice_range(haystack, haystack.len - needle.len, 0);

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

  // Forbid leading zero(es).
  if (string_starts_with(s, S("0"))) {
    return res;
  }

  for (u64 i = 0; i < s.len; i++) {
    u8 c = slice_at(s, i);

    if (!ch_is_numeric(c)) { // End of numbers sequence.
      res.remaining = slice_range(s, i, 0);
      return res;
    }

    res.n *= 10;
    res.n += (u8)slice_at(s, i) - '0';
    res.present = true;
  }
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
  int cmp = memcmp(a.data, b.data, MIN(a.len, b.len));
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

DYN(String);

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
#define log(level, msg, arena, ...)                                            \
  do {                                                                         \
    Arena xxx_tmp_arena = *arena;                                              \
    String xxx_log_line =                                                      \
        make_log_line(level, S(msg), &xxx_tmp_arena,                           \
                      LOG_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__);               \
    write(1, xxx_log_line.data, xxx_log_line.len);                             \
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
make_log_line(LogLevel level, String msg, Arena *arena, i32 args_count, ...) {
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

SLICE(String);

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

typedef struct {
  Error err;
  StringSlice string_slice;
} JsonParseStringStrResult;

typedef enum {
  HS_ERR_INVALID_HTTP_REQUEST = 0x800,
  HS_ERR_INVALID_HTTP_RESPONSE = 0x801,
  HS_ERR_INVALID_FORM_DATA = 0x802,
  HS_ERR_INVALID_JSON = 0x803,
} HS_Error;

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

[[maybe_unused]] [[nodiscard]] static JsonParseStringStrResult
json_decode_string_slice(String s, Arena *arena) {
  JsonParseStringStrResult res = {0};
  if (s.len < 2) {
    res.err = HS_ERR_INVALID_JSON;
    return res;
  }
  if ('[' != slice_at(s, 0)) {
    res.err = HS_ERR_INVALID_JSON;
    return res;
  }

  DynString dyn = {0};
  for (u64 i = 1; i < s.len - 2;) {
    i = skip_over_whitespace(s, i);

    u8 c = slice_at(s, i);
    if ('"' != c) { // Opening quote.
      res.err = HS_ERR_INVALID_JSON;
      return res;
    }
    i += 1;

    String remaining = slice_range(s, i, 0);
    i64 end_quote_idx = string_indexof_unescaped_byte(remaining, '"');
    if (-1 == end_quote_idx) {
      res.err = HS_ERR_INVALID_JSON;
      return res;
    }

    ASSERT(0 <= end_quote_idx);

    String str = slice_range(s, i, i + (u64)end_quote_idx);
    String unescaped = json_unescape_string(str, arena);
    *dyn_push(&dyn, arena) = unescaped;

    i += (u64)end_quote_idx;

    if ('"' != c) { // Closing quote.
      res.err = HS_ERR_INVALID_JSON;
      return res;
    }
    i += 1;

    i = skip_over_whitespace(s, i);
    if (i + 1 == s.len) {
      break;
    }

    c = slice_at(s, i);
    if (',' != c) {
      res.err = HS_ERR_INVALID_JSON;
      return res;
    }
    i += 1;
  }

  if (']' != slice_at(s, s.len - 1)) {
    res.err = HS_ERR_INVALID_JSON;
    return res;
  }

  res.string_slice = dyn_slice(StringSlice, dyn);
  return res;
}

[[maybe_unused]] [[nodiscard]] static String string_clone(String s,
                                                          Arena *arena) {
  String res = {.data = arena_new(arena, u8, s.len), .len = s.len};
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

typedef struct {
  Error error;
  String content;
} ReadFileResult;

[[maybe_unused]] static ReadFileResult file_read_full(String path,
                                                      Arena *arena) {

  ReadFileResult res = {0};
  char *path_c = string_to_cstr(path, arena);

  int fd = open(path_c, O_RDONLY);
  if (fd < 0) {
    res.error = (Error)errno;
    return res;
  }

  struct stat st = {0};
  if (-1 == stat(path_c, &st)) {
    res.error = (Error)errno;
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
      res.error = (Error)errno;
      goto end;
    }

    if (0 == read_n) {
      res.error = (Error)EINVAL;
      goto end;
    }

    ASSERT((u64)read_n <= space.len);

    sb.len += (u64)read_n;
  }

end:
  close(fd);
  res.content = dyn_slice(String, sb);
  return res;
}

typedef struct {
  u32 ip;   // Host order.
  u16 port; // Host order.
} Ipv4Address;

DYN(Ipv4Address);

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

typedef int Socket;
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
[[maybe_unused]] [[nodiscard]] static AioQueueCreateResult
net_aio_queue_create();

typedef enum {
  AIO_EVENT_KIND_IN,
  AIO_EVENT_KIND_OUT,
  AIO_EVENT_KIND_ERR,
} AioEventKind;

typedef enum {
  AIO_EVENT_ACTION_KIND_ADD,
  AIO_EVENT_ACTION_KIND_MOD,
  AIO_EVENT_ACTION_KIND_DEL,
} AioEventActionKind;

typedef struct {
  Socket socket;
  AioEventKind kind;
  AioEventActionKind action;
  u64 user_data;
} AioEvent;

SLICE(AioEvent);

[[maybe_unused]] [[nodiscard]] static Error
net_aio_queue_ctl(AioQueue queue, AioEventSlice events);

[[maybe_unused]] [[nodiscard]] static Error
net_aio_queue_wait(AioQueue queue, AioEventSlice events, i64 timeout_ms,
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
  return 0;
}

[[maybe_unused]] [[nodiscard]] static Ipv4AddressAcceptResult
net_tcp_accept(Socket sock) {
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

[[maybe_unused]] [[nodiscard]] static AioQueueCreateResult
net_aio_queue_create() {
  AioQueueCreateResult res = {0};
  int queue = epoll_create(1 /* Ignored */);
  if (-1 == queue) {
    res.err = (Error)errno;
  }
  res.res = (AioQueue)queue;
  return res;
}

[[maybe_unused]] [[nodiscard]] static Error
net_aio_queue_ctl(AioQueue queue, AioEventSlice events) {
  for (u64 i = 0; i < events.len; i++) {
    AioEvent event = slice_at(events, i);

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
    default:
      ASSERT(0);
    }

    struct epoll_event epoll_event = {0};
    switch (event.kind) {
    case AIO_EVENT_KIND_IN:
      epoll_event.events |= EPOLLIN;
      break;
    case AIO_EVENT_KIND_OUT:
      epoll_event.events |= EPOLLOUT;
      break;
    case AIO_EVENT_KIND_ERR:
    default:
      ASSERT(0);
    }
    epoll_event.data.u64 = event.user_data;

    int res_epoll = epoll_ctl((int)queue, op, event.socket, &epoll_event);
    if (-1 == res_epoll) {
      return (Error)errno;
    }
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Error
net_aio_queue_wait(AioQueue queue, AioEventSlice events, i64 timeout_ms,
                   Arena arena) {
  struct epoll_event *epoll_events =
      arena_new(&arena, struct epoll_event, events.len);

  int res_epoll =
      epoll_wait((int)queue, epoll_events, (int)events.len, (int)timeout_ms);
  if (-1 == res_epoll) {
    return (Error)errno;
  }

  for (u64 i = 0; i < events.len; i++) {
    AioEvent *event = slice_at_ptr(&events, i);
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
  }

  return 0;
}
#endif

RESULT(u64) IoCountResult;
RESULT(String) IoResult;

typedef IoCountResult (*ReadFn)(void *self, u8 *buf, size_t buf_len);

// TODO: Guard with `ifdef`?
// TODO: Windows?
[[maybe_unused]] [[nodiscard]] static IoCountResult
unix_read(void *self, u8 *buf, size_t buf_len) {
  ASSERT(nullptr != self);

  int fd = (int)(u64)self;
  ssize_t n_read = read(fd, buf, buf_len);

  IoCountResult res = {0};
  if (n_read < 0) {
    res.err = (Error)errno;
  } else {
    res.res = (u64)n_read;
  }

  return res;
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

    u64 write_len1 = MIN(can_write1, data.len);
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
    u64 n_read = MIN(data.len, can_read);

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

    u64 read_len1 = MIN(can_read1, data.len);
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

[[maybe_unused]] [[nodiscard]] static String
ring_buffer_read_until_excl(RingBuffer *rg, String needle, Arena *arena) {
  String res = {0};
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

  res = string_make((u64)idx, arena);
  ASSERT(ring_buffer_read_slice(rg, res));

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
  ReadFn read_fn;

  RingBuffer rg;
} BufferedReader;

[[maybe_unused]] [[nodiscard]] static int
buffered_reader_fd(BufferedReader *br) {
  ASSERT(br->ctx);
  return (int)(u64)br->ctx;
}

[[maybe_unused]] [[nodiscard]] static IoResult
buffered_reader_read(BufferedReader *br, u64 count, Arena *arena) {
  ASSERT(nullptr != br->read_fn);

  IoResult res = {0};

  // Try a buffered read first.

  if (ring_buffer_read_space(br->rg) < count) {
  }
  {
    Arena cpy = *arena;
    u64 dst_len = MIN(ring_buffer_read_space(br->rg), count);
    String dst = string_make(dst_len, arena);
    bool ok = ring_buffer_read_slice(&br->rg, dst);
    if (ok) {
      res.res = dst;
      return res;
    }
    // Reset arena.
    *arena = cpy;
  }

  // We have to do I/O.
  {
    Arena tmp = *arena;
    String dst = string_make(ring_buffer_write_space(br->rg), &tmp);
    IoCountResult res_io = br->read_fn(br->ctx, dst.data, dst.len);
    if (res_io.err) {
      res.err = res_io.err;
      return res;
    }

    if (0 == res_io.res) {
      return res;
    }

    dst.len = res_io.res;

    ASSERT(ring_buffer_write_slice(&br->rg, dst));
  }
  return buffered_reader_read(br, count, arena);
}

[[maybe_unused]] [[nodiscard]] static IoResult
reader_read_exactly(Reader *r, u64 count, Arena *arena) {
  IoResult res = {0};
  DynU8 sb = {0};
  dyn_ensure_cap(&sb, count, arena);

  for (u64 i = 0; i < count + 1; i++) {
    if (sb.len == count) { // The end.
      res.res = dyn_slice(String, sb);
      return res;
    }

    String space = dyn_space(String, &sb);
    u64 rem = count - sb.len;
    String dst = slice_range(space, 0, rem);
    IoCountResult res_io = r->read_fn(r->ctx, dst.data, dst.len);
    if (res_io.err) {
      res.err = res_io.err;
      return res;
    }
    if (0 == res_io.res) {
      res.err = (Error)EOF;
      return res;
    }
    sb.len += res_io.res;
    ASSERT(sb.len <= sb.cap);
    ASSERT(sb.len <= count);
  }

  ASSERT(res.res.len == count);
  return res;
}

[[maybe_unused]] [[nodiscard]] static BufferedReader
buffered_reader_make(int fd, Arena *arena) {
  BufferedReader r = {0};
  r.ctx = (void *)(u64)fd;
  r.rg.data = string_make(1024, arena);

  // TODO: Windows.
  r.read_fn = unix_read;

  return r;
}

typedef struct {
  String s;
  u64 idx;
} TestMemReadContext;

[[nodiscard]] [[maybe_unused]]
static IoCountResult test_buffered_reader_read_from_slice(void *ctx, u8 *buf,
                                                          size_t buf_len) {
  TestMemReadContext *mem_ctx = ctx;

  ASSERT(buf != nullptr);
  ASSERT(mem_ctx->s.data != nullptr);

  IoCountResult res = {0};
  if (mem_ctx->idx >= mem_ctx->s.len) {
    // End.
    return res;
  }

  const u64 remaining = mem_ctx->s.len - mem_ctx->idx;
  const u64 can_fill = MIN(remaining, buf_len);
  ASSERT(can_fill <= remaining);

  res.res = can_fill;
  memcpy(buf, mem_ctx->s.data + mem_ctx->idx, can_fill);

  ASSERT(mem_ctx->idx + can_fill <= mem_ctx->s.len);
  mem_ctx->idx += can_fill;
  ASSERT(mem_ctx->idx <= mem_ctx->s.len);
  return res;
}

[[maybe_unused]] [[nodiscard]]
static IoCountResult
test_buffered_reader_read_from_slice_one(void *ctx, u8 *buf, size_t buf_len) {
  (void)buf_len;

  TestMemReadContext *mem_ctx = ctx;

  ASSERT(buf != nullptr);
  ASSERT(mem_ctx->s.data != nullptr);

  IoCountResult res = {0};
  if (mem_ctx->idx >= mem_ctx->s.len) {
    // End.
    return res;
  }

  const u64 remaining = mem_ctx->s.len - mem_ctx->idx;
  const u64 can_fill = 1;
  ASSERT(can_fill <= remaining);

  res.res = can_fill;
  memcpy(buf, mem_ctx->s.data + mem_ctx->idx, can_fill);

  ASSERT(mem_ctx->idx + can_fill <= mem_ctx->s.len);
  mem_ctx->idx += can_fill;
  ASSERT(mem_ctx->idx <= mem_ctx->s.len);
  return res;
}

[[maybe_unused]] [[nodiscard]] static BufferedReader
test_buffered_reader_make(TestMemReadContext *ctx, Arena *arena) {
  BufferedReader r = {0};
  r.ctx = ctx;
  r.read_fn = test_buffered_reader_read_from_slice;
  r.rg.data = string_make(1024, arena);

  return r;
}

[[maybe_unused]] [[nodiscard]] static IoResult
buffered_reader_read_until_slice(BufferedReader *br, String needle,
                                 Arena *arena) {

  IoResult res = {0};
  DynU8 sb = {0};

  for (u64 i = 0; i < 128; i++) {

    IoResult res_io = buffered_reader_read(br, 1024, arena);
    if (res_io.err) {
      res.err = res_io.err;
      return res;
    }
    if (slice_is_empty(res_io.res)) {
      res.err = (Error)EOF;
      return res;
    }

    dyn_append_slice(&sb, res_io.res, arena);

    // OPTIMIZATION: Could search only most recent part of sb minus the needle
    // len.
    String to_search = dyn_slice(String, sb);
    i64 idx = string_indexof_string(to_search, needle);
    if (idx == -1) {
      continue;
    }

    res.res = slice_range(to_search, 0, (u64)idx + needle.len);
#if 0
    buffered_reader_put_back(br,
                             slice_range(to_search, (u64)idx + needle.len, 0));
#endif

    return res;
  }

  res.err = (Error)EOF;
  return res;
}

typedef struct {
  int fd;
} Writer;

[[maybe_unused]] [[nodiscard]] static Error writer_write_all_sync(Writer *w,
                                                                  String s) {
  for (u64 idx = 0; idx < s.len;) {
    const String to_write = slice_range(s, idx, 0);
    ssize_t written_n = write(w->fd, to_write.data, to_write.len);
    if (-1 == written_n) {
      return (Error)errno;
    }

    ASSERT((u64)written_n <= s.len);
    ASSERT(idx <= s.len);
    idx += (u64)written_n;
    ASSERT(idx <= s.len);
  }
  return 0;
}

typedef enum { HM_UNKNOWN, HM_GET, HM_POST } HttpMethod;

[[maybe_unused]]
String static http_method_to_s(HttpMethod m) {
  switch (m) {
  case HM_UNKNOWN:
    return S("unknown");
  case HM_GET:
    return S("GET");
  case HM_POST:
    return S("POST");
  default:
    ASSERT(0);
  }
}

typedef struct {
  String key, value;
} KeyValue;

typedef struct {
  KeyValue *data;
  u64 len, cap;
} DynKeyValue;

typedef enum {
  HTTP_PARSE_STATE_NONE,
  HTTP_PARSE_STATE_PARSED_STATUS_LINE,
  HTTP_PARSE_STATE_PARSED_ALL_HEADERS,
  HTTP_PARSE_STATE_DONE,
} HttpParseState;

typedef struct {
  String id;
  DynString path_components;
  DynKeyValue url_parameters;
  HttpMethod method;
  DynKeyValue headers;
  String body;
} HttpRequest;

RESULT(HttpRequest) HttpRequestParseResult;

typedef struct {
  HttpRequest req;
  HttpParseState state;
} HttpRequestParse;

typedef struct {
  u16 status;
  DynKeyValue headers;
  Error err;

  // TODO: union{file_path,body}?
  String file_path;
  String body;
} HttpResponse;

RESULT(StringSlice) StringSliceResult;

#if 0
[[maybe_unused]] [[nodiscard]] static HttpRequestParseResult
http_parse_status_line(String status_line, Arena *arena) {
  HttpRequestParseResult res = {0};

  if (slice_is_empty(status_line)) {
    res.err = HS_ERR_INVALID_HTTP_REQUEST;
    return res;
  }

  SplitIterator it = string_split(status_line, ' ');

  {
    SplitResult method = string_split_next(&it);
    if (!method.ok) {
      res.err = HS_ERR_INVALID_HTTP_REQUEST;
      return res;
    }

    if (string_eq(method.s, S("GET"))) {
      res.res.method = HM_GET;
    } else if (string_eq(method.s, S("POST"))) {
      res.res.method = HM_POST;
    } else {
      // FIXME: More.
      res.err = HS_ERR_INVALID_HTTP_REQUEST;
      return res;
    }
  }

  {
    SplitResult path = string_split_next(&it);
    if (!path.ok) {
      res.err = HS_ERR_INVALID_HTTP_REQUEST;
      return res;
    }

    if (slice_is_empty(path.s)) {
      res.err = HS_ERR_INVALID_HTTP_REQUEST;
      return res;
    }

    if (path.s.data[0] != '/') {
      res.err = HS_ERR_INVALID_HTTP_REQUEST;
      return res;
    }

    res.res.path_components = http_parse_relative_path(path.s, true, arena);
  }

  {
    SplitResult http_version = string_split_next(&it);
    if (!http_version.ok) {
      res.err = HS_ERR_INVALID_HTTP_REQUEST;
      return res;
    }

    if (!string_eq(http_version.s, S("HTTP/1.1"))) {
      res.err = HS_ERR_INVALID_HTTP_REQUEST;
      return res;
    }
  }

  return res;
}
#endif

[[maybe_unused]]
static void http_push_header(DynKeyValue *headers, String key, String value,
                             Arena *arena) {
  *dyn_push(headers, arena) = (KeyValue){.key = key, .value = value};
}

// TODO: Split serializing body?
[[maybe_unused]] [[nodiscard]] static String
http_request_serialize(HttpRequest req, Arena *arena) {
  DynU8 sb = {0};
  dyn_append_slice(&sb, http_method_to_s(req.method), arena);
  dyn_append_slice(&sb, S(" /"), arena);

  for (u64 i = 0; i < req.path_components.len; i++) {
    String path_component = dyn_at(req.path_components, i);
    dyn_append_slice(&sb, path_component, arena);

    if (i < req.path_components.len - 1) {
      *dyn_push(&sb, arena) = '/';
    }
  }

  if (req.url_parameters.len > 0) {
    *dyn_push(&sb, arena) = '?';
    for (u64 i = 0; i < req.url_parameters.len; i++) {
      KeyValue param = dyn_at(req.url_parameters, i);
      url_encode_string(&sb, param.key, param.value, arena);

      if (i < req.url_parameters.len - 1) {
        *dyn_push(&sb, arena) = '&';
      }
    }
  }

  dyn_append_slice(&sb, S(" HTTP/1.1"), arena);
  dyn_append_slice(&sb, S("\r\n"), arena);

  for (u64 i = 0; i < req.headers.len; i++) {
    KeyValue header = dyn_at(req.headers, i);
    dyn_append_slice(&sb, header.key, arena);
    dyn_append_slice(&sb, S(": "), arena);
    dyn_append_slice(&sb, header.value, arena);
    dyn_append_slice(&sb, S("\r\n"), arena);
  }
  dyn_append_slice(&sb, S("\r\n"), arena);
  dyn_append_slice(&sb, req.body, arena);

  return dyn_slice(String, sb);
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
  StringSlice path_components;
} UrlAuthority;

RESULT(UrlAuthority) UrlAuthorityResult;

typedef struct {
  String scheme;
  String username, password;
  String host; // Including subdomains.
  StringSlice path_components;
  // TODO: DynKeyValue url_parameters;
  u16 port;
  // TODO: fragment.
} Url;

RESULT(Url) ParseUrlResult;

[[maybe_unused]] [[nodiscard]] static StringSliceResult
url_parse_path_components(String s, Arena *arena) {
  StringSliceResult res = {0};

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

  SplitIterator split_it_slash = string_split(s, '/');
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

  res.res = dyn_slice(StringSlice, components);
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

  ParseNumberResult port_parse = string_parse_u64(s);
  if (!slice_is_empty(port_parse.remaining)) {
    res.err = EINVAL;
    return res;
  }
  res.res = (u16)port_parse.n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static UrlAuthorityResult
url_parse_authority(String s, Arena *arena) {
  UrlAuthorityResult res = {0};

  String remaining = s;
  // User info.
  {
    i64 sep_idx = string_indexof_byte(remaining, '@');
    if (-1 != sep_idx) {
      String user_info = slice_range(remaining, 0, (u64)sep_idx);
      UrlUserInfoResult res_user_info = url_parse_user_info(user_info);
      if (res_user_info.err) {
        res.err = res_user_info.err;
        return res;
      }

      remaining = slice_range(remaining, (u64)sep_idx + 1, 0);
    }
  }

  // Host.
  {
    i64 sep_port_idx = string_indexof_byte(remaining, ':');
    if (-1 != sep_port_idx) {
      res.res.host = slice_range(remaining, 0, (u64)sep_port_idx);
      remaining = slice_range(remaining, (u64)sep_port_idx + 1, 0);

      i64 sep_path_idx = string_indexof_byte(remaining, '/');
      String port = remaining;
      if (-1 != sep_path_idx) {
        port = slice_range(remaining, 0, (u64)sep_path_idx);
        remaining = slice_range(remaining, (u64)sep_path_idx + 1, 0);
      }

      PortResult res_port = url_parse_port(port);
      if (res_port.err) {
        res.err = res_port.err;
        return res;
      }
      res.res.port = res_port.res;
    } else {
      res.res.host = remaining;
      remaining = (String){0};
    }
  }
  if (slice_is_empty(res.res.host)) {
    res.err = EINVAL;
    return res;
  }

  if (string_starts_with(remaining, S("/"))) {
    remaining = slice_range(remaining, 1UL, 0UL);

    StringSliceResult res_path_components =
        url_parse_path_components(remaining, arena);
    if (res_path_components.err) {
      res.err = res_path_components.err;
      return res;
    }
    res.res.path_components = res_path_components.res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool url_is_scheme_valid(String scheme) {
  if (slice_is_empty(scheme)) {
    return false;
  }

  u8 first = slice_at(scheme, 0);
  if (!ch_is_alpha(first)) {
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

[[maybe_unused]] [[nodiscard]] static ParseUrlResult url_parse(String s,
                                                               Arena *arena) {
  ParseUrlResult res = {0};

  String remaining = s;

  i64 scheme_sep_idx = string_indexof_byte(remaining, ':');
  if (-1 == scheme_sep_idx) {
    res.err = EINVAL;
    return res;
  }
  String scheme = slice_range(remaining, 0, (u64)scheme_sep_idx);
  if (!url_is_scheme_valid(scheme)) {
    res.err = EINVAL;
    return res;
  }
  res.res.scheme = scheme;

  remaining = slice_range(remaining, (u64)scheme_sep_idx + 1, 0);

  // TODO: Be less strict hier.
  if (!string_starts_with(remaining, S("//"))) {
    res.err = EINVAL;
    return res;
  }
  remaining = slice_range(remaining, 2UL, 0UL); // Consume `//`.

  i64 authority_sep_idx = string_indexof_any_byte(remaining, S("?#"));
  String authority = remaining;
  if (-1 != authority_sep_idx) {
    authority = slice_range(remaining, 0, (u64)authority_sep_idx);
    remaining = slice_range(remaining, (u64)authority_sep_idx + 1, 0);
  }

  UrlAuthorityResult res_authority = url_parse_authority(authority, arena);
  if (res_authority.err) {
    res.err = res_authority.err;
    return res;
  }
  res.res.host = res_authority.res.host;
  res.res.port = res_authority.res.port;
  res.res.username = res_authority.res.user_info.username;
  res.res.password = res_authority.res.user_info.password;

  // Path, optional.
  // Query parameters, optional.
  if (-1 != authority_sep_idx) {
    bool is_sep_fragment = slice_at(remaining, authority_sep_idx) == '#';
    bool is_sep_query_params = slice_at(remaining, authority_sep_idx) == '?';
    ASSERT(is_sep_fragment || is_sep_query_params);

    if (is_sep_query_params) {
      ASSERT(0 && "TODO");
    }
    if (is_sep_fragment) {
      ASSERT(0 && "TODO");
    }
  }

  return res;
}

#if 0
static const u64 HTTP_REQUEST_LINES_MAX_COUNT = 512;
[[nodiscard]] static Error
http_read_headers(BufferedReader *reader, DynKeyValue *headers, Arena *arena) {
  dyn_ensure_cap(headers, 30, arena);

  for (u64 _i = 0; _i < HTTP_REQUEST_LINES_MAX_COUNT; _i++) {
    const IoResult res_io =
        buffered_reader_read_until_slice(reader, S("\r\n"), arena);

    if (res_io.err) {
      return res_io.err;
    }

    if (slice_is_empty(res_io.res)) {
      break;
    }

    SplitIterator it = string_split(res_io.res, ':');
    SplitResult key = string_split_next(&it);
    if (!key.ok) {
      return HS_ERR_INVALID_HTTP_REQUEST;
    }

    String key_trimmed = string_trim(key.s, ' ');

    String value = it.s; // Remainder.
    String value_trimmed = string_trim(value, ' ');

    KeyValue header = {.key = key_trimmed, .value = value_trimmed};
    *dyn_push(headers, arena) = header;
  }
  return 0;
}

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
#endif

[[maybe_unused]] [[nodiscard]] static Error
http_request_parse_next(HttpRequestParse *parse, RingBuffer *rg, Arena *arena) {
  String nr = S("\r\n");

  switch (parse->state) {
  case HTTP_PARSE_STATE_NONE: {
    String line = ring_buffer_read_until_excl(rg, nr, arena);
    if (slice_is_empty(line)) {
      return 0;
    }
  } break;
  case HTTP_PARSE_STATE_PARSED_STATUS_LINE: {
    String line = ring_buffer_read_until_excl(rg, nr, arena);
    if (slice_is_empty(line)) {
      return 0;
    }
  } break;
  case HTTP_PARSE_STATE_PARSED_ALL_HEADERS: {
    String line = ring_buffer_read_until_excl(rg, nr, arena);
    if (slice_is_empty(line)) {
      return 0;
    }
  } break;
  case HTTP_PARSE_STATE_DONE:
    return 0;
  default:
    ASSERT(0);
  }

  ASSERT(0);
}

#if 0
[[maybe_unused]] [[nodiscard]] static HttpRequest
request_read(BufferedReader *reader, Arena *arena) {
  const IoResult status_line =
      buffered_reader_read_until_slice(reader, S("\r\n"), arena);
  if (status_line.err) {
    return (HttpRequest){.err = status_line.err};
  }

  HttpRequest req = http_parse_status_line(status_line.res, arena);
  if (req.err) {
    return req;
  }

  req.err = http_read_headers(reader, &req.headers, arena);
  if (req.err) {
    return req;
  }

  ParseNumberResult content_length =
      request_parse_content_length_maybe(req, arena);
  if (!content_length.present) {
    req.err = HS_ERR_INVALID_HTTP_REQUEST;
    return req;
  }

  if (content_length.present) {
    IoResult res_io =
        reader_read_exactly((Reader *)reader, content_length.n, arena);
    if (res_io.err) {
      req.err = res_io.err;
      return req;
    }
    req.body = res_io.res;
  }

  return req;
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
        slice_range(io_result.res, http1_1_version_needle.len, 0);
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
        res.err = HS_ERR_INVALID_FORM_DATA;
        return res;
      }
      u8 c1 = in.data[i + 1];
      u8 c2 = in.data[i + 2];

      if (!(ch_is_hex_digit(c1) && ch_is_hex_digit(c2))) {
        res.err = HS_ERR_INVALID_FORM_DATA;
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
  res.remaining = slice_range(in, i, 0);
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

      SplitIterator it_semicolon = string_split(h.value, ';');
      for (u64 j = 0; j < h.value.len; j++) {
        SplitResult split_semicolon = string_split_next(&it_semicolon);
        if (!split_semicolon.ok) {
          break;
        }

        SplitIterator it_equals = string_split(split_semicolon.s, '=');
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

#endif
