#ifndef CSTD_LIB_C
#define CSTD_LIB_C
#define _POSIX_C_SOURCE 200809L
#define __XSI_VISIBLE 600
#define __BSD_VISIBLE 1
#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE 1
#include "sha1.c"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/sendfile.h>
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
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

#define CLAMP(n, min, max)                                                     \
  do {                                                                         \
    if (*(n) < (min)) {                                                        \
      *(n) = (min);                                                            \
    }                                                                          \
    if (*(n) > (max)) {                                                        \
      *(n) = (max);                                                            \
    }                                                                          \
  } while (0)

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
                              : (ASSERT(nullptr != arr), (&(arr)[idx])))

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

[[maybe_unused]] [[nodiscard]] static bool ch_is_alphanumeric(u8 c) {
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') ||
         ('a' <= c && c <= 'z');
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
  (ASSERT((start) <= (end == 0 ? (s).len : end)), ASSERT((start) <= (s).len),  \
   ASSERT((end == 0 ? (s).len : end) <= (s).len),                              \
   (0 == (s).len) ? s                                                          \
                  : (typeof((s))){                                             \
                        .data = &(s).data[start],                              \
                        .len = (end == 0 ? (s).len : end) - (start),           \
                    })

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
  res.remaining = slice_range(haystack, 1, 0);
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

    if (!('0' <= c && c <= '9')) { // End of numbers sequence.
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

[[maybe_unused]] [[nodiscard]] static char *string_to_cstr(String s,
                                                           Arena *arena) {
  char *res = arena_alloc(arena, 1, 1, s.len + 1);
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
  ((T){.data = (dyn)->data, .len = (dyn)->cap - (dyn)->len})

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

#define arena_new(a, t, n) (t *)arena_alloc(a, sizeof(t), _Alignof(t), n)

[[maybe_unused]] [[nodiscard]] static String string_dup(String src,
                                                        Arena *arena) {
  String dst = {
      .len = src.len,
      .data = arena_new(arena, u8, src.len),
  };
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
  HS_ERR_INVALID_HTTP_REQUEST,
  HS_ERR_INVALID_HTTP_RESPONSE,
  HS_ERR_INVALID_FORM_DATA,
  HS_ERR_INVALID_JSON,
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

RESULT(int) CreateSocketResult;
[[maybe_unused]] [[nodiscard]] static CreateSocketResult
net_create_tcp_socket();
[[maybe_unused]] [[nodiscard]] static Error net_close_socket(int sock_fd);
[[maybe_unused]] [[nodiscard]] static Error net_set_nodelay(int sock_fd,
                                                            bool enabled);
[[maybe_unused]] [[nodiscard]] static Error
net_connect_ipv4(int sock_fd, Ipv4Address address);
typedef struct {
  Ipv4Address address;
  int socket;
} Ipv4AddressSocket;
RESULT(Ipv4AddressSocket) DnsResolveIpv4AddressSocketResult;
[[maybe_unused]] [[nodiscard]] static DnsResolveIpv4AddressSocketResult
net_dns_resolve_ipv4_tcp(String host, u16 port, Arena arena);

#if defined(__linux__) || defined(__FreeBSD__) // TODO: More Unices.
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

static CreateSocketResult net_create_tcp_socket() {
  CreateSocketResult res = {0};

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock_fd) {
    res.err = (Error)errno;
    return res;
  }

  res.res = sock_fd;

  return res;
}

static Error net_close_socket(int sock_fd) { return (Error)close(sock_fd); }

static Error net_set_nodelay(int sock_fd, bool enabled) {
  int opt = enabled;
  if (-1 == setsockopt(sock_fd, SOL_TCP, TCP_NODELAY, &opt, sizeof(opt))) {
    return (Error)errno;
  }

  return 0;
}

static Error net_connect_ipv4(int sock_fd, Ipv4Address address) {
  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(address.port),
      .sin_addr = {htonl(address.ip)},
  };

  if (-1 == connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr))) {
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

    if (-1 == connect(res_create_socket.res, rp->ai_addr, rp->ai_addrlen)) {
      // TODO: EINPROGRESS in case of non-blocking.
      res.err = (Error)errno;
      (void)net_close_socket(res_create_socket.res);
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

#else
#error "TODO"
#endif

RESULT(u64) IoCountResult;
RESULT(String) IoResult;

typedef IoCountResult (*ReadFn)(void *self, u8 *buf, size_t buf_len);

typedef struct {
  int fd;

  // TODO: Should probably be a ring buffer?
  u64 buf_idx;
  DynU8 buf;
} BufferedReader;

[[maybe_unused]] [[nodiscard]] static IoResult
buffered_reader_read(BufferedReader *br, u64 count) {
  ASSERT(br->buf_idx <= br->buf.len);

  IoResult res = {0};

  if (br->buf_idx == br->buf.len) { // If the buffer is empty.
    ASSERT(br->buf.len == 0);
    ASSERT(br->buf.cap <= 4096);
    String space = dyn_space(String, &br->buf);
    ssize_t read_n = read(br->fd, space.data, space.len);
    if (-1 == read_n) {
      res.err = (Error)errno;
      return res;
    }

    if (0 == read_n) {
      return res;
    }

    br->buf.len = (u64)read_n;
  }

  ASSERT(br->buf_idx < br->buf.len);

  // Buffered read.
  {
    u64 read_count_target = MIN(count, br->buf.len);
    String buffered =
        slice_range(dyn_slice(String, br->buf), br->buf_idx, read_count_target);
    res.res = buffered;
    br->buf_idx = br->buf.len = 0;
    return res;
  }
}

[[maybe_unused]] [[nodiscard]] static IoResult
buffered_reader_read_exactly(BufferedReader *r, u64 count, Arena *arena) {
  dyn_ensure_cap(&r->buf, count, arena);

  IoResult res = {0};

  for (u64 i = 0; i < count; i++) {
    if (r->buf.len == count) { // The end.
      res.res = dyn_slice(String, r->buf);
      return res;
    }

    u64 rem_count = count - r->buf.len;
    IoResult res_io = buffered_reader_read(r, rem_count);
    if (res_io.err) {
      res.err = res_io.err;
      return res;
    }
    if (0 == res_io.res.len) {
      res.err = (Error)EOF;
      return res;
    }

    r->buf.len += res_io.res.len;
    ASSERT(r->buf.len <= count);
  }

  ASSERT(res.res.len == count);
  return res;
}

[[maybe_unused]] [[nodiscard]] static BufferedReader
buffered_reader_make(int fd, Arena *arena) {
  BufferedReader r = {0};
  r.fd = fd;
  dyn_ensure_cap(&r.buf, 4096, arena);

  return r;
}

[[maybe_unused]] static void buffered_reader_put_back(BufferedReader *br,
                                                      String data) {
  ASSERT(0 == br->buf_idx);
  ASSERT(data.len <= br->buf.cap);
  if (0 == data.len) {
    return;
  }
  ASSERT(nullptr != data.data);
  ASSERT(nullptr != br->buf.data);

  memcpy(&br->buf.data, data.data, data.len);
  br->buf.len = data.len;
}

[[maybe_unused]] [[nodiscard]] static IoResult
buffered_reader_read_until_slice(BufferedReader *br, String needle,
                                 Arena *arena) {
  ASSERT(br->buf.cap != 0);
  u64 BUFFERED_READER_MAX_READ_BYTES = 4096;

  IoResult res = {0};
  DynU8 sb = {0};

  for (u64 i = 0; i < 128; i++) {
    IoResult res_io = buffered_reader_read(br, BUFFERED_READER_MAX_READ_BYTES);
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
    buffered_reader_put_back(br,
                             slice_range(to_search, (u64)idx + needle.len, 0));

    return res;
  }

  res.err = (Error)EOF;
  return res;
}

[[maybe_unused]] [[nodiscard]] static IoResult
buffered_reader_read_until_end(BufferedReader *br, Arena *arena) {
  ASSERT(br->buf.cap != 0);

  u64 BUFFERED_READER_MAX_READ_BYTES = 4096;

  IoResult res = {0};
  DynU8 sb = {0};

  for (u64 i = 0; i < 128; i++) {
    IoResult res_io = buffered_reader_read(br, BUFFERED_READER_MAX_READ_BYTES);
    if (res_io.err) {
      res.err = res_io.err;
      return res;
    }
    if (slice_is_empty(res_io.res)) {
      res.res = dyn_slice(String, sb);
      return res;
    }

    dyn_append_slice(&sb, res_io.res, arena);
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

typedef struct {
  String id;
  String path_raw;
  DynString path_components;
  DynKeyValue url_parameters;
  HttpMethod method;
  DynKeyValue headers;
  String body;
  Error err;
} HttpRequest;

typedef struct {
  u16 status;
  DynKeyValue headers;
  Error err;

  // TODO: union{file_path,body}?
  String file_path;
  String body;
} HttpResponse;

[[maybe_unused]] [[nodiscard]] static DynString
http_parse_relative_path(String s, bool must_start_with_slash, Arena *arena) {
  if (must_start_with_slash) {
    ASSERT(string_starts_with(s, S("/")));
  }

  DynString res = {0};

  SplitIterator split_it_question = string_split(s, '?');
  String work = string_split_next(&split_it_question).s;

  SplitIterator split_it_slash = string_split(work, '/');
  for (u64 i = 0; i < s.len; i++) { // Bound.
    SplitResult split = string_split_next(&split_it_slash);
    if (!split.ok) {
      break;
    }

    if (slice_is_empty(split.s)) {
      continue;
    }

    *dyn_push(&res, arena) = split.s;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static HttpRequest
request_parse_status_line(String status_line, Arena *arena) {
  HttpRequest req = {.id = make_unique_id_u128_string(arena)};

  if (slice_is_empty(status_line)) {
    req.err = HS_ERR_INVALID_HTTP_REQUEST;
    return req;
  }

  SplitIterator it = string_split(status_line, ' ');

  {
    SplitResult method = string_split_next(&it);
    if (!method.ok) {
      req.err = HS_ERR_INVALID_HTTP_REQUEST;
      return req;
    }

    if (string_eq(method.s, S("GET"))) {
      req.method = HM_GET;
    } else if (string_eq(method.s, S("POST"))) {
      req.method = HM_POST;
    } else {
      // FIXME: More.
      req.err = HS_ERR_INVALID_HTTP_REQUEST;
      return req;
    }
  }

  {
    SplitResult path = string_split_next(&it);
    if (!path.ok) {
      req.err = HS_ERR_INVALID_HTTP_REQUEST;
      return req;
    }

    if (slice_is_empty(path.s)) {
      req.err = HS_ERR_INVALID_HTTP_REQUEST;
      return req;
    }

    if (path.s.data[0] != '/') {
      req.err = HS_ERR_INVALID_HTTP_REQUEST;
      return req;
    }

    req.path_raw = path.s;
    req.path_components = http_parse_relative_path(path.s, true, arena);
  }

  {
    SplitResult http_version = string_split_next(&it);
    if (!http_version.ok) {
      req.err = HS_ERR_INVALID_HTTP_REQUEST;
      return req;
    }

    if (!string_eq(http_version.s, S("HTTP/1.1"))) {
      req.err = HS_ERR_INVALID_HTTP_REQUEST;
      return req;
    }
  }

  return req;
}

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
  String scheme;
  String username, password;
  String host; // Including subdomains.
  DynString path_components;
  // TODO: DynKeyValue url_parameters;
  u16 port;
  // TODO: fragment.
} Url;

RESULT(Url) ParseUrlResult;

[[maybe_unused]] [[nodiscard]] static ParseUrlResult url_parse(String s,
                                                               Arena *arena) {
  ParseUrlResult res = {0};

  String remaining = s;

  // Scheme, mandatory.
  {
    String scheme_sep = S("://");
    i64 scheme_sep_idx = string_indexof_string(remaining, scheme_sep);
    if (scheme_sep_idx <= 1) { // Absent/empty scheme.
      res.err = EINVAL;
      return res;
    }
    res.res.scheme = slice_range(remaining, 0, (u64)scheme_sep_idx);
    remaining = slice_range(remaining, (u64)scheme_sep_idx + scheme_sep.len, 0);
  }

  // Username/password, optional.
  {
    i64 user_password_sep_idx = string_indexof_byte(remaining, '@');
    if (user_password_sep_idx >= 0) {
      ASSERT(0 && "TODO");
    }
  }

  // Host, mandatory (?).
  {
    i64 any_sep_idx = string_indexof_any_byte(remaining, S(":/?#"));
    if (-1 == any_sep_idx) {
      res.err = EINVAL;
      res.res.host = remaining;
      if (0 == res.res.host.len) {
        return res;
      }

      return res;
    }

    res.res.host = slice_range(remaining, 0, (u64)any_sep_idx);
    if (0 == res.res.host.len) {
      res.err = EINVAL;
      return res;
    }

    bool is_port_sep = slice_at(remaining, any_sep_idx) == ':';
    remaining =
        slice_range(remaining, (u64)any_sep_idx + (is_port_sep ? 1 : 0), 0);

    if (is_port_sep) {
      ParseNumberResult port_parse = string_parse_u64(remaining);
      if (!port_parse.present) { // Empty/invalid port.
        res.err = EINVAL;
        return res;
      }
      if (port_parse.n > UINT16_MAX) { // Port too big.
        res.err = EINVAL;
        return res;
      }
      if (0 == port_parse.n) { // Zero port e.g. `http://abc:0`.
        res.err = EINVAL;
        return res;
      }
      res.res.port = (u16)port_parse.n;
      remaining = port_parse.remaining;
    }
  }

  // Path, optional.
  // Query parameters, optional.
  // FIXME: Messy code.
  {
    i64 any_sep_idx = string_indexof_any_byte(remaining, S("/?#"));
    if (-1 == any_sep_idx) {
      res.err = EINVAL;
      return res;
    }

    bool is_sep_path = slice_at(remaining, any_sep_idx) == '/';
    bool is_sep_fragment = slice_at(remaining, any_sep_idx) == '#';
    bool is_sep_query_params = slice_at(remaining, any_sep_idx) == '?';
    if (is_sep_query_params) {
      ASSERT(0 && "TODO");
    }
    if (is_sep_fragment) {
      ASSERT(0 && "TODO");
    } else if (is_sep_path) {
      if (any_sep_idx != 0) {
        res.err = EINVAL;
        return res;
      }

      String path_raw = slice_range(remaining, (u64)any_sep_idx + 1, 0);
      res.res.path_components =
          http_parse_relative_path(path_raw, false, arena);
    } else {
      ASSERT(0);
    }
  }

  return res;
}

#endif
