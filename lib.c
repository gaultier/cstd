#ifndef CSTD_LIB_C
#define CSTD_LIB_C

// TODO: IPv6.
// TODO: *Pool allocator?*
// TODO: Randomize arena guard pages.
// TODO: HTTP compression (gzip, etc)
// TODO: TLS 1.3
// TODO: Pprof memory profiling.
// TODO: [Unix] Human-readable stacktrace.
// TODO: Get PIE offset for better call stack.
// Low priority:
// TODO: Test untested functions.
// TODO: [Unix] CLI argument parser.

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__) ||        \
    defined(__unix__)
#define PG_OS_UNIX
#endif

#if defined(__unix__)
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 1
#endif

#include "sha1.c"
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PG_PATH_MAX 4096

#define PG_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define PG_MAX(a, b) (((a) < (b)) ? (b) : (a))

#define PG_KiB (1024ULL)
#define PG_MiB (1024ULL * PG_KiB)
#define PG_GiB (1024ULL * PG_MiB)
#define PG_TiB (1024ULL * PG_GiB)

#define PG_Nanoseconds (1ULL)
#define PG_Microseconds (1000ULL * PG_Nanoseconds)
#define PG_Milliseconds (1000ULL * PG_Microseconds)
#define PG_Seconds (1000ULL * PG_Milliseconds)

#define PG_CONTAINER_OF(ptr, type, member)                                     \
  ((type *)(void *)((char *)(ptr) - offsetof(type, member)))

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
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;
typedef ssize_t isize;

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
#ifdef PG_OS_UNIX
#include <errno.h>
#define PG_ERR_INVALID_VALUE EINVAL
#define PG_ERR_IO EIO
#define PG_ERR_TOO_BIG E2BIG
#else
// Use the x86_64 Linux errno values.
#define PG_ERR_INVALID_VALUE 22
#define PG_ERR_IO 5
#define PG_ERR_TOO_BIG 7
#endif

PG_RESULT(i32) PgI32Result;
PG_RESULT(u64) PgU64Result;
PG_RESULT(bool) PgBoolResult;
PG_RESULT(void *) PgVoidPtrResult;
PG_OK(u32) Pgu32Ok;
PG_OK(u64) Pgu64Ok;

PG_DYN(u8) Pgu8Dyn;
PG_SLICE(u8) Pgu8Slice;
PG_DYN(char *) PgCstrDyn;
typedef Pgu8Slice PgString;

#define PG_STATIC_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define PG_CLAMP(min, n, max) ((n) < (min) ? (min) : (n) > (max) ? (max) : n)

#define PG_SUB_SAT(a, b) ((a) > (b) ? ((a) - (b)) : 0)

[[maybe_unused]] [[nodiscard]] static u64 pg_ns_to_ms(u64 ns) {
  return ns / 1'000'000;
}

#define PG_STACKTRACE_MAX 64
#define PG_LOG_STRING_MAX 256
#define PG_LOG_LINE_MAX_LENGTH 8192

[[maybe_unused]] static u64
pg_fill_call_stack(u64 call_stack[PG_STACKTRACE_MAX]);

[[maybe_unused]] static void pg_stacktrace_print(const char *file, int line,
                                                 const char *function) {
  fprintf(stderr, "ASSERT: %s:%d:%s\n", file, line, function);

  // TODO
#if 0
  u64 call_stack[PG_STACKTRACE_MAX] = {0};
  u64 callstack_len = pg_fill_call_stack(call_stack);

  for (u64 i = 0; i < callstack_len; i++) {
    fprintf(stderr, "%#" PRIx64 "\n", call_stack[i]);
  }

  puts("");
#endif
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

typedef int PgSocket;
typedef union {
  int fd;
  void *ptr;
} PgFileDescriptor;

PG_RESULT(PgFileDescriptor) PgFileDescriptorResult;

typedef enum {
  PG_FILE_ACCESS_NONE = 0,
  PG_FILE_ACCESS_READ = 1 << 0,
  PG_FILE_ACCESS_WRITE = 1 << 1,
  PG_FILE_ACCESS_READ_WRITE = 1 << 2,
} PgFileAccess;

static const u64 PG_FILE_ACCESS_ALL =
    PG_FILE_ACCESS_READ | PG_FILE_ACCESS_WRITE | PG_FILE_ACCESS_READ_WRITE;

// Non-owning.
typedef struct PgQueue {
  struct PgQueue *prev, *next;
} PgQueue;

[[maybe_unused]]
static void pg_queue_init(PgQueue *queue) {
  PG_ASSERT(queue);
  queue->next = queue;
  queue->prev = queue;
}

[[nodiscard]] [[maybe_unused]]
static bool pg_queue_is_empty(PgQueue *queue) {
  bool is_empty = queue->next == queue;
  if (is_empty) {
    PG_ASSERT(queue->prev == queue);
  }

  return is_empty;
}

[[maybe_unused]]
static void pg_queue_insert_tail(PgQueue *queue, PgQueue *elem) {
  PG_ASSERT(queue);
  PG_ASSERT(elem);
  PG_ASSERT(queue->next);
  PG_ASSERT(queue->prev);

  elem->next = queue;
  elem->prev = queue->prev;
  queue->prev->next = elem;
  queue->prev = elem;

  PG_ASSERT(queue->next);
  PG_ASSERT(queue->prev);
  PG_ASSERT(elem->next);
  PG_ASSERT(elem->prev);
}

[[maybe_unused]]
static void pg_queue_remove(PgQueue *elem) {
  PG_ASSERT(elem);
  PG_ASSERT(elem->next);
  PG_ASSERT(elem->prev);

  elem->prev->next = elem->next;
  elem->next->prev = elem->prev;
}

// Non-owning.
typedef struct PgHeapNode {
  struct PgHeapNode *left, *right, *parent;
} PgHeapNode;

typedef bool (*PgHeapLessThanFn)(PgHeapNode *a, PgHeapNode *b);

// Non-owning.
typedef struct {
  PgHeapNode *root;
  u64 count;
} PgHeap;

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

typedef struct {
  PgString left, right;
  bool ok;
} PgStringCut;

[[maybe_unused]] [[nodiscard]] static PgStringCut
pg_string_cut_byte(PgString s, u8 needle) {
  PgStringCut res = {0};

  i64 idx = pg_string_indexof_byte(s, needle);
  if (-1 == idx) {
    return res;
  }

  res.left = PG_SLICE_RANGE(s, 0, (u64)idx);
  res.right = PG_SLICE_RANGE_START(s, (u64)idx + 1);
  res.ok = true;

  return res;
}

[[nodiscard]] static i64 pg_string_last_indexof_byte(PgString haystack,
                                                     u8 needle) {
  for (i64 i = (i64)haystack.len - 1; i >= 0; i--) {
    u8 c = PG_SLICE_AT(haystack, i);
    if (needle == c) {
      return i;
    }
  }
  return -1;
}

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

  if (needle.len == haystack.len) {
    return pg_string_eq(haystack, needle) ? 0 : -1;
  }

  PG_ASSERT(nullptr != haystack.data);
  PG_ASSERT(nullptr != needle.data);
  u64 j = 0;
  u8 needle_first = PG_SLICE_AT(needle, 0);

  for (u64 _i = 0; _i < haystack.len - needle.len; _i++) {
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

[[maybe_unused]] [[nodiscard]] static PgStringCut
pg_string_cut_string(PgString s, PgString needle) {
  PgStringCut res = {0};

  i64 idx = pg_string_indexof_string(s, needle);
  if (-1 == idx) {
    return res;
  }

  res.left = PG_SLICE_RANGE(s, 0, (u64)idx);
  res.right = PG_SLICE_RANGE_START(s, (u64)idx + needle.len);
  res.ok = true;

  return res;
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

[[maybe_unused]] [[nodiscard]] static bool pg_string_contains(PgString haystack,
                                                              PgString needle) {
  return -1 != pg_string_indexof_string(haystack, needle);
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

  // For stats
  u8 *start_original;
  // For releasing the arena.
  u8 *os_start;
  u64 os_alloc_size;
} PgArena;

[[maybe_unused]] [[nodiscard]] static u64 pg_arena_mem_use(PgArena arena) {
  PG_ASSERT(arena.start >= arena.start_original);
  PG_ASSERT(arena.end >= arena.start_original);

  u64 res = (u64)(arena.start - arena.start_original);

  u64 original_usable_alloc_size = (u64)(arena.end - arena.start_original);
  PG_ASSERT(res <= original_usable_alloc_size);

  return res;
}

[[maybe_unused]] [[nodiscard]] static u64
pg_arena_mem_available(PgArena arena) {
  PG_ASSERT(arena.end >= arena.start);
  PG_ASSERT(arena.end >= arena.start_original);

  u64 res = (u64)(arena.end - arena.start);

  u64 original_usable_alloc_size = (u64)(arena.end - arena.start_original);
  PG_ASSERT(res <= original_usable_alloc_size);

  return res;
}

__attribute((malloc, alloc_size(2, 4), alloc_align(3)))
[[maybe_unused]] [[nodiscard]] static void *
pg_try_arena_alloc(PgArena *a, u64 size, u64 align, u64 count) {
  PG_ASSERT(a->start != nullptr);

  const u64 padding = (-(u64)a->start & (align - 1));
  PG_ASSERT(padding <= align);

  const i64 available = (i64)a->end - (i64)a->start - (i64)padding;
  PG_ASSERT(available >= 0);
  if (count > (u64)available / size) {
    return nullptr;
  }

  void *res = a->start + padding;
  PG_ASSERT(res != nullptr);
  PG_ASSERT(res <= (void *)a->end);

  a->start += padding + count * size;
  PG_ASSERT(a->start <= a->end);
  PG_ASSERT((u64)a->start % align == 0); // Aligned.

  return memset(res, 0, count * size);
}

__attribute((malloc, alloc_size(4, 6), alloc_align(5)))
[[maybe_unused]] [[nodiscard]] static void *
pg_try_arena_realloc(PgArena *a, void *ptr, u64 elem_count_old, u64 size,
                     u64 align, u64 count) {
  PG_ASSERT((u64)a->start >= (u64)ptr);

  u64 array_end = (u64)ptr + elem_count_old * size;

  if ((u64)a->start == array_end) { // Optimization.
    // This is the case of growing the array which is at the end of the arena.
    // In that case we can simply bump the arena pointer and avoid any copies.
    a->start += size * (count - elem_count_old);
    return ptr;
  }

  return pg_try_arena_alloc(a, size, align, count);
}

__attribute((malloc, alloc_size(2, 4), alloc_align(3)))
[[maybe_unused]] [[nodiscard]] static void *
pg_arena_alloc(PgArena *a, u64 size, u64 align, u64 count) {
  void *res = pg_try_arena_alloc(a, size, align, count);
  PG_ASSERT(res);
  return res;
}

#define pg_arena_new(a, t, n) (t *)pg_arena_alloc(a, sizeof(t), _Alignof(t), n)

#define pg_try_arena_new(a, t, n)                                              \
  ((t *)pg_try_arena_alloc((a), sizeof(t), _Alignof(t), (n)))

typedef struct PgAllocator PgAllocator;

typedef void *(*PgAllocFn)(PgAllocator *allocator, u64 sizeof_type,
                           u64 alignof_type, u64 elem_count);
typedef void *(*PgReallocFn)(PgAllocator *allocator, void *ptr,
                             u64 elem_count_old, u64 sizeof_type,
                             u64 alignof_type, u64 elem_count);
typedef void (*PgFreeFn)(PgAllocator *allocator, void *ptr);
struct PgAllocator {
  PgAllocFn alloc_fn;
  PgReallocFn realloc_fn;
  PgFreeFn free_fn;
};

[[nodiscard]]
static void *pg_alloc_heap_libc(PgAllocator *allocator, u64 sizeof_type,
                                u64 alignof_type, u64 elem_count) {
  (void)allocator;
  (void)alignof_type;
  return calloc(sizeof_type, elem_count);
}

[[nodiscard]]
static void *pg_realloc_heap_libc(PgAllocator *allocator, void *ptr,
                                  u64 elem_count_old, u64 sizeof_type,
                                  u64 alignof_type, u64 elem_count) {
  (void)allocator;
  (void)elem_count_old;
  (void)alignof_type;
  return realloc(ptr, sizeof_type * elem_count);
}

static void pg_free_heap_libc(PgAllocator *allocator, void *ptr) {
  (void)allocator;
  free(ptr);
}

typedef struct {
  PgAllocFn alloc_fn;
  PgReallocFn realloc_fn;
  PgFreeFn free_fn;
} PgHeapAllocator;
static_assert(sizeof(PgHeapAllocator) == sizeof(PgAllocator));

[[maybe_unused]] [[nodiscard]] static PgHeapAllocator pg_make_heap_allocator() {
  return (PgHeapAllocator){.alloc_fn = pg_alloc_heap_libc,
                           .realloc_fn = pg_realloc_heap_libc,
                           .free_fn = pg_free_heap_libc};
}

[[maybe_unused]] [[nodiscard]] static PgAllocator *
pg_heap_allocator_as_allocator(PgHeapAllocator *allocator) {
  return (PgAllocator *)allocator;
}

typedef struct {
  PgAllocFn alloc_fn;
  PgReallocFn realloc_fn;
  PgFreeFn free_fn;
  // Pprof.
  u64 alloc_objects_count, alloc_space, in_use_objects_count, in_use_space;
  PgFileDescriptor heap_profile_file;
} PgTracingAllocator;
static_assert(sizeof(PgTracingAllocator) >= sizeof(PgAllocator));

[[nodiscard]]
static void *pg_alloc_tracing(PgAllocator *allocator, u64 sizeof_type,
                              u64 alignof_type, u64 elem_count) {
  PgTracingAllocator *tracing_allocator = (PgTracingAllocator *)allocator;
  (void)tracing_allocator;

  // TODO: Better tracing e.g. with pprof.
  fprintf(stderr,
          "allocation sizeof_type=%" PRIu64 " alignof_type=%" PRIu64
          " elem_count=%" PRIu64 "\n",
          sizeof_type, alignof_type, elem_count);

  // TODO: Be aware of `align` here?
  u64 space = sizeof_type * elem_count;

  tracing_allocator->alloc_objects_count += elem_count;
  tracing_allocator->alloc_space += space;
  tracing_allocator->in_use_objects_count += elem_count;
  tracing_allocator->in_use_space += space;

  fprintf(
      stderr,
      "%" PRIu64 ": %" PRIu64 " [%" PRIu64 ": %" PRIu64 "] @ TODO call stack\n",
      tracing_allocator->in_use_objects_count, tracing_allocator->in_use_space,
      tracing_allocator->alloc_objects_count, tracing_allocator->alloc_space);

  return calloc(sizeof_type, elem_count);
}

[[nodiscard]]
static void *pg_realloc_tracing(PgAllocator *allocator, void *ptr,
                                u64 elem_count_old, u64 sizeof_type,
                                u64 alignof_type, u64 elem_count) {
  PgTracingAllocator *tracing_allocator = (PgTracingAllocator *)allocator;
  (void)tracing_allocator;
  PG_ASSERT(elem_count_old <= elem_count);

  // TODO: Better tracing e.g. with pprof.
  fprintf(stderr,
          "allocation sizeof_type=%" PRIu64 " alignof_type=%" PRIu64
          " elem_count=%" PRIu64 "\n",
          sizeof_type, alignof_type, elem_count);

  // TODO: Be aware of `align` here?
  u64 space = sizeof_type * (elem_count - elem_count_old);

  tracing_allocator->alloc_objects_count += (elem_count - elem_count_old);
  tracing_allocator->alloc_space += space;
  tracing_allocator->in_use_objects_count += elem_count - elem_count_old;
  tracing_allocator->in_use_space += space;

  fprintf(
      stderr,
      "%" PRIu64 ": %" PRIu64 " [%" PRIu64 ": %" PRIu64 "] @ TODO call stack\n",
      tracing_allocator->in_use_objects_count, tracing_allocator->in_use_space,
      tracing_allocator->alloc_objects_count, tracing_allocator->alloc_space);

  return realloc(ptr, space);
}

static void pg_free_tracing(PgAllocator *allocator, void *ptr) {
  PgTracingAllocator *tracing_allocator = (PgTracingAllocator *)allocator;
  (void)tracing_allocator;
  // FIXME
  u64 sizeof_type = 0;
  u64 elem_count = 0;

  fprintf(stderr,
          "free ptr=%p sizeof_type=%" PRIu64 " elem_count=%" PRIu64 "\n", ptr,
          sizeof_type, elem_count);

  // TODO: Be aware of `align` here?
  u64 space = sizeof_type * elem_count;

  tracing_allocator->in_use_objects_count -= elem_count;
  tracing_allocator->in_use_space -= space;

  fprintf(
      stderr,
      "%" PRIu64 ": %" PRIu64 " [%" PRIu64 ": %" PRIu64 "] @ TODO call stack\n",
      tracing_allocator->in_use_objects_count, tracing_allocator->in_use_space,
      tracing_allocator->alloc_objects_count, tracing_allocator->alloc_space);

  free(ptr);
}

[[maybe_unused]] [[nodiscard]] static PgTracingAllocator
pg_make_tracing_allocator(PgFileDescriptor heap_profile_file) {
  return (PgTracingAllocator){
      .alloc_fn = pg_alloc_tracing,
      .realloc_fn = pg_realloc_tracing,
      .free_fn = pg_free_tracing,
      .heap_profile_file = heap_profile_file,
  };
}

[[maybe_unused]] [[nodiscard]] static PgAllocator *
pg_tracing_allocator_as_allocator(PgTracingAllocator *allocator) {
  return (PgAllocator *)allocator;
}

[[maybe_unused]] [[nodiscard]] static void *pg_alloc(PgAllocator *allocator,
                                                     u64 sizeof_type,
                                                     u64 alignof_type,
                                                     u64 elem_count) {
  PG_ASSERT(allocator);
  PG_ASSERT(allocator->alloc_fn);
  return allocator->alloc_fn(allocator, sizeof_type, alignof_type, elem_count);
}

[[maybe_unused]] [[nodiscard]] static void *
pg_realloc(PgAllocator *allocator, void *ptr, u64 elem_count_old,
           u64 sizeof_type, u64 alignof_type, u64 elem_count) {
  PG_ASSERT(allocator);
  PG_ASSERT(allocator->realloc_fn);
  PG_ASSERT(ptr);
  return allocator->realloc_fn(allocator, ptr, elem_count_old, sizeof_type,
                               alignof_type, elem_count);
}

[[maybe_unused]] static void pg_free(PgAllocator *allocator, void *ptr) {
  PG_ASSERT(allocator);
  PG_ASSERT(allocator->alloc_fn);

  if (ptr) {
    allocator->free_fn(allocator, ptr);
  }
}

typedef struct {
  PgAllocFn alloc_fn;
  PgReallocFn realloc_fn;
  PgFreeFn free_fn;
  PgArena *arena;
} PgArenaAllocator;

static_assert(sizeof(PgArenaAllocator) >= sizeof(PgAllocator));

[[maybe_unused]] [[nodiscard]]
static void *pg_alloc_arena(PgAllocator *allocator, u64 sizeof_type,
                            u64 alignof_type, u64 elem_count) {
  PgArenaAllocator *arena_allocator = (PgArenaAllocator *)allocator;
  PgArena *arena = arena_allocator->arena;
  return pg_try_arena_alloc(arena, sizeof_type, alignof_type, elem_count);
}

[[maybe_unused]] [[nodiscard]]
static void *pg_realloc_arena(PgAllocator *allocator, void *ptr,
                              u64 elem_count_old, u64 sizeof_type,
                              u64 alignof_type, u64 elem_count) {
  PgArenaAllocator *arena_allocator = (PgArenaAllocator *)allocator;
  PgArena *arena = arena_allocator->arena;
  (void)ptr;
  return pg_try_arena_realloc(arena, ptr, elem_count_old, sizeof_type,
                              alignof_type, elem_count);
}

[[maybe_unused]]
static void pg_free_arena(PgAllocator *allocator, void *ptr) {
  (void)allocator;
  (void)ptr;

  // TODO: Free if ptr is the last allocation.
}

[[maybe_unused]] [[nodiscard]] static PgArenaAllocator
pg_make_arena_allocator(PgArena *arena) {
  return (PgArenaAllocator){
      .alloc_fn = pg_alloc_arena,
      .realloc_fn = pg_realloc_arena,
      .arena = arena,
      .free_fn = pg_free_arena,
  };
}

[[maybe_unused]] [[nodiscard]] static PgAllocator *
pg_arena_allocator_as_allocator(PgArenaAllocator *allocator) {
  return (PgAllocator *)allocator;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_make(u64 len, PgAllocator *allocator) {
  PgString res = {0};
  res.len = len;
  res.data = pg_alloc(allocator, sizeof(u8), _Alignof(u8), len);
  PG_ASSERT(res.data);
  return res;
}

[[maybe_unused]] [[nodiscard]] static char *
pg_string_to_cstr(PgString s, PgAllocator *allocator) {
  char *res = (char *)pg_alloc(allocator, sizeof(u8), 1, s.len + 1);
  PG_ASSERT(res);
  memcpy(res, s.data, s.len);

  PG_ASSERT(0 == PG_C_ARRAY_AT(res, s.len + 1, s.len));

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool pg_cstr_mut_from_string(char *str_c,
                                                                   PgString s) {
  PG_ASSERT(str_c);

  if (s.len >= PG_PATH_MAX) {
    return false;
  }
  memcpy(str_c, s.data, s.len);

  PG_ASSERT(0 == PG_C_ARRAY_AT(str_c, s.len + 1, s.len));

  return true;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_cstr_to_string(char *s) {
  return (PgString){
      .data = (u8 *)s,
      .len = strlen(s),
  };
}

typedef enum {
  PG_STRING_CMP_LESS = -1,
  PG_STRING_CMP_EQ = 0,
  PG_STRING_CMP_GREATER = 1,
} PgStringCompare;

[[maybe_unused]] [[nodiscard]] static PgStringCompare
pg_string_cmp(PgString a, PgString b) {
  int cmp = memcmp(a.data, b.data, PG_MIN(a.len, b.len));
  if (cmp < 0) {
    return PG_STRING_CMP_LESS;
  } else if (cmp > 0) {
    return PG_STRING_CMP_GREATER;
  } else if (a.len == b.len) {
    return PG_STRING_CMP_EQ;
  }

  PG_ASSERT(0 == cmp);
  PG_ASSERT(a.len != b.len);

  if (a.len < b.len) {
    return PG_STRING_CMP_LESS;
  }
  if (a.len > b.len) {
    return PG_STRING_CMP_GREATER;
  }
  PG_ASSERT(0);
}

[[maybe_unused]] static void PG_DYN_GROW(void *slice, u64 size, u64 align,
                                         u64 count, PgAllocator *allocator) {
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
  // PG_ASSERT(array_end < (u64)a->end);

  if (nullptr ==
      PgReplica.data) { // First allocation ever for this dynamic array.
    PgReplica.data = pg_alloc(allocator, size, align, new_cap);
    PG_ASSERT(PgReplica.data);
  } else { // General case.
    PgReplica.data = pg_realloc(allocator, PgReplica.data, PgReplica.cap, size,
                                align, new_cap);
    PG_ASSERT(PgReplica.data);
  }
  PgReplica.cap = new_cap;

  PG_ASSERT(nullptr != slice);
  memcpy(slice, &PgReplica, sizeof(PgReplica));
}

#define PG_DYN_ENSURE_CAP(dyn, new_cap, allocator)                             \
  ((dyn)->cap < (new_cap))                                                     \
      ? PG_DYN_GROW(dyn, sizeof(*(dyn)->data), _Alignof((dyn)->data[0]),       \
                    new_cap, allocator),                                       \
      PG_ASSERT((dyn)->cap >= (new_cap)), PG_ASSERT((dyn)->data), 0 : 0,       \
      PG_ASSERT((dyn)->data), 0

#define PG_DYN_SPACE(T, dyn)                                                   \
  ((T){.data = (dyn)->data + (dyn)->len, .len = (dyn)->cap - (dyn)->len})

PG_DYN(PgString) PgStringDyn;
PG_RESULT(PgStringDyn) PgStringDynResult;

#define PG_DYN_PUSH(s, allocator)                                              \
  (PG_DYN_ENSURE_CAP(s, (s)->len + 1, allocator),                              \
   (s)->len > 0 ? PG_ASSERT((s)->data) : 0, (s)->data + (s)->len++)

#define PG_DYN_PUSH_WITHIN_CAPACITY(s)                                         \
  (PG_ASSERT(((s)->len < (s)->cap)), (s)->len > 0 ? PG_ASSERT((s)->data) : 0,  \
   ((s)->data + (s)->len++))

#define PG_DYN_POP(s)                                                          \
  do {                                                                         \
    PG_ASSERT((s)->len > 0);                                                   \
    PG_ASSERT((s)->data);                                                      \
    memset(PG_SLICE_LAST_PTR(s), 0, sizeof((s)->data[(s)->len - 1]));          \
    (s)->len -= 1;                                                             \
  } while (0)

#define PG_SLICE_LAST_PTR(s)                                                   \
  PG_C_ARRAY_AT_PTR((s)->data, (s)->len, (s)->len - 1)

#define PG_SLICE_LAST(s) PG_C_ARRAY_AT((s).data, (s).len, (s).len - 1)

#define PG_DYN_APPEND_SLICE(dst, src, allocator)                               \
  do {                                                                         \
    PG_DYN_ENSURE_CAP(dst, (dst)->len + (src).len, (allocator));               \
    for (u64 _iii = 0; _iii < src.len; _iii++) {                               \
      *PG_DYN_PUSH(dst, allocator) = PG_SLICE_AT(src, _iii);                   \
    }                                                                          \
  } while (0)

#define PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(dst, src)                          \
  do {                                                                         \
    for (u64 _iii = 0; _iii < src.len; _iii++) {                               \
      *PG_DYN_PUSH_WITHIN_CAPACITY(dst) = PG_SLICE_AT(src, _iii);              \
    }                                                                          \
  } while (0)

#define PG_DYN_SLICE(T, dyn) ((T){.data = dyn.data, .len = dyn.len})

typedef PgU64Result (*ReadFn)(void *self, u8 *buf, size_t buf_len);
typedef PgU64Result (*WriteFn)(void *self, u8 *buf, size_t buf_len);

[[maybe_unused]] [[nodiscard]] static Pgu8Dyn
pg_sb_make_with_cap(u64 cap, PgAllocator *allocator) {
  Pgu8Dyn res = {0};
  PG_DYN_ENSURE_CAP(&res, cap, allocator);
  PG_ASSERT(res.data);
  return res;
}

typedef struct {
  void *ctx;
  ReadFn read_fn;
} PgReader;

typedef struct {
  PgFileDescriptor file;
  WriteFn write_fn;
  PgFileDescriptor ctx;
  // Only useful for writing to a string builder (aka `Pgu8Dyn`).
  PgAllocator *allocator; // TODO: Should it be instead in DYN_ structs?
} PgWriter;

typedef struct {
  u64 idx_read, idx_write;
  PgString data;
} PgRing;

[[maybe_unused]] static PgRing pg_ring_make(u64 cap, PgAllocator *allocator) {
  return (PgRing){.data = pg_string_make(cap + 1, allocator)};
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

[[maybe_unused]] [[nodiscard]] static bool
pg_ring_read_ptr(PgRing *rg, u8 *data, u64 data_len) {
  PgString s = {.data = data, .len = data_len};
  return pg_ring_read_slice(rg, s);
}
[[maybe_unused]] [[nodiscard]] static bool
pg_ring_write_ptr(PgRing *rg, u8 *data, u64 data_len) {
  PgString s = {.data = data, .len = data_len};
  return pg_ring_write_slice(rg, s);
}

#define pg_ring_read_struct(ring, val)                                         \
  (pg_ring_read_space(*(ring)) < (sizeof(*(val)))                              \
       ? false                                                                 \
       : (pg_ring_read_ptr(ring, (u8 *)val, sizeof(*val))))

#define pg_ring_write_struct(ring, val)                                        \
  (pg_ring_write_space(*(ring)) < sizeof((val))                                \
       ? false                                                                 \
       : (pg_ring_write_ptr(ring, (u8 *)&val, sizeof(val))))

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_ring_read_until_excl(PgRing *rg, PgString needle, PgAllocator *allocator) {
  PgStringOk res = {0};
  i64 idx = -1;

  {
    PgRing cpy_rg = *rg;

    PgString dst = pg_string_make(pg_ring_read_space(*rg), allocator);
    PG_ASSERT(pg_ring_read_slice(rg, dst));
    *rg = cpy_rg; // Reset.
    pg_free(allocator, dst.data);

    idx = pg_string_indexof_string(dst, needle);
    if (-1 == idx) {
      return res;
    }
  }

  res.ok = true;
  res.res = pg_string_make((u64)idx, allocator);
  PG_ASSERT(pg_ring_read_slice(rg, res.res));

  // Read and throw away the needle.
  {
    PgString dst_needle = pg_string_make(needle.len, allocator);
    PG_ASSERT(pg_ring_read_slice(rg, dst_needle));
    PG_ASSERT(pg_string_eq(needle, dst_needle));
    pg_free(allocator, dst_needle.data);
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_read_u8(PgRing *rg,
                                                           u8 *val) {
  PgString s = {.len = sizeof(*val), .data = val};
  return pg_ring_read_slice(rg, s);
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_read_u32(PgRing *rg,
                                                            u32 *val) {
  PgString s = {.len = sizeof(*val), .data = (u8 *)val};
  return pg_ring_read_slice(rg, s);
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_writer_string_builder_write(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgWriter *w = self;
  Pgu8Dyn *sb = w->ctx.ptr;

  PgString s = {.data = buf, .len = buf_len};
  PG_DYN_APPEND_SLICE(sb, s, w->allocator);

  return (PgU64Result){.res = buf_len};
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_reader_ring_read(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgReader *r = self;
  PgRing *ring = r->ctx;

  PgString s = {.data = buf, .len = PG_MIN(buf_len, pg_ring_read_space(*ring))};
  PG_ASSERT(true == pg_ring_read_slice(ring, s));

  return (PgU64Result){.res = s.len};
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_writer_ring_write(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgWriter *w = self;
  PgRing *ring = w->ctx.ptr;

  PgString s = {.data = buf,
                .len = PG_MIN(buf_len, pg_ring_write_space(*ring))};
  PG_ASSERT(true == pg_ring_write_slice(ring, s));

  return (PgU64Result){.res = s.len};
}

[[nodiscard]] [[maybe_unused]] static PgWriter
pg_writer_make_from_string_builder(Pgu8Dyn *sb, PgAllocator *allocator) {
  PgWriter w = {0};
  w.ctx.ptr = sb;
  w.allocator = allocator;
  w.write_fn = pg_writer_string_builder_write;
  return w;
}

[[nodiscard]] [[maybe_unused]] static PgWriter
pg_writer_make_from_ring(PgRing *ring) {
  PgWriter w = {0};
  w.ctx.ptr = ring;
  w.write_fn = pg_writer_ring_write;
  return w;
}

[[maybe_unused]] [[nodiscard]] static PgError pg_writer_write_u8(PgWriter *w,
                                                                 u8 c) {
  PG_ASSERT(nullptr != w->write_fn);

  PgU64Result res = w->write_fn(w, &c, 1);
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

    PgU64Result res = w->write_fn(w, remaining.data, remaining.len);
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

[[nodiscard]] [[maybe_unused]] static PgU64Result
pg_writer_write_from_reader(PgWriter *w, PgReader *r) {
  PgU64Result res = {0};

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
  u64 idx = PG_STATIC_ARRAY_LEN(tmp);

  while (n > 0) {
    idx -= 1;
    PG_C_ARRAY_AT(tmp, PG_STATIC_ARRAY_LEN(tmp), idx) = '0' + (n % 10);
    n /= 10;
  }

  PG_ASSERT(idx <= PG_STATIC_ARRAY_LEN(tmp));

  PgString s = {.data = tmp + idx, .len = PG_STATIC_ARRAY_LEN(tmp) - idx};

  return pg_writer_write_all_string(w, s);
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_writer_write_i64_as_string(PgWriter *w, i64 n) {
  u8 tmp[30] = {0};
  u64 idx = PG_STATIC_ARRAY_LEN(tmp);

  u64 val = n < 0 ? (u64)-n : (u64)n;
  while (val > 0) {
    idx -= 1;
    PG_C_ARRAY_AT(tmp, PG_STATIC_ARRAY_LEN(tmp), idx) = '0' + (val % 10);
    val /= 10;
  }

  if (n < 0) {
    idx -= 1;
    PG_C_ARRAY_AT(tmp, PG_STATIC_ARRAY_LEN(tmp), idx) = '-';
  }

  PG_ASSERT(idx <= PG_STATIC_ARRAY_LEN(tmp));

  PgString s = {.data = tmp + idx, .len = PG_STATIC_ARRAY_LEN(tmp) - idx};

  return pg_writer_write_all_string(w, s);
}

[[maybe_unused]] static void pg_u32_to_u8x4_be(u32 n, PgString *dst) {
  PG_ASSERT(sizeof(n) == dst->len);

  *(PG_SLICE_AT_PTR(dst, 0)) = (u8)(n >> 24);
  *(PG_SLICE_AT_PTR(dst, 1)) = (u8)(n >> 16);
  *(PG_SLICE_AT_PTR(dst, 2)) = (u8)(n >> 8);
  *(PG_SLICE_AT_PTR(dst, 3)) = (u8)(n >> 0);
}

[[maybe_unused]] static void
pg_string_builder_append_u32(Pgu8Dyn *dyn, u32 n, PgAllocator *allocator) {

  u8 data[sizeof(n)] = {0};
  PgString s = {.data = data, .len = sizeof(n)};
  pg_u32_to_u8x4_be(n, &s);
  PG_DYN_APPEND_SLICE(dyn, s, allocator);
}

[[maybe_unused]] static void
pg_string_builder_append_u32_within_capacity(Pgu8Dyn *dyn, u32 n) {

  u8 data[sizeof(n)] = {0};
  PgString s = {.data = data, .len = sizeof(n)};
  pg_u32_to_u8x4_be(n, &s);
  PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(dyn, s);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_u64_to_string(u64 n, PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 25, allocator);
  PgWriter w = pg_writer_make_from_string_builder(&sb, allocator);

  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, n));

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static u8 pg_u8_to_character_hex(u8 n) {
  PG_ASSERT(n < 16);
  const u8 lut[] = "0123456789abcdef";
  return lut[n];
}

[[maybe_unused]] [[nodiscard]] static u8 pg_u8_to_character_hex_upper(u8 n) {
  PG_ASSERT(n < 16);
  const u8 lut[] = "0123456789ABCDEF";
  return lut[n];
}

[[maybe_unused]] [[nodiscard]]
static PgError pg_writer_write_u8_hex_upper(PgWriter *w, u8 n) {

  u8 c1 = n & 15; // i.e. `% 16`.
  u8 c2 = n >> 4; // i.e. `/ 16`

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

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_dup(PgString src, PgAllocator *allocator) {
  PgString dst = pg_string_make(src.len, allocator);
  memcpy(dst.data, src.data, src.len);

  return dst;
}

[[maybe_unused]] [[nodiscard]] static u64
pg_round_up_multiple_of(u64 n, u64 multiple) {
  PG_ASSERT(0 != multiple);
  if (0 == n % multiple) {
    return n;
  }

  u64 factor = n / multiple;
  u64 res = (factor + 1) * multiple;
  PG_ASSERT(0 == res % multiple);

  return res;
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

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_clone(PgString s, PgAllocator *allocator) {
  PgString res = pg_string_make(s.len, allocator);
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

  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgString a_clone = pg_string_clone(a, (PgAllocator *)&arena_allocator);
  PgString b_clone = pg_string_clone(b, (PgAllocator *)&arena_allocator);

  pg_string_lowercase_ascii_mut(a_clone);
  pg_string_lowercase_ascii_mut(b_clone);

  return pg_string_eq(a_clone, b_clone);
}

typedef struct {
  u8 data[PG_SHA1_DIGEST_LENGTH];
} PgSha1;

#if defined(__x86_64__) && defined(__SSSE3__) && defined(__SHA__)
#include <immintrin.h>
// Process as many 64 bytes chunks as possible.
static void pg_sha1_process_x86(uint32_t state[5], const uint8_t data[],
                                uint32_t length) {
  __m128i ABCD, ABCD_SAVE, E0, E0_SAVE, E1;
  __m128i MSG0, MSG1, MSG2, MSG3;
  const __m128i MASK =
      _mm_set_epi64x(0x0001020304050607ULL, 0x08090a0b0c0d0e0fULL);

  /* Load initial values */
  ABCD = _mm_loadu_si128((const __m128i *)(void *)state);
  E0 = _mm_set_epi32((int)state[4], 0, 0, 0);
  ABCD = _mm_shuffle_epi32(ABCD, 0x1B);

  while (length >= 64) {
    /* Save current state  */
    ABCD_SAVE = ABCD;
    E0_SAVE = E0;

    /* Rounds 0-3 */
    MSG0 = _mm_loadu_si128((const __m128i *)(void *)(data + 0));
    MSG0 = _mm_shuffle_epi8(MSG0, MASK);
    E0 = _mm_add_epi32(E0, MSG0);
    E1 = ABCD;
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 0);

    /* Rounds 4-7 */
    MSG1 = _mm_loadu_si128((const __m128i *)(void *)(data + 16));
    MSG1 = _mm_shuffle_epi8(MSG1, MASK);
    E1 = _mm_sha1nexte_epu32(E1, MSG1);
    E0 = ABCD;
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 0);
    MSG0 = _mm_sha1msg1_epu32(MSG0, MSG1);

    /* Rounds 8-11 */
    MSG2 = _mm_loadu_si128((const __m128i *)(void *)(data + 32));
    MSG2 = _mm_shuffle_epi8(MSG2, MASK);
    E0 = _mm_sha1nexte_epu32(E0, MSG2);
    E1 = ABCD;
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 0);
    MSG1 = _mm_sha1msg1_epu32(MSG1, MSG2);
    MSG0 = _mm_xor_si128(MSG0, MSG2);

    /* Rounds 12-15 */
    MSG3 = _mm_loadu_si128((const __m128i *)(void *)(data + 48));
    MSG3 = _mm_shuffle_epi8(MSG3, MASK);
    E1 = _mm_sha1nexte_epu32(E1, MSG3);
    E0 = ABCD;
    MSG0 = _mm_sha1msg2_epu32(MSG0, MSG3);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 0);
    MSG2 = _mm_sha1msg1_epu32(MSG2, MSG3);
    MSG1 = _mm_xor_si128(MSG1, MSG3);

    /* Rounds 16-19 */
    E0 = _mm_sha1nexte_epu32(E0, MSG0);
    E1 = ABCD;
    MSG1 = _mm_sha1msg2_epu32(MSG1, MSG0);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 0);
    MSG3 = _mm_sha1msg1_epu32(MSG3, MSG0);
    MSG2 = _mm_xor_si128(MSG2, MSG0);

    /* Rounds 20-23 */
    E1 = _mm_sha1nexte_epu32(E1, MSG1);
    E0 = ABCD;
    MSG2 = _mm_sha1msg2_epu32(MSG2, MSG1);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 1);
    MSG0 = _mm_sha1msg1_epu32(MSG0, MSG1);
    MSG3 = _mm_xor_si128(MSG3, MSG1);

    /* Rounds 24-27 */
    E0 = _mm_sha1nexte_epu32(E0, MSG2);
    E1 = ABCD;
    MSG3 = _mm_sha1msg2_epu32(MSG3, MSG2);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 1);
    MSG1 = _mm_sha1msg1_epu32(MSG1, MSG2);
    MSG0 = _mm_xor_si128(MSG0, MSG2);

    /* Rounds 28-31 */
    E1 = _mm_sha1nexte_epu32(E1, MSG3);
    E0 = ABCD;
    MSG0 = _mm_sha1msg2_epu32(MSG0, MSG3);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 1);
    MSG2 = _mm_sha1msg1_epu32(MSG2, MSG3);
    MSG1 = _mm_xor_si128(MSG1, MSG3);

    /* Rounds 32-35 */
    E0 = _mm_sha1nexte_epu32(E0, MSG0);
    E1 = ABCD;
    MSG1 = _mm_sha1msg2_epu32(MSG1, MSG0);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 1);
    MSG3 = _mm_sha1msg1_epu32(MSG3, MSG0);
    MSG2 = _mm_xor_si128(MSG2, MSG0);

    /* Rounds 36-39 */
    E1 = _mm_sha1nexte_epu32(E1, MSG1);
    E0 = ABCD;
    MSG2 = _mm_sha1msg2_epu32(MSG2, MSG1);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 1);
    MSG0 = _mm_sha1msg1_epu32(MSG0, MSG1);
    MSG3 = _mm_xor_si128(MSG3, MSG1);

    /* Rounds 40-43 */
    E0 = _mm_sha1nexte_epu32(E0, MSG2);
    E1 = ABCD;
    MSG3 = _mm_sha1msg2_epu32(MSG3, MSG2);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 2);
    MSG1 = _mm_sha1msg1_epu32(MSG1, MSG2);
    MSG0 = _mm_xor_si128(MSG0, MSG2);

    /* Rounds 44-47 */
    E1 = _mm_sha1nexte_epu32(E1, MSG3);
    E0 = ABCD;
    MSG0 = _mm_sha1msg2_epu32(MSG0, MSG3);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 2);
    MSG2 = _mm_sha1msg1_epu32(MSG2, MSG3);
    MSG1 = _mm_xor_si128(MSG1, MSG3);

    /* Rounds 48-51 */
    E0 = _mm_sha1nexte_epu32(E0, MSG0);
    E1 = ABCD;
    MSG1 = _mm_sha1msg2_epu32(MSG1, MSG0);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 2);
    MSG3 = _mm_sha1msg1_epu32(MSG3, MSG0);
    MSG2 = _mm_xor_si128(MSG2, MSG0);

    /* Rounds 52-55 */
    E1 = _mm_sha1nexte_epu32(E1, MSG1);
    E0 = ABCD;
    MSG2 = _mm_sha1msg2_epu32(MSG2, MSG1);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 2);
    MSG0 = _mm_sha1msg1_epu32(MSG0, MSG1);
    MSG3 = _mm_xor_si128(MSG3, MSG1);

    /* Rounds 56-59 */
    E0 = _mm_sha1nexte_epu32(E0, MSG2);
    E1 = ABCD;
    MSG3 = _mm_sha1msg2_epu32(MSG3, MSG2);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 2);
    MSG1 = _mm_sha1msg1_epu32(MSG1, MSG2);
    MSG0 = _mm_xor_si128(MSG0, MSG2);

    /* Rounds 60-63 */
    E1 = _mm_sha1nexte_epu32(E1, MSG3);
    E0 = ABCD;
    MSG0 = _mm_sha1msg2_epu32(MSG0, MSG3);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 3);
    MSG2 = _mm_sha1msg1_epu32(MSG2, MSG3);
    MSG1 = _mm_xor_si128(MSG1, MSG3);

    /* Rounds 64-67 */
    E0 = _mm_sha1nexte_epu32(E0, MSG0);
    E1 = ABCD;
    MSG1 = _mm_sha1msg2_epu32(MSG1, MSG0);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 3);
    MSG3 = _mm_sha1msg1_epu32(MSG3, MSG0);
    MSG2 = _mm_xor_si128(MSG2, MSG0);

    /* Rounds 68-71 */
    E1 = _mm_sha1nexte_epu32(E1, MSG1);
    E0 = ABCD;
    MSG2 = _mm_sha1msg2_epu32(MSG2, MSG1);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 3);
    MSG3 = _mm_xor_si128(MSG3, MSG1);

    /* Rounds 72-75 */
    E0 = _mm_sha1nexte_epu32(E0, MSG2);
    E1 = ABCD;
    MSG3 = _mm_sha1msg2_epu32(MSG3, MSG2);
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E0, 3);

    /* Rounds 76-79 */
    E1 = _mm_sha1nexte_epu32(E1, MSG3);
    E0 = ABCD;
    ABCD = (__m128i)_mm_sha1rnds4_epu32(ABCD, E1, 3);

    /* Combine state */
    E0 = _mm_sha1nexte_epu32(E0, E0_SAVE);
    ABCD = _mm_add_epi32(ABCD, ABCD_SAVE);

    data += 64;
    length -= 64;
  }

  /* Save state */
  ABCD = _mm_shuffle_epi32(ABCD, 0x1B);
  _mm_storeu_si128((__m128i *)(void *)state, ABCD);
  state[4] = (u32)_mm_extract_epi32(E0, 3);
}

[[maybe_unused]] static PgSha1 pg_sha1(PgString s) {
  PG_SHA1_CTX ctx = {0};
  PG_SHA1Init(&ctx);

  // Process as many 64 bytes chunks as possible.
  pg_sha1_process_x86(ctx.state, s.data, (u32)s.len);

  u64 len_rounded_down_64 = (s.len / 64) * 64;
  u64 rem = s.len % 64;
  memcpy(ctx.buffer, s.data + len_rounded_down_64, rem);

  ctx.count = s.len * 8;
  PgSha1 res = {0};
  PG_SHA1Final(res.data, &ctx);

  return res;
}

#else
[[maybe_unused]] static PgSha1 pg_sha1(PgString s) {
  PG_SHA1_CTX ctx = {0};
  PG_SHA1Init(&ctx);
  PG_SHA1Update(&ctx, s.data, s.len);
  PgSha1 res = {0};
  PG_SHA1Final(res.data, &ctx);
  return res;
}
#endif

typedef struct {
  u32 ip;   // Host order.
  u16 port; // Host order.
} PgIpv4Address;

PG_DYN(PgIpv4Address) PgIpv4AddressDyn;
PG_SLICE(PgIpv4Address) PgIpv4AddressSlice;

[[maybe_unused]] [[nodiscard]] static PgString
pg_net_ipv4_address_to_string(PgIpv4Address address, PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 3 * 4 + 4 + 5, allocator);
  PgWriter w = pg_writer_make_from_string_builder(&sb, allocator);

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

[[nodiscard]] static bool pg_bitfield_get_ptr(u8 *bitfield, u64 bitfield_len,
                                              u64 idx_bit) {
  PG_ASSERT(idx_bit < bitfield_len * 8);

  u64 idx_byte = idx_bit / 8;

  return PG_C_ARRAY_AT(bitfield, bitfield_len, idx_byte) & (1 << (idx_bit % 8));
}

[[maybe_unused]] [[nodiscard]] static bool pg_bitfield_get(PgString bitfield,
                                                           u64 idx_bit) {
  return pg_bitfield_get_ptr(bitfield.data, bitfield.len, idx_bit);
}

static void pg_bitfield_set_ptr(u8 *bitfield, u64 bitfield_len, u64 idx_bit,
                                bool val) {
  PG_ASSERT(idx_bit < bitfield_len * 8);

  u64 idx_byte = idx_bit / 8;

  u8 *ptr = PG_C_ARRAY_AT_PTR(bitfield, bitfield_len, idx_byte);
  if (val) {
    *ptr |= 1 << (idx_bit % 8);
  } else {
    *ptr &= ~(1 << (idx_bit % 8));
  }

  PG_ASSERT(val == pg_bitfield_get_ptr(bitfield, bitfield_len, idx_bit));
}

[[maybe_unused]] static void pg_bitfield_set(PgString bitfield, u64 idx_bit,
                                             bool val) {
  pg_bitfield_set_ptr(bitfield.data, bitfield.len, idx_bit, val);
}

[[maybe_unused]] [[nodiscard]] static u64 pg_bitfield_count(PgString bitfield) {
  u64 res = 0;
  for (u64 i = 0; i < bitfield.len; i++) {
    u8 c = PG_SLICE_AT(bitfield, i);
    res += (u8)__builtin_popcount((u8)c);
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Ok
pg_bitfield_get_first_zero(PgString bitfield) {
  Pgu64Ok res = {0};

  for (u64 i = 0; i < bitfield.len; i++) {
    u8 c = PG_SLICE_AT(bitfield, i);
    if (0xff == c) {
      continue;
    }

    u64 bit_idx = 0;
    u8 bit_pattern = 1;
    // TODO: Check correctness.
    for (u64 j = 0; j < 8; j++) {
      if (0 == (c & bit_pattern)) {
        bit_idx = j;
        break;
      }
      bit_pattern <<= 1;
    }
    PG_ASSERT(bit_idx < 8);
    PG_ASSERT(bit_idx > 0);

    res.res = i * 8 + (bit_idx - 1);
    res.ok = true;
    return res;
  }
  return res;
}

typedef enum {
  PG_CLOCK_KIND_MONOTONIC,
  PG_CLOCK_KIND_REALTIME,
  // TODO: More?
} PgClockKind;

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_time_ns_now(PgClockKind clock_kind);

typedef struct {
  u64 state;
} PgRng;

// From https://nullprogram.com/blog/2017/09/21/.
// PCG.
[[nodiscard]] [[maybe_unused]] static u32
pg_rand_u32_min_incl_max_incl(PgRng *rng, u32 min_incl, u32 max_incl) {
  PG_ASSERT(rng);
  PG_ASSERT(min_incl <= max_incl);

  uint64_t m = 0x9b60933458e17d7d;
  uint64_t a = 0xd737232eeccdf7ed;
  rng->state = rng->state * m + a;
  int shift = 29 - (rng->state >> 61);
  u32 rand = (u32)(rng->state >> shift);

  u32 res = min_incl + (u32)((float)rand * ((float)max_incl - (float)min_incl) /
                             (float)UINT32_MAX);
  PG_ASSERT(min_incl <= res);
  PG_ASSERT(res <= max_incl);
  return res;
}

[[nodiscard]] [[maybe_unused]] static u32
pg_rand_u32_min_incl_max_excl(PgRng *rng, u32 min_incl, u32 max_excl) {
  PG_ASSERT(max_excl > 0);
  return pg_rand_u32_min_incl_max_incl(rng, min_incl, max_excl - 1);
}

[[maybe_unused]] static void pg_rand_string_mut(PgRng *rng, PgString s) {
  for (u64 i = 0; i < s.len; i++) {
    *PG_C_ARRAY_AT_PTR(s.data, s.len, i) =
        (u8)pg_rand_u32_min_incl_max_incl(rng, 0, UINT8_MAX);
  }
}

[[maybe_unused]] [[nodiscard]] static Pgu32Ok
pg_bitfield_get_first_zero_rand(PgString bitfield, u32 len, PgRng *rng) {
  PG_ASSERT(len <= bitfield.len);

  Pgu32Ok res = {0};

  u32 start = pg_rand_u32_min_incl_max_excl(rng, 0, len);
  for (u64 i = 0; i < len; i++) {
    u32 idx = (start + i) % len;
    if (pg_bitfield_get(bitfield, idx)) {
      continue;
    }
    res.res = idx;
    res.ok = true;
    return res;
  }
  return res;
}

[[maybe_unused]] static void pg_rand_string_mut(PgRng *rng, PgString s);

[[nodiscard]] [[maybe_unused]] static PgRng pg_rand_make() {
  PgRng rng = {0};
  // Rely on ASLR.
  PgU64Result now = pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC);
  PG_ASSERT(0 == now.err);
  rng.state = (u64)(&pg_rand_make) ^ now.res;

  return rng;
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_writer_file_write(void *self, u8 *buf, size_t buf_len);

[[nodiscard]] static u64 pg_os_get_page_size();

typedef enum [[clang::flag_enum]] {
  PG_VIRTUAL_MEM_FLAGS_NONE = 0,
  PG_VIRTUAL_MEM_FLAGS_READ = 1,
  PG_VIRTUAL_MEM_FLAGS_WRITE = 2,
  PG_VIRTUAL_MEM_FLAGS_EXEC = 4,
} PgVirtualMemFlags;

[[nodiscard]] i32 pg_os_get_last_error();
[[nodiscard]] PgVoidPtrResult pg_virtual_mem_alloc(u64 size,
                                                   PgVirtualMemFlags flags);
[[nodiscard]] PgError pg_virtual_mem_protect(void *ptr, u64 size,
                                             PgVirtualMemFlags flags_new);
[[nodiscard]] PgError pg_virtual_mem_release(void *ptr, u64 size);

[[nodiscard]] i32 pg_virtual_mem_flags_to_os_flags(PgVirtualMemFlags flags);

[[maybe_unused]] [[nodiscard]] static PgArena
pg_arena_make_from_virtual_mem(u64 size) {
  PG_ASSERT(size > 0);

  u64 page_size = pg_os_get_page_size();
  u64 size_rounded_up_to_page_size = pg_round_up_multiple_of(size, page_size);
  PG_ASSERT(0 == size_rounded_up_to_page_size % page_size);

  u64 os_alloc_size = size_rounded_up_to_page_size;
  // Page guard before.
  PG_ASSERT(false == ckd_add(&os_alloc_size, os_alloc_size, page_size));
  // Page guard after.
  PG_ASSERT(false == ckd_add(&os_alloc_size, os_alloc_size, page_size));

  PgVoidPtrResult res_alloc = pg_virtual_mem_alloc(
      os_alloc_size, PG_VIRTUAL_MEM_FLAGS_READ | PG_VIRTUAL_MEM_FLAGS_WRITE);
  if (res_alloc.err) {
    // TODO: Better error handling.
    PG_ASSERT(0);
  }
  PG_ASSERT(res_alloc.res);
  u8 *alloc = res_alloc.res;

  u64 page_guard_before = (u64)alloc;

  PG_ASSERT(false == ckd_add((u64 *)&alloc, (u64)alloc, page_size));
  PG_ASSERT(page_guard_before + page_size == (u64)alloc);

  u64 page_guard_after = (u64)0;
  PG_ASSERT(false == ckd_add(&page_guard_after, (u64)alloc,
                             size_rounded_up_to_page_size));
  PG_ASSERT((u64)alloc + size_rounded_up_to_page_size == page_guard_after);
  PG_ASSERT(page_guard_before + page_size + size_rounded_up_to_page_size ==
            page_guard_after);

  PG_ASSERT(0 == pg_virtual_mem_protect((void *)page_guard_before, page_size,
                                        PG_VIRTUAL_MEM_FLAGS_NONE));

  PG_ASSERT(0 == pg_virtual_mem_protect((void *)page_guard_after, page_size,
                                        PG_VIRTUAL_MEM_FLAGS_NONE));

  // Trigger a page fault preemptively to detect invalid virtual memory
  // mappings.
  *(u8 *)alloc = 0;

  PG_ASSERT(os_alloc_size >= 2 * page_size);
  return (PgArena){
      .start_original = alloc,
      .start = alloc,
      .end = (u8 *)alloc + size,
      .os_start = (u8 *)page_guard_before,
      .os_alloc_size = os_alloc_size,
  };
}

[[maybe_unused]] [[nodiscard]] static PgError pg_arena_release(PgArena *arena) {
  if (nullptr == arena->start) {
    return 0;
  }

  PG_ASSERT(nullptr != arena->end);
  PG_ASSERT(nullptr != arena->os_start);

  return pg_virtual_mem_release(arena->os_start, arena->os_alloc_size);
}

#ifdef PG_OS_UNIX
#define PG_PATH_SEPARATOR '/'
#define PG_PATH_SEPARATOR_S "/"
#else
#define PG_PATH_SEPARATOR '\\'
#define PG_PATH_SEPARATOR_S "\\"
#endif

[[maybe_unused]] [[nodiscard]] static PgString pg_path_base_name(PgString s) {
  i64 idx = pg_string_last_indexof_byte(s, PG_PATH_SEPARATOR);
  if (-1 == idx) {
    return s;
  } else {
    return PG_SLICE_RANGE_START(s, (u64)idx + 1);
  }
}

[[maybe_unused]] [[nodiscard]] static PgString pg_path_stem(PgString s) {
  PgString base_name = pg_path_base_name(s);
  i64 idx = pg_string_last_indexof_byte(base_name, '.');
  if (-1 == idx) {
    return base_name;
  }

  return PG_SLICE_RANGE(base_name, 0, (u64)idx);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_concat(PgString left, PgString right, PgAllocator *allocator) {
  PgString res = pg_string_make(left.len + right.len, allocator);
  PG_ASSERT(res.data);

  memcpy(res.data, left.data, left.len);
  memcpy(res.data + left.len, right.data, right.len);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgReader
pg_reader_make_from_file(PgFileDescriptor file);

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_file_descriptor(PgFileDescriptor file) {
  PgWriter w = {
      .ctx = file,
      .write_fn = pg_writer_file_write,
  };

  return w;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdin();
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdout();
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stderr();

[[maybe_unused]] static PgU64Result pg_file_read_at(PgFileDescriptor file,
                                                    PgString buf, u64 offset);

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_file_open(PgString path, PgFileAccess access, bool create_if_not_exists,
             PgAllocator *allocator);

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_close(PgFileDescriptor file);

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_truncate(PgFileDescriptor file, u64 size);

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_file_size(PgFileDescriptor file);

[[nodiscard]] static PgU64Result pg_file_read(PgFileDescriptor file,
                                              PgString dst);
[[maybe_unused]] static PgU64Result pg_file_write(PgFileDescriptor file,
                                                  PgString s);

[[maybe_unused]] static PgStringResult
pg_file_read_full_from_descriptor(PgFileDescriptor file, u64 size,
                                  PgAllocator *allocator) {
  PgStringResult res = {0};

  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, size, allocator);

  for (u64 lim = 0; lim < size; lim++) {
    if (size == sb.len) {
      break;
    }

    PgString space = {.data = sb.data + sb.len, .len = sb.cap - sb.len};
    PgU64Result res_read = pg_file_read(file, space);
    if (res_read.err) {
      res.err = (PgError)pg_os_get_last_error();
      goto end;
    }

    u64 read_n = res_read.res;
    if (0 == read_n) {
      res.err = (PgError)PG_ERR_INVALID_VALUE;
      goto end;
    }

    PG_ASSERT((u64)read_n <= space.len);

    sb.len += (u64)read_n;
  }

end:
  if (res.err && sb.data) {
    pg_free(allocator, sb.data);
    return res;
  }

  res.res = PG_DYN_SLICE(PgString, sb);
  return res;
}

[[maybe_unused]] static PgError
pg_file_write_full_with_descriptor(PgFileDescriptor file, PgString content) {
  PgError err = 0;
  PgString remaining = content;
  for (u64 lim = 0; lim < content.len; lim++) {
    if (pg_string_is_empty(remaining)) {
      break;
    }

    PgU64Result res_write = pg_file_write(file, remaining);
    if (res_write.err) {
      err = (PgError)pg_os_get_last_error();
      goto end;
    }

    u64 write_n = res_write.res;
    if (0 == write_n) {
      err = (PgError)PG_ERR_INVALID_VALUE;
      goto end;
    }

    remaining = PG_SLICE_RANGE_START(remaining, write_n);
  }

end:
  return err;
}

[[maybe_unused]] static PgStringResult
pg_file_read_full_from_path(PgString path, PgAllocator *allocator) {
  PgStringResult res = {0};

  PgFileDescriptorResult res_file =
      pg_file_open(path, PG_FILE_ACCESS_READ, false, allocator);
  if (res_file.err) {
    res.err = res_file.err;
    return res;
  }

  PgFileDescriptor file = res_file.res;

  PgU64Result res_size = pg_file_size(file);
  if (res_size.err) {
    res.err = res_size.err;
    goto end;
  }

  u64 size = res_size.res;

  res = pg_file_read_full_from_descriptor(file, size, allocator);

end:
  (void)pg_file_close(file);

  return res;
}

[[maybe_unused]] static PgError
pg_file_write_full(PgString path, PgString content, PgAllocator *allocator) {
  PgError err = 0;

  PgFileDescriptorResult res_file =
      pg_file_open(path, PG_FILE_ACCESS_WRITE, true, allocator);
  if (res_file.err) {
    err = res_file.err;
    return err;
  }

  PgFileDescriptor file = res_file.res;

  err = pg_file_write_full_with_descriptor(file, content);

  (void)pg_file_close(file);

  return err;
}

typedef struct {
  i32 exit_status;
  i32 signal;
  bool exited, signaled, core_dumped, stopped;
  // Only if `spawn_options.stdout == PG_CHILD_PROCESS_STD_IO_PIPE`.
  PgString stdout_captured;
  // Only if `spawn_options.stderr == PG_CHILD_PROCESS_STD_IO_PIPE`.
  PgString stderr_captured;
} PgProcessStatus;
PG_RESULT(PgProcessStatus) PgProcessExitResult;

typedef struct {
  PgString stdout_captured, stderr_captured;
} PgProcessCaptureStd;
PG_RESULT(PgProcessCaptureStd) PgProcessCaptureStdResult;

typedef struct {
  u64 pid;
  // Only if `spawn_options.stdin == PG_CHILD_PROCESS_STD_IO_PIPE`.
  PgFileDescriptor stdin_pipe;
  // Only if `spawn_options.stdout == PG_CHILD_PROCESS_STD_IO_PIPE`.
  PgFileDescriptor stdout_pipe;
  // Only if `spawn_options.stderr == PG_CHILD_PROCESS_STD_IO_PIPE`.
  PgFileDescriptor stderr_pipe;
} PgProcess;
PG_RESULT(PgProcess) PgProcessResult;

typedef enum {
  PG_CHILD_PROCESS_STD_IO_INHERIT,
  // `/dev/null` on Unix, `NUL` on Windows.
  PG_CHILD_PROCESS_STD_IO_IGNORE,
  PG_CHILD_PROCESS_STD_IO_PIPE,
  PG_CHILD_PROCESS_STD_IO_CLOSE,
} PgChildProcessStdIo;

typedef struct {
  PgChildProcessStdIo stdin_capture, stdout_capture, stderr_capture;
  // TODO: env, cwd, etc.
} PgProcessSpawnOptions;

[[nodiscard]] [[maybe_unused]]
static PgProcessResult pg_process_spawn(PgString path, PgStringSlice args,
                                        PgProcessSpawnOptions options,
                                        PgAllocator *allocator);

[[nodiscard]] [[maybe_unused]]
static PgProcessExitResult pg_process_wait(PgProcess process,
                                           PgAllocator *allocator);

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_copy_with_descriptors(PgFileDescriptor dst, PgFileDescriptor src,
                              Pgu64Ok offset, u64 len);

#ifdef PG_OS_UNIX
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/sendfile.h>
#endif

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_copy_with_descriptors(PgFileDescriptor dst, PgFileDescriptor src,
                              Pgu64Ok offset_opt, u64 len) {
  PG_ASSERT(offset_opt.res < len);

  u64 copied = 0;

#ifdef __linux__
  off_t *offset_ptr = offset_opt.ok ? (off_t *)&offset_opt.res : nullptr;

  while (copied < len) {
    u64 sendfile_len = len - copied;
    ssize_t ret = sendfile(dst.fd, src.fd, offset_ptr, sendfile_len);
    if (-1 == ret) {
      return (PgError)errno;
    }
    if (0 == ret) {
      return (copied == len) ? 0 : PG_ERR_IO;
    }
    copied += (u64)ret;
    offset_ptr = offset_ptr ? offset_ptr : (off_t *)&offset_opt.res;
  }
#else
  // TODO: sendfile on freebsd.
  PG_ASSERT(0 && "todo");

  for (u64 _i = 0; _i < res_size.res; _i++) {
    u8 tmp[4096] = {0};
    ssize_t ret_read = pread(src.fd, tmp, PG_STATIC_ARRAY_LEN(tmp), offset);
    if (-1 == ret_read) {
      res.err = (PgError)errno;
      return res;
    }
    if (0 == ret_read) {
      return res;
    }

    u64 j = 0;
    for (; j < ret_read;) {
      ssize_t ret_write = pwrite(dst.fd, tmp + j, ret_read - j, offset + j);
      if (-1 == ret_write) {
        res.err = (PgError)errno;
        return res;
      }
      if (0 == ret_write) {
        return res;
      }
      j += ret_write;
    }
    PG_ASSERT(j == ret_read);

    copied += ret_read;
    offset += ret_read;
  }
#endif

  return 0;
}

#define PG_PIPE_READ 0
#define PG_PIPE_WRITE 1

[[nodiscard]] [[maybe_unused]]
static PgProcessResult pg_process_spawn(PgString path, PgStringSlice args,
                                        PgProcessSpawnOptions options,
                                        PgAllocator *allocator) {
  PgProcessResult res = {0};

  char *path_c = (char *)pg_string_to_cstr(path, allocator);

  PgCstrDyn args_c = {0};
  PG_DYN_ENSURE_CAP(&args_c, args.len + 2, allocator);
  *PG_DYN_PUSH(&args_c, allocator) = path_c;

  for (u64 i = 0; i < args.len; i++) {
    PgString arg = PG_SLICE_AT(args, i);

    *PG_DYN_PUSH(&args_c, allocator) = pg_string_to_cstr(arg, allocator);
  }
  PG_ASSERT(args.len + 1 == args_c.len);

  int stdin_pipe[2] = {0};
  {
    if (PG_CHILD_PROCESS_STD_IO_PIPE == options.stdin_capture) {
      int ret = pipe(stdin_pipe);
      if (-1 == ret) {
        res.err = (PgError)errno;
        goto end;
      }
    }
  }

  int stdout_pipe[2] = {0};
  {
    if (PG_CHILD_PROCESS_STD_IO_PIPE == options.stdout_capture) {
      int ret = pipe(stdout_pipe);
      if (-1 == ret) {
        res.err = (PgError)errno;
        goto end;
      }
    }
  }

  int stderr_pipe[2] = {0};
  {
    if (PG_CHILD_PROCESS_STD_IO_PIPE == options.stderr_capture) {
      int ret = pipe(stderr_pipe);
      if (-1 == ret) {
        res.err = (PgError)errno;
        goto end;
      }
    }
  }

  int pid = fork();

  if (-1 == pid) {
    res.err = (PgError)errno;
    goto end;
  }

  if (0 == pid) { // Child.

    bool need_ignore =
        (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stdin_capture) ||
        (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stdout_capture) ||
        (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stderr_capture);

    int fd_ignore = 0;
    if (need_ignore) {
      fd_ignore = open("/dev/null", O_RDWR, 0);
      PG_ASSERT(-1 != fd_ignore);
    }

    if (PG_CHILD_PROCESS_STD_IO_PIPE == options.stdin_capture) {
      PG_ASSERT(0 == close(stdin_pipe[PG_PIPE_WRITE]));
      int ret_dup2 = dup2(stdin_pipe[PG_PIPE_READ], STDIN_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    } else if (PG_CHILD_PROCESS_STD_IO_CLOSE == options.stdin_capture) {
      PG_ASSERT(0 == close(STDIN_FILENO));
    } else if (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stdin_capture) {
      int ret_dup2 = dup2(fd_ignore, STDIN_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    }

    if (PG_CHILD_PROCESS_STD_IO_PIPE == options.stdout_capture) {
      PG_ASSERT(0 == close(stdout_pipe[PG_PIPE_READ]));
      int ret_dup2 = dup2(stdout_pipe[PG_PIPE_WRITE], STDOUT_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    } else if (PG_CHILD_PROCESS_STD_IO_CLOSE == options.stdout_capture) {
      PG_ASSERT(0 == close(STDOUT_FILENO));
    } else if (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stdout_capture) {
      int ret_dup2 = dup2(fd_ignore, STDOUT_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    }

    if (PG_CHILD_PROCESS_STD_IO_PIPE == options.stderr_capture) {
      PG_ASSERT(0 == close(stderr_pipe[PG_PIPE_READ]));
      int ret_dup2 = dup2(stderr_pipe[PG_PIPE_WRITE], STDERR_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    } else if (PG_CHILD_PROCESS_STD_IO_CLOSE == options.stderr_capture) {
      PG_ASSERT(0 == close(STDERR_FILENO));
    } else if (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stderr_capture) {
      int ret_dup2 = dup2(fd_ignore, STDERR_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    }

    execvp(path_c, args_c.data);
    PG_ASSERT(0 && "unreachable");
  }

  PG_ASSERT(pid > 0); // Parent.
  res.res.pid = (u64)pid;
  res.res.stdin_pipe.fd = stdin_pipe[PG_PIPE_WRITE];
  res.res.stdout_pipe.fd = stdout_pipe[PG_PIPE_READ];
  res.res.stderr_pipe.fd = stderr_pipe[PG_PIPE_READ];

end:
  for (u64 i = 0; i < args_c.len; i++) {
    char *arg_c = PG_SLICE_AT(args_c, i);
    pg_free(allocator, arg_c);
  }

  if (stdin_pipe[PG_PIPE_READ]) {
    PG_ASSERT(0 == close(stdin_pipe[PG_PIPE_READ]));
  }
  if (stdout_pipe[PG_PIPE_WRITE]) {
    PG_ASSERT(0 == close(stdout_pipe[PG_PIPE_WRITE]));
  }
  if (stderr_pipe[PG_PIPE_WRITE]) {
    PG_ASSERT(0 == close(stderr_pipe[PG_PIPE_WRITE]));
  }

  if (res.err) {
    if (stdin_pipe[PG_PIPE_WRITE]) {
      PG_ASSERT(0 == close(stdin_pipe[PG_PIPE_WRITE]));
    }
    if (stdout_pipe[PG_PIPE_READ]) {
      PG_ASSERT(0 == close(stdout_pipe[PG_PIPE_READ]));
    }
    if (stderr_pipe[PG_PIPE_READ]) {
      PG_ASSERT(0 == close(stderr_pipe[PG_PIPE_READ]));
    }
  }

  return res;
}

[[nodiscard]] [[maybe_unused]]
static PgProcessCaptureStdResult
pg_process_capture_std_io(PgProcess process, PgAllocator *allocator) {
  PgProcessCaptureStdResult res = {0};
  Pgu8Dyn stdout_sb = {0};
  Pgu8Dyn stderr_sb = {0};

  if (process.stdout_pipe.fd) {
    stdout_sb = pg_sb_make_with_cap(4096, allocator);
  }

  if (process.stderr_pipe.fd) {
    stderr_sb = pg_sb_make_with_cap(4096, allocator);
  }

  while (process.stdout_pipe.fd || process.stderr_pipe.fd) {
    struct pollfd pollfds[2] = {0};
    nfds_t pollfds_len = 0;

    if (process.stdout_pipe.fd) {
      pollfds[pollfds_len++] = (struct pollfd){
          .fd = process.stdout_pipe.fd,
          .events = POLLIN,
      };
    }

    if (process.stderr_pipe.fd) {
      pollfds[pollfds_len++] = (struct pollfd){
          .fd = process.stderr_pipe.fd,
          .events = POLLIN,
      };
    }

    int res_poll = poll(pollfds, pollfds_len, -1);
    if (-1 == res_poll) {
      res.err = (PgError)errno;
      goto end;
    }

    if (0 == res_poll) {
      break;
    }

    for (u64 i = 0; i < (u64)res_poll; i++) {
      struct pollfd pollfd = PG_C_ARRAY_AT(pollfds, pollfds_len, i);

      PgFileDescriptor *fd = &process.stdout_pipe;
      Pgu8Dyn *sb = &stdout_sb;
      if (pollfd.fd == process.stderr_pipe.fd) {
        fd = &process.stderr_pipe;
        sb = &stderr_sb;
      }

      if (pollfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        PG_ASSERT(0 == close(pollfd.fd));
        fd->fd = 0;
        continue;
      }

      PG_ASSERT(pollfd.revents & POLLIN);

      u8 tmp[4096] = {0};
      ssize_t read_n = read(pollfd.fd, tmp, PG_STATIC_ARRAY_LEN(tmp));
      if (-1 == read_n) {
        res.err = (PgError)errno;
        goto end;
      }
      if (0 == read_n) {
        PG_ASSERT(0 == close(pollfd.fd));
        fd->fd = 0;
        continue;
      }

      PgString actually_read = {.data = tmp, .len = (u64)read_n};
      PG_DYN_APPEND_SLICE(sb, actually_read, allocator);
    }
  }

end:
  if (res.err) {
    pg_free(allocator, stdout_sb.data);
    pg_free(allocator, stderr_sb.data);
  }

  if (process.stdin_pipe.fd) {
    PG_ASSERT(0 == close(process.stdin_pipe.fd));
  }
  if (process.stdout_pipe.fd) {
    PG_ASSERT(0 == close(process.stdout_pipe.fd));
  }
  if (process.stderr_pipe.fd) {
    PG_ASSERT(0 == close(process.stderr_pipe.fd));
  }

  res.res.stdout_captured = PG_DYN_SLICE(PgString, stdout_sb);
  res.res.stderr_captured = PG_DYN_SLICE(PgString, stderr_sb);
  return res;
}

[[nodiscard]] [[maybe_unused]]
static PgProcessExitResult pg_process_wait(PgProcess process,
                                           PgAllocator *allocator) {
  PgProcessExitResult res = {0};

  PgProcessCaptureStdResult res_capture =
      pg_process_capture_std_io(process, allocator);
  if (res_capture.err) {
    res.err = res_capture.err;
    return res;
  }

  int status = 0;
  int ret = waitpid((pid_t)process.pid, &status, 0);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.res.exit_status = WEXITSTATUS(status);
  res.res.exited = WIFEXITED(status);
  res.res.signaled = WIFSIGNALED(status);
  res.res.signal = WTERMSIG(status);
  res.res.core_dumped = WCOREDUMP(status);
  res.res.stopped = WIFSTOPPED(status);
  res.res.stdout_captured = res_capture.res.stdout_captured;
  res.res.stderr_captured = res_capture.res.stderr_captured;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdin() {
  return (PgFileDescriptor){.fd = STDIN_FILENO};
}
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdout() {
  return (PgFileDescriptor){.fd = STDOUT_FILENO};
}
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stderr() {
  return (PgFileDescriptor){.fd = STDERR_FILENO};
}

[[nodiscard]] i32 pg_os_get_last_error() { return errno; }

[[nodiscard]] i32 pg_virtual_mem_flags_to_os_flags(PgVirtualMemFlags flags) {
  u64 res = 0;
  if (flags & PG_VIRTUAL_MEM_FLAGS_READ) {
    res |= PROT_READ;
  }
  if (flags & PG_VIRTUAL_MEM_FLAGS_WRITE) {
    res |= PROT_WRITE;
  }
  if (flags & PG_VIRTUAL_MEM_FLAGS_EXEC) {
    res |= PROT_EXEC;
  }
  return (i32)res;
}

[[nodiscard]] PgVoidPtrResult pg_virtual_mem_alloc(u64 size,
                                                   PgVirtualMemFlags flags) {
  PG_ASSERT(size > 0);

  PgVoidPtrResult res = {0};
  res.res = mmap(nullptr, size, pg_virtual_mem_flags_to_os_flags(flags),
                 MAP_ANON | MAP_PRIVATE, -1, 0);
  if (!res.res) {
    res.err = (PgError)pg_os_get_last_error();
  }
  return res;
}

[[nodiscard]] PgError pg_virtual_mem_protect(void *ptr, u64 size,
                                             PgVirtualMemFlags flags_new) {
  if (-1 == mprotect(ptr, size, pg_virtual_mem_flags_to_os_flags(flags_new))) {
    return (PgError)pg_os_get_last_error();
  }
  return 0;
}

[[nodiscard]] PgError pg_virtual_mem_release(void *ptr, u64 size) {
  if (-1 == munmap(ptr, size)) {
    return (PgError)pg_os_get_last_error();
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_writer_unix_file_write(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgWriter *w = self;

  PgFileDescriptor file = w->ctx;
  isize n = 0;
  do {
    n = write(file.fd, buf, buf_len);
  } while (-1 == n && EINTR == errno);

  PgU64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.res = (u64)n;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_writer_file_write(void *self, u8 *buf, size_t buf_len) {
  return pg_writer_unix_file_write(self, buf, buf_len);
}

[[maybe_unused]] [[nodiscard]] static clockid_t
pg_clock_to_linux(PgClockKind clock_kind) {
  switch (clock_kind) {
  case PG_CLOCK_KIND_MONOTONIC:
    return CLOCK_MONOTONIC;
  case PG_CLOCK_KIND_REALTIME:
    return CLOCK_REALTIME;
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_time_ns_now(PgClockKind clock) {
  PgU64Result res = {0};

  struct timespec ts = {0};
  int ret = 0;
  do {
    ret = clock_gettime(pg_clock_to_linux(clock), &ts);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.res = (u64)ts.tv_sec * PG_Seconds + (u64)ts.tv_nsec;

  return res;
}

[[maybe_unused]] static u64
pg_fill_call_stack(u64 call_stack[PG_STACKTRACE_MAX]) {
  u64 *frame_pointer = __builtin_frame_address(0);
  u64 res = 0;

  while (res < PG_STACKTRACE_MAX && frame_pointer != 0 &&
         ((uint64_t)frame_pointer & 7) == 0 && *frame_pointer != 0) {
    u64 instruction_pointer = *(frame_pointer + 1);
    frame_pointer = (u64 *)*frame_pointer;

    // `ip` points to the return instruction in the caller, once this call is
    // done. But: We want the location of the call i.e. the `call xxx`
    // instruction, so we subtract one byte to point inside it, which is not
    // quite 'at' it, but good enough.
    call_stack[res++] = instruction_pointer - 1;
  }

  return res;
}

[[nodiscard]] static u64 pg_os_get_page_size() {
  i64 ret = 0;
  do {
    ret = sysconf(_SC_PAGE_SIZE);
  } while (-1 == ret && EINTR == errno);

  PG_ASSERT(ret > 0);

  return (u64)ret;
}

[[maybe_unused]] static PgU64Result pg_file_read(PgFileDescriptor file,
                                                 PgString buf) {
  PgU64Result res = {0};

  isize n = 0;
  do {
    n = read(file.fd, buf.data, buf.len);
  } while (-1 == n && EINTR == errno);

  if (-1 == n) {
    res.err = (PgError)errno;
    return res;
  }

  res.res = (u64)n;
  return res;
}

[[maybe_unused]] static PgU64Result pg_file_write(PgFileDescriptor file,
                                                  PgString s) {
  PgU64Result res = {0};

  isize n = 0;
  do {
    n = write(file.fd, s.data, s.len);
  } while (-1 == n && EINTR == errno);

  if (-1 == n) {
    res.err = (PgError)errno;
    return res;
  }

  res.res = (u64)n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_truncate(PgFileDescriptor file, u64 size) {
  if (-1 == ftruncate(file.fd, (i64)size)) {
    return (PgError)pg_os_get_last_error();
  }
  return 0;
}

[[maybe_unused]] static PgU64Result pg_file_read_at(PgFileDescriptor file,
                                                    PgString buf, u64 offset) {
  PgU64Result res = {0};

  isize n = 0;
  do {
    n = pread(file.fd, buf.data, buf.len, (off_t)offset);
  } while (-1 == n && EINTR == errno);

  if (-1 == n) {
    res.err = (PgError)errno;
    return res;
  }

  res.res = (u64)n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_file_open(PgString path, PgFileAccess access, bool create_if_not_exists,
             PgAllocator *allocator) {
  PgFileDescriptorResult res = {0};

  int flags = 0;
  switch (access & PG_FILE_ACCESS_ALL) {
  case PG_FILE_ACCESS_READ:
    flags = O_RDONLY;
    break;
  case PG_FILE_ACCESS_WRITE:
    flags = O_WRONLY;
    break;
  case PG_FILE_ACCESS_READ_WRITE:
    flags = O_RDWR;
    break;
  case PG_FILE_ACCESS_NONE:
  default:
    PG_ASSERT(0);
  }

  if (create_if_not_exists) {
    flags |= O_CREAT;
  }

  char *path_os = pg_string_to_cstr(path, allocator);
  int fd = open(path_os, flags, 0600);
  pg_free(allocator, path_os);

  if (-1 == fd) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.res.fd = fd;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_close(PgFileDescriptor file) {
  if (-1 == close(file.fd)) {
    return (PgError)pg_os_get_last_error();
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_file_size(PgFileDescriptor file) {
  PgU64Result res = {0};
  struct stat st = {0};

  int ret = 0;
  do {
    ret = fstat(file.fd, &st);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.res = (u64)st.st_size;
  return res;
}

#else

// -- Win32 ---

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <io.h>
#include <windows.h>

[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdin() {
  return (PgFileDescriptor){.ptr = GetStdHandle(STD_INPUT_HANDLE)};
}
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdout() {
  return (PgFileDescriptor){.ptr = GetStdHandle(STD_OUTPUT_HANDLE)};
}
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stderr() {
  return (PgFileDescriptor){.ptr = GetStdHandle(STD_ERROR_HANDLE)};
}

typedef struct {
  wchar_t *data;
  u64 len;
} PgWtf16String;

PG_RESULT(PgWtf16String) PgWtf16StringResult;

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_writer_win32_write(void *self, u8 *buf, size_t buf_len) {
  PG_ASSERT(nullptr != self);
  PG_ASSERT(nullptr != buf);

  PgWriter *w = self;

  PgFileDescriptor file = w->ctx;
  HANDLE handle = file.ptr;
  DWORD n = 0;
  bool ok = WriteFile(handle, buf, (DWORD)buf_len, &n, nullptr);
  PgU64Result res = {0};
  if (!ok) {
    res.err = (PgError)pg_os_get_last_error();
  } else {
    res.res = (u64)n;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_writer_file_write(void *self, u8 *buf, size_t buf_len) {
  return pg_writer_win32_write(self, buf, buf_len);
}

[[nodiscard]] static u64 pg_os_get_page_size() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  u64 res = (u64)info.dwPageSize;

  PG_ASSERT(res > 0);
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_time_ns_now(PgClockKind clock_kind) {
  PgU64Result res = {0};

  switch (clock_kind) {
  case PG_CLOCK_KIND_MONOTONIC: {
    static LARGE_INTEGER frequency = {0};
    if (!frequency.QuadPart) {
      QueryPerformanceFrequency(&frequency);
      PG_ASSERT(frequency.QuadPart);
    }
    LARGE_INTEGER counter = {0};
    PG_ASSERT(QueryPerformanceCounter(&counter));
    double seconds = (double)counter.QuadPart / (double)frequency.QuadPart;

    res.res = (u64)(PG_Seconds * seconds);
    return res;
  }
  case PG_CLOCK_KIND_REALTIME: {
    FILETIME ft = {0};
    GetSystemTimeAsFileTime(&ft);
    res.res = (((u64)ft.dwHighDateTime) << 32) | ((u64)ft.dwLowDateTime);
    return res;
  }
  default:
    PG_ASSERT(0);
  }

  return res;
}

[[nodiscard]] i32 pg_os_get_last_error() { return (i32)GetLastError(); }

[[nodiscard]] i32 pg_virtual_mem_flags_to_os_flags(PgVirtualMemFlags flags) {
  if (PG_VIRTUAL_MEM_FLAGS_NONE == flags) {
    return PAGE_NOACCESS;
  }

  if (flags & PG_VIRTUAL_MEM_FLAGS_WRITE) {
    return PAGE_READWRITE;
  }

  if (PG_VIRTUAL_MEM_FLAGS_READ == flags) {
    return PAGE_READONLY;
  }

  PG_ASSERT(0 && "todo");
}

[[nodiscard]] PgVoidPtrResult pg_virtual_mem_alloc(u64 size,
                                                   PgVirtualMemFlags flags) {
  PG_ASSERT(size > 0);

  PgVoidPtrResult res = {0};
  // Note: We will reserve the guard pages right now.

  res.res = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE,
                         (DWORD)pg_virtual_mem_flags_to_os_flags(flags));
  if (!res.res) {
    res.err = (PgError)pg_os_get_last_error();
  }
  return res;
}

[[nodiscard]] PgError pg_virtual_mem_protect(void *ptr, u64 size,
                                             PgVirtualMemFlags flags_new) {

  DWORD flags_old = 0;
  if (0 == VirtualProtect(ptr, size,
                          (DWORD)pg_virtual_mem_flags_to_os_flags(flags_new),
                          &flags_old)) {
    return (PgError)pg_os_get_last_error();
  }
  return 0;
}

[[nodiscard]] PgError pg_virtual_mem_release(void *ptr, u64 size) {
  (void)size;

  if (0 == VirtualFree(ptr, 0, MEM_RELEASE)) {
    return (PgError)pg_os_get_last_error();
  }
  return 0;
}

[[nodiscard]] static PgWtf16StringResult
pg_string_to_wtf16(PgString s, PgAllocator *allocator) {
  int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                 (const char *)s.data, (i32)s.len, nullptr, 0);
  PgWtf16StringResult res = {0};

  if (0 == wlen) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.res.len = (u64)wlen + 1;
  res.res.data =
      pg_alloc(allocator, sizeof(wchar_t), _Alignof(wchar_t), res.res.len);
  PG_ASSERT(res.res.data);
  PG_ASSERT(0 != MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                     (const char *)s.data, (i32)s.len,
                                     res.res.data, (i32)res.res.len));
  PG_ASSERT(0 == res.res.data[res.res.len - 1]);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_truncate(PgFileDescriptor file, u64 size) {
  LARGE_INTEGER li_offset;
  li_offset.QuadPart = (LONGLONG)size;
  if (!SetFilePointerEx(file.ptr, li_offset, nullptr, FILE_BEGIN)) {
    return (PgError)pg_os_get_last_error();
  }

  if (!SetEndOfFile(file.ptr)) {
    return (PgError)pg_os_get_last_error();
  }

  li_offset.QuadPart = 0;
  if (!SetFilePointerEx(file.ptr, li_offset, nullptr, FILE_BEGIN)) {
    return (PgError)pg_os_get_last_error();
  }

  return 0;
}

[[nodiscard]] static PgFileDescriptorResult
pg_file_open(PgString path, PgFileAccess access, bool create_if_not_exists,
             PgAllocator *allocator) {
  PgFileDescriptorResult res = {0};

  DWORD desired_access = 0;
  DWORD creation_disposition =
      create_if_not_exists ? CREATE_ALWAYS : OPEN_EXISTING;

  switch (access) {
  case PG_FILE_ACCESS_READ:
    desired_access = GENERIC_READ;
    break;
  case PG_FILE_ACCESS_WRITE:
    desired_access = GENERIC_WRITE;
    break;
  case PG_FILE_ACCESS_READ_WRITE:
    desired_access = GENERIC_READ | GENERIC_WRITE;
    break;
  case PG_FILE_ACCESS_NONE:
  default:
    PG_ASSERT(0);
  }

  PgWtf16StringResult res_path_os = pg_string_to_wtf16(path, allocator);
  if (res_path_os.err) {
    res.err = res_path_os.err;
    return res;
  }

  HANDLE handle = CreateFileW(
      res_path_os.res.data, desired_access, FILE_SHARE_READ | FILE_SHARE_DELETE,
      nullptr, creation_disposition, FILE_ATTRIBUTE_NORMAL, nullptr);

  pg_free(allocator, res_path_os.res.data);

  if (INVALID_HANDLE_VALUE == handle) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.res.ptr = handle;
  return res;
}

[[nodiscard]] static PgError pg_file_close(PgFileDescriptor file) {
  if (0 == CloseHandle(file.ptr)) {
    return (PgError)pg_os_get_last_error();
  }

  return 0;
}

[[nodiscard]] static PgU64Result pg_file_size(PgFileDescriptor file) {
  PgU64Result res = {0};

  LARGE_INTEGER size;
  if (0 == GetFileSizeEx(file.ptr, &size)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.res = (u64)size.QuadPart;
  return res;
}

[[nodiscard]] static PgU64Result pg_file_read(PgFileDescriptor file,
                                              PgString dst) {
  PgU64Result res = {0};

  if (0 == ReadFile(file.ptr, dst.data, (DWORD)dst.len, (LPDWORD)&res.res,
                    nullptr)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  return res;
}

[[nodiscard]] static PgU64Result pg_file_write(PgFileDescriptor file,
                                               PgString s) {
  PgU64Result res = {0};

  if (0 ==
      WriteFile(file.ptr, s.data, (DWORD)s.len, (LPDWORD)&res.res, nullptr)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  return res;
}

[[nodiscard]] static PgU64Result pg_file_read_at(PgFileDescriptor file,
                                                 PgString dst, u64 offset) {
  PgU64Result res = {0};

  LARGE_INTEGER li_offset;
  li_offset.QuadPart = (LONGLONG)offset;
  if (!SetFilePointerEx(file.ptr, li_offset, nullptr, FILE_BEGIN)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  if (!SetEndOfFile(file.ptr)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res = pg_file_read(file, dst);

  li_offset.QuadPart = 0;
  if (!SetFilePointerEx(file.ptr, li_offset, nullptr, FILE_BEGIN)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  return res;
}

#endif

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
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(arena);
  *PG_DYN_PUSH(headers, (PgAllocator *)&arena_allocator) =
      (PgKeyValue){.key = key, .value = value};
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
pg_html_sanitize(PgString s, PgAllocator *allocator) {
  Pgu8Dyn res = {0};
  PG_DYN_ENSURE_CAP(&res, s.len, allocator);
  for (u64 i = 0; i < s.len; i++) {
    u8 c = PG_SLICE_AT(s, i);

    if ('&' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&amp"), allocator);
    } else if ('<' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&lt"), allocator);
    } else if ('>' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&gt"), allocator);
    } else if ('"' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&quot"), allocator);
    } else if ('\'' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&#x27"), allocator);
    } else {
      *PG_DYN_PUSH(&res, allocator) = c;
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
pg_url_parse_path_components(PgString s, PgAllocator *allocator) {
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

    *PG_DYN_PUSH(&components, allocator) = split.res;
  }

  res.res = components;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgDynKeyValueResult
pg_url_parse_query_parameters(PgString s, PgAllocator *allocator) {
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
      *PG_DYN_PUSH(&res.res, allocator) = (PgKeyValue){.key = k, .value = v};
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
pg_url_parse_after_authority(PgString s, PgAllocator *allocator) {
  PgUrlResult res = {0};
  PgString remaining = s;

  PgStringPairConsumeAny path_components_and_rem =
      pg_string_consume_until_any_byte_excl(remaining, PG_S("?#"));
  remaining = path_components_and_rem.right;

  // Path, optional.
  if (pg_string_starts_with(s, PG_S("/"))) {
    PG_ASSERT(!PG_SLICE_IS_EMPTY(path_components_and_rem.left));

    PgStringDynResult res_path_components =
        pg_url_parse_path_components(path_components_and_rem.left, allocator);
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
        pg_url_parse_query_parameters(path_components_and_rem.right, allocator);
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

[[maybe_unused]] [[nodiscard]] static PgUrlResult
pg_url_parse(PgString s, PgAllocator *allocator) {
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
      pg_url_parse_after_authority(remaining, allocator);
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
pg_http_parse_request_status_line(PgString status_line,
                                  PgAllocator *allocator) {
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
    PgUrlResult res_url = pg_url_parse_after_authority(path, allocator);
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
pg_http_read_response(PgRing *rg, u64 max_http_headers,
                      PgAllocator *allocator) {
  PgHttpResponseReadResult res = {0};
  PgString sep = PG_S("\r\n\r\n");

  PgStringOk s = pg_ring_read_until_excl(rg, sep, allocator);
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

    *PG_DYN_PUSH(&res.res.headers, allocator) = res_kv.res;
  }
  if (!PG_SLICE_IS_EMPTY(it.s)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.done = true;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgHttpRequestReadResult
pg_http_read_request(PgRing *rg, u64 max_http_headers, PgAllocator *allocator) {
  PgHttpRequestReadResult res = {0};
  PgString sep = PG_S("\r\n\r\n");

  PgStringOk s = pg_ring_read_until_excl(rg, sep, allocator);
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
      pg_http_parse_request_status_line(res_split.res, allocator);
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

    *PG_DYN_PUSH(&res.res.headers, allocator) = res_kv.res;
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

[[maybe_unused]] static PgString
pg_http_request_to_string(PgHttpRequest req, PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb,
                    // TODO: Tweak this number?
                    128 + req.url.path_components.len * 64 +
                        req.url.query_parameters.len * 64 +
                        req.headers.len * 128,
                    allocator);
  PgWriter w = pg_writer_make_from_string_builder(&sb, allocator);

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

[[maybe_unused]] [[nodiscard]] static PgU64Result
pg_http_headers_parse_content_length(PgKeyValueSlice headers, PgArena arena) {
  PgU64Result res = {0};

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

#if 0
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
                              PgAllocator *allocator) {
  PgFormDataKVElementParseResult res = {0};
  Pgu8Dyn data = {0};
  PG_DYN_ENSURE_CAP(&data, in.len * 2, allocator);

  u64 i = 0;
  for (; i < in.len; i++) {
    u8 c = in.data[i];

    if ('+' == c) {
      *PG_DYN_PUSH(&data, allocator) = ' ';
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
      *PG_DYN_PUSH(&data, allocator) = utf8_character;
      i += 2; // Consume 2 characters.
    } else if (pg_character_terminator == c) {
      i += 1; // Consume.
      break;
    } else {
      *PG_DYN_PUSH(&data, allocator) = c;
    }
  }

  res.data = PG_DYN_SLICE(PgString, data);
  res.remaining = PG_SLICE_RANGE_START(in, i);
  return res;
}

[[nodiscard]] static PgFormDataKVParseResult
pg_form_data_kv_parse(PgString in, PgAllocator *allocator) {
  PgFormDataKVParseResult res = {0};

  PgString remaining = in;

  PgFormDataKVElementParseResult key_parsed =
      pg_form_data_kv_parse_element(remaining, '=', allocator);
  if (key_parsed.err) {
    res.err = key_parsed.err;
    return res;
  }
  res.kv.key = key_parsed.data;

  remaining = key_parsed.remaining;

  PgFormDataKVElementParseResult value_parsed =
      pg_form_data_kv_parse_element(remaining, '&', allocator);
  if (value_parsed.err) {
    res.err = value_parsed.err;
    return res;
  }
  res.kv.value = value_parsed.data;
  res.remaining = value_parsed.remaining;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFormDataParseResult
pg_form_data_parse(PgString in, PgAllocator *allocator) {
  PgFormDataParseResult res = {0};

  PgString remaining = in;

  for (u64 i = 0; i < in.len; i++) { // Bound.
    if (PG_SLICE_IS_EMPTY(remaining)) {
      break;
    }

    PgFormDataKVParseResult kv = pg_form_data_kv_parse(remaining, allocator);
    if (kv.err) {
      res.err = kv.err;
      return res;
    }

    *PG_DYN_PUSH(&res.form, allocator) = kv.kv;

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
pg_html_make(PgString title, PgAllocator *allocator) {
  PgHtmlDocument res = {0};

  {

    PgHtmlElement tag_head = {.kind = PG_HTML_HEAD};
    {
      PgHtmlElement tag_meta = {.kind = PG_HTML_META};
      {
        *PG_DYN_PUSH(&tag_meta.attributes, allocator) =
            (PgKeyValue){.key = PG_S("charset"), .value = PG_S("utf-8")};
      }
      *PG_DYN_PUSH(&tag_head.children, allocator) = tag_meta;
    }
    {
      PgHtmlElement tag_title = {.kind = PG_HTML_TITLE, .text = title};
      *PG_DYN_PUSH(&tag_head.children, allocator) = tag_title;
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
#endif

typedef enum {
  PG_LOG_VALUE_STRING,
  PG_LOG_VALUE_U64,
  PG_LOG_VALUE_I64,
  PG_LOG_VALUE_IPV4_ADDRESS,
} PgLogValueKind;

typedef enum {
  PG_LOG_LEVEL_DEBUG,
  PG_LOG_LEVEL_INFO,
  PG_LOG_LEVEL_ERROR,
} PgLogLevel;

typedef struct PgLogger PgLogger;
typedef PgString (*PgMakeLogLineFn)(u8 *mem, u64 mem_len, PgLogger *logger,
                                    PgLogLevel level, PgString msg,
                                    i32 args_count, ...);
struct PgLogger {
  PgLogLevel level;
  PgWriter writer;
  PgMakeLogLineFn make_log_line;
  u64 monotonic_epoch;
};

typedef struct {
  PgLogValueKind kind;
  union {
    PgString s;
    u32 n32;
    u64 n64;
    i32 s32;
    i64 s64;
    PgIpv4Address ipv4_address;
  };
} PgLogValue;

typedef struct {
  PgString key;
  PgLogValue value;
} PgLogEntry;

#if 0
[[maybe_unused]] [[nodiscard]] static PgString
pg_log_make_log_line_json(PgLogLevel level, PgString msg, PgArena *arena,
                          i32 args_count, ...);
#endif
[[maybe_unused]] [[nodiscard]] static PgString
pg_log_make_log_line_logfmt(u8 *mem, u64 mem_len, PgLogger *logger,
                            PgLogLevel level, PgString msg, i32 args_count,
                            ...);

[[maybe_unused]] [[nodiscard]] static PgLogger
pg_log_make_logger_stdout_logfmt(PgLogLevel level) {
  PgLogger logger = {
      .level = level,
      .writer = pg_writer_make_from_file_descriptor(pg_os_stdout()),
      .make_log_line = pg_log_make_log_line_logfmt,
      .monotonic_epoch = pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC).res,
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
  default:
    PG_ASSERT(false);
  }
}

#define pg_log_cu8(k, v) pg_log_u8(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_u8(PgString k, u8 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

#define pg_log_cu16(k, v) pg_log_u16(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_u16(PgString k, u16 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

#define pg_log_cu32(k, v) pg_log_u32(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_u32(PgString k, u32 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

#define pg_log_cu64(k, v) pg_log_u64(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_u64(PgString k, u64 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = v,
  };
}

#define pg_log_ci32(k, v) pg_log_i32(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_i32(PgString k, i32 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_I64,
      .value.s64 = (i64)v,
  };
}

#define pg_log_cerr(k, v) pg_log_i32(PG_S(k), (i32)v)
#define pg_log_err(k, v) pg_log_i32(k, (i32)v)

#define pg_log_ci64(k, v) pg_log_i64(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_i64(PgString k, i64 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_I64,
      .value.s64 = v,
  };
}

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_s(PgString k,
                                                          PgString v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_STRING,
      .value.s = v,
  };
}

#define pg_log_cs(k, v) pg_log_s(PG_S(k), v)

#define pg_log_cipv4(k, v) pg_log_ipv4(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_ipv4(PgString k,
                                                             PgIpv4Address v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_IPV4_ADDRESS,
      .value.ipv4_address = v,
  };
}

#define PG_LOG_ARGS_COUNT(...)                                                 \
  (sizeof((PgLogEntry[]){__VA_ARGS__}) / sizeof(PgLogEntry))
#define pg_log(logger, lvl, msg, ...)                                          \
  do {                                                                         \
    if (nullptr == (logger)) {                                                 \
      break;                                                                   \
    }                                                                          \
    PG_ASSERT(nullptr != (logger)->make_log_line);                             \
    if ((logger)->level > (lvl)) {                                             \
      break;                                                                   \
    };                                                                         \
    u8 mem[PG_LOG_LINE_MAX_LENGTH] = {0};                                      \
    PgString xxx_log_line = (logger)->make_log_line(                           \
        mem, PG_STATIC_ARRAY_LEN(mem), logger, lvl, PG_S(msg),                 \
        PG_LOG_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__);                          \
    (logger)->writer.write_fn(&(logger)->writer, xxx_log_line.data,            \
                              xxx_log_line.len);                               \
  } while (0)

#if 0
[[maybe_unused]] [[nodiscard]] static PgString
pg_json_escape_string(PgString entry, PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 2 + entry.len * 2, allocator);
  *PG_DYN_PUSH(&sb, allocator) = '"';

  for (u64 i = 0; i < entry.len; i++) {
    u8 c = PG_SLICE_AT(entry, i);
    if ('"' == c) {
      *PG_DYN_PUSH(&sb, allocator) = '\\';
      *PG_DYN_PUSH(&sb, allocator) = '"';
    } else if ('\\' == c) {
      *PG_DYN_PUSH(&sb, allocator) = '\\';
      *PG_DYN_PUSH(&sb, allocator) = '\\';
    } else if ('\b' == c) {
      *PG_DYN_PUSH(&sb, allocator) = '\\';
      *PG_DYN_PUSH(&sb, allocator) = 'b';
    } else if ('\f' == c) {
      *PG_DYN_PUSH(&sb, allocator) = '\\';
      *PG_DYN_PUSH(&sb, allocator) = 'f';
    } else if ('\n' == c) {
      *PG_DYN_PUSH(&sb, allocator) = '\\';
      *PG_DYN_PUSH(&sb, allocator) = 'n';
    } else if ('\r' == c) {
      *PG_DYN_PUSH(&sb, allocator) = '\\';
      *PG_DYN_PUSH(&sb, allocator) = 'r';
    } else if ('\t' == c) {
      *PG_DYN_PUSH(&sb, allocator) = '\\';
      *PG_DYN_PUSH(&sb, allocator) = 't';
    } else {
      *PG_DYN_PUSH(&sb, allocator) = c;
    }
  }
  *PG_DYN_PUSH(&sb, allocator) = '"';

  return PG_DYN_SLICE(PgString, sb);
}
#endif

[[maybe_unused]] static void pg_logfmt_escape_u8(Pgu8Dyn *sb, u8 c,
                                                 PgAllocator *allocator) {
  if (' ' == c || c == '-' || c == '_' || c == ':' || c == ',' || c == '.' ||
      pg_character_is_alphanumeric(c)) {
    *PG_DYN_PUSH(sb, allocator) = c;
  } else {
    u8 c1 = c % 16;
    u8 c2 = c / 16;
    PG_DYN_APPEND_SLICE(sb, PG_S("\\x"), allocator);
    *PG_DYN_PUSH(sb, allocator) = pg_u8_to_character_hex(c2);
    *PG_DYN_PUSH(sb, allocator) = pg_u8_to_character_hex(c1);
  }
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_logfmt_escape_string(PgString entry, PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 2 + PG_CLAMP(0, entry.len, PG_LOG_STRING_MAX + 4) * 2,
                    allocator);
  *PG_DYN_PUSH(&sb, allocator) = '"';

  if (entry.len <= PG_LOG_STRING_MAX) {
    for (u64 i = 0; i < entry.len; i++) {
      u8 c = PG_SLICE_AT(entry, i);
      pg_logfmt_escape_u8(&sb, c, allocator);
    }
  } else {
    for (u64 i = 0; i < PG_LOG_STRING_MAX / 2; i++) {
      u8 c = PG_SLICE_AT(entry, i);
      pg_logfmt_escape_u8(&sb, c, allocator);
    }
    PG_DYN_APPEND_SLICE(&sb, PG_S("[..]"), allocator);
    for (u64 i = (entry.len - PG_LOG_STRING_MAX / 2); i < entry.len; i++) {
      u8 c = PG_SLICE_AT(entry, i);
      pg_logfmt_escape_u8(&sb, c, allocator);
    }
  }
  *PG_DYN_PUSH(&sb, allocator) = '"';

  return PG_DYN_SLICE(PgString, sb);
}

#if 0
[[maybe_unused]] [[nodiscard]] static PgString
pg_json_unescape_string(PgString entry, PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, entry.len, allocator);

  for (u64 i = 0; i < entry.len; i++) {
    u8 c = PG_SLICE_AT(entry, i);
    u8 next = i + 1 < entry.len ? PG_SLICE_AT(entry, i + 1) : 0;

    if ('\\' == c) {
      if ('"' == next) {
        *PG_DYN_PUSH(&sb, allocator) = '"';
        i += 1;
      } else if ('\\' == next) {
        *PG_DYN_PUSH(&sb, allocator) = '\\';
        i += 1;
      } else if ('b' == next) {
        *PG_DYN_PUSH(&sb, allocator) = '\b';
        i += 1;
      } else if ('f' == next) {
        *PG_DYN_PUSH(&sb, allocator) = '\f';
        i += 1;
      } else if ('n' == next) {
        *PG_DYN_PUSH(&sb, allocator) = '\n';
        i += 1;
      } else if ('r' == next) {
        *PG_DYN_PUSH(&sb, allocator) = '\r';
        i += 1;
      } else if ('t' == next) {
        *PG_DYN_PUSH(&sb, allocator) = '\t';
        i += 1;
      } else {
        *PG_DYN_PUSH(&sb, allocator) = c;
      }
    } else {
      *PG_DYN_PUSH(&sb, allocator) = c;
    }
  }

  return PG_DYN_SLICE(PgString, sb);
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_writer_write_json_object_key_string_value_string(PgWriter *w, PgString key,
                                                    PgString value,
                                                    PgAllocator *allocator) {
  PgError err = 0;

  PgString pg_json_key = pg_json_escape_string(key, allocator);

  err = pg_writer_write_all_string(w, pg_json_key);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, ':');
  if (err) {
    return err;
  }

  PgString pg_json_value = pg_json_escape_string(value, allocator);
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
                                                 u64 value,
                                                 PgAllocator *allocator) {
  PgError err = 0;

  PgString pg_json_key = pg_json_escape_string(key, allocator);

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
pg_log_make_log_line_json(PgLogLevel level, PgString msg,
                          PgAllocator *allocator, i32 args_count, ...) {
  PgU64Result res_monotonic_ns = pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC);
  PgU64Result res_timestamp_ns = pg_time_ns_now(PG_CLOCK_KIND_REALTIME);
  // Ignore clock errors.

  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 256, allocator);
  PgWriter w = pg_writer_make_from_string_builder(&sb, allocator);

  PG_ASSERT(0 == pg_writer_write_u8(&w, '{'));

  PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_string(
                     &w, PG_S("level"), pg_log_level_to_string(level), allocator));
  PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_u64(
                     &w, PG_S("timestamp_ns"), res_timestamp_ns.res, allocator));
  PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_u64(
                     &w, PG_S("monotonic_ns"), res_monotonic_ns.res, allocator));
  PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_string(
                     &w, PG_S("message"), msg, allocator));

  va_list argp = {0};
  va_start(argp, args_count);
  for (i32 i = 0; i < args_count; i++) {
    PgLogEntry entry = va_arg(argp, PgLogEntry);

    switch (entry.value.kind) {
    case PG_LOG_VALUE_STRING: {
      PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_string(
                         &w, entry.key, entry.value.s, allocator));
      break;
    }
    case PG_LOG_VALUE_U64:
      PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_u64(
                         &w, entry.key, entry.value.n64, allocator));
      break;
    case PG_LOG_VALUE_IPV4_ADDRESS: {
      PgString ipv4_addr_str =
          pg_net_ipv4_address_to_string(entry.value.ipv4_address, allocator);
      PG_ASSERT(0 == pg_writer_write_json_object_key_string_value_string(
                         &w, entry.key, ipv4_addr_str, allocator));
    } break;
    default:
      PG_ASSERT(0 && "invalid PgLogValueKind");
    }
  }
  va_end(argp);

  PG_ASSERT(pg_string_ends_with(PG_DYN_SLICE(PgString, sb), PG_S(",")));
  PG_DYN_POP(&sb);
  PG_DYN_APPEND_SLICE(&sb, PG_S("}\n"), allocator);

  return PG_DYN_SLICE(PgString, sb);
}
#endif

[[nodiscard]] static PgArena pg_arena_make_from_mem(u8 *data, u64 len) {
  PgArena arena = {
      .start = data,
      .end = data + len,
      .start_original = data,
  };

  return arena;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_log_make_log_line_logfmt(u8 *mem, u64 mem_len, PgLogger *logger,
                            PgLogLevel level, PgString msg, i32 args_count,
                            ...) {
  PgArena arena = pg_arena_make_from_mem(mem, mem_len);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Ignore clock errors.
  u64 monotonic_ns =
      pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC).res - logger->monotonic_epoch;
  u64 timestamp_ns = pg_time_ns_now(PG_CLOCK_KIND_REALTIME).res;

  // FIXME: `try` alloc.
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 256, allocator);
  PgWriter w = pg_writer_make_from_string_builder(&sb, allocator);

  PG_ASSERT(0 == pg_writer_write_all_string(&w, PG_S("level=")));
  PG_ASSERT(0 == pg_writer_write_all_string(&w, pg_log_level_to_string(level)));

  PG_ASSERT(0 == pg_writer_write_all_string(&w, PG_S(" timestamp_ns=")));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, timestamp_ns));

  PG_ASSERT(0 == pg_writer_write_all_string(&w, PG_S(" monotonic_ns=")));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, monotonic_ns));

  PG_ASSERT(0 == pg_writer_write_all_string(&w, PG_S(" message=")));
  PG_ASSERT(0 == pg_writer_write_all_string(
                     &w, pg_logfmt_escape_string(msg, allocator)));

  va_list argp = {0};
  va_start(argp, args_count);
  for (i32 i = 0; i < args_count; i++) {
    PgLogEntry entry = va_arg(argp, PgLogEntry);
    PG_ASSERT(0 == pg_writer_write_u8(&w, ' '));
    PG_ASSERT(0 == pg_writer_write_all_string(&w, entry.key));
    PG_ASSERT(0 == pg_writer_write_u8(&w, '='));

    switch (entry.value.kind) {
    case PG_LOG_VALUE_STRING: {
      PG_ASSERT(0 ==
                pg_writer_write_all_string(
                    &w, pg_logfmt_escape_string(entry.value.s, allocator)));
      break;
    }
    case PG_LOG_VALUE_U64:
      PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, entry.value.n64));
      break;
    case PG_LOG_VALUE_I64:
      PG_ASSERT(0 == pg_writer_write_i64_as_string(&w, entry.value.s64));
      break;
    case PG_LOG_VALUE_IPV4_ADDRESS: {
      PgString ipv4_addr_str =
          pg_net_ipv4_address_to_string(entry.value.ipv4_address, allocator);
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

#if 0
[[maybe_unused]] [[nodiscard]] static PgString
pg_json_encode_string_slice(PgStringSlice strings, PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, strings.len * 128, allocator);
  *PG_DYN_PUSH(&sb, allocator) = '[';

  for (u64 i = 0; i < strings.len; i++) {
    PgString s = PG_SLICE_AT(strings, i);
    PgString encoded = pg_json_escape_string(s, allocator);
    PG_DYN_APPEND_SLICE(&sb, encoded, allocator);

    if (i + 1 < strings.len) {
      *PG_DYN_PUSH(&sb, allocator) = ',';
    }
  }

  *PG_DYN_PUSH(&sb, allocator) = ']';

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
    PgString unescaped = pg_json_unescape_string(str, allocator);
    *PG_DYN_PUSH(&dyn, allocator) = unescaped;

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

static void pg_heap_node_swap(PgHeap *heap, PgHeapNode *parent,
                              PgHeapNode *child) {
  PG_ASSERT(parent);
  PG_ASSERT(child);
  PG_ASSERT(child->parent == parent);

  PgHeapNode parent_before = *parent;
  PgHeapNode child_before = *child;

  // Fix the `parent` field of the grand-children nodes.
  if (child_before.left) {
    child->left->parent = parent;
  }
  if (child_before.right) {
    child->right->parent = parent;
  }

  // Fix the `parent` field of the sibling node.
  PgHeapNode *sibling =
      (child == parent_before.left) ? parent_before.right : parent_before.left;
  if (sibling) {
    sibling->parent = child;
  }

  // Is the old parent the root node?
  // Then the new parent (i.e. `child`) should now be the min-heap root.
  if (nullptr == parent_before.parent) {
    heap->root = child;
  } else if (parent_before.parent && parent_before.parent->left == parent) {
    // Fix grand-parent left|right.
    // Parent is the left node of grand-parent.
    parent->parent->left = child;
  } else if (parent_before.parent && parent_before.parent->right == parent) {
    // Parent is the right node of grand-parent.
    parent->parent->right = child;
  }

  // Swap parent and child.
  parent->left = child_before.left;
  parent->right = child_before.right;
  parent->parent = child;
  child->parent = parent_before.parent;

  if (parent_before.left == child) {
    child->left = parent;
    child->right = parent_before.right;
  } else {
    child->left = parent_before.left;
    child->right = parent;
  }

  // Grand-kids are the same as before.
  PG_ASSERT(parent->left == child_before.left);
  PG_ASSERT(parent->right == child_before.right);

  // The old parent (i.e. new child) has the old child (i.e.e new parent) as its
  // parent.
  PG_ASSERT(parent->parent == child);

  if (child == parent_before.left) {
    PG_ASSERT(child->left == parent);
  } else {
    PG_ASSERT(child->right == parent);
  }

  // Grand-parent is the same as before.
  PG_ASSERT(child->parent == parent_before.parent);
}

// Compute the path from the min (root) to the max (the left-most node
// of the bottom row).
// The path is stored in binary, from root to bottom with:
// - 0: left
// - 1: right
// The least significant bit stores the root, etc.
// And `path_len` stores the path length (since it could be all zeroes, we
// cannot compute the path length after the fact with bit operations).
static void pg_heap_compute_path_from_root_to_left_most_bottom(u64 items_count,
                                                               u64 *path,
                                                               u64 *path_len) {
  u64 n = 0;
  *path = 0;

  for (*path_len = 0, n = items_count; n >= 2; *path_len += 1, n /= 2) {
    *path = (*path << 1) | (n & 1);
  }
  PG_ASSERT(*path_len <= items_count);
}

[[maybe_unused]]
static void pg_heap_insert(PgHeap *heap, PgHeapNode *node,
                           PgHeapLessThanFn less_than) {
  PG_ASSERT(node);
  PG_ASSERT(less_than);

  //  1. Add the element to the bottom level of the heap at the leftmost open
  //  space.
  //  2. Compare the added element with its parent; if they are in the correct
  //  order, stop.
  //  3. If not, swap the element with its parent and return to the previous
  //  step.

  u64 path = 0;
  u64 path_len = 0;
  pg_heap_compute_path_from_root_to_left_most_bottom(heap->count + 1, &path,
                                                     &path_len);

  PgHeapNode **parent = &heap->root;
  PgHeapNode **child = parent;

  while (path_len > 0) {
    parent = child;
    // TODO: Could use a (2 entries) lookup table, or a simple pointer offset
    // math, to make it branchless.
    if (path & 1) {
      child = &(*child)->right;
    } else {
      child = &(*child)->left;
    }
    path >>= 1;
    path_len -= 1;
  }

  *node = (PgHeapNode){0};
  node->parent = *parent;
  *child = node;
  heap->count += 1;

  while (node->parent && less_than(node, node->parent)) {
    pg_heap_node_swap(heap, node->parent, node);
  }

  PG_ASSERT(heap->root);
}

[[maybe_unused]]
static void pg_heap_node_sanity_check(PgHeapNode *node,
                                      PgHeapLessThanFn less_than) {
  PG_ASSERT(less_than);

  if (!node) {
    return;
  }

  if (node->parent) {
    PG_ASSERT(less_than(node->parent, node));
  }

  // > if the last level of the tree is not complete, the nodes of that level
  // are filled from left to right.
  PG_ASSERT(!(node->right && !node->left));

  // > all levels of the tree, except possibly the last one (deepest) are fully
  // filled

  PgHeapNode *it = node->parent;
  while (it) {
    PG_ASSERT(it->left && it->right);

    it = it->parent;
  }
}

typedef bool (*PgHeapIterFn)(PgHeapNode *node, u64 depth, bool left, void *ctx);

[[maybe_unused]] static void pg_heap_node_iter(PgHeapNode *node,
                                               PgHeapIterFn iter_fn, u64 depth,
                                               bool left, void *ctx) {
  if (!node) {
    return;
  }

  if (!iter_fn(node, depth, left, ctx)) {
    return;
  }

  pg_heap_node_iter(node->left, iter_fn, depth + 1, true, ctx);
  pg_heap_node_iter(node->right, iter_fn, depth + 1, false, ctx);
}

[[maybe_unused]] static void pg_heap_node_remove(PgHeap *heap, PgHeapNode *node,
                                                 PgHeapLessThanFn less_than) {
  PG_ASSERT(heap);
  if (0 == heap->count) {
    return;
  }

  PG_ASSERT(heap->root);
  PG_ASSERT(nullptr == heap->root->parent);
  PG_ASSERT(node);
  PG_ASSERT(less_than);

  //    Deleting the root:
  //    1. Replace the root of the heap with the last element on the last level.
  //    2. Compare the new root with its children; if they are in the correct
  //    order, stop.
  //    3. If not, swap the element with one of its children and
  //    return to the previous step. (Swap with its smaller child in a min-heap
  //    and its larger child in a max-heap.)
  //
  // Deleting an arbitrary element can be done as follows:
  //
  //     1. Swap the element we want to delete with the last element. Remove the
  //     last element after the swap.
  //     2. Down-heapify or up-heapify to restore the heap property. In a
  //     min-heap, up-heapify is only required when the new key of
  //     element i is smaller than the previous one
  //     because only the heap-property of the parent element might be violated.
  //     Assuming that the heap-property was valid between element i
  //     and its children before the element swap, it can't be
  //     violated by a now smaller key value. When the new key is
  //     greater than the previous one then only a down-heapify is required
  //     because the heap-property might only be violated in the child elements.

  u64 path = 0;
  u64 path_len = 0;
  pg_heap_compute_path_from_root_to_left_most_bottom(heap->count, &path,
                                                     &path_len);

  PgHeapNode **max = &heap->root;

  while (path_len > 0) {
    // TODO: Could use a (2 entries) lookup table, or a simple pointer offset
    // math, to make it branchless.
    if (path & 1) {
      max = &(*max)->right;
    } else {
      max = &(*max)->left;
    }
    path >>= 1;
    path_len -= 1;
  }

  heap->count -= 1;

  // Unlink the max node.
  PgHeapNode *child = *max;
  *max = nullptr;

  // Removing either the max node or the last node in the tree?
  if (child == node) {
    if (child == heap->root) {
      PG_ASSERT(0 == heap->count);
      heap->root = nullptr;
    }
    // Nothing else to do.
    return;
  }

  // Replace the node to be deleted with the max node.
  {
    child->left = node->left;
    child->right = node->right;
    child->parent = node->parent;

    // Fix-up the `parent` field of children.
    if (child->left) {
      child->left->parent = child;
    }
    if (child->right) {
      child->right->parent = child;
    }

    // Fix the `left|right` fields of the parent.
    if (!node->parent) { // We are removing the root.
      heap->root = child;
    } else if (node->parent->left ==
               node) { // Node is the left child of its parent.
      node->parent->left = child;
    } else if (node->parent->right ==
               node) { // Node is the right child of its parent.
      node->parent->right = child;
    }
  }

  // Walk down the subtree and ensure that `parent < child`.
  // If it's not the case, swap parent and child.
  for (;;) {
    PgHeapNode *smallest = child;
    if (child->left && less_than(child->left, smallest)) {
      smallest = child->left;
    }

    if (child->right && less_than(child->right, smallest)) {
      smallest = child->right;
    }

    if (smallest == child) {
      break;
    }

    pg_heap_node_swap(heap, child, smallest);
  }

  // `max` node is actually not guaranteed to be the global maximum in the tree.
  // So we need to walk up the tree and ensure that `parent < child`.

  while (child->parent && less_than(child, child->parent)) {
    pg_heap_node_swap(heap, child->parent, child);
  }
}

[[maybe_unused]] static void pg_heap_dequeue(PgHeap *heap,
                                             PgHeapLessThanFn less_than) {
  pg_heap_node_remove(heap, heap->root, less_than);
}

#endif
