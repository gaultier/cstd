#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"

#ifndef CSTD_LIB_C
#define CSTD_LIB_C

// TODO: Document all functions.
// TODO: Get machine stack trace.
// TODO: Get human readable stack trace.
// TODO: IPv6.
// TODO: *Pool allocator?*
// TODO: Randomize arena guard pages.
// TODO: HTTP compression (gzip, etc)
// TODO: TLS 1.3
// TODO: Pprof/Chromium Trace Format memory profiling.
// TODO: [Unix] Human-readable stacktrace.
// TODO: Get PIE offset for better call stack.
// TODO: Test untested functions (add test coverage to build).
// TODO: [Windows] Unix/Windows parity.

#if defined(__linux__)
#define PG_OS_LINUX
#endif

#if defined(__FreeBSD__)
#define PG_OS_FREEBSD
#endif

#if defined(__APPLE__)
#define PG_OS_APPLE
#define _DARWIN_C_SOURCE
#endif

#if defined(PG_OS_LINUX) || defined(PG_OS_FREEBSD) || defined(PG_OS_APPLE) ||  \
    defined(__unix__)
#define PG_OS_UNIX
#endif

#if defined(PG_OS_UNIX)
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 1
#endif

#if defined(PG_OS_LINUX)
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/sendfile.h>
#endif

#include "sha1.c"
#include <inttypes.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PG_OS_UNIX
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#endif

#ifdef PG_OS_UNIX
#define PG_PATH_SEPARATOR '/'
#define PG_PATH_SEPARATOR_S "/"
#else
#define PG_PATH_SEPARATOR '\\'
#define PG_PATH_SEPARATOR_S "\\"
#endif

// Check that __COUNTER__ is defined and that __COUNTER__ increases by 1
// every time it is expanded. X + 1 == X + 0 is used in case X is defined to be
// empty. If X is empty the expression becomes (+1 == +0).
#if defined(__COUNTER__) && (__COUNTER__ + 1 == __COUNTER__ + 0)
#define PG_PRIVATE_UNIQUE_ID __COUNTER__
#else
#define PG_PRIVATE_UNIQUE_ID __LINE__
#endif

// Helpers for generating unique variable names
#define PG_PRIVATE_NAME(n) PG_PRIVATE_CONCAT(n, PG_PRIVATE_UNIQUE_ID)
#define PG_PRIVATE_CONCAT(a, b) PG_PRIVATE_CONCAT2(a, b)
#define PG_PRIVATE_CONCAT2(a, b) a##b
#define PG_PAD(n) u8 PG_PRIVATE_NAME(_padding)[n]

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

#define PG_BIT_CLEAR(n, idx) (n & ~(1 << (idx)))
#define PG_BIT_SET(n, idx) (n | (1 << (idx)))
#define PG_BIT_TOGGLE(n, idx) (n ^ (1 << (idx)))

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
typedef __uint128_t u128;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef __int128_t i128;

typedef size_t usize;
typedef ssize_t isize;

typedef float f32;
typedef double f64;

#define PG_RESULT(T)                                                           \
  typedef struct {                                                             \
    PgError err;                                                               \
    T value;                                                                   \
  }

#define PG_OPTION(T)                                                           \
  typedef struct {                                                             \
    T value;                                                                   \
    bool has_value;                                                            \
  }

// TODO: Separate error type?
typedef u64 PgError;
#define PG_ERR_EOF 4095
#ifdef PG_OS_UNIX
#define PG_ERR_INVALID_VALUE EINVAL
#define PG_ERR_IO EIO
#define PG_ERR_TOO_BIG E2BIG
#define PG_ERR_EAGAIN EAGAIN
#else
// Use the x86_64 Linux errno values.
#define PG_ERR_INVALID_VALUE 22
#define PG_ERR_IO 5
#define PG_ERR_TOO_BIG 7
#endif

#define PG_ERR_CLI_MISSING_REQUIRED_OPTION 0xff'00'00
#define PG_ERR_CLI_MISSING_REQUIRED_OPTION_VALUE 0xff'00'01
#define PG_ERR_CLI_UNKNOWN_OPTION 0xff'00'02
#define PG_ERR_CLI_FORBIDEN_OPTION_VALUE 0xff'00'03
#define PG_ERR_CLI_MALFORMED_OPTION 0xff'00'04

PG_RESULT(u8) Pgu8Result;
PG_RESULT(u16) Pgu16Result;
PG_RESULT(u32) Pgu32Result;
PG_RESULT(u64) Pgu64Result;

PG_RESULT(i8) Pgi8Result;
PG_RESULT(i16) Pgi16Result;
PG_RESULT(i32) Pgi32Result;
PG_RESULT(i64) Pgi64Result;

PG_RESULT(bool) PgBoolResult;
PG_OPTION(bool) PgBoolOption;
PG_SLICE(bool) PgBoolSlice;
PG_DYN(bool) PgBoolDyn;

PG_RESULT(void *) PgVoidPtrResult;

PG_OPTION(u8) Pgu8Option;
PG_OPTION(u16) Pgu16Option;
PG_OPTION(u32) Pgu32Option;
PG_OPTION(u64) Pgu64Option;

PG_OPTION(i8) Pgi8Option;
PG_OPTION(i16) Pgi16Option;
PG_OPTION(i32) Pgi32Option;
PG_OPTION(i64) Pgi64Option;

PG_DYN(u8) Pgu8Dyn;
PG_DYN(u16) Pgu16Dyn;
PG_DYN(u32) Pgu32Dyn;
PG_DYN(u64) Pgu64Dyn;

PG_DYN(i8) Pgi8Dyn;
PG_DYN(i16) Pgi16Dyn;
PG_DYN(i32) Pgi32Dyn;
PG_DYN(i64) Pgi64Dyn;

PG_SLICE(u8) Pgu8Slice;
PG_SLICE(u16) Pgu16Slice;
PG_SLICE(u32) Pgu32Slice;
PG_SLICE(u64) Pgu64Slice;

PG_SLICE(i8) Pgi8Slice;
PG_SLICE(i16) Pgi16Slice;
PG_SLICE(i32) Pgi32Slice;
PG_SLICE(i64) Pgi64Slice;

PG_DYN(char *) PgCstrDyn;
typedef Pgu8Slice PgString;
PG_DYN(PgString) PgStringDyn;
PG_RESULT(PgStringDyn) PgStringDynResult;

PG_SLICE(void) PgAnySlice;
PG_DYN(void) PgAnyDyn;

PG_OPTION(Pgu8Slice) Pgu8SliceOption;
PG_RESULT(Pgu8Slice) Pgu8SliceResult;

PG_RESULT(Pgu64Option) Pgu64OptionResult;

PG_RESULT(f64) Pgf64Result;
PG_OPTION(f64) Pgf64Option;
PG_SLICE(f64) Pgf64Slice;
PG_DYN(f64) Pgf64Dyn;

#define PG_STATIC_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define PG_SLICE_FROM_C(arr) {.data = arr, .len = PG_STATIC_ARRAY_LEN(arr)}

// Clamp a value in the range `[min, max]`.
#define PG_CLAMP(min, n, max) ((n) < (min) ? (min) : (n) > (max) ? (max) : n)

#define PG_SUB_SAT(a, b) ((a) > (b) ? ((a) - (b)) : 0)

[[maybe_unused]] [[nodiscard]] static u64 pg_ns_to_ms(u64 ns) {
  return ns / 1'000'000;
}

#define PG_STACKTRACE_MAX 128

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

typedef union {
  int fd;
  void *ptr;
} PgFileDescriptor;
PG_RESULT(PgFileDescriptor) PgFileDescriptorResult;
PG_OPTION(PgFileDescriptor) PgFileDescriptorOption;

typedef struct {
  PgFileDescriptor first, second;
} PgFileDescriptorPair;
PG_RESULT(PgFileDescriptorPair) PgFileDescriptorPairResult;

typedef enum {
  PG_NET_SOCKET_DOMAIN_LOCAL,
  PG_NET_SOCKET_DOMAIN_IPV4,
  PG_NET_SOCKET_DOMAIN_IPV6
  // More...
} PgNetSocketDomain;

typedef enum {
  PG_NET_SOCKET_TYPE_TCP,
  PG_NET_SOCKET_TYPE_UDP,
  // More...
} PgNetSocketType;

typedef enum {
  PG_NET_SOCKET_OPTION_NONE,
  // More..
} PgNetSocketOption;

typedef enum {
  PG_FILE_ACCESS_NONE = 0,
  PG_FILE_ACCESS_READ = 1 << 0,
  PG_FILE_ACCESS_WRITE = 1 << 1,
  PG_FILE_ACCESS_READ_WRITE = 1 << 2,
} PgFileAccess;

static const u64 PG_FILE_ACCESS_ALL =
    PG_FILE_ACCESS_READ | PG_FILE_ACCESS_WRITE | PG_FILE_ACCESS_READ_WRITE;

typedef u32 PgRune;
PG_OPTION(PgRune) PgRuneOption;

typedef struct {
  PgRune rune;
  PgError err;
  bool end;
} PgRuneUtf8Result;

#define PG_UTF8_REPLACEMENT_CHARACTER ((PgRune)0xfffdU)

typedef struct {
  PgString s;
  u64 idx;
} PgUtf8Iterator;

typedef struct {
  PgString s;
  PgString sep;
} PgSplitIterator;

typedef struct {
  PgString left, right;
  bool has_value;
} PgStringCut;

typedef struct {
  Pgu8Slice left, right;
  bool has_value;
} PgBytesCut;

typedef struct {
  PgString left, right;
  bool consumed;
} PgStringPairConsume;

typedef struct {
  PgString left, right;
  bool consumed;
  PgRune matched;
} PgStringPairConsumeAny;

typedef struct {
  u64 n;
  bool present;
  PgString remaining;
} PgParseNumberResult;

typedef struct {
  u8 *start;
  u8 *end;

  // For stats
  u8 *start_original;
  // For releasing the arena.
  u8 *os_start;
  u64 os_alloc_size;
} PgArena;

typedef struct {
  PgAllocFn alloc_fn;
  PgReallocFn realloc_fn;
  PgFreeFn free_fn;
} PgHeapAllocator;
static_assert(sizeof(PgHeapAllocator) == sizeof(PgAllocator));

typedef struct {
  PgAllocFn alloc_fn;
  PgReallocFn realloc_fn;
  PgFreeFn free_fn;
  PgArena *arena;
} PgArenaAllocator;
static_assert(sizeof(PgArenaAllocator) >= sizeof(PgAllocator));

typedef enum {
  PG_CMP_LESS = -1,
  PG_CMP_EQ = 0,
  PG_CMP_GREATER = 1,
} PgCompare;

// Ring buffer.
// Invariants:
// - `idx_read < data.len`
// - `idx_write < data.len`
// - `count <= data.len`
// - Empty <=> `count == 0`.
// - No empty slot to signal something
typedef struct {
  u64 idx_read, idx_write;
  PgString data;
  u64 count;
} PgRing;

typedef enum {
  PG_READER_KIND_NONE,
  PG_READER_KIND_BYTES,
  PG_READER_KIND_FILE,
  PG_READER_KIND_SOCKET,
} PgReaderKind;

typedef struct {
  PgReaderKind kind;
  union {
    Pgu8Slice bytes;
    PgFileDescriptor file;
    PgFileDescriptor socket;
  } u;
  PgRing ring;
} PgReader;

typedef enum {
  PG_WRITER_KIND_NONE, // no-op.
  PG_WRITER_KIND_FILE,
  PG_WRITER_KIND_BYTES,
  PG_WRITER_KIND_SOCKET,
} PgWriterKind;

typedef struct {
  PgWriterKind kind;
  union {
    PgFileDescriptor file;
    Pgu8Dyn bytes;
    PgFileDescriptor socket;
  } u;
  // In case of buffered writer;
  PgRing ring;
} PgWriter;

typedef struct {
  u8 data[PG_SHA1_DIGEST_LENGTH];
} PgSha1;

typedef Pgu64Result (*WriteFn)(PgWriter *w, Pgu8Slice src,
                               PgAllocator *allocator);
typedef PgError (*FlushFn)(PgWriter *w);
typedef PgError (*CloseFn)(void *self);

typedef struct {
  u32 ip;   // Host order.
  u16 port; // Host order.
} PgIpv4Address;
PG_DYN(PgIpv4Address) PgIpv4AddressDyn;
PG_SLICE(PgIpv4Address) PgIpv4AddressSlice;

// Example: nodes=6
// Matrix:
//   | 0 1 2 3 4 5
// ---------------
// 0 | x
// 1 | 0 x
// 2 | 1 0 x
// 3 | 0 1 1 x
// 4 | 0 0 0 0 x
// 5 | 1 1 1 1 1 x
// => Bitfield: [0 1 0 0 1 1 0 0 0 0 1 1 1 1 1] .
typedef struct {
  u64 nodes_count, bitfield_len;
  // Stores the lower triangular half (without diagonal).
  PgString bitfield;
} PgAdjacencyMatrix;

typedef struct {
  PgAdjacencyMatrix matrix;
  u64 row, col;
  bool scan_mode_column;
} PgAdjacencyMatrixNeighborIterator;

typedef struct {
  u64 row, col, node;
  bool edge;
  bool has_value;
} PgAdjacencyMatrixNeighbor;

typedef enum {
  PG_CLOCK_KIND_MONOTONIC,
  PG_CLOCK_KIND_REALTIME,
} PgClockKind;

typedef struct {
  u64 state;
} PgRng;

typedef enum [[clang::flag_enum]] {
  PG_VIRTUAL_MEM_FLAGS_NONE = 0,
  PG_VIRTUAL_MEM_FLAGS_READ = 1,
  PG_VIRTUAL_MEM_FLAGS_WRITE = 2,
  PG_VIRTUAL_MEM_FLAGS_EXEC = 4,
} PgVirtualMemFlags;

typedef struct {
  u64 start_incl, end_excl, idx;
} Pgu64Range;
PG_OPTION(Pgu64Range) Pgu64RangeOption;

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

typedef struct {
  PgIpv4Address address;
  PgFileDescriptor socket;
} PgIpv4AddressSocket;
PG_RESULT(PgIpv4AddressSocket) PgDnsResolveIpv4AddressSocketResult;

typedef struct {
  PgIpv4Address address;
  PgFileDescriptor socket;
  PgError err;
} PgIpv4AddressAcceptResult;

typedef enum {
  PG_HTTP_METHOD_UNKNOWN,
  PG_HTTP_METHOD_OPTIONS,
  PG_HTTP_METHOD_GET,
  PG_HTTP_METHOD_HEAD,
  PG_HTTP_METHOD_POST,
  PG_HTTP_METHOD_PUT,
  PG_HTTP_METHOD_DELETE,
  PG_HTTP_METHOD_TRACE,
  PG_HTTP_METHOD_CONNECT,
  PG_HTTP_METHOD_EXTENSION,
} PgHttpMethod;

typedef struct {
  PgString key, value;
} PgStringKeyValue;
PG_RESULT(PgStringKeyValue) PgStringKeyValueResult;
PG_DYN(PgStringKeyValue) PgStringKeyValueDyn;
PG_SLICE(PgStringKeyValue) PgStringKeyValueSlice;
PG_RESULT(PgStringKeyValueDyn) PgStringDynKeyValueResult;

typedef struct {
  PgString scheme;
  PgString username, password;
  PgString host; // Including subdomains.
  PgStringDyn path_components;
  PgStringKeyValueDyn query_parameters;
  u16 port;
  // TODO: fragment.
} PgUrl;

typedef struct {
  PgUrl url; // Does not have a scheme, domain, port.
  PgHttpMethod method;
  PgStringKeyValueDyn headers;
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
  PgStringKeyValueDyn headers;
} PgHttpResponse;

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

typedef struct {
  bool done;
  PgHttpResponse resp;
  PgError err;
} PgHttpResponseReadResult;

typedef struct {
  bool done;
  PgHttpRequest req;
  PgError err;
} PgHttpRequestReadResult;

typedef enum {
  PG_LOG_VALUE_STRING,
  PG_LOG_VALUE_U64,
  PG_LOG_VALUE_I64,
  PG_LOG_VALUE_IPV4_ADDRESS,
  // TODO: IPV6_ADDRESS
} PgLogValueKind;

typedef enum {
  PG_LOG_LEVEL_DEBUG,
  PG_LOG_LEVEL_INFO,
  PG_LOG_LEVEL_ERROR,
} PgLogLevel;

typedef enum {
  PG_LOG_FORMAT_LOGFMT,
} PgLogFormat;

typedef struct {
  PgLogLevel level;
  PgWriter writer;
  PgLogFormat format;
  u64 monotonic_epoch;
  // NOTE: Normally we do not store an allocator in a struct,
  // we prefer to pass it explicitly.
  // But for conciness when logging, we make an exception.
  PgAllocator *allocator;
} PgLogger;

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

typedef struct {
  u8 value[16];
  u8 version;
} PgUuid;

typedef enum {
  PG_HTML_TOKEN_KIND_NONE,
  PG_HTML_TOKEN_KIND_TEXT,
  PG_HTML_TOKEN_KIND_TAG_OPENING,
  PG_HTML_TOKEN_KIND_TAG_CLOSING,
  PG_HTML_TOKEN_KIND_ATTRIBUTE,
  PG_HTML_TOKEN_KIND_COMMENT,
  PG_HTML_TOKEN_KIND_DOCTYPE,
} PgHtmlTokenKind;

typedef struct {
  PgHtmlTokenKind kind;
  u32 start, end;
  union {
#if 0
    PgKeyValue attribute;
#endif
    PgString tag;
    PgString doctype;
    PgString text;
    PgString comment;
  };
} PgHtmlToken;
PG_RESULT(PgHtmlToken) PgHtmlTokenResult;
PG_DYN(PgHtmlToken) PgHtmlTokenDyn;
PG_SLICE(PgHtmlToken) PgHtmlTokenSlice;
PG_RESULT(PgHtmlTokenDyn) PgHtmlTokenDynResult;

typedef enum {
  PG_HTML_PARSE_ERROR_NONE = 0,
  PG_HTML_PARSE_ERROR_INCORRECTLY_CLOSED_COMMENT = 0x600,
  PG_HTML_PARSE_ERROR_INVALID_FIRST_CHARACTER_OF_TAG_NAME = 0x601,
  PG_HTML_PARSE_ERROR_EOF_IN_TAG = 0x602,
  PG_HTML_PARSE_ERROR_UNEXPECTED_CHARACTER_IN_ATTRIBUTE_NAME = 0x603,
  PG_HTML_PARSE_ERROR_UNEXPECTED_EQUALS_SIGN_BEFORE_ATTRIBUTE_NAME = 0x604,
  PG_HTML_PARSE_ERROR_EOF_IN_DOCTYPE = 0x605,
  PG_HTML_PARSE_ERROR_MISSING_WHITESPACE_BEFORE_DOCTYPE_NAME = 0x606,
  PG_HTML_PARSE_ERROR_MISSING_DOCTYPE_NAME = 0x607,
  PG_HTML_PARSE_ERROR_EOF_IN_COMMENT = 0x608,
} PgHtmlParseError;

typedef struct PgLinkedListNode PgLinkedListNode;
struct PgLinkedListNode {
  PgLinkedListNode *next;
};

typedef struct PgHtmlNode PgHtmlNode;
struct PgHtmlNode {
  PgHtmlToken token_start, token_end;
  PgLinkedListNode parent, next_sibling, first_child;
};
PG_RESULT(PgHtmlNode *) PgHtmlNodePtrResult;

// TODO: Remove and use enum values instead.
static const u32 PgElfProgramHeaderTypeLoad = 1;
static const u32 PgElfProgramHeaderFlagsExecutable = 1;
static const u32 PgElfProgramHeaderFlagsReadable = 4;

typedef enum : u32 {
  PG_ELF_SECTION_HEADER_KIND_NULL = 0,
  PG_ELF_SECTION_HEADER_KIND_PROGBITS = 1,
  PG_ELF_SECTION_HEADER_KIND_SYMTAB = 2,
  PG_ELF_SECTION_HEADER_KIND_STRTAB = 3,
  PG_ELF_SECTION_HEADER_KIND_RELA = 4,
  PG_ELF_SECTION_HEADER_KIND_HASH = 5,
  PG_ELF_SECTION_HEADER_KIND_DYNAMIC = 6,
  PG_ELF_SECTION_HEADER_KIND_NOTE = 7,
  PG_ELF_SECTION_HEADER_KIND_NOBITS = 8,
  PG_ELF_SECTION_HEADER_KIND_REL = 9,
  PG_ELF_SECTION_HEADER_KIND_SHLIB = 10,
  PG_ELF_SECTION_HEADER_KIND_DYNSYM = 11,
} PgElfSectionHeaderKind;

typedef struct {
  u32 name;
  u8 info;
  u8 other;
  u16 section_header_table_index;
  u64 value;
  u64 size;
} PgElfSymbolTableEntry;
static_assert(24 == sizeof(PgElfSymbolTableEntry));
PG_DYN(PgElfSymbolTableEntry) PgElfSymbolTableEntryDyn;

typedef enum : u8 {
  PG_ELF_SYMBOL_BIND_LOCAL = 0,
  PG_ELF_SYMBOL_BIND_GLOBAL = 1,
  PG_ELF_SYMBOL_BIND_WEAK = 2,
  PG_ELF_SYMBOL_BIND_LOPROC = 13,
  PG_ELF_SYMBOL_BIND_HIPROC = 15,
} PgElfSymbolBind;

typedef enum : u8 {
  PG_ELF_SYMBOL_TYPE_NONE = 0,
  PG_ELF_SYMBOL_TYPE_OBJECT = 1,
  PG_ELF_SYMBOL_TYPE_FUNC = 2,
  PG_ELF_SYMBOL_TYPE_SECTION = 3,
  PG_ELF_SYMBOL_TYPE_FILE = 4,
  PG_ELF_SYMBOL_TYPE_LOPROC = 13,
  PG_ELF_SYMBOL_TYPE_HIPROC = 15,
} PgElfSymbolType;

typedef enum : u64 {
  PG_ELF_SECTION_HEADER_FLAG_WRITE = 1 << 0,
  PG_ELF_SECTION_HEADER_FLAG_ALLOC = 1 << 1,
  PG_ELF_SECTION_HEADER_FLAG_EXECINSTR = 1 << 2,
  PG_ELF_SECTION_HEADER_FLAG_MASKPROC = 0xf0000000,
} PgElfSectionHeaderFlag;

typedef struct {
  u32 type;
  u32 flags;
  u64 p_offset;
  u64 p_vaddr;
  u64 p_paddr;
  u64 p_filesz;
  u64 p_memsz;
  u64 alignment;
} PgElfProgramHeader;
static_assert(56 == sizeof(PgElfProgramHeader));
PG_DYN(PgElfProgramHeader) PgElfProgramHeaderDyn;

typedef struct {
  u32 name;
  PgElfSectionHeaderKind kind;
  u64 flags;
  u64 addr;
  u64 offset;
  u64 size;
  u32 link;
  u32 info;
  u64 align;
  u64 entsize;
} PgElfSectionHeader;
static_assert(64 == sizeof(PgElfSectionHeader));
PG_DYN(PgElfSectionHeader) PgElfSectionHeaderDyn;
PG_SLICE(PgElfSectionHeader) PgElfSectionHeaderSlice;
PG_OPTION(PgElfSectionHeader) PgElfSectionHeaderOption;

typedef enum {
  PG_ELF_KNOWN_SECTION_TEXT,
  PG_ELF_KNOWN_SECTION_RODATA,
  PG_ELF_KNOWN_SECTION_DATA,
  PG_ELF_KNOWN_SECTION_STRTAB,
  PG_ELF_KNOWN_SECTION_RELOC_TEXT,
  PG_ELF_KNOWN_SECTION_SYMTAB,
} PgElfKnownSection;

typedef enum : u8 {
  PG_ELF_ENDIAN_LITTLE = 1,
} PgElfEndianness;

typedef enum : u16 {
  PG_ELF_TYPE_NONE,
  PG_ELF_TYPE_RELOCATABLE_FILE,
  PG_ELF_TYPE_EXECUTABLE_FILE,
  PG_ELF_TYPE_SHARED_OBJECT_FILE,
  PG_ELF_TYPE_CORE_FILE,
} PgElfType;

typedef enum : u8 {
  PG_ELF_CLASS_NONE,
  PG_ELF_CLASS_32_BITS,
  PG_ELF_CLASS_64_BITS,
} PgElfClass;

typedef enum : u16 {
  PG_ELF_ARCH_NONE = 0,
  PG_ELF_ARCH_SPARC = 2,
  PG_ELF_ARCH_386 = 3,
  PG_ELF_ARCH_SPARC_32_PLUS = 18,
  PG_ELF_ARCH_SPARC_V9 = 43,
  PG_ELF_ARCH_AMD64 = 62,
} PgElfArchitecture;

typedef struct {
  u8 magic[4];
  PgElfClass class;
  PgElfEndianness endianness;
  u8 elf_header_version;
  u8 abi_version;
  PG_PAD(8);
  PgElfType type;
  PgElfArchitecture architecture;
  u32 elf_version;
  u64 entrypoint_address;
  u64 program_header_offset;
  u64 section_header_offset;
  u32 cpu_flags;
  u16 header_size;
  u16 program_header_entry_size;
  u16 program_header_entries_count;
  u16 section_header_entry_size;
  u16 section_header_entries_count;
  // The section header table index of the entry that is associated with the
  // section name string table.
  u16 section_header_shstrtab_index;
} PgElfHeader;
static_assert(24 == offsetof(PgElfHeader, entrypoint_address));
static_assert(52 == offsetof(PgElfHeader, header_size));
static_assert(64 == sizeof(PgElfHeader));

typedef struct {
  Pgu8Slice bytes;

  PgElfHeader header;
  PgElfProgramHeaderDyn program_headers;
  PgElfSectionHeaderDyn section_headers;

  // Cache useful section header data.
  PgElfSymbolTableEntryDyn symtab;
  Pgu8Slice strtab_bytes;
  Pgu8Slice shtrtab_bytes;
  Pgu8Slice section_header_text_bytes;
  u32 section_header_text_idx;
} PgElf;
PG_RESULT(PgElf) PgElfResult;

typedef void (*PgHttpHandler)(PgHttpRequest req, PgReader *reader,
                              PgWriter *writer, PgLogger *logger,
                              PgAllocator *allocator, void *ctx);

typedef struct {
  u16 port;
  u64 listen_backlog;
  u64 http_handler_arena_mem;
  PgHttpHandler handler;
  void *ctx;
} PgHttpServerOptions;

PG_RESULT(PgString) PgStringResult;
PG_OPTION(PgString) PgStringOption;
PG_SLICE(PgString) PgStringSlice;

PG_RESULT(PgStringSlice) PgStringSliceResult;

#ifdef PG_OS_UNIX
typedef pthread_t PgThread;
#else
// FIXME
typedef bool PgThread;
#endif

PG_RESULT(PgThread) PgThreadResult;
PG_SLICE(PgThread) PgThreadSlice;
PG_DYN(PgThread) PgThreadDyn;

#ifdef PG_OS_UNIX
typedef pthread_mutex_t PgMutex;
#else
// FIXME
typedef bool PgMutex;
#endif

typedef enum {
  PG_MUTEX_KIND_PLAIN,
  PG_MUTEX_KIND_RECURSIVE,
} PgMutexKind;

#ifdef PG_OS_UNIX
typedef pthread_cond_t PgConditionVar;
#else
// FIXME
typedef bool PgConditionVar;
#endif

typedef i32 (*PgThreadFn)(void *data);

typedef struct PgThreadPoolTask PgThreadPoolTask;
struct PgThreadPoolTask {
  void *data;
  PgThreadPoolTask *next;
  PgThreadFn fn;
};

typedef struct {
  PgThreadSlice workers;

  // Linked list.
  PgThreadPoolTask *tasks;
  PgMutex tasks_mtx;
  PgConditionVar tasks_cnd;

  bool done;
} PgThreadPool;
PG_RESULT(PgThreadPool) PgThreadPoolResult;

typedef enum [[clang::flag_enum]] {
  PG_AIO_EVENT_KIND_NONE = 0,
  PG_AIO_EVENT_KIND_FILE_MODIFIED = 1 << 1,
  PG_AIO_EVENT_KIND_FILE_DELETED = 1 << 2,
  PG_AIO_EVENT_KIND_FILE_CREATED = 1 << 3,
  PG_AIO_EVENT_KIND_READABLE = 1 << 4,
  PG_AIO_EVENT_KIND_WRITABLE = 1 << 5,

  PG_AIO_EVENT_KIND_ERROR = 1 << 6,
  PG_AIO_EVENT_KIND_EOF = 1 << 7,
  // More...
} PgAioEventKind;

typedef struct {
  PgAioEventKind kind;
  u64 user_data;
  PgFileDescriptor fd;
  PgString name;
} PgAioEvent;
PG_DYN(PgAioEvent) PgAioEventDyn;
PG_SLICE(PgAioEvent) PgAioEventSlice;
PG_RESULT(PgAioEvent) PgAioEventResult;
PG_OPTION(PgAioEvent) PgAioEventOption;

typedef struct {
  PgFileDescriptor aio;
#ifdef PG_OS_LINUX
  PgFileDescriptorOption inotify;
#endif
} PgAio;
PG_RESULT(PgAio) PgAioResult;

typedef struct {
#ifdef PG_OS_UNIX
  DIR *dir;
#else
  // TODO: Windows.
#endif
} PgDirectory;
PG_RESULT(PgDirectory) PgDirectoryResult;

#ifdef PG_OS_UNIX
typedef struct dirent *PgDirectoryEntry;
#else
// TODO: Windows.
#endif
PG_RESULT(PgDirectoryEntry) PgDirectoryEntryResult;

typedef enum {
  PG_WALK_DIRECTORY_KIND_NONE = 0,
  PG_WALK_DIRECTORY_KIND_FILE = 1 << 0,
  PG_WALK_DIRECTORY_KIND_DIRECTORY = 1 << 1,
  PG_WALK_DIRECTORY_KIND_RECURSE = 1 << 2,
  PG_WALK_DIRECTORY_KIND_IGNORE_ERRORS = 1 << 3,
} PgWalkDirectoryOption;

typedef struct {
  u32 length;
  u16 version;
  u32 header_length;
  u8 min_instruction_length;
  u8 max_ops_per_inst;
  u8 default_is_stmt;
  i8 line_base;
  u8 line_range;
  u8 opcode_base;
  u8 std_opcode_lengths[12];
} PgDwarfDebugLineHeader;

typedef enum : u8 {
  PG_DWARF_LNS_EXTENDED_OP,
  PG_DWARF_LNS_COPY,
  PG_DWARF_LNS_ADVANCE_PC,
  PG_DWARF_LNS_ADVANCE_LINE,
  PG_DWARF_LNS_SET_FILE,
  PG_DWARF_LNS_SET_COLUMN,
  PG_DWARF_LNS_NEGATE_STMT,
  PG_DWARF_LNS_SET_BASIC_BLOCK,
  PG_DWARF_LNS_CONST_ADD_PC,
  PG_DWARF_LNS_FIXED_ADVANCE_PC,
  PG_DWARF_LNS_SET_PROLOGUE_END,
  PG_DWARF_LNS_SET_EPILOGUE_BEGIN,
  PG_DWARF_LNS_SET_ISA,
} PgDwarfLns;

typedef enum {
  PG_DWARF_TAG_NULL = 0X0000,
  PG_DWARF_TAG_ARRAY_TYPE = 0X0001,
  PG_DWARF_TAG_CLASS_TYPE = 0X0002,
  PG_DWARF_TAG_ENTRY_POINT = 0X0003,
  PG_DWARF_TAG_ENUMERATION_TYPE = 0X0004,
  PG_DWARF_TAG_FORMAL_PARAMETER = 0X0005,
  PG_DWARF_TAG_IMPORTED_DECLARATION = 0X0008,
  PG_DWARF_TAG_LABEL = 0X000A,
  PG_DWARF_TAG_LEXICAL_BLOCK = 0X000B,
  PG_DWARF_TAG_MEMBER = 0X000D,
  PG_DWARF_TAG_POINTER_TYPE = 0X000F,
  PG_DWARF_TAG_REFERENCE_TYPE = 0X0010,
  PG_DWARF_TAG_COMPILE_UNIT = 0X0011,
  PG_DWARF_TAG_STRING_TYPE = 0X0012,
  PG_DWARF_TAG_STRUCTURE_TYPE = 0X0013,
  PG_DWARF_TAG_SUBROUTINE_TYPE = 0X0015,
  PG_DWARF_TAG_TYPEDEF = 0X0016,
  PG_DWARF_TAG_UNION_TYPE = 0X0017,
  PG_DWARF_TAG_UNSPECIFIED_PARAMETERS = 0X0018,
  PG_DWARF_TAG_VARIANT = 0X0019,
  PG_DWARF_TAG_COMMON_BLOCK = 0X001A,
  PG_DWARF_TAG_COMMON_INCLUSION = 0X001B,
  PG_DWARF_TAG_INHERITANCE = 0X001C,
  PG_DWARF_TAG_INLINED_SUBROUTINE = 0X001D,
  PG_DWARF_TAG_MODULE = 0X001E,
  PG_DWARF_TAG_PTR_TO_MEMBER_TYPE = 0X001F,
  PG_DWARF_TAG_SET_TYPE = 0X0020,
  PG_DWARF_TAG_SUBRANGE_TYPE = 0X0021,
  PG_DWARF_TAG_WITH_STMT = 0X0022,
  PG_DWARF_TAG_ACCESS_DECLARATION = 0X0023,
  PG_DWARF_TAG_BASE_TYPE = 0X0024,
  PG_DWARF_TAG_CATCH_BLOCK = 0X0025,
  PG_DWARF_TAG_CONST_TYPE = 0X0026,
  PG_DWARF_TAG_CONSTANT = 0X0027,
  PG_DWARF_TAG_ENUMERATOR = 0X0028,
  PG_DWARF_TAG_FILE_TYPE = 0X0029,
  PG_DWARF_TAG_FRIEND = 0X002A,
  PG_DWARF_TAG_NAMELIST = 0X002B,
  PG_DWARF_TAG_NAMELIST_ITEM = 0X002C,
  PG_DWARF_TAG_PACKED_TYPE = 0X002D,
  PG_DWARF_TAG_SUBPROGRAM = 0X002E,
  PG_DWARF_TAG_TEMPLATE_TYPE_PARAMETER = 0X002F,
  PG_DWARF_TAG_TEMPLATE_VALUE_PARAMETER = 0X0030,
  PG_DWARF_TAG_THROWN_TYPE = 0X0031,
  PG_DWARF_TAG_TRY_BLOCK = 0X0032,
  PG_DWARF_TAG_VARIANT_PART = 0X0033,
  PG_DWARF_TAG_VARIABLE = 0X0034,
  PG_DWARF_TAG_VOLATILE_TYPE = 0X0035,
  PG_DWARF_TAG_DWARF_PROCEDURE = 0X0036,
  PG_DWARF_TAG_RESTRICT_TYPE = 0X0037,
  PG_DWARF_TAG_INTERFACE_TYPE = 0X0038,
  PG_DWARF_TAG_NAMESPACE = 0X0039,
  PG_DWARF_TAG_IMPORTED_MODULE = 0X003A,
  PG_DWARF_TAG_UNSPECIFIED_TYPE = 0X003B,
  PG_DWARF_TAG_PARTIAL_UNIT = 0X003C,
  PG_DWARF_TAG_IMPORTED_UNIT = 0X003D,
  PG_DWARF_TAG_CONDITION = 0X003F,
  PG_DWARF_TAG_SHARED_TYPE = 0X0040,
  PG_DWARF_TAG_TYPE_UNIT = 0X0041,
  PG_DWARF_TAG_RVALUE_REFERENCE_TYPE = 0X0042,
  PG_DWARF_TAG_TEMPLATE_ALIAS = 0X0043,
} PgDwarfTag;

typedef enum : u16 {
  PG_DWARF_AT_NONE = 0,
  PG_DWARF_AT_SIBLING = 0X01,
  PG_DWARF_AT_LOCATION = 0X02,
  PG_DWARF_AT_NAME = 0X03,
  PG_DWARF_AT_ORDERING = 0X09,
  PG_DWARF_AT_BYTE_SIZE = 0X0B,
  PG_DWARF_AT_BIT_OFFSET = 0X0C,
  PG_DWARF_AT_BIT_SIZE = 0X0D,
  PG_DWARF_AT_STMT_LIST = 0X10,
  PG_DWARF_AT_LOW_PC = 0X11,
  PG_DWARF_AT_HIGH_PC = 0X12,
  PG_DWARF_AT_LANGUAGE = 0X13,
  PG_DWARF_AT_DISCR = 0X15,
  PG_DWARF_AT_DISCR_VALUE = 0X16,
  PG_DWARF_AT_VISIBILITY = 0X17,
  PG_DWARF_AT_IMPORT = 0X18,
  PG_DWARF_AT_STRING_LENGTH = 0X19,
  PG_DWARF_AT_COMMON_REFERENCE = 0X1A,
  PG_DWARF_AT_COMP_DIR = 0X1B,
  PG_DWARF_AT_CONST_VALUE = 0X1C,
  PG_DWARF_AT_CONTAINING_TYPE = 0X1D,
  PG_DWARF_AT_DEFAULT_VALUE = 0X1E,
  PG_DWARF_AT_INLINE = 0X20,
  PG_DWARF_AT_IS_OPTIONAL = 0X21,
  PG_DWARF_AT_LOWER_BOUND = 0X22,
  PG_DWARF_AT_PRODUCER = 0X25,
  PG_DWARF_AT_PROTOTYPED = 0X27,
  PG_DWARF_AT_RETURN_ADDR = 0X2A,
  PG_DWARF_AT_START_SCOPE = 0X2C,
  PG_DWARF_AT_BIT_STRIDE = 0X2E,
  PG_DWARF_AT_UPPER_BOUND = 0X2F,
  PG_DWARF_AT_ABSTRACT_ORIGIN = 0X31,
  PG_DWARF_AT_ACCESSIBILITY = 0X32,
  PG_DWARF_AT_ADDRESS_CLASS = 0X33,
  PG_DWARF_AT_ARTIFICIAL = 0X34,
  PG_DWARF_AT_BASE_TYPES = 0X35,
  PG_DWARF_AT_CALLING_CONVENTION = 0X36,
  PG_DWARF_AT_COUNT = 0X37,
  PG_DWARF_AT_DATA_MEMBER_LOCATION = 0X38,
  PG_DWARF_AT_DECL_COLUMN = 0X39,
  PG_DWARF_AT_DECL_FILE = 0X3A,
  PG_DWARF_AT_DECL_LINE = 0X3B,
  PG_DWARF_AT_DECLARATION = 0X3C,
  PG_DWARF_AT_DISCR_LIST = 0X3D,
  PG_DWARF_AT_ENCODING = 0X3E,
  PG_DWARF_AT_EXTERNAL = 0X3F,
  PG_DWARF_AT_FRAME_BASE = 0X40,
  PG_DWARF_AT_FRIEND = 0X41,
  PG_DWARF_AT_IDENTIFIER_CASE = 0X42,
  PG_DWARF_AT_MACRO_INFO = 0X43,
  PG_DWARF_AT_NAMELIST_ITEM = 0X44,
  PG_DWARF_AT_PRIORITY = 0X45,
  PG_DWARF_AT_SEGMENT = 0X46,
  PG_DWARF_AT_SPECIFICATION = 0X47,
  PG_DWARF_AT_STATIC_LINK = 0X48,
  PG_DWARF_AT_TYPE = 0X49,
  PG_DWARF_AT_USE_LOCATION = 0X4A,
  PG_DWARF_AT_VARIABLE_PARAMETER = 0X4B,
  PG_DWARF_AT_VIRTUALITY = 0X4C,
  PG_DWARF_AT_VTABLE_ELEM_LOCATION = 0X4D,
  PG_DWARF_AT_ALLOCATED = 0X4E,
  PG_DWARF_AT_ASSOCIATED = 0X4F,
  PG_DWARF_AT_DATA_LOCATION = 0X50,
  PG_DWARF_AT_BYTE_STRIDE = 0X51,
  PG_DWARF_AT_ENTRY_PC = 0X52,
  PG_DWARF_AT_USE_UTF8 = 0X53,
  PG_DWARF_AT_EXTENSION = 0X54,
  PG_DWARF_AT_RANGES = 0X55,
  PG_DWARF_AT_TRAMPOLINE = 0X56,
  PG_DWARF_AT_CALL_COLUMN = 0X57,
  PG_DWARF_AT_CALL_FILE = 0X58,
  PG_DWARF_AT_CALL_LINE = 0X59,
  PG_DWARF_AT_DESCRIPTION = 0X5A,
  PG_DWARF_AT_BINARY_SCALE = 0X5B,
  PG_DWARF_AT_DECIMAL_SCALE = 0X5C,
  PG_DWARF_AT_SMALL = 0X5D,
  PG_DWARF_AT_DECIMAL_SIGN = 0X5E,
  PG_DWARF_AT_DIGIT_COUNT = 0X5F,
  PG_DWARF_AT_PICTURE_STRING = 0X60,
  PG_DWARF_AT_MUTABLE = 0X61,
  PG_DWARF_AT_THREADS_SCALED = 0X62,
  PG_DWARF_AT_EXPLICIT = 0X63,
  PG_DWARF_AT_OBJECT_POINTER = 0X64,
  PG_DWARF_AT_ENDIANITY = 0X65,
  PG_DWARF_AT_ELEMENTAL = 0X66,
  PG_DWARF_AT_PURE = 0X67,
  PG_DWARF_AT_RECURSIVE = 0X68,
  PG_DWARF_AT_SIGNATURE = 0X69,
  PG_DWARF_AT_MAIN_SUBPROGRAM = 0X6A,
  PG_DWARF_AT_DATA_BIT_OFFSET = 0X6B,
  PG_DWARF_AT_CONST_EXPR = 0X6C,
  PG_DWARF_AT_ENUM_CLASS = 0X6D,
  PG_DWARF_AT_LINKAGE_NAME = 0X6E,
  PG_DWARF_AT_STRING_LENGTH_BIT_SIZE = 0X6F,
  PG_DWARF_AT_STRING_LENGTH_BYTE_SIZE = 0X70,
  PG_DWARF_AT_RANK = 0X71,
  PG_DWARF_AT_STR_OFFSETS_BASE = 0X72,
  PG_DWARF_AT_ADDR_BASE = 0X73,
  PG_DWARF_AT_RNGLISTS_BASE = 0X74,
  PG_DWARF_AT_DWO_ID = 0X75,
  PG_DWARF_AT_DWO_NAME = 0X76,
  PG_DWARF_AT_REFERENCE = 0X77,
  PG_DWARF_AT_RVALUE_REFERENCE = 0X78,
  PG_DWARF_AT_MACROS = 0X79,
  PG_DWARF_AT_CALL_ALL_CALLS = 0X7A,
  PG_DWARF_AT_CALL_ALL_SOURCE_CALLS = 0X7B,
  PG_DWARF_AT_CALL_ALL_TAIL_CALLS = 0X7C,
  PG_DWARF_AT_CALL_RETURN_PC = 0X7D,
  PG_DWARF_AT_CALL_VALUE = 0X7E,
  PG_DWARF_AT_CALL_ORIGIN = 0X7F,
  PG_DWARF_AT_CALL_PARAMETER = 0X80,
  PG_DWARF_AT_CALL_PC = 0X81,
  PG_DWARF_AT_CALL_TAIL_CALL = 0X82,
  PG_DWARF_AT_CALL_TARGET = 0X83,
  PG_DWARF_AT_CALL_TARGET_CLOBBERED = 0X84,
  PG_DWARF_AT_CALL_DATA_LOCATION = 0X85,
  PG_DWARF_AT_CALL_DATA_VALUE = 0X86,
  PG_DWARF_AT_NORETURN = 0X87,
  PG_DWARF_AT_ALIGNMENT = 0X88,
  PG_DWARF_AT_EXPORT_SYMBOLS = 0X89,
  PG_DWARF_AT_DELETED = 0X8A,
  PG_DWARF_AT_DEFAULTED = 0X8B,
  PG_DWARF_AT_LOCLISTS_BASE = 0X8C,
  PG_DWARF_AT_MIPS_LOOP_BEGIN = 0X2002,
  PG_DWARF_AT_MIPS_TAIL_LOOP_BEGIN = 0X2003,
  PG_DWARF_AT_MIPS_EPILOG_BEGIN = 0X2004,
  PG_DWARF_AT_MIPS_LOOP_UNROLL_FACTOR = 0X2005,
  PG_DWARF_AT_MIPS_SOFTWARE_PIPELINE_DEPTH = 0X2006,
  PG_DWARF_AT_MIPS_LINKAGE_NAME = 0X2007,
  PG_DWARF_AT_MIPS_STRIDE = 0X2008,
  PG_DWARF_AT_MIPS_ABSTRACT_NAME = 0X2009,
  PG_DWARF_AT_MIPS_CLONE_ORIGIN = 0X200A,
  PG_DWARF_AT_MIPS_HAS_INLINES = 0X200B,
  PG_DWARF_AT_MIPS_STRIDE_BYTE = 0X200C,
  PG_DWARF_AT_MIPS_STRIDE_ELEM = 0X200D,
  PG_DWARF_AT_MIPS_PTR_DOPETYPE = 0X200E,
  PG_DWARF_AT_MIPS_ALLOCATABLE_DOPETYPE = 0X200F,
  PG_DWARF_AT_MIPS_ASSUMED_SHAPE_DOPETYPE = 0X2010,
  PG_DWARF_AT_MIPS_ASSUMED_SIZE = 0X2011,
  PG_DWARF_AT_SF_NAMES = 0X2101,
  PG_DWARF_AT_SRC_INFO = 0X2102,
  PG_DWARF_AT_MAC_INFO = 0X2103,
  PG_DWARF_AT_SRC_COORDS = 0X2104,
  PG_DWARF_AT_BODY_BEGIN = 0X2105,
  PG_DWARF_AT_BODY_END = 0X2106,
  PG_DWARF_AT_GNU_VECTOR = 0X2107,
  PG_DWARF_AT_GNU_TEMPLATE_NAME = 0X2110,
  PG_DWARF_AT_GNU_ODR_SIGNATURE = 0X210F,
  PG_DWARF_AT_GNU_CALL_SITE_VALUE = 0X2111,
  PG_DWARF_AT_GNU_CALL_SITE_DATA_VALUE = 0X2112,
  PG_DWARF_AT_GNU_CALL_SITE_TARGET = 0X2113,
  PG_DWARF_AT_GNU_CALL_SITE_TARGET_CLOBBERED = 0X2114,
  PG_DWARF_AT_GNU_TAIL_CALL = 0X2115,
  PG_DWARF_AT_GNU_ALL_TAIL_CALL_SITES = 0X2116,
  PG_DWARF_AT_GNU_ALL_CALL_SITES = 0X2117,
  PG_DWARF_AT_GNU_ALL_SOURCE_CALL_SITES = 0X2118,
  PG_DWARF_AT_GNU_MACROS = 0X2119,
  PG_DWARF_AT_GNU_DWO_NAME = 0X2130,
  PG_DWARF_AT_GNU_DWO_ID = 0X2131,
  PG_DWARF_AT_GNU_RANGES_BASE = 0X2132,
  PG_DWARF_AT_GNU_ADDR_BASE = 0X2133,
  PG_DWARF_AT_GNU_PUBNAMES = 0X2134,
  PG_DWARF_AT_GNU_PUBTYPES = 0X2135,
  PG_DWARF_AT_GNU_DISCRIMINATOR = 0X2136,
  PG_DWARF_AT_BORLAND_PROPERTY_READ = 0X3B11,
  PG_DWARF_AT_BORLAND_PROPERTY_WRITE = 0X3B12,
  PG_DWARF_AT_BORLAND_PROPERTY_IMPLEMENTS = 0X3B13,
  PG_DWARF_AT_BORLAND_PROPERTY_INDEX = 0X3B14,
  PG_DWARF_AT_BORLAND_PROPERTY_DEFAULT = 0X3B15,
  PG_DWARF_AT_BORLAND_DELPHI_UNIT = 0X3B20,
  PG_DWARF_AT_BORLAND_DELPHI_CLASS = 0X3B21,
  PG_DWARF_AT_BORLAND_DELPHI_RECORD = 0X3B22,
  PG_DWARF_AT_BORLAND_DELPHI_METACLASS = 0X3B23,
  PG_DWARF_AT_BORLAND_DELPHI_CONSTRUCTOR = 0X3B24,
  PG_DWARF_AT_BORLAND_DELPHI_DESTRUCTOR = 0X3B25,
  PG_DWARF_AT_BORLAND_DELPHI_ANONYMOUS_METHOD = 0X3B26,
  PG_DWARF_AT_BORLAND_DELPHI_INTERFACE = 0X3B27,
  PG_DWARF_AT_BORLAND_DELPHI_ABI = 0X3B28,
  PG_DWARF_AT_BORLAND_DELPHI_RETURN = 0X3B29,
  PG_DWARF_AT_BORLAND_DELPHI_FRAMEPTR = 0X3B30,
  PG_DWARF_AT_BORLAND_CLOSURE = 0X3B31,
  PG_DWARF_AT_LLVM_INCLUDE_PATH = 0X3E00,
  PG_DWARF_AT_LLVM_CONFIG_MACROS = 0X3E01,
  PG_DWARF_AT_LLVM_SYSROOT = 0X3E02,
  PG_DWARF_AT_LLVM_TAG_OFFSET = 0X3E03,
  PG_DWARF_AT_LLVM_APINOTES = 0X3E07,
  PG_DWARF_AT_APPLE_OPTIMIZED = 0X3FE1,
  PG_DWARF_AT_APPLE_FLAGS = 0X3FE2,
  PG_DWARF_AT_APPLE_ISA = 0X3FE3,
  PG_DWARF_AT_APPLE_BLOCK = 0X3FE4,
  PG_DWARF_AT_APPLE_MAJOR_RUNTIME_VERS = 0X3FE5,
  PG_DWARF_AT_APPLE_RUNTIME_CLASS = 0X3FE6,
  PG_DWARF_AT_APPLE_OMIT_FRAME_PTR = 0X3FE7,
  PG_DWARF_AT_APPLE_PROPERTY_NAME = 0X3FE8,
  PG_DWARF_AT_APPLE_PROPERTY_GETTER = 0X3FE9,
  PG_DWARF_AT_APPLE_PROPERTY_SETTER = 0X3FEA,
  PG_DWARF_AT_APPLE_PROPERTY_ATTRIBUTE = 0X3FEB,
  PG_DWARF_AT_APPLE_OBJC_COMPLETE_TYPE = 0X3FEC,
  PG_DWARF_AT_APPLE_PROPERTY = 0X3FED,
  PG_DWARF_AT_APPLE_OBJC_DIRECT = 0X3FEE,
  PG_DWARF_AT_APPLE_SDK = 0X3FEF,
} PgDwarfAttribute;

typedef enum : u8 {
  PG_DWARF_FORM_NONE = 0,
  PG_DWARF_FORM_ADDR = 0X01,
  PG_DWARF_FORM_BLOCK2 = 0X03,
  PG_DWARF_FORM_BLOCK4 = 0X04,
  PG_DWARF_FORM_DATA2 = 0X05,
  PG_DWARF_FORM_DATA4 = 0X06,
  PG_DWARF_FORM_DATA8 = 0X07,
  PG_DWARF_FORM_STRING = 0X08,
  PG_DWARF_FORM_BLOCK = 0X09,
  PG_DWARF_FORM_BLOCK1 = 0X0A,
  PG_DWARF_FORM_DATA1 = 0X0B,
  PG_DWARF_FORM_FLAG = 0X0C,
  PG_DWARF_FORM_SDATA = 0X0D,
  PG_DWARF_FORM_STRP = 0X0E,
  PG_DWARF_FORM_UDATA = 0X0F,
  PG_DWARF_FORM_REF_ADDR = 0X10,
  PG_DWARF_FORM_REF1 = 0X11,
  PG_DWARF_FORM_REF2 = 0X12,
  PG_DWARF_FORM_REF4 = 0X13,
  PG_DWARF_FORM_REF8 = 0X14,
  PG_DWARF_FORM_REF_UDATA = 0X15,
  PG_DWARF_FORM_INDIRECT = 0X16,
  PG_DWARF_FORM_SEC_OFFSET = 0X17,
  PG_DWARF_FORM_EXPRLOC = 0X18,
  PG_DWARF_FORM_FLAG_PRESENT = 0X19,
  PG_DWARF_FORM_REF_SIG8 = 0X20,
} PgDwarfForm;

typedef enum : u8 {
  PG_DWARF_LNE_NONE = 0,
  PG_DWARF_LNE_END_SEQUENCE = 1,
  PG_DWARF_LNE_SET_ADDRESS,
  PG_DWARF_LNE_DEFINE_FILE,
  PG_DWARF_LNE_SET_DISCRIMINATOR,
} PgDwarfLne;

typedef struct {
  PgDwarfAttribute attribute;
  PgDwarfForm form;
  PG_PAD(1);
} PgDwarfAttributeForm;
PG_DYN(PgDwarfAttributeForm) PgDwarfAttributeFormDyn;

typedef struct {
  PgDwarfAttributeFormDyn attribute_forms;
  u8 type;
  u8 tag;
  PG_PAD(6);
} PgDwarfAbbreviationEntry;
PG_DYN(PgDwarfAbbreviationEntry) PgDwarfAbbreviationEntryDyn;

typedef struct {
  PgDwarfAbbreviationEntryDyn entries;
} PgDwarfAbbreviation;

typedef struct {
  u64 low_pc;
  PgString fn_name;
  PgString directory;
  PgString file;
  u16 high_pc;
  u16 line;
  PG_PAD(4);
} PgDwarfFunctionDeclaration;

typedef struct {
  u64 pc;
  u64 file;
  u32 line;
  PG_PAD(4);
} PgDwarfLineEntry;

typedef struct {
  u64 address;
  u64 file;
  u16 line;
  bool is_stmt;
  PG_PAD(5);
  // TODO: track column?
} PgDwarfLineSectionFsm;

// typedef struct {
//   PgString directory;
//   PgString file;
//   PgString fn_name;
//   u64 line;
// } PgStacktraceEntry;

// typedef struct {
//   Pgu8Slice contents;
//   PgString directory;
//   PgString file;
//   Pgu16Slice newline_offsets;
// } source_file_t;

// typedef struct {
//   u64 pie_displacement;
//   pg_array_t(dw_fn_decl_t) fn_decls;
//   pg_array_t(dw_line_entry_t) line_entries;
//   pg_array_t(dw_string_t) debug_str_strings;
//   pg_array_t(char *) debug_line_files;
//   pg_array_t(source_file_t) sources;
// } debug_data_t;

static const char dw_tag_str[][40] = {
    [PG_DWARF_TAG_NULL] = "PG_DWARF_TAG_NULL",
    [PG_DWARF_TAG_ARRAY_TYPE] = "PG_DWARF_TAG_ARRAY_TYPE",
    [PG_DWARF_TAG_CLASS_TYPE] = "PG_DWARF_TAG_CLASS_TYPE",
    [PG_DWARF_TAG_ENTRY_POINT] = "PG_DWARF_TAG_ENTRY_POINT",
    [PG_DWARF_TAG_ENUMERATION_TYPE] = "PG_DWARF_TAG_ENUMERATION_TYPE",
    [PG_DWARF_TAG_FORMAL_PARAMETER] = "PG_DWARF_TAG_FORMAL_PARAMETER",
    [PG_DWARF_TAG_IMPORTED_DECLARATION] = "PG_DWARF_TAG_IMPORTED_DECLARATION",
    [PG_DWARF_TAG_LABEL] = "PG_DWARF_TAG_LABEL",
    [PG_DWARF_TAG_LEXICAL_BLOCK] = "PG_DWARF_TAG_LEXICAL_BLOCK",
    [PG_DWARF_TAG_MEMBER] = "PG_DWARF_TAG_MEMBER",
    [PG_DWARF_TAG_POINTER_TYPE] = "PG_DWARF_TAG_POINTER_TYPE",
    [PG_DWARF_TAG_REFERENCE_TYPE] = "PG_DWARF_TAG_REFERENCE_TYPE",
    [PG_DWARF_TAG_COMPILE_UNIT] = "PG_DWARF_TAG_COMPILE_UNIT",
    [PG_DWARF_TAG_STRING_TYPE] = "PG_DWARF_TAG_STRING_TYPE",
    [PG_DWARF_TAG_STRUCTURE_TYPE] = "PG_DWARF_TAG_STRUCTURE_TYPE",
    [PG_DWARF_TAG_SUBROUTINE_TYPE] = "PG_DWARF_TAG_SUBROUTINE_TYPE",
    [PG_DWARF_TAG_TYPEDEF] = "PG_DWARF_TAG_TYPEDEF",
    [PG_DWARF_TAG_UNION_TYPE] = "PG_DWARF_TAG_UNION_TYPE",
    [PG_DWARF_TAG_UNSPECIFIED_PARAMETERS] =
        "PG_DWARF_TAG_UNSPECIFIED_PARAMETERS",
    [PG_DWARF_TAG_VARIANT] = "PG_DWARF_TAG_VARIANT",
    [PG_DWARF_TAG_COMMON_BLOCK] = "PG_DWARF_TAG_COMMON_BLOCK",
    [PG_DWARF_TAG_COMMON_INCLUSION] = "PG_DWARF_TAG_COMMON_INCLUSION",
    [PG_DWARF_TAG_INHERITANCE] = "PG_DWARF_TAG_INHERITANCE",
    [PG_DWARF_TAG_INLINED_SUBROUTINE] = "PG_DWARF_TAG_INLINED_SUBROUTINE",
    [PG_DWARF_TAG_MODULE] = "PG_DWARF_TAG_MODULE",
    [PG_DWARF_TAG_PTR_TO_MEMBER_TYPE] = "PG_DWARF_TAG_PTR_TO_MEMBER_TYPE",
    [PG_DWARF_TAG_SET_TYPE] = "PG_DWARF_TAG_SET_TYPE",
    [PG_DWARF_TAG_SUBRANGE_TYPE] = "PG_DWARF_TAG_SUBRANGE_TYPE",
    [PG_DWARF_TAG_WITH_STMT] = "PG_DWARF_TAG_WITH_STMT",
    [PG_DWARF_TAG_ACCESS_DECLARATION] = "PG_DWARF_TAG_ACCESS_DECLARATION",
    [PG_DWARF_TAG_BASE_TYPE] = "PG_DWARF_TAG_BASE_TYPE",
    [PG_DWARF_TAG_CATCH_BLOCK] = "PG_DWARF_TAG_CATCH_BLOCK",
    [PG_DWARF_TAG_CONST_TYPE] = "PG_DWARF_TAG_CONST_TYPE",
    [PG_DWARF_TAG_CONSTANT] = "PG_DWARF_TAG_CONSTANT",
    [PG_DWARF_TAG_ENUMERATOR] = "PG_DWARF_TAG_ENUMERATOR",
    [PG_DWARF_TAG_FILE_TYPE] = "PG_DWARF_TAG_FILE_TYPE",
    [PG_DWARF_TAG_FRIEND] = "PG_DWARF_TAG_FRIEND",
    [PG_DWARF_TAG_NAMELIST] = "PG_DWARF_TAG_NAMELIST",
    [PG_DWARF_TAG_NAMELIST_ITEM] = "PG_DWARF_TAG_NAMELIST_ITEM",
    [PG_DWARF_TAG_PACKED_TYPE] = "PG_DWARF_TAG_PACKED_TYPE",
    [PG_DWARF_TAG_SUBPROGRAM] = "PG_DWARF_TAG_SUBPROGRAM",
    [PG_DWARF_TAG_TEMPLATE_TYPE_PARAMETER] =
        "PG_DWARF_TAG_TEMPLATE_TYPE_PARAMETER",
    [PG_DWARF_TAG_TEMPLATE_VALUE_PARAMETER] =
        "PG_DWARF_TAG_TEMPLATE_VALUE_PARAMETER",
    [PG_DWARF_TAG_THROWN_TYPE] = "PG_DWARF_TAG_THROWN_TYPE",
    [PG_DWARF_TAG_TRY_BLOCK] = "PG_DWARF_TAG_TRY_BLOCK",
    [PG_DWARF_TAG_VARIANT_PART] = "PG_DWARF_TAG_VARIANT_PART",
    [PG_DWARF_TAG_VARIABLE] = "PG_DWARF_TAG_VARIABLE",
    [PG_DWARF_TAG_VOLATILE_TYPE] = "PG_DWARF_TAG_VOLATILE_TYPE",
    [PG_DWARF_TAG_DWARF_PROCEDURE] = "PG_DWARF_TAG_DWARF_PROCEDURE",
    [PG_DWARF_TAG_RESTRICT_TYPE] = "PG_DWARF_TAG_RESTRICT_TYPE",
    [PG_DWARF_TAG_INTERFACE_TYPE] = "PG_DWARF_TAG_INTERFACE_TYPE",
    [PG_DWARF_TAG_NAMESPACE] = "PG_DWARF_TAG_NAMESPACE",
    [PG_DWARF_TAG_IMPORTED_MODULE] = "PG_DWARF_TAG_IMPORTED_MODULE",
    [PG_DWARF_TAG_UNSPECIFIED_TYPE] = "PG_DWARF_TAG_UNSPECIFIED_TYPE",
    [PG_DWARF_TAG_PARTIAL_UNIT] = "PG_DWARF_TAG_PARTIAL_UNIT",
    [PG_DWARF_TAG_IMPORTED_UNIT] = "PG_DWARF_TAG_IMPORTED_UNIT",
    [PG_DWARF_TAG_CONDITION] = "PG_DWARF_TAG_CONDITION",
    [PG_DWARF_TAG_SHARED_TYPE] = "PG_DWARF_TAG_SHARED_TYPE",
    [PG_DWARF_TAG_TYPE_UNIT] = "PG_DWARF_TAG_TYPE_UNIT",
    [PG_DWARF_TAG_RVALUE_REFERENCE_TYPE] = "PG_DWARF_TAG_RVALUE_REFERENCE_TYPE",
    [PG_DWARF_TAG_TEMPLATE_ALIAS] = "PG_DWARF_TAG_TEMPLATE_ALIAS",
};

static const char dw_form_str[][30] = {
    [PG_DWARF_FORM_ADDR] = "PG_DWARF_FORM_ADDR",
    [PG_DWARF_FORM_BLOCK2] = "PG_DWARF_FORM_BLOCK2",
    [PG_DWARF_FORM_BLOCK4] = "PG_DWARF_FORM_BLOCK4",
    [PG_DWARF_FORM_DATA2] = "PG_DWARF_FORM_DATA2",
    [PG_DWARF_FORM_DATA4] = "PG_DWARF_FORM_DATA4",
    [PG_DWARF_FORM_DATA8] = "PG_DWARF_FORM_DATA8",
    [PG_DWARF_FORM_STRING] = "PG_DWARF_FORM_STRING",
    [PG_DWARF_FORM_BLOCK] = "PG_DWARF_FORM_BLOCK",
    [PG_DWARF_FORM_BLOCK1] = "PG_DWARF_FORM_BLOCK1",
    [PG_DWARF_FORM_DATA1] = "PG_DWARF_FORM_DATA1",
    [PG_DWARF_FORM_FLAG] = "PG_DWARF_FORM_FLAG",
    [PG_DWARF_FORM_SDATA] = "PG_DWARF_FORM_SDATA",
    [PG_DWARF_FORM_STRP] = "PG_DWARF_FORM_STRP",
    [PG_DWARF_FORM_UDATA] = "PG_DWARF_FORM_UDATA",
    [PG_DWARF_FORM_REF_ADDR] = "PG_DWARF_FORM_REF_ADDR",
    [PG_DWARF_FORM_REF1] = "PG_DWARF_FORM_REF1",
    [PG_DWARF_FORM_REF2] = "PG_DWARF_FORM_REF2",
    [PG_DWARF_FORM_REF4] = "PG_DWARF_FORM_REF4",
    [PG_DWARF_FORM_REF8] = "PG_DWARF_FORM_REF8",
    [PG_DWARF_FORM_REF_UDATA] = "PG_DWARF_FORM_REF_UDATA",
    [PG_DWARF_FORM_INDIRECT] = "PG_DWARF_FORM_INDIRECT",
    [PG_DWARF_FORM_SEC_OFFSET] = "PG_DWARF_FORM_SEC_OFFSET",
    [PG_DWARF_FORM_EXPRLOC] = "PG_DWARF_FORM_EXPRLOC",
    [PG_DWARF_FORM_FLAG_PRESENT] = "PG_DWARF_FORM_FLAG_PRESENT",
    [PG_DWARF_FORM_REF_SIG8] = "PG_DWARF_FORM_REF_SIG8",
};

typedef enum : i32 {
  PG_ONCE_UNINITIALIZED,
  PG_ONCE_INITIALIZING,
  PG_ONCE_INITIALIZED,
} PgOnce;

// ---------------- Functions.

#define PG_S(s) ((PgString){.data = (u8 *)s, .len = sizeof(s) - 1})

[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdin();
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdout();
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stderr();

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorPairResult pg_pipe_make();

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_file_read_at(PgFileDescriptor file, PgString buf, u64 offset);

[[maybe_unused]] [[nodiscard]] PgDirectoryResult
pg_directory_open(PgString name);

[[maybe_unused]] [[nodiscard]] PgError pg_directory_close(PgDirectory dir);

[[maybe_unused]] [[nodiscard]] PgDirectoryEntryResult
pg_directory_read(PgDirectory *dir);

[[maybe_unused]] [[nodiscard]] static bool
pg_dirent_is_file(PgDirectoryEntry dirent);

[[maybe_unused]] [[nodiscard]] static bool
pg_dirent_is_directory(PgDirectoryEntry dirent);

[[maybe_unused]] [[nodiscard]] static PgString
pg_dirent_name(PgDirectoryEntry dirent);

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_file_open(PgString path, PgFileAccess access, u64 mode,
             bool create_if_not_exists, PgAllocator *allocator);

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_close(PgFileDescriptor file);

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_truncate(PgFileDescriptor file, u64 size);

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_file_size(PgFileDescriptor file);

[[nodiscard]] static Pgu64Result pg_file_read(PgFileDescriptor file,
                                              PgString dst);

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_file_write(PgFileDescriptor file, PgString s);

[[maybe_unused]] [[nodiscard]] static PgAioResult pg_aio_init();

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_aio_register_interest_fs_name(PgAio *aio, PgString name,
                                 PgAioEventKind interest,
                                 PgAllocator *allocator);

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_register_interest_fd(PgAio aio, PgFileDescriptor fd,
                            PgAioEventKind interest);

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_unregister_interest(PgAio aio, PgFileDescriptor fd,
                           PgAioEventKind interest);

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_aio_wait(PgAio aio, PgAioEventSlice events_out, Pgu32Option timeout_ms);

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_aio_wait_cqe(PgAio aio, PgRing *cqe, Pgu32Option timeout_ms);

// TODO: Thread attributes?
[[maybe_unused]] [[nodiscard]] static PgThreadResult
pg_thread_create(PgThreadFn fn, void *fn_data);

[[maybe_unused]] static void pg_thread_yield();

[[maybe_unused]] [[nodiscard]] PgError pg_thread_join(PgThread thread);

[[maybe_unused]] [[nodiscard]] PgError pg_mtx_init(PgMutex *mutex,
                                                   PgMutexKind type);
[[maybe_unused]] void pg_mtx_destroy(PgMutex *mutex);
[[maybe_unused]] [[nodiscard]] PgError pg_mtx_lock(PgMutex *mutex);
[[maybe_unused]] [[nodiscard]] PgError pg_mtx_trylock(PgMutex *mutex);
[[maybe_unused]] [[nodiscard]] PgError
pg_mtx_timedlock(PgMutex *mutex, const struct timespec *time_point);
[[maybe_unused]] [[nodiscard]] PgError pg_mtx_unlock(PgMutex *mutex);

[[maybe_unused]] [[nodiscard]] PgError pg_cnd_init(PgConditionVar *cond);
[[maybe_unused]] void pg_cnd_destroy(PgConditionVar *cond);
[[maybe_unused]] [[nodiscard]] PgError pg_cnd_wait(PgConditionVar *cond,
                                                   PgMutex *mutex);
[[maybe_unused]] [[nodiscard]] PgError pg_cnd_broadcast(PgConditionVar *cond);
[[maybe_unused]] [[nodiscard]] PgError pg_cnd_signal(PgConditionVar *cond);
[[maybe_unused]] [[nodiscard]] PgError
pg_cnd_timedwait(PgConditionVar *cond, PgMutex *mutex,
                 const struct timespec *time_point);

#if 0
static PgString dw_attribute_to_str(dw_attribute_t attr) {
  switch (attr) {
  case PG_DWARF_AT_SIBLING:
    return "PG_DWARF_AT_SIBLING";
  case PG_DWARF_AT_LOCATION:
    return "PG_DWARF_AT_LOCATION";
  case PG_DWARF_AT_NAME:
    return "PG_DWARF_AT_NAME";
  case PG_DWARF_AT_ORDERING:
    return "PG_DWARF_AT_ORDERING";
  case PG_DWARF_AT_BYTE_SIZE:
    return "PG_DWARF_AT_BYTE_SIZE";
  case PG_DWARF_AT_BIT_OFFSET:
    return "PG_DWARF_AT_BIT_OFFSET";
  case PG_DWARF_AT_BIT_SIZE:
    return "PG_DWARF_AT_BIT_SIZE";
  case PG_DWARF_AT_STMT_LIST:
    return "PG_DWARF_AT_STMT_LIST";
  case PG_DWARF_AT_LOW_PC:
    return "PG_DWARF_AT_LOW_PC";
  case PG_DWARF_AT_HIGH_PC:
    return "PG_DWARF_AT_HIGH_PC";
  case PG_DWARF_AT_LANGUAGE:
    return "PG_DWARF_AT_LANGUAGE";
  case PG_DWARF_AT_DISCR:
    return "PG_DWARF_AT_DISCR";
  case PG_DWARF_AT_DISCR_VALUE:
    return "PG_DWARF_AT_DISCR_VALUE";
  case PG_DWARF_AT_VISIBILITY:
    return "PG_DWARF_AT_VISIBILITY";
  case PG_DWARF_AT_IMPORT:
    return "PG_DWARF_AT_IMPORT";
  case PG_DWARF_AT_STRING_LENGTH:
    return "PG_DWARF_AT_STRING_LENGTH";
  case PG_DWARF_AT_COMMON_REFERENCE:
    return "PG_DWARF_AT_COMMON_REFERENCE";
  case PG_DWARF_AT_COMP_DIR:
    return "PG_DWARF_AT_COMP_DIR";
  case PG_DWARF_AT_CONST_VALUE:
    return "PG_DWARF_AT_CONST_VALUE";
  case PG_DWARF_AT_CONTAINING_TYPE:
    return "PG_DWARF_AT_CONTAINING_TYPE";
  case PG_DWARF_AT_DEFAULT_VALUE:
    return "PG_DWARF_AT_DEFAULT_VALUE";
  case PG_DWARF_AT_INLINE:
    return "PG_DWARF_AT_INLINE";
  case PG_DWARF_AT_IS_OPTIONAL:
    return "PG_DWARF_AT_IS_OPTIONAL";
  case PG_DWARF_AT_LOWER_BOUND:
    return "PG_DWARF_AT_LOWER_BOUND";
  case PG_DWARF_AT_PRODUCER:
    return "PG_DWARF_AT_PRODUCER";
  case PG_DWARF_AT_PROTOTYPED:
    return "PG_DWARF_AT_PROTOTYPED";
  case PG_DWARF_AT_RETURN_ADDR:
    return "PG_DWARF_AT_RETURN_ADDR";
  case PG_DWARF_AT_START_SCOPE:
    return "PG_DWARF_AT_START_SCOPE";
  case PG_DWARF_AT_BIT_STRIDE:
    return "PG_DWARF_AT_BIT_STRIDE";
  case PG_DWARF_AT_UPPER_BOUND:
    return "PG_DWARF_AT_UPPER_BOUND";
  case PG_DWARF_AT_ABSTRACT_ORIGIN:
    return "PG_DWARF_AT_ABSTRACT_ORIGIN";
  case PG_DWARF_AT_ACCESSIBILITY:
    return "PG_DWARF_AT_ACCESSIBILITY";
  case PG_DWARF_AT_ADDRESS_CLASS:
    return "PG_DWARF_AT_ADDRESS_CLASS";
  case PG_DWARF_AT_ARTIFICIAL:
    return "PG_DWARF_AT_ARTIFICIAL";
  case PG_DWARF_AT_BASE_TYPES:
    return "PG_DWARF_AT_BASE_TYPES";
  case PG_DWARF_AT_CALLING_CONVENTION:
    return "PG_DWARF_AT_CALLING_CONVENTION";
  case PG_DWARF_AT_COUNT:
    return "PG_DWARF_AT_COUNT";
  case PG_DWARF_AT_DATA_MEMBER_LOCATION:
    return "PG_DWARF_AT_DATA_MEMBER_LOCATION";
  case PG_DWARF_AT_DECL_COLUMN:
    return "PG_DWARF_AT_DECL_COLUMN";
  case PG_DWARF_AT_DECL_FILE:
    return "PG_DWARF_AT_DECL_FILE";
  case PG_DWARF_AT_DECL_LINE:
    return "PG_DWARF_AT_DECL_LINE";
  case PG_DWARF_AT_DECLARATION:
    return "PG_DWARF_AT_DECLARATION";
  case PG_DWARF_AT_DISCR_LIST:
    return "PG_DWARF_AT_DISCR_LIST";
  case PG_DWARF_AT_ENCODING:
    return "PG_DWARF_AT_ENCODING";
  case PG_DWARF_AT_EXTERNAL:
    return "PG_DWARF_AT_EXTERNAL";
  case PG_DWARF_AT_FRAME_BASE:
    return "PG_DWARF_AT_FRAME_BASE";
  case PG_DWARF_AT_FRIEND:
    return "PG_DWARF_AT_FRIEND";
  case PG_DWARF_AT_IDENTIFIER_CASE:
    return "PG_DWARF_AT_IDENTIFIER_CASE";
  case PG_DWARF_AT_MACRO_INFO:
    return "PG_DWARF_AT_MACRO_INFO";
  case PG_DWARF_AT_NAMELIST_ITEM:
    return "PG_DWARF_AT_NAMELIST_ITEM";
  case PG_DWARF_AT_PRIORITY:
    return "PG_DWARF_AT_PRIORITY";
  case PG_DWARF_AT_SEGMENT:
    return "PG_DWARF_AT_SEGMENT";
  case PG_DWARF_AT_SPECIFICATION:
    return "PG_DWARF_AT_SPECIFICATION";
  case PG_DWARF_AT_STATIC_LINK:
    return "PG_DWARF_AT_STATIC_LINK";
  case PG_DWARF_AT_TYPE:
    return "PG_DWARF_AT_TYPE";
  case PG_DWARF_AT_USE_LOCATION:
    return "PG_DWARF_AT_USE_LOCATION";
  case PG_DWARF_AT_VARIABLE_PARAMETER:
    return "PG_DWARF_AT_VARIABLE_PARAMETER";
  case PG_DWARF_AT_VIRTUALITY:
    return "PG_DWARF_AT_VIRTUALITY";
  case PG_DWARF_AT_VTABLE_ELEM_LOCATION:
    return "PG_DWARF_AT_VTABLE_ELEM_LOCATION";
  case PG_DWARF_AT_ALLOCATED:
    return "PG_DWARF_AT_ALLOCATED";
  case PG_DWARF_AT_ASSOCIATED:
    return "PG_DWARF_AT_ASSOCIATED";
  case PG_DWARF_AT_DATA_LOCATION:
    return "PG_DWARF_AT_DATA_LOCATION";
  case PG_DWARF_AT_BYTE_STRIDE:
    return "PG_DWARF_AT_BYTE_STRIDE";
  case PG_DWARF_AT_ENTRY_PC:
    return "PG_DWARF_AT_ENTRY_PC";
  case PG_DWARF_AT_USE_UTF8:
    return "PG_DWARF_AT_USE_UTF8";
  case PG_DWARF_AT_EXTENSION:
    return "PG_DWARF_AT_EXTENSION";
  case PG_DWARF_AT_RANGES:
    return "PG_DWARF_AT_RANGES";
  case PG_DWARF_AT_TRAMPOLINE:
    return "PG_DWARF_AT_TRAMPOLINE";
  case PG_DWARF_AT_CALL_COLUMN:
    return "PG_DWARF_AT_CALL_COLUMN";
  case PG_DWARF_AT_CALL_FILE:
    return "PG_DWARF_AT_CALL_FILE";
  case PG_DWARF_AT_CALL_LINE:
    return "PG_DWARF_AT_CALL_LINE";
  case PG_DWARF_AT_DESCRIPTION:
    return "PG_DWARF_AT_DESCRIPTION";
  case PG_DWARF_AT_BINARY_SCALE:
    return "PG_DWARF_AT_BINARY_SCALE";
  case PG_DWARF_AT_DECIMAL_SCALE:
    return "PG_DWARF_AT_DECIMAL_SCALE";
  case PG_DWARF_AT_SMALL:
    return "PG_DWARF_AT_SMALL";
  case PG_DWARF_AT_DECIMAL_SIGN:
    return "PG_DWARF_AT_DECIMAL_SIGN";
  case PG_DWARF_AT_DIGIT_COUNT:
    return "PG_DWARF_AT_DIGIT_COUNT";
  case PG_DWARF_AT_PICTURE_STRING:
    return "PG_DWARF_AT_PICTURE_STRING";
  case PG_DWARF_AT_MUTABLE:
    return "PG_DWARF_AT_MUTABLE";
  case PG_DWARF_AT_THREADS_SCALED:
    return "PG_DWARF_AT_THREADS_SCALED";
  case PG_DWARF_AT_EXPLICIT:
    return "PG_DWARF_AT_EXPLICIT";
  case PG_DWARF_AT_OBJECT_POINTER:
    return "PG_DWARF_AT_OBJECT_POINTER";
  case PG_DWARF_AT_ENDIANITY:
    return "PG_DWARF_AT_ENDIANITY";
  case PG_DWARF_AT_ELEMENTAL:
    return "PG_DWARF_AT_ELEMENTAL";
  case PG_DWARF_AT_PURE:
    return "PG_DWARF_AT_PURE";
  case PG_DWARF_AT_RECURSIVE:
    return "PG_DWARF_AT_RECURSIVE";
  case PG_DWARF_AT_SIGNATURE:
    return "PG_DWARF_AT_SIGNATURE";
  case PG_DWARF_AT_MAIN_SUBPROGRAM:
    return "PG_DWARF_AT_MAIN_SUBPROGRAM";
  case PG_DWARF_AT_DATA_BIT_OFFSET:
    return "PG_DWARF_AT_DATA_BIT_OFFSET";
  case PG_DWARF_AT_CONST_EXPR:
    return "PG_DWARF_AT_CONST_EXPR";
  case PG_DWARF_AT_ENUM_CLASS:
    return "PG_DWARF_AT_ENUM_CLASS";
  case PG_DWARF_AT_LINKAGE_NAME:
    return "PG_DWARF_AT_LINKAGE_NAME";
  case PG_DWARF_AT_STRING_LENGTH_BIT_SIZE:
    return "PG_DWARF_AT_STRING_LENGTH_BIT_SIZE";
  case PG_DWARF_AT_STRING_LENGTH_BYTE_SIZE:
    return "PG_DWARF_AT_STRING_LENGTH_BYTE_SIZE";
  case PG_DWARF_AT_RANK:
    return "PG_DWARF_AT_RANK";
  case PG_DWARF_AT_STR_OFFSETS_BASE:
    return "PG_DWARF_AT_STR_OFFSETS_BASE";
  case PG_DWARF_AT_ADDR_BASE:
    return "PG_DWARF_AT_ADDR_BASE";
  case PG_DWARF_AT_RNGLISTS_BASE:
    return "PG_DWARF_AT_RNGLISTS_BASE";
  case PG_DWARF_AT_DWO_ID:
    return "PG_DWARF_AT_DWO_ID";
  case PG_DWARF_AT_DWO_NAME:
    return "PG_DWARF_AT_DWO_NAME";
  case PG_DWARF_AT_REFERENCE:
    return "PG_DWARF_AT_REFERENCE";
  case PG_DWARF_AT_RVALUE_REFERENCE:
    return "PG_DWARF_AT_RVALUE_REFERENCE";
  case PG_DWARF_AT_MACROS:
    return "PG_DWARF_AT_MACROS";
  case PG_DWARF_AT_CALL_ALL_CALLS:
    return "PG_DWARF_AT_CALL_ALL_CALLS";
  case PG_DWARF_AT_CALL_ALL_SOURCE_CALLS:
    return "PG_DWARF_AT_CALL_ALL_SOURCE_CALLS";
  case PG_DWARF_AT_CALL_ALL_TAIL_CALLS:
    return "PG_DWARF_AT_CALL_ALL_TAIL_CALLS";
  case PG_DWARF_AT_CALL_RETURN_PC:
    return "PG_DWARF_AT_CALL_RETURN_PC";
  case PG_DWARF_AT_CALL_VALUE:
    return "PG_DWARF_AT_CALL_VALUE";
  case PG_DWARF_AT_CALL_ORIGIN:
    return "PG_DWARF_AT_CALL_ORIGIN";
  case PG_DWARF_AT_CALL_PARAMETER:
    return "PG_DWARF_AT_CALL_PARAMETER";
  case PG_DWARF_AT_CALL_PC:
    return "PG_DWARF_AT_CALL_PC";
  case PG_DWARF_AT_CALL_TAIL_CALL:
    return "PG_DWARF_AT_CALL_TAIL_CALL";
  case PG_DWARF_AT_CALL_TARGET:
    return "PG_DWARF_AT_CALL_TARGET";
  case PG_DWARF_AT_CALL_TARGET_CLOBBERED:
    return "PG_DWARF_AT_CALL_TARGET_CLOBBERED";
  case PG_DWARF_AT_CALL_DATA_LOCATION:
    return "PG_DWARF_AT_CALL_DATA_LOCATION";
  case PG_DWARF_AT_CALL_DATA_VALUE:
    return "PG_DWARF_AT_CALL_DATA_VALUE";
  case PG_DWARF_AT_NORETURN:
    return "PG_DWARF_AT_NORETURN";
  case PG_DWARF_AT_ALIGNMENT:
    return "PG_DWARF_AT_ALIGNMENT";
  case PG_DWARF_AT_EXPORT_SYMBOLS:
    return "PG_DWARF_AT_EXPORT_SYMBOLS";
  case PG_DWARF_AT_DELETED:
    return "PG_DWARF_AT_DELETED";
  case PG_DWARF_AT_DEFAULTED:
    return "PG_DWARF_AT_DEFAULTED";
  case PG_DWARF_AT_LOCLISTS_BASE:
    return "PG_DWARF_AT_LOCLISTS_BASE";
  case PG_DWARF_AT_MIPS_LOOP_BEGIN:
    return "PG_DWARF_AT_MIPS_LOOP_BEGIN";
  case PG_DWARF_AT_MIPS_TAIL_LOOP_BEGIN:
    return "PG_DWARF_AT_MIPS_TAIL_LOOP_BEGIN";
  case PG_DWARF_AT_MIPS_EPILOG_BEGIN:
    return "PG_DWARF_AT_MIPS_EPILOG_BEGIN";
  case PG_DWARF_AT_MIPS_LOOP_UNROLL_FACTOR:
    return "PG_DWARF_AT_MIPS_LOOP_UNROLL_FACTOR";
  case PG_DWARF_AT_MIPS_SOFTWARE_PIPELINE_DEPTH:
    return "PG_DWARF_AT_MIPS_SOFTWARE_PIPELINE_DEPTH";
  case PG_DWARF_AT_MIPS_LINKAGE_NAME:
    return "PG_DWARF_AT_MIPS_LINKAGE_NAME";
  case PG_DWARF_AT_MIPS_STRIDE:
    return "PG_DWARF_AT_MIPS_STRIDE";
  case PG_DWARF_AT_MIPS_ABSTRACT_NAME:
    return "PG_DWARF_AT_MIPS_ABSTRACT_NAME";
  case PG_DWARF_AT_MIPS_CLONE_ORIGIN:
    return "PG_DWARF_AT_MIPS_CLONE_ORIGIN";
  case PG_DWARF_AT_MIPS_HAS_INLINES:
    return "PG_DWARF_AT_MIPS_HAS_INLINES";
  case PG_DWARF_AT_MIPS_STRIDE_BYTE:
    return "PG_DWARF_AT_MIPS_STRIDE_BYTE";
  case PG_DWARF_AT_MIPS_STRIDE_ELEM:
    return "PG_DWARF_AT_MIPS_STRIDE_ELEM";
  case PG_DWARF_AT_MIPS_PTR_DOPETYPE:
    return "PG_DWARF_AT_MIPS_PTR_DOPETYPE";
  case PG_DWARF_AT_MIPS_ALLOCATABLE_DOPETYPE:
    return "PG_DWARF_AT_MIPS_ALLOCATABLE_DOPETYPE";
  case PG_DWARF_AT_MIPS_ASSUMED_SHAPE_DOPETYPE:
    return "PG_DWARF_AT_MIPS_ASSUMED_SHAPE_DOPETYPE";
  case PG_DWARF_AT_MIPS_ASSUMED_SIZE:
    return "PG_DWARF_AT_MIPS_ASSUMED_SIZE";
  case PG_DWARF_AT_SF_NAMES:
    return "PG_DWARF_AT_SF_NAMES";
  case PG_DWARF_AT_SRC_INFO:
    return "PG_DWARF_AT_SRC_INFO";
  case PG_DWARF_AT_MAC_INFO:
    return "PG_DWARF_AT_MAC_INFO";
  case PG_DWARF_AT_SRC_COORDS:
    return "PG_DWARF_AT_SRC_COORDS";
  case PG_DWARF_AT_BODY_BEGIN:
    return "PG_DWARF_AT_BODY_BEGIN";
  case PG_DWARF_AT_BODY_END:
    return "PG_DWARF_AT_BODY_END";
  case PG_DWARF_AT_GNU_VECTOR:
    return "PG_DWARF_AT_GNU_VECTOR";
  case PG_DWARF_AT_GNU_TEMPLATE_NAME:
    return "PG_DWARF_AT_GNU_TEMPLATE_NAME";
  case PG_DWARF_AT_GNU_ODR_SIGNATURE:
    return "PG_DWARF_AT_GNU_ODR_SIGNATURE";
  case PG_DWARF_AT_GNU_CALL_SITE_VALUE:
    return "PG_DWARF_AT_GNU_CALL_SITE_VALUE";
  case PG_DWARF_AT_GNU_CALL_SITE_DATA_VALUE:
    return "PG_DWARF_AT_GNU_CALL_SITE_DATA_VALUE";
  case PG_DWARF_AT_GNU_CALL_SITE_TARGET:
    return "PG_DWARF_AT_GNU_CALL_SITE_TARGET";
  case PG_DWARF_AT_GNU_CALL_SITE_TARGET_CLOBBERED:
    return "PG_DWARF_AT_GNU_CALL_SITE_TARGET_CLOBBERED";
  case PG_DWARF_AT_GNU_TAIL_CALL:
    return "PG_DWARF_AT_GNU_TAIL_CALL";
  case PG_DWARF_AT_GNU_ALL_TAIL_CALL_SITES:
    return "PG_DWARF_AT_GNU_ALL_TAIL_CALL_SITES";
  case PG_DWARF_AT_GNU_ALL_CALL_SITES:
    return "PG_DWARF_AT_GNU_ALL_CALL_SITES";
  case PG_DWARF_AT_GNU_ALL_SOURCE_CALL_SITES:
    return "PG_DWARF_AT_GNU_ALL_SOURCE_CALL_SITES";
  case PG_DWARF_AT_GNU_MACROS:
    return "PG_DWARF_AT_GNU_MACROS";
  case PG_DWARF_AT_GNU_DWO_NAME:
    return "PG_DWARF_AT_GNU_DWO_NAME";
  case PG_DWARF_AT_GNU_DWO_ID:
    return "PG_DWARF_AT_GNU_DWO_ID";
  case PG_DWARF_AT_GNU_RANGES_BASE:
    return "PG_DWARF_AT_GNU_RANGES_BASE";
  case PG_DWARF_AT_GNU_ADDR_BASE:
    return "PG_DWARF_AT_GNU_ADDR_BASE";
  case PG_DWARF_AT_GNU_PUBNAMES:
    return "PG_DWARF_AT_GNU_PUBNAMES";
  case PG_DWARF_AT_GNU_PUBTYPES:
    return "PG_DWARF_AT_GNU_PUBTYPES";
  case PG_DWARF_AT_GNU_DISCRIMINATOR:
    return "PG_DWARF_AT_GNU_DISCRIMINATOR";
  case PG_DWARF_AT_BORLAND_PROPERTY_READ:
    return "PG_DWARF_AT_BORLAND_PROPERTY_READ";
  case PG_DWARF_AT_BORLAND_PROPERTY_WRITE:
    return "PG_DWARF_AT_BORLAND_PROPERTY_WRITE";
  case PG_DWARF_AT_BORLAND_PROPERTY_IMPLEMENTS:
    return "PG_DWARF_AT_BORLAND_PROPERTY_IMPLEMENTS";
  case PG_DWARF_AT_BORLAND_PROPERTY_INDEX:
    return "PG_DWARF_AT_BORLAND_PROPERTY_INDEX";
  case PG_DWARF_AT_BORLAND_PROPERTY_DEFAULT:
    return "PG_DWARF_AT_BORLAND_PROPERTY_DEFAULT";
  case PG_DWARF_AT_BORLAND_DELPHI_UNIT:
    return "PG_DWARF_AT_BORLAND_DELPHI_UNIT";
  case PG_DWARF_AT_BORLAND_DELPHI_CLASS:
    return "PG_DWARF_AT_BORLAND_DELPHI_CLASS";
  case PG_DWARF_AT_BORLAND_DELPHI_RECORD:
    return "PG_DWARF_AT_BORLAND_DELPHI_RECORD";
  case PG_DWARF_AT_BORLAND_DELPHI_METACLASS:
    return "PG_DWARF_AT_BORLAND_DELPHI_METACLASS";
  case PG_DWARF_AT_BORLAND_DELPHI_CONSTRUCTOR:
    return "PG_DWARF_AT_BORLAND_DELPHI_CONSTRUCTOR";
  case PG_DWARF_AT_BORLAND_DELPHI_DESTRUCTOR:
    return "PG_DWARF_AT_BORLAND_DELPHI_DESTRUCTOR";
  case PG_DWARF_AT_BORLAND_DELPHI_ANONYMOUS_METHOD:
    return "PG_DWARF_AT_BORLAND_DELPHI_ANONYMOUS_METHOD";
  case PG_DWARF_AT_BORLAND_DELPHI_INTERFACE:
    return "PG_DWARF_AT_BORLAND_DELPHI_INTERFACE";
  case PG_DWARF_AT_BORLAND_DELPHI_ABI:
    return "PG_DWARF_AT_BORLAND_DELPHI_ABI";
  case PG_DWARF_AT_BORLAND_DELPHI_RETURN:
    return "PG_DWARF_AT_BORLAND_DELPHI_RETURN";
  case PG_DWARF_AT_BORLAND_DELPHI_FRAMEPTR:
    return "PG_DWARF_AT_BORLAND_DELPHI_FRAMEPTR";
  case PG_DWARF_AT_BORLAND_CLOSURE:
    return "PG_DWARF_AT_BORLAND_CLOSURE";
  case PG_DWARF_AT_LLVM_INCLUDE_PATH:
    return "PG_DWARF_AT_LLVM_INCLUDE_PATH";
  case PG_DWARF_AT_LLVM_CONFIG_MACROS:
    return "PG_DWARF_AT_LLVM_CONFIG_MACROS";
  case PG_DWARF_AT_LLVM_SYSROOT:
    return "PG_DWARF_AT_LLVM_SYSROOT";
  case PG_DWARF_AT_LLVM_TAG_OFFSET:
    return "PG_DWARF_AT_LLVM_TAG_OFFSET";
  case PG_DWARF_AT_LLVM_APINOTES:
    return "PG_DWARF_AT_LLVM_APINOTES";
  case PG_DWARF_AT_APPLE_OPTIMIZED:
    return "PG_DWARF_AT_APPLE_OPTIMIZED";
  case PG_DWARF_AT_APPLE_FLAGS:
    return "PG_DWARF_AT_APPLE_FLAGS";
  case PG_DWARF_AT_APPLE_ISA:
    return "PG_DWARF_AT_APPLE_ISA";
  case PG_DWARF_AT_APPLE_BLOCK:
    return "PG_DWARF_AT_APPLE_BLOCK";
  case PG_DWARF_AT_APPLE_MAJOR_RUNTIME_VERS:
    return "PG_DWARF_AT_APPLE_MAJOR_RUNTIME_VERS";
  case PG_DWARF_AT_APPLE_RUNTIME_CLASS:
    return "PG_DWARF_AT_APPLE_RUNTIME_CLASS";
  case PG_DWARF_AT_APPLE_OMIT_FRAME_PTR:
    return "PG_DWARF_AT_APPLE_OMIT_FRAME_PTR";
  case PG_DWARF_AT_APPLE_PROPERTY_NAME:
    return "PG_DWARF_AT_APPLE_PROPERTY_NAME";
  case PG_DWARF_AT_APPLE_PROPERTY_GETTER:
    return "PG_DWARF_AT_APPLE_PROPERTY_GETTER";
  case PG_DWARF_AT_APPLE_PROPERTY_SETTER:
    return "PG_DWARF_AT_APPLE_PROPERTY_SETTER";
  case PG_DWARF_AT_APPLE_PROPERTY_ATTRIBUTE:
    return "PG_DWARF_AT_APPLE_PROPERTY_ATTRIBUTE";
  case PG_DWARF_AT_APPLE_OBJC_COMPLETE_TYPE:
    return "PG_DWARF_AT_APPLE_OBJC_COMPLETE_TYPE";
  case PG_DWARF_AT_APPLE_PROPERTY:
    return "PG_DWARF_AT_APPLE_PROPERTY";
  case PG_DWARF_AT_APPLE_OBJC_DIRECT:
    return "PG_DWARF_AT_APPLE_OBJC_DIRECT";
  case PG_DWARF_AT_APPLE_SDK:
    return "PG_DWARF_AT_APPLE_SDK";
  default:
    PG_ASSERT(0);
  }
}
#endif

static void pg_once_mark_as_done(_Atomic PgOnce *done) {
  // PG_ONCE_INITIALIZING -> PG_ONCE_INITIALIZED.
  atomic_store_explicit(done, PG_ONCE_INITIALIZED, memory_order_release);
}

[[nodiscard]] static bool pg_once_do(_Atomic PgOnce *done) {

  // Loop until initialized.
  while (PG_ONCE_INITIALIZED !=
         atomic_load_explicit(done, memory_order_acquire)) {
    i32 expected = PG_ONCE_UNINITIALIZED;

    // Try PG_ONCE_UNINITIALIZED -> PG_ONCE_INITIALIZING.
    if (atomic_compare_exchange_weak_explicit(
            done, &expected, PG_ONCE_INITIALIZING, memory_order_acq_rel,
            memory_order_acquire)) {
      // Caller will do the initialization and then call `pg_once_mark_as_done`.
      return true;
    } else {
      // Another thread is doing the initialization.

      // If the other thread is done, return.
      if (PG_ONCE_INITIALIZED == expected) {
        return false;
      } else if (PG_ONCE_INITIALIZING == expected) {
        // Busy wait by looping again, for simplicity.
        pg_thread_yield();
      }
    }
  }

  // Already initialized.
  return false;
}

[[maybe_unused]] static u64
pg_fill_call_stack(u64 pie_offset, u64 call_stack[PG_STACKTRACE_MAX]);

[[nodiscard]] static u64 pg_self_pie_get_offset();

[[maybe_unused]] inline static void
pg_stacktrace_print(const char *file, int line, const char *function) {
  u64 pie_offset = pg_self_pie_get_offset();

  fprintf(stderr, "ASSERT: %s:%d:%s\n", file, line, function);

  u64 call_stack[PG_STACKTRACE_MAX] = {0};
  u64 callstack_len = pg_fill_call_stack(pie_offset, call_stack);

  for (u64 i = 0; i < callstack_len; i++) {
    fprintf(stderr, "%#" PRIx64 "\n", call_stack[i]);
  }

  puts("");
}

#define PG_ASSERT_TRAP_ONLY(x) ((x) ? (0) : (__builtin_trap(), 0))

#define PG_ASSERT(x)                                                           \
  ((x) ? (0)                                                                   \
       : (pg_stacktrace_print(__FILE__, __LINE__, __FUNCTION__),               \
          fflush(stdout), fflush(stderr), __builtin_trap(), 0))

static u8 *pg_memcpy(void *restrict dst, void *restrict src, u64 len) {
  if (!dst) {
    return dst;
  }
  if (!src) {
    return dst;
  }
  if (0 == len) {
    return dst;
  }

  return memcpy(dst, src, len);
}

static u8 *pg_memmove(void *dst, void *src, u64 len) {
  if (!dst) {
    return dst;
  }
  if (!src) {
    return dst;
  }
  if (0 == len) {
    return dst;
  }

  return memmove(dst, src, len);
}

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

#define PG_ROUNDUP(nbytes, pad)                                                \
  (u64)((long)((nbytes) + ((pad) - 1)) & ~(long)((pad) - 1))

#define PG_SLICE_SWAP_REMOVE(s, idx)                                           \
  do {                                                                         \
    if ((i64)(idx) >= (i64)((s)->len)) {                                       \
      __builtin_trap();                                                        \
    }                                                                          \
    *(PG_C_ARRAY_AT_PTR((s)->data, (s)->len, (idx))) =                         \
        PG_C_ARRAY_AT((s)->data, (s)->len, (s)->len - 1);                      \
    (s)->len -= 1;                                                             \
  } while (0)

#define PG_SLICE_REMOVE_AT(s, idx)                                             \
  do {                                                                         \
    PG_ASSERT((s)->len > 0);                                                   \
    PG_ASSERT((idx) < (s)->len);                                               \
    if ((idx) + 1 < (s)->len) {                                                \
      memmove(PG_SLICE_AT_PTR(s, idx), PG_SLICE_AT_PTR(s, (idx) + 1),          \
              ((s)->len - (idx + 1)) * sizeof(PG_SLICE_AT(*(s), 0)));          \
    }                                                                          \
    (s)->len -= 1;                                                             \
  } while (0)

[[maybe_unused]] [[nodiscard]] static u64 pg_hash_fnv(Pgu8Slice s) {
  u64 hash = 0x100;
  for (u64 i = 0; i < s.len; i++) {
    u8 c = PG_SLICE_AT(s, i);
    hash ^= c;
    hash *= 1111111111111111111;
  }
  return hash;
}

[[maybe_unused]] [[nodiscard]] static bool pg_rune_is_hex_digit(PgRune c) {
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') ||
         ('a' <= c && c <= 'f');
}

[[maybe_unused]] [[nodiscard]] static bool
pg_rune_ascii_is_alphabetical_uppercase(PgRune c) {
  return ('A' <= c && c <= 'Z');
}

[[maybe_unused]] [[nodiscard]] static bool
pg_rune_ascii_is_alphabetical_lowercase(PgRune c) {
  return ('a' <= c && c <= 'z');
}

[[maybe_unused]] [[nodiscard]] static bool
pg_rune_ascii_is_alphabetical(PgRune c) {
  return pg_rune_ascii_is_alphabetical_uppercase(c) ||
         pg_rune_ascii_is_alphabetical_lowercase(c);
}

static const PgString PG_ASCII_SPACES = PG_S(" \f\n\r\t");

[[maybe_unused]] [[nodiscard]] static bool pg_rune_ascii_is_space(PgRune c) {
  return ' ' == c || '\t' == c || '\n' == c || '\f' == c || '\r' == c;
}

[[maybe_unused]] [[nodiscard]] static bool pg_rune_ascii_is_numeric(PgRune c) {
  return ('0' <= c && c <= '9');
}

[[maybe_unused]] [[nodiscard]] static bool pg_rune_ascii_is_hex(PgRune c) {
  return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
         ('A' <= c && c <= 'F');
}

[[maybe_unused]] [[nodiscard]] static bool
pg_rune_ascii_is_alphanumeric(PgRune c) {
  return pg_rune_ascii_is_numeric(c) || pg_rune_ascii_is_alphabetical(c);
}

[[maybe_unused]] [[nodiscard]] static PgRune
pg_rune_ascii_to_lower_case(PgRune c) {
  PG_ASSERT(c <= 0x7f);

  if ('A' <= c && c <= 'Z') {
    return c + ('a' - 'A');
  }
  return c;
}

[[maybe_unused]] [[nodiscard]] static PgRune
pg_rune_ascii_to_upper_case(PgRune c) {
  if ('a' <= c && c <= 'z') {
    return c - ('a' - 'A');
  }
  return c;
}

[[maybe_unused]] [[nodiscard]] static u8 pg_rune_from_hex(PgRune c) {
  PG_ASSERT(pg_rune_is_hex_digit(c));

  if ('0' <= c && c <= '9') {
    return (u8)c - '0';
  }

  if ('A' <= c && c <= 'F') {
    return 10 + (u8)c - 'A';
  }

  if ('a' <= c && c <= 'f') {
    return 10 + (u8)c - 'a';
  }

  PG_ASSERT(false);
}

#define PG_SLICE_IS_EMPTY(s)                                                   \
  (((s).len == 0) ? true : (PG_ASSERT(nullptr != (s).data), false))

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

[[maybe_unused]] [[nodiscard]] static PgUtf8Iterator
pg_make_utf8_iterator(PgString s) {
  PgUtf8Iterator it = {0};
  it.s = s;

  return it;
}

[[maybe_unused]] [[nodiscard]] static bool pg_string_is_empty(PgString s) {
  return PG_SLICE_IS_EMPTY(s);
}

// TODO: If this becomes a performance bottleneck there are more optimized ways
// to implement that with SIMD.
[[maybe_unused]] [[nodiscard]] static PgRuneUtf8Result
pg_utf8_iterator_peek_next(PgUtf8Iterator it) {
  PgRuneUtf8Result res = {0};

  PgString s = PG_SLICE_RANGE_START(it.s, it.idx);
  if (pg_string_is_empty(s)) {
    res.end = true;
    return res;
  }

  u8 c0 = PG_SLICE_AT(s, 0);

  // One byte.
  if (c0 <= 0b0111'1111) {
    res.rune = c0;
    return res;
  }

  // 2 bytes.
  if (c0 <= 0b1101'1110) {
    if (s.len < 2) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    u8 c1 = PG_SLICE_AT(s, 1);
    PgRune rune0 = (PgRune)c0 & 0b0001'1111;
    PgRune rune1 = ((PgRune)c1 & 0b0011'1111);
    res.rune = (rune0 << 6) | rune1;
    if (res.rune < 0x80) { // Overlong.
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    return res;
  }

  // 3 bytes.
  if (c0 <= 0b1110'1111) {
    if (s.len < 3) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    u8 c1 = PG_SLICE_AT(s, 1);
    u8 c2 = PG_SLICE_AT(s, 2);
    PgRune rune0 = ((PgRune)c0 & 0b0000'1111);
    PgRune rune1 = ((PgRune)c1 & 0b0011'1111);
    PgRune rune2 = ((PgRune)c2 & 0b0011'1111);

    res.rune = (rune0 << 12) | (rune1 << 6) | rune2;
    if (res.rune < 0x0800) { // Overlong.
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    return res;
  }

  // 4 bytes.
  if (c0 <= 0b1111'0111) {
    if (s.len < 4) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    u8 c1 = PG_SLICE_AT(s, 1);
    u8 c2 = PG_SLICE_AT(s, 2);
    u8 c3 = PG_SLICE_AT(s, 3);
    PgRune rune0 = (PgRune)c0 & 0b0000'0111;
    PgRune rune1 = (PgRune)c1 & 0b0011'1111;
    PgRune rune2 = (PgRune)c2 & 0b0011'1111;
    PgRune rune3 = ((PgRune)c3 & 0b0011'1111);
    res.rune = (rune0 << 18) | (rune1 << 12) | (rune2 << 6) | rune3;
    if (res.rune < 0x01'00'00) { // Overlong.
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    if (res.rune >= 0x10FFFF) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    return res;
  }

  res.err = PG_ERR_INVALID_VALUE;
  return res;
}

[[maybe_unused]] [[nodiscard]] static u64 pg_utf8_rune_bytes_count(PgRune c) {
  if (c < 0x0080) {
    return 1;
  }
  if (c < 0x0800) {
    return 2;
  }
  if (c < 0x10000) {
    return 3;
  }
  if (0x10000 <= c && c <= 0x10FFFF) {
    return 4;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgRuneUtf8Result
pg_utf8_iterator_next(PgUtf8Iterator *it) {
  PgRuneUtf8Result res = pg_utf8_iterator_peek_next(*it);
  it->idx += pg_utf8_rune_bytes_count(res.rune);

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_utf8_count_runes(PgString s) {
  Pgu64Result res = {0};

  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  u64 len = 0;
  for (;; len++) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err) {
      res.err = res_rune.err;
      return res;
    }
    if (res_rune.end) {
      break;
    }
  }

  res.value = len;
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_string_is_ascii_alphabetical(PgString s) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);

  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.end) {
      return true;
    }
    if (res_rune.err) {
      return false;
    }

    PgRune rune = res_rune.rune;

    if (!pg_rune_ascii_is_alphabetical(rune)) {
      return false;
    }
  }
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_trim_left(PgString s,
                                                                   PgRune c) {
  PgString res = s;

  PgUtf8Iterator it = pg_make_utf8_iterator(s);

  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.end) {
      break;
    }
    if (res_rune.err) {
      return res;
    }

    PgRune rune = res_rune.rune;

    if (rune != c) {
      return res;
    }
    res = PG_SLICE_RANGE_START(s, it.idx);
  }

  return res;
}

[[nodiscard]] static bool pg_utf8_is_continuation_byte(u8 byte) {
  return 0b1000'0000 == (byte & 0b1100'0000);
}

[[nodiscard]] static Pgu64Result pg_string_find_last_lead_byte(PgString s) {
  Pgu64Result res = {0};

  for (i64 i = (i64)s.len - 1; i >= 0; i--) {
    u8 byte = PG_SLICE_AT(s, i);
    if (pg_utf8_is_continuation_byte(byte)) {
      continue;
    }

    if (byte <= 0b0111'1111) {
      if ((u64)i + 1 != s.len) {
        res.err = PG_ERR_INVALID_VALUE;
        return res;
      }
      res.value = (u64)i;
      return res;
    }
    if (byte <= 0b1101'1110) {
      if ((u64)i + 2 != s.len) {
        res.err = PG_ERR_INVALID_VALUE;
        return res;
      }
      res.value = (u64)i;
      return res;
    }
    if (byte <= 0b1110'1111) {
      if ((u64)i + 3 != s.len) {
        res.err = PG_ERR_INVALID_VALUE;
        return res;
      }
      res.value = (u64)i;
      return res;
    }

    if (byte <= 0b1111'0111) {
      if ((u64)i + 4 != s.len) {
        res.err = PG_ERR_INVALID_VALUE;
        return res;
      }
      res.value = (u64)i;
      return res;
    }
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.err = PG_ERR_INVALID_VALUE;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgRuneOption pg_string_first(PgString s) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
  PgRuneOption res = {0};

  if (res_rune.err || res_rune.end) {
    return res;
  }

  res.has_value = true;
  res.value = res_rune.rune;
  return res;
}

[[nodiscard]] static PgRuneOption pg_string_last(PgString s) {
  PgRuneOption res = {0};

  Pgu64Result res_find = pg_string_find_last_lead_byte(s);
  if (res_find.err) {
    return res;
  }

  return pg_string_first(PG_SLICE_RANGE_START(s, res_find.value));
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_trim_right(PgString s,
                                                                    PgRune c) {
  while (!pg_string_is_empty(s)) {
    PgRuneOption last_opt = pg_string_last(s);
    if (!last_opt.has_value) {
      return s;
    }

    PgRune last = last_opt.value;
    if (last == c) {
      u64 rune_bytes_count = pg_utf8_rune_bytes_count(last);
      PG_ASSERT(s.len >= rune_bytes_count);
      s.len -= rune_bytes_count;
    } else {
      break;
    }
  }

  return s;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_trim(PgString s,
                                                              PgRune c) {
  PgString res = pg_string_trim_left(s, c);
  res = pg_string_trim_right(res, c);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_trim_space_left(PgString s) {
  PgString res = s;
  for (u64 i = 0; i < PG_ASCII_SPACES.len; i++) {
    u8 space = PG_SLICE_AT(PG_ASCII_SPACES, i);
    res = pg_string_trim_left(res, space);
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_trim_space_right(PgString s) {
  PgString res = s;
  for (u64 i = 0; i < PG_ASCII_SPACES.len; i++) {
    u8 space = PG_SLICE_AT(PG_ASCII_SPACES, i);
    res = pg_string_trim_right(res, space);
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_trim_space(PgString s) {
  PgString res = s;
  for (u64 i = 0; i < PG_ASCII_SPACES.len; i++) {
    u8 space = PG_SLICE_AT(PG_ASCII_SPACES, i);
    res = pg_string_trim(res, space);
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgSplitIterator
pg_string_split_string(PgString s, PgString sep) {
  return (PgSplitIterator){.s = s, .sep = sep};
}

[[maybe_unused]] [[nodiscard]] static i64
pg_string_index_of_rune(PgString haystack, PgRune needle) {
  if (PG_SLICE_IS_EMPTY(haystack)) {
    return -1;
  }

  PgUtf8Iterator it = pg_make_utf8_iterator(haystack);

  u64 idx = 0;
  for (;;) {
    idx = it.idx;
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || res_rune.end) {
      return -1;
    }

    if (needle == res_rune.rune) {
      return (i64)idx;
    }
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static PgStringCut
pg_string_cut_rune(PgString s, PgRune needle) {
  PgStringCut res = {0};

  PgUtf8Iterator it = pg_make_utf8_iterator(s);

  u64 idx = 0;
  for (;;) {
    idx = it.idx;
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || res_rune.end) {
      return res;
    }

    if (needle == res_rune.rune) {
      res.left = PG_SLICE_RANGE(s, 0, idx);
      res.right = PG_SLICE_RANGE_START(s, it.idx);
      res.has_value = true;
      return res;
    }
  }

  PG_ASSERT(0);
}

[[nodiscard]] static i64 pg_string_last_index_of_rune(PgString haystack,
                                                      PgRune needle) {
  while (!pg_string_is_empty(haystack)) {
    PgRuneOption last_opt = pg_string_last(haystack);
    if (!last_opt.has_value) {
      break;
    }

    PgRune last = last_opt.value;
    u64 rune_bytes_count = pg_utf8_rune_bytes_count(last);
    PG_ASSERT(haystack.len >= rune_bytes_count);
    haystack.len -= rune_bytes_count;

    if (last == needle) {
      return (i64)haystack.len;
    }
  }
  return -1;
}

[[maybe_unused]] [[nodiscard]] static bool pg_bytes_eq(Pgu8Slice a,
                                                       Pgu8Slice b) {
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

[[maybe_unused]] [[nodiscard]]
static Pgu64Option pg_bytes_index_of_byte(Pgu8Slice haystack, u8 needle) {
  Pgu64Option res = {0};

  for (u64 i = 0; i < haystack.len; i++) {
    u8 it = PG_SLICE_AT(haystack, i);
    if (needle == it) {
      res.value = i;
      res.has_value = true;
      return res;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]]
static Pgu64Option pg_bytes_last_index_of_byte(Pgu8Slice haystack, u8 needle) {
  Pgu64Option res = {0};

  for (i64 i = (i64)haystack.len - 1; i >= 0; i--) {
    u8 it = PG_SLICE_AT(haystack, i);
    if (needle == it) {
      res.value = (u64)i;
      res.has_value = true;
      return res;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_bytes_starts_with(Pgu8Slice haystack, Pgu8Slice needle) {
  if (needle.len > haystack.len) {
    return false;
  }

  return pg_bytes_eq(PG_SLICE_RANGE(haystack, 0, needle.len), needle);
}

[[maybe_unused]] [[nodiscard]] static bool
pg_bytes_ends_with(Pgu8Slice haystack, Pgu8Slice needle) {
  if (needle.len > haystack.len) {
    return false;
  }

  return pg_bytes_eq(PG_SLICE_RANGE_START(haystack, haystack.len - needle.len),
                     needle);
}

[[maybe_unused]] [[nodiscard]]
static Pgu64Option pg_bytes_index_of_bytes(Pgu8Slice haystack,
                                           Pgu8Slice needle) {
  Pgu64Option res = {0};

  if (PG_SLICE_IS_EMPTY(needle)) {
    return res;
  }

  if (needle.len > haystack.len) {
    return res;
  }

  for (u64 i = 0; i < haystack.len; i++) {
    if (pg_bytes_starts_with(PG_SLICE_RANGE_START(haystack, (u64)i), needle)) {
      res.value = (u64)i;
      res.has_value = true;
      return res;
    }
  }

  return res;
}

[[nodiscard]]
static bool pg_bytes_contains_any_byte(Pgu8Slice haystack, Pgu8Slice needles) {
  for (u64 i = 0; i < haystack.len; i++) {
    u8 c = PG_SLICE_AT(haystack, i);
    for (u64 j = 0; j < needles.len; j++) {
      u8 n = PG_SLICE_AT(needles, j);

      if (c == n) {
        return true;
      }
    }
  }
  return false;
}

[[maybe_unused]] [[nodiscard]]
static Pgu64Option pg_bytes_last_index_of_bytes(Pgu8Slice haystack,
                                                Pgu8Slice needle) {
  Pgu64Option res = {0};

  if (PG_SLICE_IS_EMPTY(needle)) {
    return res;
  }

  if (needle.len > haystack.len) {
    return res;
  }

  for (i64 i = (i64)haystack.len - 1; i >= 0; i--) {
    if (pg_bytes_ends_with(PG_SLICE_RANGE_START(haystack, (u64)i), needle)) {
      res.value = (u64)i;
      res.has_value = true;
      return res;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgBytesCut
pg_bytes_cut_byte(Pgu8Slice haystack, u8 needle) {
  PgBytesCut res = {0};

  if (PG_SLICE_IS_EMPTY(haystack)) {
    return res;
  }

  PG_ASSERT(haystack.data);
  // TODO: Use `pg_bytes_index_of_byte`?
  u8 *ret = memchr(haystack.data, needle, haystack.len);

  if (!ret) {
    return res;
  }

  res.has_value = true;
  res.left.data = haystack.data;
  res.left.len = (u64)(ret - haystack.data);
  res.right.data = ret + 1;
  res.right.len = haystack.len - res.left.len - 1;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgBytesCut
pg_bytes_cut_bytes_excl(Pgu8Slice haystack, Pgu8Slice needle) {
  PgBytesCut res = {0};

  Pgu64Option search_opt = pg_bytes_index_of_bytes(haystack, needle);
  if (!search_opt.has_value) {
    return res;
  }

  res.has_value = true;
  res.left = PG_SLICE_RANGE(haystack, 0, search_opt.value);
  res.right = PG_SLICE_RANGE_START(haystack, search_opt.value);
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool pg_string_eq(PgString a,
                                                        PgString b) {
  return pg_bytes_eq(a, b);
}

[[maybe_unused]] [[nodiscard]] static i64
pg_string_index_of_string(PgString haystack, PgString needle) {
  if (0 == needle.len || needle.len > haystack.len) {
    return -1;
  }

  for (u64 i = 0; i <= haystack.len - needle.len; i++) {
    if (pg_string_eq(PG_SLICE_RANGE(haystack, i, i + needle.len), needle)) {
      return (i64)i;
    }
  }
  return -1;
}

[[maybe_unused]] [[nodiscard]] static PgStringCut
pg_string_cut_string(PgString s, PgString needle) {
  PgStringCut res = {0};

  i64 idx = pg_string_index_of_string(s, needle);
  if (-1 == idx) {
    return res;
  }

  res.left = PG_SLICE_RANGE(s, 0, (u64)idx);
  res.right = PG_SLICE_RANGE_START(s, (u64)idx + needle.len);
  res.has_value = true;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringOption
pg_string_split_next(PgSplitIterator *it) {
  if (PG_SLICE_IS_EMPTY(it->s)) {
    return (PgStringOption){0};
  }

  for (u64 _i = 0; _i < it->s.len; _i++) {
    i64 idx = pg_string_index_of_string(it->s, it->sep);
    if (-1 == idx) {
      // Last element.
      PgStringOption res = {.value = it->s, .has_value = true};
      it->s = (PgString){0};
      return res;
    }

    if (idx == 0) { // Multiple contiguous separators.
      it->s = PG_SLICE_RANGE_START(it->s, (u64)idx + it->sep.len);
      continue;
    } else {
      PgStringOption res = {.value = PG_SLICE_RANGE(it->s, 0, (u64)idx),
                            .has_value = true};
      it->s = PG_SLICE_RANGE_START(it->s, (u64)idx + it->sep.len);

      return res;
    }
  }
  return (PgStringOption){0};
}

[[maybe_unused]] [[nodiscard]] static PgStringPairConsume
pg_string_consume_until_rune_excl(PgString haystack, PgRune needle) {
  PgStringPairConsume res = {0};

  i64 idx = pg_string_index_of_rune(haystack, needle);
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
pg_string_consume_until_rune_incl(PgString haystack, PgRune needle) {
  PgStringPairConsume res = {0};

  i64 idx = pg_string_index_of_rune(haystack, needle);
  if (-1 == idx) {
    res.left = haystack;
    res.right = haystack;
    return res;
  }

  res.left = PG_SLICE_RANGE(haystack, 0, (u64)idx);
  res.right = PG_SLICE_RANGE_START(haystack,
                                   (u64)idx + pg_utf8_rune_bytes_count(needle));
  res.consumed = true;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringPairConsumeAny
pg_string_consume_until_any_rune_incl(PgString haystack, PgString needles) {
  PgStringPairConsumeAny res = {0};

  PgUtf8Iterator it = pg_make_utf8_iterator(needles);

  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || res_rune.end) {
      break;
    }

    PgRune needle = res_rune.rune;
    PgStringPairConsume res_consume =
        pg_string_consume_until_rune_incl(haystack, needle);
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
pg_string_consume_until_any_rune_excl(PgString haystack, PgString needles) {
  PgStringPairConsumeAny res = {0};

  PgUtf8Iterator it = pg_make_utf8_iterator(needles);
  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || res_rune.end) {
      break;
    }

    PgRune needle = res_rune.rune;
    PgStringPairConsume res_consume =
        pg_string_consume_until_rune_excl(haystack, needle);
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
pg_string_index_of_any_rune(PgString haystack, PgString needles) {
  PgUtf8Iterator it = pg_make_utf8_iterator(needles);
  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || res_rune.end) {
      break;
    }

    PgRune needle = res_rune.rune;

    i64 idx = pg_string_index_of_rune(haystack, needle);
    if (-1 != idx) {
      return idx;
    }
  }
  return -1;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_string_contains_rune(PgString haystack, PgRune needle) {
  return -1 != pg_string_index_of_rune(haystack, needle);
}

[[maybe_unused]] [[nodiscard]] static bool pg_string_contains(PgString haystack,
                                                              PgString needle) {
  return -1 != pg_string_index_of_string(haystack, needle);
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

[[maybe_unused]] [[nodiscard]] static PgStringOption
pg_string_consume_rune(PgString haystack, PgRune needle) {
  PgStringOption res = {0};

  PgUtf8Iterator it = pg_make_utf8_iterator(haystack);
  PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);

  if (res_rune.err || res_rune.end) {
    return res;
  }

  if (needle != res_rune.rune) {
    return res;
  }

  res.value.data = haystack.data + it.idx;
  res.value.len = haystack.len - it.idx;
  res.has_value = true;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringOption
pg_string_consume_string(PgString haystack, PgString needle) {
  PgStringOption res = {0};
  res.value = haystack;

  for (u64 i = 0; i < needle.len; i++) {
    res = pg_string_consume_rune(res.value, PG_SLICE_AT(needle, i));
    if (!res.has_value) {
      return res;
    }
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringOption
pg_string_consume_any_string(PgString haystack, PgStringSlice needles) {
  PgStringOption res = {0};
  res.value = haystack;

  for (u64 i = 0; i < needles.len; i++) {
    res = pg_string_consume_string(res.value, PG_SLICE_AT(needles, i));
    if (res.has_value) {
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

[[maybe_unused]] [[nodiscard]] static PgParseNumberResult
pg_string_parse_u64(PgString s, u64 base, bool forbig_leading_zeroes) {
  PG_ASSERT(10 == base || 16 == base);

  PgParseNumberResult res = {0};
  res.remaining = s;

  static const u8 hex_to_num[256] = {
      ['0'] = 0,  ['1'] = 1,  ['2'] = 2,  ['3'] = 3,  ['4'] = 4,  ['5'] = 5,
      ['6'] = 6,  ['7'] = 7,  ['8'] = 8,  ['9'] = 9,  ['a'] = 10, ['b'] = 11,
      ['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15, ['A'] = 10, ['B'] = 11,
      ['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15,

  };

  // Forbid leading zero(es) if there is more than one digit.
  if (forbig_leading_zeroes && pg_string_starts_with(s, PG_S("0")) &&
      s.len >= 2 && pg_rune_ascii_is_numeric(PG_SLICE_AT(s, 1))) {
    return res;
  }

  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  u64 last_idx = 0;
  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || res_rune.end) {
      break;
    }

    PgRune c = res_rune.rune;
    if (10 == base &&
        !pg_rune_ascii_is_numeric(c)) { // End of numbers sequence.
      break;
    }
    if (16 == base && !pg_rune_ascii_is_hex(c)) { // End of numbers sequence.
      break;
    }

    res.n *= base;
    if (10 == base) {
      res.n += (u8)c - '0';
    } else if (16 == base) {
      res.n += hex_to_num[c];
    }
    res.present = true;
    last_idx = it.idx;
  }
  res.remaining = PG_SLICE_RANGE_START(s, last_idx);
  return res;
}

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

[[maybe_unused]] [[nodiscard]]
__attribute((malloc, alloc_size(2, 4), alloc_align(3))) static void *
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

[[maybe_unused]] [[nodiscard]]
__attribute((malloc, alloc_size(4, 6), alloc_align(5))) static void *
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

  const u64 padding = (-(u64)a->start & (align - 1));
  PG_ASSERT(padding <= align);

  void *res = a->start + padding;
  PG_ASSERT(res != nullptr);
  PG_ASSERT(res <= (void *)a->end);

  memmove(res, ptr, size * count);

  a->start += padding + count * size;
  PG_ASSERT(a->start <= a->end);
  PG_ASSERT((u64)a->start % align == 0); // Aligned.

  return res;
}

[[maybe_unused]] [[nodiscard]]
__attribute((malloc, alloc_size(2, 4), alloc_align(3))) static void *
pg_arena_alloc(PgArena *a, u64 size, u64 align, u64 count) {
  void *res = pg_try_arena_alloc(a, size, align, count);
  PG_ASSERT(res);
  return res;
}

#define pg_arena_new(a, t, n)                                                  \
  (t *)pg_arena_alloc(a, sizeof(t), _Alignof(typeof(t)), n)

#define pg_try_arena_new(a, t, n)                                              \
  ((t *)pg_try_arena_alloc((a), sizeof(t), _Alignof(typeof(t)), (n)))

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

[[maybe_unused]] [[nodiscard]] static PgHeapAllocator pg_make_heap_allocator() {
  return (PgHeapAllocator){
      .alloc_fn = pg_alloc_heap_libc,
      .realloc_fn = pg_realloc_heap_libc,
      .free_fn = pg_free_heap_libc,
  };
}

[[maybe_unused]] [[nodiscard]] static PgAllocator *
pg_heap_allocator_as_allocator(PgHeapAllocator *allocator) {
  return (PgAllocator *)allocator;
}

[[maybe_unused]] [[nodiscard]]
static PgAllocator *pg_heap_allocator() {
  static PgHeapAllocator pg_heap_allocator_ = {
      .alloc_fn = pg_alloc_heap_libc,
      .realloc_fn = pg_realloc_heap_libc,
      .free_fn = pg_free_heap_libc,
  };
  return pg_heap_allocator_as_allocator(&pg_heap_allocator_);
}

[[maybe_unused]] [[nodiscard]] static void *pg_alloc(PgAllocator *allocator,
                                                     u64 sizeof_type,
                                                     u64 alignof_type,
                                                     u64 elem_count) {
  PG_ASSERT(allocator);
  PG_ASSERT(allocator->alloc_fn);
  return allocator->alloc_fn(allocator, sizeof_type, alignof_type, elem_count);
}

#define PG_NEW(T, allocator) pg_alloc(allocator, sizeof(T), _Alignof(T), 1)

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

[[maybe_unused]] [[nodiscard]] static Pgu8Slice
pg_bytes_make(u64 len, PgAllocator *allocator) {
  Pgu8Slice res = {0};
  res.len = len;
  res.data = pg_alloc(allocator, sizeof(u8), _Alignof(u8), len);
  PG_ASSERT(res.data);
  return res;
}

[[maybe_unused]] [[nodiscard]] static char *
pg_string_to_cstr(PgString s, PgAllocator *allocator) {
  char *res = (char *)pg_alloc(allocator, sizeof(u8), 1, s.len + 1);
  PG_ASSERT(res);
  pg_memcpy(res, s.data, s.len);

  PG_ASSERT(0 == PG_C_ARRAY_AT(res, s.len + 1, s.len));

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_cstr_to_string(char *s) {
  return (PgString){
      .data = (u8 *)s,
      .len = strlen(s),
  };
}

[[maybe_unused]] [[nodiscard]] static PgCompare pg_string_cmp(PgString a,
                                                              PgString b) {
  int cmp = memcmp(a.data, b.data, PG_MIN(a.len, b.len));
  if (cmp < 0) {
    return PG_CMP_LESS;
  } else if (cmp > 0) {
    return PG_CMP_GREATER;
  } else if (a.len == b.len) {
    return PG_CMP_EQ;
  }

  PG_ASSERT(0 == cmp);
  PG_ASSERT(a.len != b.len);

  if (a.len < b.len) {
    return PG_CMP_LESS;
  }
  if (a.len > b.len) {
    return PG_CMP_GREATER;
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static int pg_string_cmp_qsort(const void *a,
                                                              const void *b) {
  return pg_string_cmp(*(const PgString *)a, *(const PgString *)b);
}

[[maybe_unused]] static void PG_DYN_GROW(void *slice, u64 size, u64 align,
                                         u64 count, PgAllocator *allocator) {
  PG_ASSERT(nullptr != slice);

  struct {
    void *data;
    u64 len;
    u64 cap;
  } PgReplica;

  pg_memcpy(&PgReplica, slice, sizeof(PgReplica));
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
  pg_memcpy(slice, &PgReplica, sizeof(PgReplica));
}

#define PG_DYN_ENSURE_CAP(dyn, new_cap, allocator)                             \
  ((new_cap > 0) && (dyn)->cap < (new_cap))                                    \
      ? PG_DYN_GROW(dyn, sizeof(*(dyn)->data),                                 \
                    _Alignof(typeof((dyn)->data[0])), new_cap, allocator),     \
      PG_ASSERT((dyn)->cap >= (new_cap)), PG_ASSERT((dyn)->data), 0 : 0

#define PG_DYN_SPACE(T, dyn)                                                   \
  ((T){.data = (dyn)->data + (dyn)->len, .len = (dyn)->cap - (dyn)->len})

#define PG_DYN_PUSH(s, elem, allocator)                                        \
  do {                                                                         \
    PG_DYN_ENSURE_CAP(s, (s)->len + 1, allocator);                             \
    (s)->len += 1;                                                             \
    *PG_SLICE_AT_PTR(s, (s)->len - 1) = (typeof((s)->data[0]))(elem);          \
  } while (0)

#define PG_DYN_PUSH_WITHIN_CAPACITY(s)                                         \
  (PG_ASSERT(((s)->len < (s)->cap)), (s)->len > 0 ? PG_ASSERT((s)->data) : 0,  \
   ((s)->data + (s)->len++))

#define PG_DYN_POP(s)                                                          \
  (0 == (s)->len || nullptr == (s)->data) ? (__builtin_trap(), (s)->data[0])   \
                                          : ((s)->data[--(s)->len])

#define PG_SLICE_LAST_PTR(s)                                                   \
  PG_C_ARRAY_AT_PTR((s)->data, (s)->len, (s)->len - 1)

#define PG_SLICE_LAST(s) PG_C_ARRAY_AT((s).data, (s).len, (s).len - 1)

#define PG_DYN_APPEND_SLICE(dst, src, allocator)                               \
  do {                                                                         \
    PG_DYN_ENSURE_CAP(dst, (dst)->len + (src).len, (allocator));               \
    memmove((dst)->data + (dst)->len * sizeof(*(dst)->data), (src).data,       \
            (src).len * sizeof(*(dst)->data));                                 \
    (dst)->len += (src).len;                                                   \
  } while (0)

#define PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(dst, src)                          \
  do {                                                                         \
    for (u64 _iii = 0; _iii < src.len; _iii++) {                               \
      *PG_DYN_PUSH_WITHIN_CAPACITY(dst) = PG_SLICE_AT(src, _iii);              \
    }                                                                          \
  } while (0)

#define PG_DYN_SLICE(T, dyn) ((T){.data = (dyn).data, .len = (dyn).len})

#define PG_DYN_CLONE(dst, src, allocator)                                      \
  do {                                                                         \
    PG_DYN_ENSURE_CAP(dst, (src).len, allocator);                              \
    for (u64 __pg_i = 0; __pg_i < (src).len; __pg_i++) {                       \
      *PG_DYN_PUSH_WITHIN_CAPACITY(dst) = PG_SLICE_AT(src, __pg_i);            \
    }                                                                          \
  } while (0)

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_net_create_tcp_socket();
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_close(PgFileDescriptor sock);
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_set_nodelay(PgFileDescriptor sock, bool enabled);
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_connect_ipv4(PgFileDescriptor sock, PgIpv4Address address);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_set_timeout(PgFileDescriptor sock, u64 seconds, u64 microseconds);

[[maybe_unused]] [[nodiscard]] static PgDnsResolveIpv4AddressSocketResult
pg_net_dns_resolve_ipv4_tcp(PgString host, u16 port, PgAllocator *allocator);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_tcp_listen(PgFileDescriptor sock, u64 backlog);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_tcp_bind_ipv4(PgFileDescriptor sock, PgIpv4Address addr);
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_enable_reuse(PgFileDescriptor sock);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_set_blocking(PgFileDescriptor sock, bool blocking);

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_net_socket_write(PgFileDescriptor sock, PgString data);

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_net_socket_read(PgFileDescriptor sock, PgString data);

[[nodiscard]] static Pgu64Result
pg_net_socket_read_non_blocking(PgFileDescriptor socket, PgString dst);

[[maybe_unused]] [[nodiscard]] static PgIpv4AddressAcceptResult
pg_net_tcp_accept(PgFileDescriptor sock);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_get_socket_error(PgFileDescriptor socket);

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorPairResult
pg_net_make_socket_pair(PgNetSocketDomain domain, PgNetSocketType type,
                        PgNetSocketOption option);

[[maybe_unused]] [[nodiscard]] static Pgu32Result pg_process_dup();

[[nodiscard]] static PgError pg_process_avoid_child_zombies();

[[maybe_unused]] [[nodiscard]] static Pgu8Dyn
pg_string_builder_make(u64 cap, PgAllocator *allocator) {
  Pgu8Dyn res = {0};
  PG_DYN_ENSURE_CAP(&res, cap, allocator);
  PG_ASSERT(res.data);
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgRing
pg_ring_make(u64 cap, PgAllocator *allocator) {
  return (PgRing){.data = pg_string_make(cap, allocator)};
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_is_empty(PgRing rg) {
  return rg.count == 0;
}

[[nodiscard]] static bool pg_ring_is_full(PgRing rg) {
  return rg.count == rg.data.len;
}

[[nodiscard]] static u64 pg_ring_can_read_count(PgRing rg) {
  if (rg.idx_read == rg.idx_write) { // Either full or empty.
    return rg.count;
  } else if (rg.idx_read < rg.idx_write) {
    u64 res = rg.idx_write - rg.idx_read;
    PG_ASSERT(res <= rg.count);
    return res;
  } else if (rg.idx_read > rg.idx_write) {
    u64 res = (rg.data.len - rg.idx_read) + rg.idx_write;
    PG_ASSERT(res <= rg.count);
    return res;
  }
  PG_ASSERT(0);
}

[[nodiscard]] static u64 pg_ring_can_write_count(PgRing rg) {
  if (rg.idx_write == rg.idx_read) {
    return 0 == rg.count ? rg.data.len : 0;
  } else if (rg.idx_write < rg.idx_read) {
    return rg.idx_read - rg.idx_write;
  } else if (rg.idx_write > rg.idx_read) {
    return (rg.data.len - rg.idx_write) + rg.idx_read;
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static u64 pg_ring_write_bytes(PgRing *rg,
                                                              Pgu8Slice src) {
  PG_ASSERT(nullptr != rg->data.data);
  PG_ASSERT(rg->idx_read < rg->data.len);
  PG_ASSERT(rg->idx_write < rg->data.len);
  PG_ASSERT(rg->data.len > 0);
  PG_ASSERT(rg->count <= rg->data.len);

  if (pg_ring_is_full(*rg) || 0 == src.len) {
    return 0;
  }

  u64 space = rg->data.len - rg->count;
  PG_ASSERT(space > 0);

  u64 write_count = PG_MIN(space, src.len);
  PG_ASSERT(write_count <= rg->data.len);

  if (rg->idx_write + write_count <= rg->data.len) { // 1 write.
    pg_memcpy(rg->data.data + rg->idx_write, src.data, write_count);

    rg->idx_write = (rg->idx_write + write_count) % rg->data.len;
    PG_ASSERT(rg->idx_write < rg->data.len);
  } else { // 2 writes.
    // Write as much as possible until the end.
    u64 write_to_end_count = rg->data.len - rg->idx_write;
    PG_ASSERT(write_to_end_count <= rg->data.len);
    PG_ASSERT(rg->idx_write + write_to_end_count == rg->data.len);
    pg_memcpy(rg->data.data + rg->idx_write, src.data, write_to_end_count);

    // Write the rest.
    pg_memcpy(rg->data.data, src.data + write_to_end_count,
              write_count - write_to_end_count);
    rg->idx_write = write_count - write_to_end_count;
    PG_ASSERT(rg->idx_write < rg->data.len);
  }

  rg->count += write_count;
  PG_ASSERT(rg->count <= rg->data.len);

  return write_count;
}

[[maybe_unused]] [[nodiscard]] static u64 pg_ring_read_bytes(PgRing *rg,
                                                             Pgu8Slice dst) {
  PG_ASSERT(nullptr != rg->data.data);
  PG_ASSERT(rg->idx_read < rg->data.len);
  PG_ASSERT(rg->idx_write < rg->data.len);
  PG_ASSERT(rg->count <= rg->data.len);

  if (pg_ring_is_empty(*rg) || 0 == dst.len) {
    return 0;
  }

  u64 read_count = PG_MIN(rg->count, dst.len);
  PG_ASSERT(read_count <= rg->data.len);
  PG_ASSERT(read_count <= rg->count);

  if (rg->idx_read + read_count <= rg->data.len) { // 1 read.
    pg_memcpy(dst.data, rg->data.data + rg->idx_read, read_count);
    rg->idx_read = (rg->idx_read + read_count) % rg->data.len;
    PG_ASSERT(rg->idx_read < rg->data.len);
  } else { // 2 reads.
    u64 read_to_end_count = rg->data.len - rg->idx_read;
    PG_ASSERT(read_to_end_count <= rg->data.len);
    PG_ASSERT(rg->idx_read + read_to_end_count == rg->data.len);
    pg_memcpy(rg->data.data + rg->idx_read, dst.data, read_to_end_count);

    // Rest.
    pg_memcpy(rg->data.data, dst.data + read_to_end_count,
              read_count - read_to_end_count);
    rg->idx_read = read_count - read_to_end_count;
  }

  rg->count -= read_count;
  PG_ASSERT(rg->count <= rg->data.len);

  return read_count;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Option
pg_ring_index_of_byte(PgRing rg, u8 needle) {
  PG_ASSERT(rg.data.data);
  Pgu64Option res = {0};

  if (rg.idx_read <= rg.idx_write) { // 1 memchr.
    u8 *start = rg.data.data + rg.idx_read;
    u64 len =
        rg.idx_read == rg.idx_write ? rg.count : (rg.idx_write - rg.idx_read);
    u8 *find = memchr(start, needle, len);
    if (find) {
      res.has_value = true;
      res.value = (u64)(find - start);
    }
    return res;
  }

  if (rg.idx_write < rg.idx_read) { // 2 memchr.
    {
      u8 *start = rg.data.data + rg.idx_read;
      u64 len = rg.data.len - rg.idx_read;
      u8 *find = memchr(start, needle, len);
      if (find) {
        res.has_value = true;
        res.value = (u64)(find - start);
      }
    }

    {
      u8 *start = rg.data.data;
      u64 len = rg.idx_write;
      u8 *find = memchr(start, needle, len);
      if (find) {
        res.has_value = true;
        res.value = (u64)(find - start);
      }
    }
  }
  return res;
}

static void pg_ring_read_skip(PgRing *rg, u64 count) {
  u64 skip = 0;
  if (rg->idx_read == rg->idx_write) {
    if (pg_ring_is_empty(*rg)) {
      return;
    }
    skip = PG_MIN(count, rg->count);
  } else if (rg->idx_read < rg->idx_write) {
    skip = PG_MIN(rg->idx_write - rg->idx_read, count);
  } else if (rg->idx_read > rg->idx_write) {
    skip = PG_MIN(rg->idx_read - rg->idx_write, count);
  }
  PG_ASSERT(skip <= count);
  PG_ASSERT(skip <= rg->count);
  PG_ASSERT(skip <= rg->data.len);

  rg->idx_read = (rg->idx_read + skip) % rg->data.len;
  rg->count -= skip;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Option
pg_ring_index_of_bytes2(PgRing rg, u8 needle0, u8 needle1) {
  Pgu64Option res = {0};

  if (pg_ring_is_empty(rg)) {
    return res;
  }

  u64 idx = 0;

  for (u64 _i = 0; _i < rg.data.len; _i++) {
    Pgu64Option idx_opt = pg_ring_index_of_byte(rg, needle0);
    if (!idx_opt.has_value) {
      return res;
    }

    u64 idx_it = idx_opt.value;

    pg_ring_read_skip(&rg, idx_it);
    idx += idx_it;

    {
      PgRing rg_tmp = rg;
      u8 tmp[2] = {0};
      Pgu8Slice tmp_slice = {.data = tmp, .len = 2};
      tmp_slice.len = pg_ring_read_bytes(&rg_tmp, tmp_slice);
      if (tmp_slice.len != 2) {
        return res;
      }

      PG_ASSERT(PG_SLICE_AT(tmp_slice, 0) == needle0);

      if (PG_SLICE_AT(tmp_slice, 0) == needle0 &&
          PG_SLICE_AT(tmp_slice, 1) == needle1) {
        PG_ASSERT(idx < rg.data.len);

        res.has_value = true;
        res.value = idx;
        return res;
      }
    }

    pg_ring_read_skip(&rg, 1);
    idx += 1;
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_try_read_u32(PgRing *rg,
                                                                u32 *val) {
  PgString s = {
      .data = (u8 *)val,
      .len = PG_MIN(pg_ring_can_read_count(*rg), sizeof(*val)),
  };
  return sizeof(*val) == pg_ring_read_bytes(rg, s);
}

[[maybe_unused]] [[nodiscard]] static PgError pg_writer_close(PgWriter *w) {
  PG_ASSERT(w);

  switch (w->kind) {
  case PG_WRITER_KIND_SOCKET:
  case PG_WRITER_KIND_FILE:
    return pg_file_close(w->u.file);

  case PG_WRITER_KIND_NONE:
  case PG_WRITER_KIND_BYTES:
    return 0;
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static PgError pg_reader_close(PgReader *r) {
  PG_ASSERT(r);

  switch (r->kind) {
  case PG_READER_KIND_SOCKET:
  case PG_READER_KIND_FILE:
    return pg_file_close(r->u.file);

  case PG_READER_KIND_NONE:
  case PG_READER_KIND_BYTES:
    return 0;
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_string_builder(u64 cap, PgAllocator *allocator) {
  PgWriter w = {0};
  w.kind = PG_WRITER_KIND_BYTES;
  PG_DYN_ENSURE_CAP(&w.u.bytes, cap, allocator);
  return w;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_do_write(PgWriter *w, Pgu8Slice src, PgAllocator *allocator) {
  Pgu64Result res = {0};

  switch (w->kind) {
  case PG_WRITER_KIND_NONE:
    res.value = src.len;
    return res;
  case PG_WRITER_KIND_FILE:
    return pg_file_write(w->u.file, src);
  case PG_WRITER_KIND_BYTES: {
    PG_DYN_APPEND_SLICE(&w->u.bytes, src, allocator);
    res.value = src.len;
    return res;
  case PG_WRITER_KIND_SOCKET:
    return pg_net_socket_write(w->u.socket, src);
  }
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_do_write_full(PgWriter *w, Pgu8Slice s, PgAllocator *allocator) {
  PgString remaining = s;
  for (u64 _i = 0; _i < s.len; _i++) {
    if (pg_string_is_empty(remaining)) {
      break;
    }

    Pgu64Result res = pg_writer_do_write(w, remaining, allocator);
    if (res.err) {
      return res.err;
    }

    if (0 == res.value) {
      return PG_ERR_IO;
    }

    remaining = PG_SLICE_RANGE_START(remaining, res.value);
  }
  return pg_string_is_empty(remaining) ? 0 : PG_ERR_IO;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_flush(PgWriter *w, PgAllocator *allocator) {
  PG_ASSERT(w);

  // Noop.
  if (0 == w->ring.data.len) {
    return 0;
  }

  for (u64 _i = 0; _i < w->ring.data.len; _i++) {
    u8 tmp[4096] = {0};
    Pgu8Slice tmp_slice = {
        .data = tmp,
        .len = PG_STATIC_ARRAY_LEN(tmp),
    };
    tmp_slice.len = pg_ring_read_bytes(&w->ring, tmp_slice);
    if (0 == tmp_slice.len) {
      return 0;
    }

    PgError err = pg_writer_do_write_full(w, tmp_slice, allocator);
    if (err) {
      return err;
    }
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_write(PgWriter *w, Pgu8Slice src, PgAllocator *allocator) {
  PG_ASSERT(w);

  Pgu64Result res = {0};
  if (PG_SLICE_IS_EMPTY(src)) {
    return res;
  }

  if (0 == w->ring.data.len) { // Simple reader.
    return pg_writer_do_write(w, src, allocator);
  }
  // Buffered reader.
  // Could `src` fit in the ring buffer?
  if (src.len > w->ring.data.len) {
    // Do not bother going through the ring buffer, just write all we can
    // directly.

    // TODO: Questionable to use `xxx_write_full` in a function that
    // is supposed to be able to do partial writes.
    PgError err = pg_writer_do_write_full(
        w, PG_SLICE_RANGE(src, 0, src.len - w->ring.data.len), allocator);
    if (err) {
      res.err = err;
      return res;
    }
    src = PG_SLICE_RANGE(src, 0, src.len % w->ring.data.len);
  }

  // `src` fits in the ring buffer.
  PG_ASSERT(src.len <= w->ring.data.len);

  if (pg_ring_can_write_count(w->ring) < src.len) {
    // Need to flush the ring buffer to make room.
    // NOTE: In theory we could do a partial flush and if that freed enough
    // room in the ring buffer, carry on.
    PgError err = pg_writer_flush(w, allocator);
    if (err) {
      res.err = err;
      return res;
    }
  }
  PG_ASSERT(src.len == pg_ring_write_bytes(&w->ring, src));
  res.value = src.len;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_write_u8(PgWriter *w, u8 c, PgAllocator *allocator) {
  Pgu8Slice src = {.data = &c, .len = 1};
  Pgu64Result res = pg_writer_write(w, src, allocator);
  if (res.err) {
    return res.err;
  }

  return res.value == 1 ? 0 : PG_ERR_IO;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_write_full(PgWriter *w, Pgu8Slice s, PgAllocator *allocator) {
  PgString remaining = s;
  for (u64 _i = 0; _i < s.len; _i++) {
    if (pg_string_is_empty(remaining)) {
      break;
    }

    Pgu64Result res = pg_writer_write(w, remaining, allocator);
    if (res.err) {
      return res.err;
    }

    if (0 == res.value) {
      return PG_ERR_IO;
    }

    remaining = PG_SLICE_RANGE_START(remaining, res.value);
  }
  return pg_string_is_empty(remaining) ? 0 : PG_ERR_IO;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_reader_do_read(PgReader *r, Pgu8Slice dst) {
  PG_ASSERT(dst.data);
  Pgu64Result res = {0};

  switch (r->kind) {
  case PG_READER_KIND_NONE:
    return res;
  case PG_READER_KIND_BYTES: {
    u64 n = PG_MIN(dst.len, r->u.bytes.len);
    if (n > 0) {
      PG_ASSERT(dst.data);
      PG_ASSERT(r->u.bytes.data);
      pg_memcpy(dst.data, r->u.bytes.data, n);
    }
    res.value = n;

    r->u.bytes = PG_SLICE_RANGE_START(r->u.bytes, n);

    return res;
  }
  case PG_READER_KIND_SOCKET:
    return pg_net_socket_read(r->u.socket, dst);
  case PG_READER_KIND_FILE:
    return pg_file_read(r->u.file, dst);
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_buf_reader_try_fill_once(PgReader *r) {
  PG_ASSERT(r);
  PG_ASSERT(r->ring.data.len);

  u64 ring_write_space = pg_ring_can_write_count(r->ring);
  u8 tmp[4096] = {0};
  Pgu8Slice tmp_slice = {
      .data = tmp,
      .len = PG_MIN(ring_write_space, PG_STATIC_ARRAY_LEN(tmp)),
  };
  // No more space.
  if (0 == tmp_slice.len) {
    return 0;
  }

  Pgu64Result res_read = pg_reader_do_read(r, tmp_slice);
  if (PG_ERR_EAGAIN == res_read.err) {
    return 0;
  }
  if (res_read.err) {
    return res_read.err;
  }
  if (0 == res_read.value) {
    return 0;
  }

  Pgu8Slice read_data = PG_SLICE_RANGE(tmp_slice, 0, res_read.value);
  PG_ASSERT(read_data.len == pg_ring_write_bytes(&r->ring, read_data));

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_reader_read(PgReader *r, Pgu8Slice dst) {
  PG_ASSERT(dst.data);

  Pgu64Result res = {0};

  if (PG_SLICE_IS_EMPTY(dst)) {
    return res;
  }

  if (0 == r->ring.data.len) { // Simple reader.
    return pg_reader_do_read(r, dst);
  }

  // Buffered reader.
  if (0 == pg_ring_can_read_count(r->ring)) {
    // Do a real read to re-fill the ring buffer.
    PgError err = pg_buf_reader_try_fill_once(r);
    if (err) {
      res.err = err;
    }
  }

  res.value = pg_ring_read_bytes(&r->ring, dst);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError pg_reader_read_full(PgReader *r,
                                                                  Pgu8Slice s) {
  PgString remaining = s;
  for (u64 _i = 0; _i < s.len; _i++) {
    if (pg_string_is_empty(remaining)) {
      break;
    }

    Pgu64Result res = pg_reader_read(r, remaining);
    if (res.err) {
      return res.err;
    }

    if (0 == res.value) {
      return PG_ERR_IO;
    }

    remaining = PG_SLICE_RANGE_START(remaining, res.value);
  }
  return pg_string_is_empty(remaining) ? 0 : PG_ERR_IO;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_reader_do_read_non_blocking(PgReader *r, Pgu8Slice dst) {
  PG_ASSERT(dst.data);
  Pgu64Result res = {0};

  switch (r->kind) {
  case PG_READER_KIND_NONE:
    return res;
  case PG_READER_KIND_BYTES:
    return pg_reader_do_read(r, dst);
  case PG_READER_KIND_SOCKET:
    return pg_net_socket_read_non_blocking(r->u.socket, dst);
  case PG_READER_KIND_FILE:
    return pg_file_read(r->u.file, dst);
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_write_from_reader(PgWriter *w, PgReader *r, PgAllocator *allocator) {
  Pgu64Result res = {0};

  // TODO: Get a hint from the reader?
  u8 tmp[4096] = {0};
  PgString dst = {.data = tmp, .len = PG_STATIC_ARRAY_LEN(tmp)};

  res = pg_reader_read(r, dst);
  if (res.err) {
    return res;
  }
  dst.len = res.value;

  res = pg_writer_write(w, dst, allocator);
  if (res.err) {
    return res;
  }

  // WARN: In that case, there is data loss.
  // Not all readers support putting back data that could not be written out.
  if (res.value != dst.len) {
    res.err = PG_ERR_IO;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_write_u64_as_string(PgWriter *w, u64 n, PgAllocator *allocator) {
  u8 tmp[30] = {0};
  u64 idx = PG_STATIC_ARRAY_LEN(tmp);

  do {
    idx -= 1;
    PG_C_ARRAY_AT(tmp, PG_STATIC_ARRAY_LEN(tmp), idx) = '0' + (n % 10);
    n /= 10;
  } while (n > 0);

  PG_ASSERT(idx <= PG_STATIC_ARRAY_LEN(tmp));

  PgString s = {.data = tmp + idx, .len = PG_STATIC_ARRAY_LEN(tmp) - idx};

  return pg_writer_write_full(w, s, allocator);
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_write_i64_as_string(PgWriter *w, i64 n, PgAllocator *allocator) {
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

  return pg_writer_write_full(w, s, allocator);
}

[[maybe_unused]] static void pg_u32_to_u8x4_be(u32 n, PgString *dst) {
  PG_ASSERT(sizeof(n) == dst->len);

  *(PG_SLICE_AT_PTR(dst, 0)) = (u8)(n >> 24);
  *(PG_SLICE_AT_PTR(dst, 1)) = (u8)(n >> 16);
  *(PG_SLICE_AT_PTR(dst, 2)) = (u8)(n >> 8);
  *(PG_SLICE_AT_PTR(dst, 3)) = (u8)(n >> 0);
}

[[maybe_unused]] static void
pg_string_builder_append_rune(Pgu8Dyn *sb, PgRune rune,
                              PgAllocator *allocator) {
  Pgu8Slice bytes = {.data = (u8 *)&rune,
                     .len = pg_utf8_rune_bytes_count(rune)};
  PG_DYN_APPEND_SLICE(sb, bytes, allocator);
}

[[maybe_unused]] [[nodiscard]] static u8 pg_u8_to_hex_rune(u8 n) {
  PG_ASSERT(n < 16);
  const u8 lut[] = "0123456789abcdef";
  return lut[n];
}

[[maybe_unused]] static void
pg_string_builder_append_u64_hex(Pgu8Dyn *sb, PgRune rune,
                                 PgAllocator *allocator) {
  u8 tmp[64] = {0};
  u64 tmp_cap = PG_STATIC_ARRAY_LEN(tmp);

  u64 i = tmp_cap;
  do {
    PG_ASSERT(i > 0);
    i -= 1;
    *PG_C_ARRAY_AT_PTR(tmp, tmp_cap, i) = pg_u8_to_hex_rune(rune & 0xf);
  } while (rune /= 16);
  Pgu8Slice slice = {.data = tmp + i, .len = tmp_cap - i};
  PG_DYN_APPEND_SLICE(sb, slice, allocator);
}

[[maybe_unused]] static void pg_string_builder_append_string_escaped_any(
    Pgu8Dyn *sb, PgString s, PgString runes_to_escape, PgRune rune_escape,
    PgAllocator *allocator) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res_rune.err);
    if (res_rune.end) {
      break;
    }
    PgRune rune = res_rune.rune;
    if (0 == rune) {
      break;
    }

    if (-1 != pg_string_index_of_rune(runes_to_escape, rune)) {
      pg_string_builder_append_rune(sb, rune_escape, allocator);
    }
    pg_string_builder_append_rune(sb, rune, allocator);
  }
}

[[maybe_unused]] static void
pg_string_builder_append_string(Pgu8Dyn *sb, PgString s,
                                PgAllocator *allocator) {
  PG_DYN_APPEND_SLICE(sb, s, allocator);
}

[[maybe_unused]] static void pg_string_builder_append_string_escaped(
    Pgu8Dyn *sb, PgString s, PgRune rune_to_escape, PgRune rune_escape,
    PgAllocator *allocator) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res_rune.err);
    if (res_rune.end) {
      break;
    }
    PgRune rune = res_rune.rune;

    if (rune_to_escape == rune) {
      pg_string_builder_append_rune(sb, rune_escape, allocator);
    }
    pg_string_builder_append_rune(sb, rune, allocator);
  }
}

[[maybe_unused]] static void
pg_string_builder_append_js_string_escaped(Pgu8Dyn *sb, PgString s,
                                           PgAllocator *allocator) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);

  for (u64 i = 0; i < s.len; i++) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res_rune.err);
    if (res_rune.end) {
      break;
    }
    PgRune rune = res_rune.rune;
    if (0 == rune) {
      break;
    }

    switch (rune) {
    case '\'':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\'"), allocator);
      break;
    case '"':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\\""), allocator);
      break;
    case '\\':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\\\"), allocator);
      break;
    case '\t':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\t"), allocator);
      break;
    case '\n':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\n"), allocator);
      break;
    case '\r':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\r"), allocator);
      break;
    case '\f':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\f"), allocator);
      break;
    case '\b':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\b"), allocator);
      break;
    case '\v':
      PG_DYN_APPEND_SLICE(sb, PG_S("\\v"), allocator);
      break;
    default:
      if (rune <= 127) {
        pg_string_builder_append_rune(sb, rune, allocator);
      } else {
        PG_DYN_PUSH(sb, '\\', allocator);
        PG_DYN_PUSH(sb, 'u', allocator);
        PG_DYN_PUSH(sb, '{', allocator);
        pg_string_builder_append_u64_hex(sb, rune, allocator);
        PG_DYN_PUSH(sb, '}', allocator);
      }
    }
  }
}

[[maybe_unused]] static void
pg_byte_buffer_append_u32_be(Pgu8Dyn *dyn, u32 n, PgAllocator *allocator) {

  u8 data[sizeof(n)] = {0};
  PgString s = {.data = data, .len = sizeof(n)};
  pg_u32_to_u8x4_be(n, &s);
  PG_DYN_APPEND_SLICE(dyn, s, allocator);
}

[[maybe_unused]] static void
pg_byte_buffer_append_u32_be_within_capacity(Pgu8Dyn *dyn, u32 n) {

  u8 data[sizeof(n)] = {0};
  PgString s = {.data = data, .len = sizeof(n)};
  pg_u32_to_u8x4_be(n, &s);
  PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(dyn, s);
}

[[maybe_unused]] static void pg_byte_buffer_append_u8(Pgu8Dyn *dyn, u8 n,
                                                      PgAllocator *allocator) {

  PG_DYN_PUSH(dyn, n, allocator);
}

[[maybe_unused]] static void pg_byte_buffer_append_u16(Pgu8Dyn *dyn, u16 n,
                                                       PgAllocator *allocator) {

  PgString s = {.data = (u8 *)&n, .len = sizeof(n)};
  PG_DYN_APPEND_SLICE(dyn, s, allocator);
}

[[maybe_unused]] static void
pg_byte_buffer_append_u16_within_capacity(Pgu8Dyn *dyn, u16 n) {

  PgString s = {.data = (u8 *)&n, .len = sizeof(n)};
  PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(dyn, s);
}

[[maybe_unused]] static void pg_byte_buffer_append_u32(Pgu8Dyn *dyn, u32 n,
                                                       PgAllocator *allocator) {

  PgString s = {.data = (u8 *)&n, .len = sizeof(n)};
  PG_DYN_APPEND_SLICE(dyn, s, allocator);
}

[[maybe_unused]] static void
pg_byte_buffer_append_u32_within_capacity(Pgu8Dyn *dyn, u32 n) {

  PgString s = {.data = (u8 *)&n, .len = sizeof(n)};
  PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(dyn, s);
}

[[maybe_unused]] static void pg_byte_buffer_append_u64(Pgu8Dyn *dyn, u64 n,
                                                       PgAllocator *allocator) {

  PgString s = {.data = (u8 *)&n, .len = sizeof(n)};
  PG_DYN_APPEND_SLICE(dyn, s, allocator);
}

[[maybe_unused]] static void
pg_byte_buffer_append_u64_within_capacity(Pgu8Dyn *dyn, u64 n) {

  PgString s = {.data = (u8 *)&n, .len = sizeof(n)};
  PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(dyn, s);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_u64_to_string(u64 n, PgAllocator *allocator) {
  PgWriter w = pg_writer_make_string_builder(25, allocator);

  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, n, allocator));

  return PG_DYN_SLICE(PgString, w.u.bytes);
}

[[maybe_unused]] static void
pg_string_builder_append_u64(Pgu8Dyn *sb, u64 n, PgAllocator *allocator) {
  PgWriter w = {.kind = PG_WRITER_KIND_BYTES, .u.bytes = *sb};
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, n, allocator));
  *sb = w.u.bytes;
}

[[maybe_unused]] [[nodiscard]]
static PgError pg_writer_write_u8_hex_upper(PgWriter *w, u8 n,
                                            PgAllocator *allocator) {

  u8 c1 = n & 15; // i.e. `% 16`.
  u8 c2 = n >> 4; // i.e. `/ 16`

  PgError err = 0;
  err = pg_writer_write_u8(
      w, (u8)pg_rune_ascii_to_upper_case(pg_u8_to_hex_rune(c2)), allocator);
  if (err) {
    return err;
  }
  err = pg_writer_write_u8(
      w, (u8)pg_rune_ascii_to_upper_case(pg_u8_to_hex_rune(c1)), allocator);
  if (err) {
    return err;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]]
static PgString pg_bytes_to_hex_string(Pgu8Slice bytes, PgRune sep,
                                       PgAllocator *allocator) {

  Pgu8Dyn sb = pg_string_builder_make(bytes.len * 3, allocator);
  Pgu8Slice sep_slice = {
      .data = (u8 *)&sep,
      .len = pg_utf8_rune_bytes_count(sep),
  };

  for (u64 i = 0; i < bytes.len; i++) {
    u8 n = PG_SLICE_AT(bytes, i);

    u8 c1 = n & 15; // i.e. `% 16`.
    u8 c2 = n >> 4; // i.e. `/ 16`

    *PG_DYN_PUSH_WITHIN_CAPACITY(&sb) =
        (u8)pg_rune_ascii_to_upper_case(pg_u8_to_hex_rune(c2));
    *PG_DYN_PUSH_WITHIN_CAPACITY(&sb) =
        (u8)pg_rune_ascii_to_upper_case(pg_u8_to_hex_rune(c1));
    if (!pg_string_is_empty(sep_slice)) {
      PG_DYN_APPEND_SLICE(&sb, sep_slice, allocator);
    }
  }

  return PG_DYN_SLICE(PgString, sb);
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
pg_string_index_of_unescaped_rune(PgString haystack, PgRune needle,
                                  PgRune escape) {
  while (!pg_string_is_empty(haystack)) {
    i64 idx = pg_string_index_of_rune(haystack, needle);
    if (-1 == idx) {
      return -1;
    }

    PgString remaining = PG_SLICE_RANGE_START(
        haystack, (u64)idx + pg_utf8_rune_bytes_count(needle));

    PgRuneOption first_opt = pg_string_first(remaining);
    if (!first_opt.has_value) {
      return -1;
    }
    if (escape != first_opt.value) {
      return idx;
    }
    haystack = remaining;
  }
  return -1;
}

[[maybe_unused]] [[nodiscard]] static i64
pg_string_index_of_any_unescaped_rune(PgString haystack, PgString needles,
                                      PgRune escape) {
  PgUtf8Iterator it = pg_make_utf8_iterator(needles);
  for (;;) {
    PgRuneUtf8Result res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || res_rune.end) {
      break;
    }

    PgRune needle = res_rune.rune;
    i64 idx = pg_string_index_of_unescaped_rune(haystack, needle, escape);
    if (-1 != idx) {
      return idx;
    }
  }
  return -1;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_clone(PgString s, PgAllocator *allocator) {
  PgString res = pg_string_make(s.len, allocator);
  if (res.data != nullptr) {
    pg_memcpy(res.data, s.data, s.len);
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool pg_string_ieq_ascii(PgString a,
                                                               PgString b) {
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

  for (u64 i = 0; i < a.len; i++) {
    u8 c_a = PG_SLICE_AT(a, i);
    u8 c_b = PG_SLICE_AT(b, i);

    if (pg_rune_ascii_to_lower_case(c_a) != pg_rune_ascii_to_lower_case(c_b)) {
      return false;
    }
  }
  return true;
}

#if defined(__x86_64__) && defined(__SSSE3__) && defined(__SHA__)
#include <immintrin.h>
// Process as many 64 bytes chunks as possible.
static void pg_sha1_process_x86(u32 state[5], const u8 data[], u32 length) {
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

[[maybe_unused]] [[nodiscard]] static PgSha1 pg_sha1(PgString s) {
  PG_SHA1_CTX ctx = {0};
  PG_SHA1Init(&ctx);

  // Process as many 64 bytes chunks as possible.
  pg_sha1_process_x86(ctx.state, s.data, (u32)s.len);

  u64 len_rounded_down_64 = (s.len / 64) * 64;
  u64 rem = s.len % 64;
  pg_memcpy(ctx.buffer, s.data + len_rounded_down_64, rem);

  ctx.count = s.len * 8;
  PgSha1 res = {0};
  PG_SHA1Final(res.data, &ctx);

  return res;
}

#else
[[maybe_unused]] [[nodiscard]] static PgSha1 pg_sha1(PgString s) {
  PG_SHA1_CTX ctx = {0};
  PG_SHA1Init(&ctx);
  PG_SHA1Update(&ctx, s.data, s.len);
  PgSha1 res = {0};
  PG_SHA1Final(res.data, &ctx);
  return res;
}
#endif

[[maybe_unused]] [[nodiscard]] static PgString
pg_net_ipv4_address_to_string(PgIpv4Address address, PgAllocator *allocator) {
  PgWriter w = pg_writer_make_string_builder(3 * 4 + 4 + 5, allocator);

  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, (address.ip >> 24) & 0xFF,
                                               allocator));
  PG_ASSERT(0 == pg_writer_write_u8(&w, '.', allocator));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, (address.ip >> 16) & 0xFF,
                                               allocator));
  PG_ASSERT(0 == pg_writer_write_u8(&w, '.', allocator));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, (address.ip >> 8) & 0xFF,
                                               allocator));
  PG_ASSERT(0 == pg_writer_write_u8(&w, '.', allocator));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, (address.ip >> 0) & 0xFF,
                                               allocator));
  PG_ASSERT(0 == pg_writer_write_u8(&w, ':', allocator));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, address.port, allocator));

  return PG_DYN_SLICE(PgString, w.u.bytes);
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

[[maybe_unused]] [[nodiscard]] static Pgu64Option
pg_bitfield_get_first_zero(PgString bitfield) {
  Pgu64Option res = {0};

  for (u64 i = 0; i < bitfield.len; i++) {
    u8 c = PG_SLICE_AT(bitfield, i);
    if (0xff == c) {
      continue;
    }

    u64 bit_idx = 0;
    u8 bit_pattern = 1;
    for (u64 j = 0; j < 8; j++) {
      if (0 == (c & bit_pattern)) {
        bit_idx = j;
        break;
      }
      bit_pattern <<= 1;
    }
    PG_ASSERT(bit_idx < 8);
    PG_ASSERT(bit_idx > 0);

    res.value = i * 8 + (bit_idx - 1);
    res.has_value = true;
    return res;
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgAdjacencyMatrix
pg_adjacency_matrix_make(u64 nodes_count, PgAllocator *allocator) {
  PgAdjacencyMatrix res = {0};
  if (0 == nodes_count) {
    return res;
  }

  res.nodes_count = nodes_count;
  res.bitfield_len = nodes_count * (nodes_count - 1) / 2;
  res.bitfield = pg_string_make(res.bitfield_len, allocator);
  return res;
}

[[nodiscard]] static u64
pg_adjacency_matrix_row_column_to_bitfield_index(PgAdjacencyMatrix matrix,
                                                 u64 row, u64 col) {
  PG_ASSERT(row > 0);
  PG_ASSERT(col < matrix.nodes_count - 1);
  PG_ASSERT(row >= col);

  u64 idx = (row * (row - 1)) / 2 + col;
  PG_ASSERT(idx < matrix.bitfield_len);
  return idx;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_adjacency_matrix_has_edge(PgAdjacencyMatrix matrix, u64 row, u64 col) {
  u64 idx = pg_adjacency_matrix_row_column_to_bitfield_index(matrix, row, col);
  return pg_bitfield_get(matrix.bitfield, idx);
}

[[maybe_unused]] [[nodiscard]] static PgAdjacencyMatrix
pg_adjacency_matrix_clone(PgAdjacencyMatrix matrix, PgAllocator *allocator) {
  PgAdjacencyMatrix res = {0};
  res.nodes_count = matrix.nodes_count;
  res.bitfield_len = matrix.bitfield_len;
  res.bitfield = pg_string_clone(matrix.bitfield, allocator);
  return res;
}

[[maybe_unused]]
static void pg_adjacency_matrix_add_edge(PgAdjacencyMatrix *matrix, u64 row,
                                         u64 col) {
  PG_ASSERT(row < matrix->nodes_count);
  PG_ASSERT(col < matrix->nodes_count);

  u64 idx = pg_adjacency_matrix_row_column_to_bitfield_index(*matrix, row, col);

  pg_bitfield_set(matrix->bitfield, idx, 1);
}

static void pg_adjacency_matrix_remove_edge(PgAdjacencyMatrix *matrix, u64 row,
                                            u64 col) {
  PG_ASSERT(row < matrix->nodes_count);
  PG_ASSERT(col < matrix->nodes_count);

  u64 idx = pg_adjacency_matrix_row_column_to_bitfield_index(*matrix, row, col);

  pg_bitfield_set(matrix->bitfield, idx, 0);
}

[[maybe_unused]] [[nodiscard]]
static PgAdjacencyMatrixNeighborIterator
pg_adjacency_matrix_make_neighbor_iterator(PgAdjacencyMatrix matrix, u64 node) {
  PgAdjacencyMatrixNeighborIterator it = {0};
  it.matrix = matrix;
  it.row = node;
  return it;
}

[[maybe_unused]] [[nodiscard]]
static PgAdjacencyMatrixNeighbor pg_adjacency_matrix_neighbor_iterator_next(
    PgAdjacencyMatrixNeighborIterator *it) {
  PG_ASSERT(it);
  PG_ASSERT(it->row <= it->matrix.nodes_count);
  PG_ASSERT(it->col <= it->matrix.nodes_count);

  PgAdjacencyMatrixNeighbor res = {0};
  if (it->row >= it->matrix.nodes_count || it->col >= it->matrix.nodes_count ||
      it->col > it->row) { // The end.
    return res;
  }

  if (!it->scan_mode_column) { // Scan row.

    for (; it->col < it->row; it->col++) {
      res.edge = pg_adjacency_matrix_has_edge(it->matrix, it->row, it->col);
      if (res.edge) {
        res.has_value = true;
        res.col = it->col;
        res.row = it->row;
        res.node = it->col;
        PG_ASSERT(res.col != res.row);

        it->col += 1;
        return res;
      }
    }
    if (it->col == it->row) { // Skip diagonal.
      it->row += 1;
      it->scan_mode_column = true;
    }
  }

  if (it->scan_mode_column) { // Scan column.

    for (; it->row < it->matrix.nodes_count; it->row++) {
      res.edge = pg_adjacency_matrix_has_edge(it->matrix, it->row, it->col);
      if (res.edge) {
        res.has_value = true;
        res.col = it->col;
        res.row = it->row;
        res.node = it->row;
        PG_ASSERT(res.col != res.row);

        it->row += 1;
        return res;
      }
    }
  }
  return res;
}

[[maybe_unused]]
static void pg_adjacency_matrix_remove_node(PgAdjacencyMatrix *matrix,
                                            u64 node) {
  u64 row = node;
  PG_ASSERT(row < matrix->nodes_count);

  PgAdjacencyMatrixNeighborIterator it =
      pg_adjacency_matrix_make_neighbor_iterator(*matrix, node);

  PgAdjacencyMatrixNeighbor neighbor = {0};
  do {
    neighbor = pg_adjacency_matrix_neighbor_iterator_next(&it);
    if (!neighbor.has_value) {
      break;
    }

    pg_adjacency_matrix_remove_edge(matrix, neighbor.row, neighbor.col);
  } while (neighbor.has_value);
}

[[maybe_unused]] [[nodiscard]]
static bool pg_adjacency_matrix_is_empty(PgAdjacencyMatrix matrix) {
  bool set = false;
  for (u64 i = 0; i < matrix.bitfield.len; i++) {
    set |= PG_SLICE_AT(matrix.bitfield, i);
  }
  return set == 0;
}

[[maybe_unused]] [[nodiscard]]
static u64 pg_adjacency_matrix_count_neighbors(PgAdjacencyMatrix matrix,
                                               u64 node) {
  PG_ASSERT(node < matrix.nodes_count);

  u64 res = 0;
  PgAdjacencyMatrixNeighborIterator it =
      pg_adjacency_matrix_make_neighbor_iterator(matrix, node);

  PgAdjacencyMatrixNeighbor neighbor = {0};
  do {
    neighbor = pg_adjacency_matrix_neighbor_iterator_next(&it);
    res += neighbor.edge;
    PG_ASSERT(res + 1 <= matrix.nodes_count);
  } while (neighbor.has_value);

  return res;
}

// FIXME: Should print to a writer.
[[maybe_unused]] static void
pg_adjacency_matrix_print(PgAdjacencyMatrix matrix) {
  printf("    | ");
  for (u64 col = 0; col < matrix.nodes_count; col++) {
    printf("%03" PRIu64, col);
  }
  printf("\n");
  for (u64 col = 0; col < matrix.nodes_count; col++) {
    printf("-----");
  }
  printf("\n");

  for (u64 row = 0; row < matrix.nodes_count; row++) {
    printf("%03" PRIu64 " | ", row);
    for (u64 col = 0; col < matrix.nodes_count; col++) {
      if (col >= row) {
        printf("  ");
        continue;
      }

      bool edge = pg_adjacency_matrix_has_edge(matrix, row, col);
      printf("%s ", edge ? "1  " : "0  ");
    }
    printf("\n");
  }
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_time_ns_now(PgClockKind clock_kind);

// From https://nullprogram.com/blog/2017/09/21/.
// PCG.
[[maybe_unused]] [[nodiscard]] static u32
pg_rand_u32_min_incl_max_incl(PgRng *rng, u32 min_incl, u32 max_incl) {
  PG_ASSERT(rng);
  PG_ASSERT(min_incl <= max_incl);

  u64 m = 0x9b60933458e17d7d;
  u64 a = 0xd737232eeccdf7ed;
  rng->state = rng->state * m + a;
  int shift = 29 - (rng->state >> 61);
  u32 rand = (u32)(rng->state >> shift);

  u32 res = min_incl + (u32)((f32)rand * ((f32)max_incl - (f32)min_incl) /
                             (f32)UINT32_MAX);
  PG_ASSERT(min_incl <= res);
  PG_ASSERT(res <= max_incl);
  return res;
}

[[maybe_unused]] [[nodiscard]] static u32
pg_rand_u32_min_incl_max_excl(PgRng *rng, u32 min_incl, u32 max_excl) {
  PG_ASSERT(max_excl > 0);
  return pg_rand_u32_min_incl_max_incl(rng, min_incl, max_excl - 1);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_rand_string(PgRng *rng, u64 len, PgAllocator *allocator) {
  PgString res = pg_string_make(len, allocator);

  for (u64 i = 0; i < res.len; i++) {
    *PG_C_ARRAY_AT_PTR(res.data, res.len, i) =
        (u8)pg_rand_u32_min_incl_max_incl(rng, 0, UINT8_MAX);
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu32Option
pg_bitfield_get_first_zero_rand(PgString bitfield, u32 len, PgRng *rng) {
  PG_ASSERT(len <= bitfield.len);

  Pgu32Option res = {0};

  u32 start = pg_rand_u32_min_incl_max_excl(rng, 0, len);
  for (u64 i = 0; i < len; i++) {
    u32 idx = (start + i) % len;
    if (pg_bitfield_get(bitfield, idx)) {
      continue;
    }
    res.value = idx;
    res.has_value = true;
    return res;
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgRng pg_rand_make() {
  PgRng rng = {0};
  // Rely on ASLR.
  Pgu64Result now = pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC);
  PG_ASSERT(0 == now.err);
  rng.state = (u64)(&pg_rand_make) ^ now.value;

  return rng;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_close(PgFileDescriptor file);

[[nodiscard]] static u64 pg_os_get_page_size();

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
  PG_ASSERT(res_alloc.value);
  u8 *alloc = res_alloc.value;

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

[[maybe_unused]] [[nodiscard]] static Pgu64RangeOption
pg_u64_range_search(Pgu64Slice haystack, u64 needle) {
  Pgu64RangeOption res = {0};

  if (0 == haystack.len) {
    return res;
  }

  for (u64 i = 1; i < haystack.len; i++) {
    u64 elem = PG_SLICE_AT(haystack, i);
    if (needle < elem) {
      res.has_value = true;
      res.value.idx = i - 1;
      res.value.start_incl = PG_SLICE_AT(haystack, i - 1);
      res.value.end_excl = elem;

      PG_ASSERT(res.value.start_incl <= needle);
      PG_ASSERT(needle < res.value.end_excl);
      return res;
    }
  }

  return res;
}

// TODO: Trim separators in `a` and `b`?
[[maybe_unused]] [[nodiscard]] static PgString
pg_path_join(PgString a, PgString b, PgAllocator *allocator) {
  PgString sep = PG_S(PG_PATH_SEPARATOR_S);

  Pgu8Dyn sb = pg_string_builder_make(a.len + sep.len + b.len, allocator);

  PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(&sb, a);
  PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(&sb, sep);
  PG_DYN_APPEND_SLICE_WITHIN_CAPACITY(&sb, b);

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static PgString pg_path_base_name(PgString s) {
  i64 idx = pg_string_last_index_of_rune(s, PG_PATH_SEPARATOR);
  if (-1 == idx) {
    return s;
  } else {
    return PG_SLICE_RANGE_START(s, (u64)idx + 1);
  }
}

[[maybe_unused]] [[nodiscard]] static PgString pg_path_dir_name(PgString s) {
  i64 idx = pg_string_last_index_of_rune(s, PG_PATH_SEPARATOR);
  if (-1 == idx) {
    return s;
  } else {
    return PG_SLICE_RANGE(s, 0, (u64)idx);
  }
}

[[maybe_unused]] [[nodiscard]] static PgString pg_path_stem(PgString s) {
  i64 idx = pg_string_last_index_of_rune(s, '.');
  if (-1 == idx) {
    return s;
  }

  return PG_SLICE_RANGE(s, 0, (u64)idx);
}

[[maybe_unused]] [[nodiscard]] static PgString pg_file_stem(PgString s) {
  PgString base_name = pg_path_base_name(s);
  i64 idx = pg_string_last_index_of_rune(base_name, '.');
  if (-1 == idx) {
    return base_name;
  }

  return PG_SLICE_RANGE(base_name, 0, (u64)idx);
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_concat(PgString left, PgString right, PgAllocator *allocator) {
  PgString res = pg_string_make(left.len + right.len, allocator);
  PG_ASSERT(res.data);

  pg_memmove(res.data, left.data, left.len);
  pg_memmove(res.data + left.len, right.data, right.len);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_file_descriptor(PgFileDescriptor file, u64 buffer_size,
                                    PgAllocator *allocator) {
  PgWriter w = {0};
  w.kind = PG_WRITER_KIND_FILE;
  w.u.file = file;

  if (buffer_size) {
    w.ring = pg_ring_make(buffer_size, allocator);
  }

  return w;
}

[[maybe_unused]] [[nodiscard]] static PgWriter pg_writer_make_noop() {
  PgWriter w = {0};
  return w;
}

[[maybe_unused]] [[nodiscard]] static PgStringResult
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
    Pgu64Result res_read = pg_file_read(file, space);
    if (res_read.err) {
      res.err = (PgError)pg_os_get_last_error();
      goto end;
    }

    u64 read_n = res_read.value;
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

  res.value = PG_DYN_SLICE(PgString, sb);
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringResult
pg_file_read_full_from_descriptor_until_eof(PgFileDescriptor file,
                                            u64 size_hint,
                                            PgAllocator *allocator) {
  PgStringResult res = {0};

  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, size_hint, allocator);

  for (;;) {
    PG_DYN_ENSURE_CAP(&sb, 4096, allocator);
    PgString space = {.data = sb.data + sb.len, .len = sb.cap - sb.len};
    PG_ASSERT(space.len);

    Pgu64Result res_read = pg_file_read(file, space);
    if (res_read.err) {
      res.err = (PgError)pg_os_get_last_error();
      goto end;
    }

    u64 read_n = res_read.value;
    if (0 == read_n) {
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

  res.value = PG_DYN_SLICE(PgString, sb);
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgReader
pg_reader_make_from_file(PgFileDescriptor file, u64 buffer_size,
                         PgAllocator *allocator) {
  PgReader r = {0};
  r.kind = PG_READER_KIND_FILE;
  r.u.file = file;
  if (buffer_size) {
    r.ring = pg_ring_make(buffer_size, allocator);
  }
  return r;
}

[[maybe_unused]] [[nodiscard]] static PgReader
pg_reader_make_from_socket(PgFileDescriptor file, u64 buffer_size,
                           PgAllocator *allocator) {
  PgReader r = {0};
  r.kind = PG_READER_KIND_SOCKET;
  r.u.file = file;
  if (buffer_size) {
    r.ring = pg_ring_make(buffer_size, allocator);
  }
  return r;
}

[[maybe_unused]] [[nodiscard]] static PgReader
pg_reader_make_from_bytes(Pgu8Slice bytes) {
  PgReader r = {0};
  r.kind = PG_READER_KIND_BYTES;
  r.u.bytes = bytes;
  return r;
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_socket(PgFileDescriptor file, u64 buffer_size,
                           PgAllocator *allocator) {
  PgWriter w = {0};
  w.kind = PG_WRITER_KIND_SOCKET;
  w.u.file = file;
  if (buffer_size) {
    w.ring = pg_ring_make(buffer_size, allocator);
  }
  return w;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_write_full_with_descriptor(PgFileDescriptor file, Pgu8Slice content) {
  PgError err = 0;
  PgString remaining = content;
  for (u64 lim = 0; lim < content.len; lim++) {
    if (pg_string_is_empty(remaining)) {
      break;
    }

    Pgu64Result res_write = pg_file_write(file, remaining);
    if (res_write.err) {
      err = (PgError)pg_os_get_last_error();
      goto end;
    }

    u64 write_n = res_write.value;
    if (0 == write_n) {
      err = (PgError)PG_ERR_INVALID_VALUE;
      goto end;
    }

    remaining = PG_SLICE_RANGE_START(remaining, write_n);
  }

end:
  return err;
}

[[maybe_unused]] [[nodiscard]] static PgStringResult
pg_file_read_full_from_path(PgString path, PgAllocator *allocator) {
  PgStringResult res = {0};

  PgFileDescriptorResult res_file =
      pg_file_open(path, PG_FILE_ACCESS_READ, 0600, false, allocator);
  if (res_file.err) {
    res.err = res_file.err;
    return res;
  }

  PgFileDescriptor file = res_file.value;

  Pgu64Result res_size = pg_file_size(file);
  if (res_size.err) {
    res.err = res_size.err;
    goto end;
  }

  u64 size = res_size.value;

  res = pg_file_read_full_from_descriptor(file, size, allocator);

end:
  (void)pg_file_close(file);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_write_full(PgString path, PgString content, u64 mode,
                   PgAllocator *allocator) {
  PgError err = 0;

  PgFileDescriptorResult res_file =
      pg_file_open(path, PG_FILE_ACCESS_WRITE, mode, true, allocator);
  if (res_file.err) {
    err = res_file.err;
    return err;
  }

  PgFileDescriptor file = res_file.value;

  err = pg_file_write_full_with_descriptor(file, content);

  (void)pg_file_close(file);

  return err;
}

[[maybe_unused]] [[nodiscard]]
static PgProcessResult pg_process_spawn(PgString path, PgStringSlice args,
                                        PgProcessSpawnOptions options,
                                        PgAllocator *allocator);

[[maybe_unused]] [[nodiscard]]
static PgProcessExitResult
pg_process_wait(PgProcess process, u64 stdio_size_hint, u64 stderr_size_hint,
                PgAllocator *allocator);

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_send_to_socket(PgFileDescriptor dst, PgFileDescriptor src);

// This works from any kind of file descriptor to any kind of file descriptor.
// But this may be slower than OS-specific syscalls.
[[maybe_unused]] [[nodiscard]] static PgError
pg_file_copy_with_descriptors_until_eof(PgFileDescriptor dst,
                                        PgFileDescriptor src, u64 offset) {
  // NOTE: on FreeBSD, we could use `copy_file_range` (and Linux too, actually),
  // or `splice`.

  for (;;) {
    u8 read_buf[4096] = {0};
    Pgu8Slice read_slice = {.data = read_buf,
                            .len = PG_STATIC_ARRAY_LEN(read_buf)};

    Pgu64Result res_read = pg_file_read(src, read_slice);
    if (res_read.err) {
      return res_read.err;
    }
    if (0 == res_read.value) {
      return (PgError)0;
    }

    if (offset > res_read.value) {
      // Keep reading and do not write yet.

      offset -= res_read.value;
      continue;
    }

    PG_ASSERT(offset <= res_read.value);

    Pgu8Slice write_slice = {.data = read_slice.data + offset,
                             .len = res_read.value - offset};
    PgError err = pg_file_write_full_with_descriptor(dst, write_slice);
    if (err) {
      return err;
    }

    if (offset > 0) {
      offset = 0;
    }
  }
}

// ----- Start UNIX implementation ------
#ifdef PG_OS_UNIX

#define PG_PIPE_READ 0
#define PG_PIPE_WRITE 1

[[maybe_unused]] [[nodiscard]] PgDirectoryResult
pg_directory_open(PgString name) {
  PgDirectoryResult res = {0};
  if (name.len > PG_PATH_MAX - 1) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  u8 name_c[PG_PATH_MAX] = {0};
  memcpy(name_c, name.data, name.len);

  res.value.dir = opendir((char *)name_c);
  if (!res.value.dir) {
    res.err = (PgError)errno;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] PgError pg_directory_close(PgDirectory dir) {
  i32 ret = closedir(dir.dir);
  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] PgDirectoryEntryResult
pg_directory_read(PgDirectory *dir) {
  PgDirectoryEntryResult res = {0};

  errno = 0;

  res.value = readdir(dir->dir);
  if (!res.value && 0 == errno) {
    return res; // EOF
  }
  if (!res.value) {
    res.err = (PgError)errno;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_dirent_is_file(PgDirectoryEntry dirent) {
  return DT_REG == dirent->d_type;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_dirent_is_directory(PgDirectoryEntry dirent) {
  return DT_DIR == dirent->d_type;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_dirent_name(PgDirectoryEntry dirent) {
  return pg_cstr_to_string(dirent->d_name);
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_register_watch_directory(PgAio *aio, PgString name,
                                PgWalkDirectoryOption options,
                                PgAllocator *allocator) {
  bool ignore_errors = options & PG_WALK_DIRECTORY_KIND_IGNORE_ERRORS;
  bool recurse = options & PG_WALK_DIRECTORY_KIND_RECURSE;

  PgDirectoryResult res_dir = pg_directory_open(name);
  if (0 != res_dir.err) {
    return res_dir.err;
  }

  PgDirectory dir = res_dir.value;

  for (;;) {
    PgDirectoryEntryResult res_dirent = pg_directory_read(&dir);
    if (0 != res_dirent.err) {
      return res_dir.err;
    }

    if (!res_dirent.value) {
      break; // EOF.
    }

    PgDirectoryEntry dirent = res_dirent.value;
    PgString name = pg_dirent_name(dirent);

    // Do not visit parent.
    if (pg_bytes_eq(name, PG_S(".."))) {
      continue;
    }

    if (pg_dirent_is_file(dirent) && (PG_WALK_DIRECTORY_KIND_FILE & options)) {
      PgFileDescriptorResult res_fs = pg_aio_register_interest_fs_name(
          aio, name,
          PG_AIO_EVENT_KIND_FILE_MODIFIED | PG_AIO_EVENT_KIND_FILE_DELETED,
          allocator);
      if (0 != res_fs.err && !ignore_errors) {
        return res_fs.err;
      }
    }

    if (pg_dirent_is_directory(dirent) &&
        (PG_WALK_DIRECTORY_KIND_DIRECTORY & options)) {
      PgFileDescriptorResult res_fs = pg_aio_register_interest_fs_name(
          aio, name,
          PG_AIO_EVENT_KIND_FILE_MODIFIED | PG_AIO_EVENT_KIND_FILE_DELETED |
              PG_AIO_EVENT_KIND_FILE_CREATED,
          allocator);
      if (0 != res_fs.err && !ignore_errors) {
        return res_fs.err;
      }
    }

    if (pg_dirent_is_directory(dirent) && recurse) {
      PgError err =
          pg_aio_register_watch_directory(aio, name, options, allocator);
      if (0 != err && !ignore_errors) {
        return err;
      }
    }
  }

  if (0 != pg_directory_close(dir)) {
    return res_dir.err;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] PgError pg_mtx_init(PgMutex *mutex,
                                                   PgMutexKind type) {
  pthread_mutexattr_t attr = {0};
  i32 err = pthread_mutexattr_init(&attr);
  if (err) {
    return err;
  }

  if (PG_MUTEX_KIND_RECURSIVE == type) {
    err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (err) {
      goto end;
    }
  }

  err = pthread_mutex_init(mutex, &attr);

end:
  i32 fin_err = pthread_mutexattr_destroy(&attr);
  if (fin_err) {
    return err ? err : fin_err;
  }

  return (PgError)err;
}

[[maybe_unused]] void pg_mtx_destroy(PgMutex *mutex) {
  pthread_mutex_destroy(mutex);
}

[[maybe_unused]] [[nodiscard]] PgError pg_mtx_lock(PgMutex *mutex) {
  i32 ret = pthread_mutex_lock(mutex);
  if (0 != ret) {
    return (PgError)ret;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] PgError pg_mtx_trylock(PgMutex *mutex) {
  i32 ret = pthread_mutex_trylock(mutex);
  if (0 != ret) {
    return (PgError)ret;
  }

  return 0;
}

#if 0
// TODO
[[maybe_unused]][[nodiscard]] PgError pg_mtx_timedlock(PgMutex *mutex, const struct timespec *time_point);
#endif

[[maybe_unused]] [[nodiscard]] PgError pg_mtx_unlock(PgMutex *mutex) {
  i32 ret = pthread_mutex_unlock(mutex);
  if (0 != ret) {
    return (PgError)ret;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] PgError pg_cnd_init(PgConditionVar *cond) {
  i32 ret = pthread_cond_init(cond, nullptr);
  if (0 != ret) {
    return (PgError)ret;
  }

  return 0;
}

[[maybe_unused]] void pg_cnd_destroy(PgConditionVar *cond) {
  pthread_cond_destroy(cond);
}

[[maybe_unused]] [[nodiscard]] PgError pg_cnd_wait(PgConditionVar *cond,
                                                   PgMutex *mutex) {
  i32 ret = pthread_cond_wait(cond, mutex);
  if (0 != ret) {
    return (PgError)ret;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] PgError pg_cnd_broadcast(PgConditionVar *cond) {
  i32 ret = pthread_cond_broadcast(cond);
  if (0 != ret) {
    return (PgError)ret;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] PgError pg_cnd_signal(PgConditionVar *cond) {
  i32 ret = pthread_cond_signal(cond);
  if (0 != ret) {
    return (PgError)ret;
  }

  return 0;
}

#if 0
// TODO
[[maybe_unused]][[nodiscard]]PgError pg_cnd_timedwait(PgConditionVar *cond, PgMutex *mutex, const struct timespec *time_point);
#endif

[[maybe_unused]] [[nodiscard]] static PgThreadResult
pg_thread_create(PgThreadFn fn, void *fn_data) {
  PgThreadResult res = {0};

  i32 ret = pthread_create(&res.value, nullptr, (void *(*)(void *))fn, fn_data);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] PgError pg_thread_join(PgThread thread) {
  i32 ret = pthread_join(thread, nullptr);
  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorPairResult
pg_net_make_socket_pair(PgNetSocketDomain domain, PgNetSocketType type,
                        PgNetSocketOption option) {
  PgFileDescriptorPairResult res = {0};

  i32 unix_domain = 0;
  switch (domain) {
  case PG_NET_SOCKET_DOMAIN_LOCAL:
    unix_domain = AF_LOCAL;
    break;
  case PG_NET_SOCKET_DOMAIN_IPV4:
    unix_domain = AF_INET;
    break;
  case PG_NET_SOCKET_DOMAIN_IPV6:
    unix_domain = AF_INET6;
    break;
  default:
    PG_ASSERT(0);
  }

  i32 unix_type = 0;
  switch (type) {
  case PG_NET_SOCKET_TYPE_TCP:
    unix_type = SOCK_STREAM;
    break;
  case PG_NET_SOCKET_TYPE_UDP:
    unix_type = SOCK_DGRAM;
    break;
  default:
    PG_ASSERT(0);
  }

  i32 unix_option = 0;
  switch (option) {
  case PG_NET_SOCKET_OPTION_NONE:
  default:
  }

  i32 fds[2] = {0};
  i32 ret = socketpair(unix_domain, unix_type, unix_option, fds);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value.first.fd = fds[0];
  res.value.second.fd = fds[1];
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorPairResult
pg_pipe_make() {
  PgFileDescriptorPairResult res = {0};

  int fds[2] = {0};
  int ret = pipe(fds);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value.first.fd = fds[PG_PIPE_READ];
  res.value.second.fd = fds[PG_PIPE_WRITE];
  return res;
}

[[nodiscard]] static PgError pg_process_avoid_child_zombies() {
  struct sigaction handler = {.sa_handler = SIG_IGN};
  int ret = sigaction(SIGCHLD, &handler, nullptr);
  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu32Result pg_process_dup() {
  Pgu32Result res = {0};

  i32 pid = fork();

  if (-1 == pid) {
    res.err = (PgError)errno;
    return res;
  }

  res.value = (u32)pid;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_set_timeout(PgFileDescriptor sock, u64 seconds,
                          u64 microseconds) {
  struct timeval timeout = {0};
  timeout.tv_sec = (time_t)seconds;
  timeout.tv_usec = (i32)microseconds;

  i32 ret =
      setsockopt(sock.fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  if (-1 == ret) {
    return (PgError)errno;
  }
  return 0;
}

[[nodiscard]]
static PgFileDescriptorResult pg_net_create_tcp_socket() {
  PgFileDescriptorResult res = {0};

  int sock_fd = 0;
  do {
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  } while (-1 == sock_fd && EINTR == errno);

  if (-1 == sock_fd) {
    res.err = (PgError)errno;
    return res;
  }

  res.value.fd = sock_fd;

  return res;
}

[[nodiscard]]
static PgError pg_net_socket_close(PgFileDescriptor sock) {
  return pg_file_close(sock);
}

[[nodiscard]]
static PgError pg_net_set_nodelay(PgFileDescriptor sock, bool enabled) {
  int opt = enabled;
  int ret = 0;
  do {
    ret = setsockopt(sock.fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[nodiscard]]
static PgError pg_net_connect_ipv4(PgFileDescriptor sock,
                                   PgIpv4Address address) {
  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(address.port),
      .sin_addr = {htonl(address.ip)},
  };

  int ret = 0;
  do {
    ret = connect(sock.fd, (struct sockaddr *)&addr, sizeof(addr));
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    if (EINPROGRESS != errno) {
      return (PgError)errno;
    }
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]]
static PgDnsResolveIpv4AddressSocketResult
pg_net_dns_resolve_ipv4_tcp(PgString host, u16 port, PgAllocator *allocator) {
  PgDnsResolveIpv4AddressSocketResult res = {0};

  struct addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *addr_info = nullptr;
  int res_getaddrinfo = 0;
  do {
    res_getaddrinfo = getaddrinfo(
        pg_string_to_cstr(host, allocator),
        pg_string_to_cstr(pg_u64_to_string(port, allocator), allocator), &hints,
        &addr_info);
  } while (-1 == res_getaddrinfo && EINTR == errno);

  if (-1 == res_getaddrinfo) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  struct addrinfo *rp = nullptr;
  for (rp = addr_info; rp != nullptr; rp = rp->ai_next) {
    PgFileDescriptorResult res_create_socket = pg_net_create_tcp_socket();
    if (res_create_socket.err) {
      res.err = res_create_socket.err;
      continue;
    }

    // TODO: Use pg_net_connect_ipv4?
    int ret = 0;
    do {
      ret = connect(res_create_socket.value.fd, rp->ai_addr, rp->ai_addrlen);
    } while (-1 == ret && EINTR == errno);

    if (-1 == ret) {
      if (EINPROGRESS != errno) {
        (void)pg_net_socket_close(res_create_socket.value);
        continue;
      }
    }

    res.value.socket = res_create_socket.value;

    res.value.address.ip =
        ntohl(((struct sockaddr_in *)(void *)rp->ai_addr)->sin_addr.s_addr);
    res.value.address.port = port;
    break;
  }

  freeaddrinfo(addr_info);

  if (nullptr == rp) { // No address succeeded.
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_net_socket_write(PgFileDescriptor sock, PgString data) {

  i64 n = 0;
  do {
    n = send(sock.fd, data.data, data.len, MSG_NOSIGNAL);
  } while (-1 == n && EINTR == errno);

  Pgu64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.value = (u64)n;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_net_socket_read(PgFileDescriptor sock, PgString data) {

  i64 n = 0;
  do {
    n = recv(sock.fd, data.data, data.len, 0);
  } while (-1 == n && EINTR == errno);

  Pgu64Result res = {0};
  if (n < 0) {
    res.err = (PgError)errno;
  } else {
    res.value = (u64)n;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_tcp_listen(PgFileDescriptor sock, u64 backlog) {
  PG_ASSERT(backlog <= INT32_MAX);

  int ret = 0;
  do {
    ret = listen(sock.fd, (int)backlog);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_tcp_bind_ipv4(PgFileDescriptor sock, PgIpv4Address addr) {
  struct sockaddr_in addrin = {0};
  addrin.sin_family = AF_INET;
  addrin.sin_port = htons(addr.port);
  addrin.sin_addr.s_addr = htonl(addr.ip);

  int ret = 0;
  do {
    ret = bind(sock.fd, (struct sockaddr *)&addrin, sizeof(addrin));
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_get_socket_error(PgFileDescriptor sock) {
  int socket_error = -1;
  socklen_t len = sizeof(socket_error);
  int ret = 0;
  do {
    ret = getsockopt(sock.fd, SOL_SOCKET, SO_ERROR, &socket_error, &len);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    return (PgError)errno;
  }
  return (PgError)socket_error;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_enable_reuse(PgFileDescriptor sock) {
  int val = 1;
  int ret = 0;
  do {
    ret = setsockopt(sock.fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    return (PgError)errno;
  }

  do {
    ret = setsockopt(sock.fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgIpv4AddressAcceptResult
pg_net_tcp_accept(PgFileDescriptor sock) {
  PgIpv4AddressAcceptResult res = {0};

  struct sockaddr_in sockaddrin = {0};
  socklen_t sockaddrin_len = sizeof(sockaddrin);
  int sock_client = 0;
  do {
    sock_client =
        accept(sock.fd, (struct sockaddr *)&sockaddrin, &sockaddrin_len);
  } while (-1 == sock_client && EINTR == errno);

  if (-1 == sock_client) {
    res.err = (PgError)errno;
    return res;
  }

  res.socket.fd = sock_client;
  res.address.port = ntohs(sockaddrin.sin_port);
  res.address.ip = ntohl(sockaddrin.sin_addr.s_addr);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_rewind_start(PgFileDescriptor f) {
  off_t ret = lseek(f.fd, 0, SEEK_SET);
  if (-1 == ret) {
    return (PgError)errno;
  }
  return 0;
}

// TODO: Review in the context of multiple threads spawning and reaping
// processes.
[[maybe_unused]] [[nodiscard]]
static PgProcessResult pg_process_spawn(PgString path, PgStringSlice args,
                                        PgProcessSpawnOptions options,
                                        PgAllocator *allocator) {
  PgProcessResult res = {0};

  char *path_c = (char *)pg_string_to_cstr(path, allocator);

  PgCstrDyn args_c = {0};
  PG_DYN_ENSURE_CAP(&args_c, args.len + 2, allocator);
  PG_DYN_PUSH(&args_c, path_c, allocator);

  for (u64 i = 0; i < args.len; i++) {
    PgString arg = PG_SLICE_AT(args, i);

    PG_DYN_PUSH(&args_c, pg_string_to_cstr(arg, allocator), allocator);
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
      close(stdin_pipe[PG_PIPE_WRITE]);
      int ret_dup2 = dup2(stdin_pipe[PG_PIPE_READ], STDIN_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    } else if (PG_CHILD_PROCESS_STD_IO_CLOSE == options.stdin_capture) {
      close(STDIN_FILENO);
    } else if (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stdin_capture) {
      int ret_dup2 = dup2(fd_ignore, STDIN_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    }

    if (PG_CHILD_PROCESS_STD_IO_PIPE == options.stdout_capture) {
      close(stdout_pipe[PG_PIPE_READ]);
      int ret_dup2 = dup2(stdout_pipe[PG_PIPE_WRITE], STDOUT_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    } else if (PG_CHILD_PROCESS_STD_IO_CLOSE == options.stdout_capture) {
      close(STDOUT_FILENO);
    } else if (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stdout_capture) {
      int ret_dup2 = dup2(fd_ignore, STDOUT_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    }

    if (PG_CHILD_PROCESS_STD_IO_PIPE == options.stderr_capture) {
      close(stderr_pipe[PG_PIPE_READ]);
      int ret_dup2 = dup2(stderr_pipe[PG_PIPE_WRITE], STDERR_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    } else if (PG_CHILD_PROCESS_STD_IO_CLOSE == options.stderr_capture) {
      close(STDERR_FILENO);
    } else if (PG_CHILD_PROCESS_STD_IO_IGNORE == options.stderr_capture) {
      int ret_dup2 = dup2(fd_ignore, STDERR_FILENO);
      PG_ASSERT(-1 != ret_dup2);
    }

    if (-1 == execvp(path_c, args_c.data)) {
      // Not easy to give the error back to the caller here.
      // Maybe with a separate pipe?
      PG_ASSERT(0);
    }
    PG_ASSERT(0 && "unreachable");
  }

  PG_ASSERT(pid > 0); // Parent.
  res.value.pid = (u64)pid;
  res.value.stdin_pipe.fd = stdin_pipe[PG_PIPE_WRITE];
  res.value.stdout_pipe.fd = stdout_pipe[PG_PIPE_READ];
  res.value.stderr_pipe.fd = stderr_pipe[PG_PIPE_READ];

end:
  for (u64 i = 0; i < args_c.len; i++) {
    char *arg_c = PG_SLICE_AT(args_c, i);
    pg_free(allocator, arg_c);
  }

  if (stdin_pipe[PG_PIPE_READ]) {
    close(stdin_pipe[PG_PIPE_READ]);
  }
  if (stdout_pipe[PG_PIPE_WRITE]) {
    close(stdout_pipe[PG_PIPE_WRITE]);
  }
  if (stderr_pipe[PG_PIPE_WRITE]) {
    close(stderr_pipe[PG_PIPE_WRITE]);
  }

  if (res.err) {
    if (stdin_pipe[PG_PIPE_WRITE]) {
      close(stdin_pipe[PG_PIPE_WRITE]);
    }
    if (stdout_pipe[PG_PIPE_READ]) {
      close(stdout_pipe[PG_PIPE_READ]);
    }
    if (stderr_pipe[PG_PIPE_READ]) {
      close(stderr_pipe[PG_PIPE_READ]);
    }
  }

  return res;
}

// TODO: Review in the context of multiple threads spawning and reaping
// processes.
[[maybe_unused]] [[nodiscard]]
static PgProcessCaptureStdResult
pg_process_capture_std_io(PgProcess process, u64 stdio_size_hint,
                          u64 stderr_size_hint, PgAllocator *allocator) {
  PgProcessCaptureStdResult res = {0};

  Pgu8Dyn stdout_sb = {0};
  Pgu8Dyn stderr_sb = {0};

  if (process.stdout_pipe.fd) {
    stdout_sb = pg_string_builder_make(
        stdio_size_hint == 0 ? 4096 : stdio_size_hint, allocator);
  }

  if (process.stderr_pipe.fd) {
    stderr_sb = pg_string_builder_make(
        stderr_size_hint == 0 ? 4096 : stderr_size_hint, allocator);
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

      if (pollfd.revents & (POLLERR | POLLNVAL)) {
        close(pollfd.fd);
        fd->fd = 0;
        continue;
      }

      // Not sure why/how it could happen?
      if (0 == (pollfd.revents & (POLLIN | POLLHUP))) {
        continue;
      }

      u8 tmp[4096] = {0};
      ssize_t read_n = read(pollfd.fd, tmp, PG_STATIC_ARRAY_LEN(tmp));
      if (-1 == read_n) {
        res.err = (PgError)errno;
        goto end;
      }
      if (0 == read_n) {
        close(pollfd.fd);
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
    close(process.stdin_pipe.fd);
  }
  if (process.stdout_pipe.fd) {
    close(process.stdout_pipe.fd);
  }
  if (process.stderr_pipe.fd) {
    close(process.stderr_pipe.fd);
  }

  res.value.stdout_captured = PG_DYN_SLICE(PgString, stdout_sb);
  res.value.stderr_captured = PG_DYN_SLICE(PgString, stderr_sb);
  return res;
}

[[maybe_unused]] [[nodiscard]]
static PgProcessExitResult
pg_process_wait(PgProcess process, u64 stdio_size_hint, u64 stderr_size_hint,
                PgAllocator *allocator) {
  PgProcessExitResult res = {0};

  PgProcessCaptureStdResult res_capture = pg_process_capture_std_io(
      process, stdio_size_hint, stderr_size_hint, allocator);
  if (res_capture.err) {
    res.err = res_capture.err;
    return res;
  }

  int status = 0;
  int ret = 0;
  do {
    ret = waitpid((pid_t)process.pid, &status, 0);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value.exit_status = WEXITSTATUS(status);
  res.value.exited = WIFEXITED(status);
  res.value.signaled = WIFSIGNALED(status);
  res.value.signal = WTERMSIG(status);
  res.value.core_dumped = WCOREDUMP(status);
  res.value.stopped = WIFSTOPPED(status);
  res.value.stdout_captured = res_capture.value.stdout_captured;
  res.value.stderr_captured = res_capture.value.stderr_captured;

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
  res.value = mmap(nullptr, size, pg_virtual_mem_flags_to_os_flags(flags),
                   MAP_ANON | MAP_PRIVATE, -1, 0);
  if ((void *)-1 == res.value) {
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

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_time_ns_now(PgClockKind clock) {
  Pgu64Result res = {0};

  struct timespec ts = {0};
  int ret = 0;
  do {
    ret = clock_gettime(pg_clock_to_linux(clock), &ts);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value = (u64)ts.tv_sec * PG_Seconds + (u64)ts.tv_nsec;

  return res;
}

[[maybe_unused]] static u64
pg_fill_call_stack(u64 pie_offset, u64 call_stack[PG_STACKTRACE_MAX]) {
  u64 *frame_pointer = __builtin_frame_address(0);
  u64 res = 0;

  while (res < PG_STACKTRACE_MAX && frame_pointer != 0 &&
         ((u64)frame_pointer & 7) == 0 && *frame_pointer != 0) {
    u64 instruction_pointer = *(frame_pointer + 1);
    // Careful not to enter an infinite recursion of `PG_ASSERT ->
    // pg_fill_call_stack`.
    PG_ASSERT_TRAP_ONLY(instruction_pointer >= pie_offset);

    frame_pointer = (u64 *)*frame_pointer;

    // `ip` points to the return instruction in the caller, once this call
    // is done. But: We want the location of the call i.e. the `call xxx`
    // instruction, so we subtract one byte to point inside it, which is not
    // quite 'at' it, but good enough.
    call_stack[res++] = (instruction_pointer - 1) - pie_offset;
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

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_file_read(PgFileDescriptor file, PgString buf) {
  Pgu64Result res = {0};

  isize n = 0;
  do {
    n = read(file.fd, buf.data, buf.len);
  } while (-1 == n && EINTR == errno);

  if (-1 == n) {
    res.err = (PgError)errno;
    return res;
  }

  res.value = (u64)n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_file_write(PgFileDescriptor file, PgString s) {
  Pgu64Result res = {0};

  isize n = 0;
  do {
    n = write(file.fd, s.data, s.len);
  } while (-1 == n && EINTR == errno);

  if (-1 == n) {
    res.err = (PgError)errno;
    return res;
  }

  res.value = (u64)n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_truncate(PgFileDescriptor file, u64 size) {
  if (-1 == ftruncate(file.fd, (i64)size)) {
    return (PgError)pg_os_get_last_error();
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_file_read_at(PgFileDescriptor file, PgString buf, u64 offset) {
  Pgu64Result res = {0};

  isize n = 0;
  do {
    n = pread(file.fd, buf.data, buf.len, (off_t)offset);
  } while (-1 == n && EINTR == errno);

  if (-1 == n) {
    res.err = (PgError)errno;
    return res;
  }

  res.value = (u64)n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_file_open(PgString path, PgFileAccess access, u64 mode,
             bool create_if_not_exists, PgAllocator *allocator) {
  PgFileDescriptorResult res = {0};

  int flags = 0;
  switch (access & PG_FILE_ACCESS_ALL) {
  case PG_FILE_ACCESS_READ:
    flags = O_RDONLY;
    break;
  case PG_FILE_ACCESS_WRITE:
    flags = O_WRONLY | O_TRUNC;
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
  int fd = open(path_os, flags, mode ? mode : 0600);
  pg_free(allocator, path_os);

  if (-1 == fd) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.value.fd = fd;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_close(PgFileDescriptor file) {
  if (-1 == close(file.fd)) {
    return (PgError)pg_os_get_last_error();
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_file_size(PgFileDescriptor file) {
  Pgu64Result res = {0};
  struct stat st = {0};

  int ret = 0;
  do {
    ret = fstat(file.fd, &st);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.value = (u64)st.st_size;
  return res;
}

[[nodiscard]] static Pgu64Result
pg_net_socket_read_non_blocking(PgFileDescriptor socket, PgString dst) {
  Pgu64Result res = {0};

  i64 ret = recv(socket.fd, dst.data, dst.len, MSG_DONTWAIT);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value = (u64)ret;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_fd_set_blocking(PgFileDescriptor fd, bool block) {
  i32 ret = 0;
  do {
    ret = fcntl(fd.fd, F_GETFL, O_NONBLOCK);
  } while (-1 == ret && EINTR == errno);
  if (-1 == ret) {
    return (PgError)errno;
  }

  i32 flags = ret;
  if (block) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }

  do {
    ret = fcntl(fd.fd, F_SETFL, flags);
  } while (-1 == ret && EINTR == errno);
  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
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

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_win32_write(PgWriter *w, Pgu8Slice src) {
  PG_ASSERT(nullptr != w);
  if (PG_SLICE_IS_EMPTY(src)) {
    return (Pgu64Result){0};
  }

  PgFileDescriptor file = w->ctx;
  HANDLE handle = file.ptr;
  DWORD n = 0;
  bool has_value = WriteFile(handle, src.data, (DWORD)src.len, &n, nullptr);
  Pgu64Result res = {0};
  if (!has_value) {
    res.err = (PgError)pg_os_get_last_error();
  } else {
    res.value = (u64)n;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_file_write(PgWriter *w, Pgu8Slice src, PgAllocator *) {
  return pg_writer_win32_write(w, src);
}

[[nodiscard]] static u64 pg_os_get_page_size() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  u64 res = (u64)info.dwPageSize;

  PG_ASSERT(res > 0);
  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_time_ns_now(PgClockKind clock_kind) {
  Pgu64Result res = {0};

  switch (clock_kind) {
  case PG_CLOCK_KIND_MONOTONIC: {
    LARGE_INTEGER frequency = {0};
    if (!frequency.QuadPart) {
      QueryPerformanceFrequency(&frequency);
      PG_ASSERT(frequency.QuadPart);
    }
    LARGE_INTEGER counter = {0};
    PG_ASSERT(QueryPerformanceCounter(&counter));
    f64 seconds = (f64)counter.QuadPart / (f64)frequency.QuadPart;

    res.value = (u64)(PG_Seconds * seconds);
    return res;
  }
  case PG_CLOCK_KIND_REALTIME: {
    FILETIME ft = {0};
    GetSystemTimeAsFileTime(&ft);
    res.value = (((u64)ft.dwHighDateTime) << 32) | ((u64)ft.dwLowDateTime);
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

  res.value = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE,
                           (DWORD)pg_virtual_mem_flags_to_os_flags(flags));
  if (!res.value) {
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

  res.value.len = (u64)wlen + 1;
  res.value.data =
      pg_alloc(allocator, sizeof(wchar_t), _Alignof(wchar_t), res.value.len);
  PG_ASSERT(res.value.data);
  PG_ASSERT(0 != MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                     (const char *)s.data, (i32)s.len,
                                     res.value.data, (i32)res.value.len));
  PG_ASSERT(0 == res.value.data[res.value.len - 1]);

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
pg_file_open(PgString path, PgFileAccess access, u64 mode,
             bool create_if_not_exists, PgAllocator *allocator) {
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

  HANDLE handle =
      CreateFileW(res_path_os.value.data, desired_access,
                  FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr,
                  creation_disposition, FILE_ATTRIBUTE_NORMAL, nullptr);

  pg_free(allocator, res_path_os.value.data);

  if (INVALID_HANDLE_VALUE == handle) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.value.ptr = handle;
  return res;
}

[[nodiscard]] static PgError pg_file_close(PgFileDescriptor file) {
  if (0 == CloseHandle(file.ptr)) {
    return (PgError)pg_os_get_last_error();
  }

  return 0;
}

[[nodiscard]] static Pgu64Result pg_file_size(PgFileDescriptor file) {
  Pgu64Result res = {0};

  LARGE_INTEGER size;
  if (0 == GetFileSizeEx(file.ptr, &size)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.value = (u64)size.QuadPart;
  return res;
}

[[nodiscard]] static Pgu64Result pg_file_read(PgFileDescriptor file,
                                              PgString dst) {
  Pgu64Result res = {0};

  if (0 == ReadFile(file.ptr, dst.data, (DWORD)dst.len, (LPDWORD)&res.value,
                    nullptr)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  return res;
}

[[nodiscard]] static Pgu64Result pg_file_write(PgFileDescriptor file,
                                               PgString s) {
  Pgu64Result res = {0};

  if (0 ==
      WriteFile(file.ptr, s.data, (DWORD)s.len, (LPDWORD)&res.value, nullptr)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  return res;
}

[[nodiscard]] static Pgu64Result pg_file_read_at(PgFileDescriptor file,
                                                 PgString dst, u64 offset) {
  Pgu64Result res = {0};

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

[[maybe_unused]]
PgString static pg_http_method_to_string(PgHttpMethod m) {
  switch (m) {
  case PG_HTTP_METHOD_UNKNOWN:
    return PG_S("UNKNOWN");
  case PG_HTTP_METHOD_OPTIONS:
    return PG_S("OPTIONS");
  case PG_HTTP_METHOD_GET:
    return PG_S("GET");
  case PG_HTTP_METHOD_HEAD:
    return PG_S("HEAD");
  case PG_HTTP_METHOD_POST:
    return PG_S("POST");
  case PG_HTTP_METHOD_PUT:
    return PG_S("PUT");
  case PG_HTTP_METHOD_DELETE:
    return PG_S("DELETE");
  case PG_HTTP_METHOD_TRACE:
    return PG_S("TRACE");
  case PG_HTTP_METHOD_CONNECT:
    return PG_S("CONNECT");
  case PG_HTTP_METHOD_EXTENSION:
    return PG_S("EXTENSION");
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static PgHttpResponseStatusLineResult
pg_http_parse_response_status_line(PgString status_line) {
  PgHttpResponseStatusLineResult res = {0};

  PgString remaining = status_line;
  {
    PgStringOption consume_opt =
        pg_string_consume_string(remaining, PG_S("HTTP/"));
    if (!consume_opt.has_value) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume_opt.value;
  }

  {
    PgParseNumberResult res_major = pg_string_parse_u64(remaining, 10, true);
    if (!res_major.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_major.n > 3) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.value.version_major = (u8)res_major.n;
    remaining = res_major.remaining;
  }

  {
    PgStringOption consume_opt = pg_string_consume_rune(remaining, '.');
    if (!consume_opt.has_value) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume_opt.value;
  }

  {
    PgParseNumberResult res_minor = pg_string_parse_u64(remaining, 10, true);
    if (!res_minor.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_minor.n > 9) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.value.version_minor = (u8)res_minor.n;
    remaining = res_minor.remaining;
  }

  {
    PgStringOption consume_opt = pg_string_consume_rune(remaining, ' ');
    if (!consume_opt.has_value) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume_opt.value;
  }

  {
    PgParseNumberResult res_status_code =
        pg_string_parse_u64(remaining, 10, true);
    if (!res_status_code.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_status_code.n < 100 || res_status_code.n > 599) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.value.status = (u16)res_status_code.n;
    remaining = res_status_code.remaining;
  }

  // TODO: Should we keep the human-readable status code around or validate
  // it?

  return res;
}

[[maybe_unused]]
static void pg_http_push_header(PgStringKeyValueDyn *headers, PgString key,
                                PgString value, PgAllocator *allocator) {
  PG_DYN_PUSH(headers, ((PgStringKeyValue){.key = key, .value = value}),
              allocator);
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_url_encode(PgWriter *w, PgString key, PgString value,
                     PgAllocator *allocator) {
  PgError err = 0;

  for (u64 i = 0; i < key.len; i++) {
    u8 c = PG_SLICE_AT(key, i);
    if (pg_rune_ascii_is_alphanumeric(c)) {
      err = pg_writer_write_u8(w, c, allocator);
      if (err) {
        return err;
      }
    } else {
      err = pg_writer_write_u8(w, '%', allocator);
      if (err) {
        return err;
      }

      err = pg_writer_write_u8_hex_upper(w, c, allocator);
      if (err) {
        return err;
      }
    }
  }

  err = pg_writer_write_u8(w, '=', allocator);
  if (err) {
    return err;
  }

  for (u64 i = 0; i < value.len; i++) {
    u8 c = PG_SLICE_AT(value, i);
    if (pg_rune_ascii_is_alphanumeric(c)) {
      err = pg_writer_write_u8(w, c, allocator);
      if (err) {
        return err;
      }
    } else {
      err = pg_writer_write_u8(w, '%', allocator);
      if (err) {
        return err;
      }
      err = pg_writer_write_u8_hex_upper(w, c, allocator);
      if (err) {
        return err;
      }
    }
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_http_request_write_status_line(PgWriter *w, PgHttpRequest req,
                                  PgAllocator *allocator) {
  PgError err = 0;

  err =
      pg_writer_write_full(w, pg_http_method_to_string(req.method), allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_full(w, PG_S(" /"), allocator);
  if (err) {
    return err;
  }

  for (u64 i = 0; i < req.url.path_components.len; i++) {
    PgString path_component = PG_SLICE_AT(req.url.path_components, i);
    err = pg_writer_write_full(w, path_component, allocator);
    if (err) {
      return err;
    }

    if (i < req.url.path_components.len - 1) {
      err = pg_writer_write_full(w, PG_S("/"), allocator);
      if (err) {
        return err;
      }
    }
  }

  if (req.url.query_parameters.len > 0) {
    err = pg_writer_write_full(w, PG_S("?"), allocator);
    if (err) {
      return err;
    }

    for (u64 i = 0; i < req.url.query_parameters.len; i++) {
      PgStringKeyValue param = PG_SLICE_AT(req.url.query_parameters, i);
      err = pg_writer_url_encode(w, param.key, param.value, allocator);
      if (err) {
        return err;
      }

      if (i < req.url.query_parameters.len - 1) {
        err = pg_writer_write_full(w, PG_S("&"), allocator);
        if (err) {
          return err;
        }
      }
    }
  }

  err = pg_writer_write_full(w, PG_S(" HTTP/1.1\r\n"), allocator);
  if (err) {
    return err;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_http_response_write_status_line(PgWriter *w, PgHttpResponse res,
                                   PgAllocator *allocator) {
  PgError err = 0;

  err = pg_writer_write_full(w, PG_S("HTTP/"), allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_u64_as_string(w, res.version_major, allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, '.', allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_u64_as_string(w, res.version_minor, allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_u8(w, ' ', allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_u64_as_string(w, res.status, allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_full(w, PG_S("\r\n"), allocator);
  if (err) {
    return err;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_http_write_header(PgWriter *w, PgStringKeyValue header,
                     PgAllocator *allocator) {
  PgError err = 0;

  err = pg_writer_write_full(w, header.key, allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_full(w, PG_S(": "), allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_full(w, header.value, allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_full(w, PG_S("\r\n"), allocator);
  if (err) {
    return err;
  }

  return 0;
}

// NOTE: Only sanitation for including the string inside an HTML tag e.g.:
// `<div>...ESCAPED_STRING..</div>`.
// To include the string inside other context (e.g. JS, CSS, HTML
// attributes, etc), more advanced sanitation is required.
[[maybe_unused]] [[nodiscard]] static PgString
pg_html_sanitize(PgString s, PgAllocator *allocator) {
  Pgu8Dyn res = {0};
  PG_DYN_ENSURE_CAP(&res, s.len * 2, allocator);

  for (u64 i = 0; i < s.len; i++) {
    u8 c = PG_SLICE_AT(s, i);

    if ('&' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&amp;"), allocator);
    } else if ('<' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&lt;"), allocator);
    } else if ('>' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&gt;"), allocator);
    } else if ('"' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&quot;"), allocator);
    } else if ('\'' == c) {
      PG_DYN_APPEND_SLICE(&res, PG_S("&#39;"), allocator);
    } else {
      PG_DYN_PUSH(&res, c, allocator);
    }
  }

  return PG_DYN_SLICE(PgString, res);
}

[[maybe_unused]] [[nodiscard]]
static PgString pg_html_make_slug(PgString s, PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, s.len * 2, allocator);

  PgUtf8Iterator it = pg_make_utf8_iterator(s);

  for (;;) {
    PgRuneUtf8Result res = pg_utf8_iterator_next(&it);
    // TODO: Use REPLACEMENT CHARACTER?
    if (res.err) {
      return (PgString){0};
    }

    if (res.end) {
      break;
    }

    PgRune rune = res.rune;

    if (pg_rune_ascii_is_alphanumeric(rune)) {
      PG_DYN_PUSH(&sb, pg_rune_ascii_to_lower_case(rune), allocator);
    } else if ('+' == rune) {
      PG_DYN_APPEND_SLICE(&sb, PG_S("plus"), allocator);
    } else if ('#' == rune) {
      PG_DYN_APPEND_SLICE(&sb, PG_S("sharp"), allocator);
    } else if (PG_SLICE_LAST(sb) != '-') {
      // Other runes are mapped to `-`, but we avoid consecutive `-`.
      PG_DYN_PUSH(&sb, '-', allocator);
    }
  }
  PgString res = PG_DYN_SLICE(PgString, sb);

  return pg_string_trim(res, '-');
}

[[maybe_unused]] [[nodiscard]] static PgStringDynResult
pg_url_parse_path_components(PgString s, PgAllocator *allocator) {
  PgStringDynResult res = {0};

  if (-1 != pg_string_index_of_any_rune(s, PG_S("?#:"))) {
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
    PgStringOption split_opt = pg_string_split_next(&split_it_slash);
    if (!split_opt.has_value) {
      break;
    }

    if (PG_SLICE_IS_EMPTY(split_opt.value)) {
      continue;
    }

    PG_DYN_PUSH(&components, split_opt.value, allocator);
  }

  res.value = components;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_url_to_string(PgUrl u, PgAllocator *allocator) {
  Pgu8Dyn sb = pg_string_builder_make(256, allocator);
  PG_DYN_APPEND_SLICE(&sb, u.scheme, allocator);
  if (u.scheme.len) {
    PG_DYN_APPEND_SLICE(&sb, PG_S("://"), allocator);
  }

  if (u.username.len) {
    PG_DYN_APPEND_SLICE(&sb, u.username, allocator);
  }
  if (u.username.len || u.password.len) {
    PG_DYN_APPEND_SLICE(&sb, PG_S(":"), allocator);
  }

  if (u.password.len) {
    PG_DYN_APPEND_SLICE(&sb, u.password, allocator);
  }
  if (u.username.len || u.password.len) {
    PG_DYN_APPEND_SLICE(&sb, PG_S("@"), allocator);
  }

  if (u.host.len) {
    PG_DYN_APPEND_SLICE(&sb, u.host, allocator);
  }

  if (u.port) {
    PG_DYN_APPEND_SLICE(&sb, PG_S(":"), allocator);
    pg_string_builder_append_u64(&sb, u.port, allocator);
  }

  for (u64 i = 0; i < u.path_components.len; i++) {
    PgString it = PG_SLICE_AT(u.path_components, i);
    PG_DYN_APPEND_SLICE(&sb, PG_S("/"), allocator);
    PG_DYN_APPEND_SLICE(&sb, it, allocator);
  }

  if (u.query_parameters.len) {
    PG_DYN_APPEND_SLICE(&sb, PG_S("?"), allocator);

    for (u64 i = 0; i < u.query_parameters.len; i++) {
      PgStringKeyValue it = PG_SLICE_AT(u.query_parameters, i);
      if (it.key.len) {
        PG_DYN_APPEND_SLICE(&sb, it.key, allocator);
      }
      if (it.key.len && it.value.len) {
        PG_DYN_APPEND_SLICE(&sb, PG_S("="), allocator);
      }
      if (it.value.len) {
        PG_DYN_APPEND_SLICE(&sb, it.value, allocator);
      }

      PG_DYN_APPEND_SLICE(&sb, PG_S("&"), allocator);
    }
  }

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] [[nodiscard]] static PgStringDynKeyValueResult
pg_url_parse_query_parameters(PgString s, PgAllocator *allocator) {
  PgStringDynKeyValueResult res = {0};

  PgString remaining = s;
  {
    PgStringOption consume_question_opt = pg_string_consume_rune(s, '?');
    if (!consume_question_opt.has_value) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume_question_opt.value;
  }

  for (u64 _i = 0; _i < s.len; _i++) {
    PgStringPairConsume res_consume_and =
        pg_string_consume_until_rune_incl(remaining, '&');
    remaining = res_consume_and.right;

    PgString kv = res_consume_and.left;
    PgStringPairConsume res_consume_eq =
        pg_string_consume_until_rune_incl(kv, '=');
    PgString k = res_consume_eq.left;
    PgString v = res_consume_eq.consumed ? res_consume_eq.right : PG_S("");

    if (!PG_SLICE_IS_EMPTY(k)) {
      PG_DYN_PUSH(&res.value, ((PgStringKeyValue){.key = k, .value = v}),
                  allocator);
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

[[maybe_unused]] [[nodiscard]] static Pgu16Result
pg_url_parse_port(PgString s) {
  Pgu16Result res = {0};

  // Allowed.
  if (PG_SLICE_IS_EMPTY(s)) {
    return res;
  }

  PgParseNumberResult port_parse = pg_string_parse_u64(s, 10, true);
  if (!PG_SLICE_IS_EMPTY(port_parse.remaining)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  if (port_parse.n > UINT16_MAX) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  res.value = (u16)port_parse.n;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgUrlAuthorityResult
pg_url_parse_authority(PgString s) {
  PgUrlAuthorityResult res = {0};

  PgString remaining = s;
  // User info, optional.
  {
    PgStringPairConsume user_info_and_rem =
        pg_string_consume_until_rune_incl(remaining, '@');
    remaining = user_info_and_rem.right;

    if (user_info_and_rem.consumed) {
      PgUrlUserInfoResult res_user_info =
          pg_url_parse_user_info(user_info_and_rem.left);
      if (res_user_info.err) {
        res.err = res_user_info.err;
        return res;
      }
      res.value.user_info = res_user_info.value;
    }
  }

  // Host, mandatory.
  PgStringPairConsume host_and_rem =
      pg_string_consume_until_rune_incl(remaining, ':');
  {
    remaining = host_and_rem.right;
    res.value.host = host_and_rem.left;
    if (PG_SLICE_IS_EMPTY(res.value.host)) {
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
    res.value.port = res_port.value;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_url_is_scheme_valid(PgString scheme) {
  if (PG_SLICE_IS_EMPTY(scheme)) {
    return false;
  }

  u8 first = PG_SLICE_AT(scheme, 0);
  if (!pg_rune_ascii_is_alphabetical(first)) {
    return false;
  }

  for (u64 i = 0; i < scheme.len; i++) {
    u8 c = PG_SLICE_AT(scheme, i);
    if (!(pg_rune_ascii_is_alphanumeric(c) || c == '+' || c == '-' ||
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
      pg_string_consume_until_any_rune_excl(remaining, PG_S("?#"));
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
    res.value.path_components = res_path_components.value;
  }

  // Query parameters, optional.
  if (path_components_and_rem.consumed &&
      path_components_and_rem.matched == '?') {
    PgStringDynKeyValueResult res_query =
        pg_url_parse_query_parameters(path_components_and_rem.right, allocator);
    if (res_query.err) {
      res.err = res_query.err;
      return res;
    }
    res.value.query_parameters = res_query.value;
  }

  // TODO: fragments.

  PG_ASSERT(PG_SLICE_IS_EMPTY(res.value.scheme));
  PG_ASSERT(PG_SLICE_IS_EMPTY(res.value.username));
  PG_ASSERT(PG_SLICE_IS_EMPTY(res.value.password));
  PG_ASSERT(PG_SLICE_IS_EMPTY(res.value.host));
  PG_ASSERT(0 == res.value.port);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgUrlResult
pg_url_parse(PgString s, PgAllocator *allocator) {
  PgUrlResult res = {0};

  PgString remaining = s;

  // Scheme, mandatory.
  {
    PgStringPairConsume scheme_and_rem =
        pg_string_consume_until_rune_incl(remaining, ':');
    remaining = scheme_and_rem.right;

    if (!scheme_and_rem.consumed) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    if (!pg_url_is_scheme_valid(scheme_and_rem.left)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.value.scheme = scheme_and_rem.left;
  }

  // Assume `://` as separator between the scheme and authority.
  // TODO: Be less strict hier.
  {

    PgStringOption res_consume =
        pg_string_consume_string(remaining, PG_S("//"));
    if (!res_consume.has_value) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = res_consume.value;
  }

  // Authority, mandatory.
  PgStringPairConsumeAny authority_and_rem =
      pg_string_consume_until_any_rune_excl(remaining, PG_S("/?#"));
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
    res.value.host = res_authority.value.host;
    res.value.port = res_authority.value.port;
    res.value.username = res_authority.value.user_info.username;
    res.value.password = res_authority.value.user_info.password;
  }

  PgUrlResult res_after_authority =
      pg_url_parse_after_authority(remaining, allocator);
  if (res_after_authority.err) {
    res.err = res_after_authority.err;
    return res;
  }
  res.value.path_components = res_after_authority.value.path_components;
  res.value.query_parameters = res_after_authority.value.query_parameters;

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

  PgStringCut cut = pg_string_cut_rune(status_line, ' ');
  if (!cut.has_value) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  // Method.
  {
    PgString method = pg_string_trim_space(cut.left);
    if (pg_string_eq(method, PG_S("OPTIONS"))) {
      res.value.method = PG_HTTP_METHOD_OPTIONS;
    } else if (pg_string_eq(method, PG_S("GET"))) {
      res.value.method = PG_HTTP_METHOD_GET;
    } else if (pg_string_eq(method, PG_S("HEAD"))) {
      res.value.method = PG_HTTP_METHOD_HEAD;
    } else if (pg_string_eq(method, PG_S("POST"))) {
      res.value.method = PG_HTTP_METHOD_POST;
    } else if (pg_string_eq(method, PG_S("PUT"))) {
      res.value.method = PG_HTTP_METHOD_PUT;
    } else if (pg_string_eq(method, PG_S("DELETE"))) {
      res.value.method = PG_HTTP_METHOD_DELETE;
    } else if (pg_string_eq(method, PG_S("TRACE"))) {
      res.value.method = PG_HTTP_METHOD_TRACE;
    } else if (pg_string_eq(method, PG_S("CONNECT"))) {
      res.value.method = PG_HTTP_METHOD_CONNECT;
    } else {
      res.value.method = PG_HTTP_METHOD_EXTENSION;
    }
  }

  // Path.
  cut = pg_string_cut_rune(cut.right, ' ');
  {
    PgString path = pg_string_trim_space(cut.left);
    // Need to clone since the data being parsed is transient.
    path = pg_string_clone(path, allocator);
    PgUrlResult res_url = pg_url_parse_after_authority(path, allocator);
    if (res_url.err) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    res.value.url = res_url.value;
  }

  PgString remaining = pg_string_trim_space(cut.right);
  {
    PgStringOption consume_opt =
        pg_string_consume_string(remaining, PG_S("HTTP/"));
    if (!consume_opt.has_value) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume_opt.value;
  }

  {
    PgParseNumberResult res_major = pg_string_parse_u64(remaining, 10, true);
    if (!res_major.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_major.n > 3) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.value.version_major = (u8)res_major.n;
    remaining = res_major.remaining;
  }

  {
    PgStringOption consume_opt = pg_string_consume_rune(remaining, '.');
    if (!consume_opt.has_value) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = consume_opt.value;
  }

  {
    PgParseNumberResult res_minor = pg_string_parse_u64(remaining, 10, true);
    if (!res_minor.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (res_minor.n > 9) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    res.value.version_minor = (u8)res_minor.n;
    remaining = res_minor.remaining;
  }

  if (!PG_SLICE_IS_EMPTY(remaining)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringKeyValueResult
pg_http_parse_header(PgString s) {
  PgStringKeyValueResult res = {0};

  PgStringCut cut = pg_string_cut_rune(s, ':');
  if (!cut.has_value) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.value.key = pg_string_trim_space(cut.left);
  if (pg_string_is_empty(res.value.key)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.value.value = pg_string_trim_space(cut.right);
  if (pg_string_is_empty(res.value.value)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  return res;
}

typedef enum {
  PG_NEWLINE_KIND_LF,
  PG_NEWLINE_KIND_CRLF,
} PgNewlineKind;

[[maybe_unused]] [[nodiscard]] static Pgu64OptionResult
pg_reader_read_until_byte_incl(PgReader *r, Pgu8Slice dst, u8 needle) {
  PG_ASSERT(dst.data);

  if (0 == r->ring.data.len) { // Simple reader.
    Pgu64OptionResult res = {0};
    Pgu64Result res_read = pg_reader_read(r, dst);
    if (res_read.err) {
      res.err = res_read.err;
      return res;
    }

    Pgu8Slice haystack = PG_SLICE_RANGE(dst, 0, res_read.value);
    Pgu64Option search_opt = pg_bytes_index_of_byte(haystack, needle);
    if (!search_opt.has_value) {
      return res;
    }

    res.value.has_value = true;
    res.value.value = search_opt.value;
    return res;
  } else { // Buffered reader.
    PG_ASSERT(dst.len >= r->ring.data.len);

    for (u64 _i = 0; _i <= 1; _i++) {
      Pgu64OptionResult res = {0};
      Pgu64Option search_opt = pg_ring_index_of_byte(r->ring, needle);
      if (search_opt.has_value) {
        search_opt.value += 1; // Incl.
        PG_ASSERT(pg_ring_read_bytes(
                      &r->ring, PG_SLICE_RANGE(dst, 0, search_opt.value)) ==
                  search_opt.value);

        res.value.has_value = true;
        res.value.value = search_opt.value;
        return res;
      }

      // Do not overwrite data.
      if (pg_ring_is_full(r->ring)) {
        res.err = PG_ERR_TOO_BIG;
        return res;
      }

      // TODO: Should we do multiple reads?
      u64 ring_size_before = pg_ring_can_read_count(r->ring);
      PgError err = pg_buf_reader_try_fill_once(r);
      if (err) {
        res.err = err;
        return res;
      }

      u64 ring_size_after = pg_ring_can_read_count(r->ring);
      if (ring_size_before == ring_size_after) {
        // Fill failed, stop.
        return res;
      }
    }
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static Pgu64OptionResult
pg_reader_read_until_bytes2_incl(PgReader *r, Pgu8Slice dst, u8 needle0,
                                 u8 needle1) {
  PG_ASSERT(dst.data);

  if (0 == r->ring.data.len) { // Simple reader.
    Pgu64OptionResult res = {0};
    // TODO: Multiple reads?
    Pgu64Result res_read = pg_reader_read(r, dst);
    if (res_read.err) {
      res.err = res_read.err;
      return res;
    }

    u8 needle_data[2] = {needle0, needle1};
    Pgu8Slice needle = {.data = needle_data, .len = 2};
    Pgu8Slice haystack = PG_SLICE_RANGE(dst, 0, res_read.value);
    Pgu64Option search_opt = pg_bytes_index_of_bytes(haystack, needle);
    if (!search_opt.has_value) {
      return res;
    }

    res.value.has_value = true;
    res.value.value = search_opt.value;
    return res;
  } else {
    PG_ASSERT(dst.len >= r->ring.data.len);

    for (u64 _i = 0; _i <= 1; _i++) {
      Pgu64OptionResult res = {0};
      Pgu64Option search_opt =
          pg_ring_index_of_bytes2(r->ring, needle0, needle1);
      if (search_opt.has_value) {
        search_opt.value += 2; // Incl.
        PG_ASSERT(pg_ring_read_bytes(
                      &r->ring, PG_SLICE_RANGE(dst, 0, search_opt.value)) ==
                  search_opt.value);
        res.value.has_value = true;
        res.value.value = search_opt.value;
        return res;
      }

      // Do not overwrite data.
      if (pg_ring_is_full(r->ring)) {
        res.err = PG_ERR_TOO_BIG;
        return res;
      }

      // TODO: Should we do multiple reads?
      u64 ring_size_before = pg_ring_can_read_count(r->ring);
      PgError err = pg_buf_reader_try_fill_once(r);
      if (err) {
        res.err = err;
        return res;
      }

      u64 ring_size_after = pg_ring_can_read_count(r->ring);
      if (ring_size_before == ring_size_after) {
        // Fill failed, stop.
        return res;
      }
    }
    PG_ASSERT(0);
  }
}

[[nodiscard]] static Pgu64OptionResult
pg_reader_read_line(PgReader *reader, PgNewlineKind newline_kind,
                    PgString dst) {
  Pgu64OptionResult res = {0};
  switch (newline_kind) {
  case PG_NEWLINE_KIND_LF:
    res = pg_reader_read_until_byte_incl(reader, dst, '\n');
    break;
  case PG_NEWLINE_KIND_CRLF:
    res = pg_reader_read_until_bytes2_incl(reader, dst, '\r', '\n');
    break;
  default:
    PG_ASSERT(0);
  }

  if (res.err) {
    return res;
  }
  if (!res.value.has_value) {
    return res;
  }

  switch (newline_kind) {
  case PG_NEWLINE_KIND_LF: {
    PG_ASSERT(res.value.value >= 1);
    PG_ASSERT('\n' == PG_SLICE_AT(dst, res.value.value - 1));
    // Trim.
    res.value.value -= 1;
  } break;
  case PG_NEWLINE_KIND_CRLF: {
    PG_ASSERT(res.value.value >= 2);
    PG_ASSERT('\r' == PG_SLICE_AT(dst, res.value.value - 2));
    PG_ASSERT('\n' == PG_SLICE_AT(dst, res.value.value - 1));
    // Trim.
    res.value.value -= 2;
  } break;
  default:
    PG_ASSERT(0);
  }

  return res;
}

#define PG_HTTP_LINE_MAX_LEN 4096
#define PG_HTTP_HEADERS_MAX 512

[[maybe_unused]] [[nodiscard]] static PgHttpRequestReadResult
pg_http_read_request(PgReader *reader, PgAllocator *allocator) {
  PgHttpRequestReadResult res = {0};

  u8 recv[PG_HTTP_LINE_MAX_LEN] = {0};
  Pgu8Slice recv_slice = {
      .data = recv,
      .len = PG_HTTP_LINE_MAX_LEN,
  };
  // Status line.
  {
    Pgu64OptionResult res_read =
        pg_reader_read_line(reader, PG_NEWLINE_KIND_CRLF, recv_slice);
    if (res_read.err) {
      res.err = res_read.err;
      return res;
    }
    if (!res_read.value.has_value) {
      res.err = PG_ERR_EOF;
      return res;
    }

    PgString line = PG_SLICE_RANGE(recv_slice, 0, res_read.value.value);

    PgHttpRequestStatusLineResult res_status_line =
        pg_http_parse_request_status_line(line, allocator);

    if (res_status_line.err) {
      res.err = res_status_line.err;
      return res;
    }
    res.req.method = res_status_line.value.method;
    res.req.url = res_status_line.value.url;
    res.req.version_major = res_status_line.value.version_major;
    res.req.version_minor = res_status_line.value.version_minor;
  }

  // Headers.
  for (u64 i = 0; i < PG_HTTP_HEADERS_MAX; i++) {
    recv_slice.len = PG_STATIC_ARRAY_LEN(recv);
    Pgu64OptionResult res_read =
        pg_reader_read_line(reader, PG_NEWLINE_KIND_CRLF, recv_slice);
    if (res_read.err) {
      res.err = res_read.err;
      return res;
    }
    if (!res_read.value.has_value) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    PgString line = PG_SLICE_RANGE(recv_slice, 0, res_read.value.value);

    if (0 == line.len) { // `\r\n\r\n`.
      goto end;
    }

    PgStringKeyValueResult res_kv =
        pg_http_parse_header(pg_string_clone(line, allocator));
    if (res_kv.err) {
      res.err = res_kv.err;
      return res;
    }

    PG_DYN_PUSH(&res.req.headers, res_kv.value, allocator);
  }

  // Too many headers.
  res.err = PG_ERR_TOO_BIG;
  return res;

end:
  res.done = true;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_http_write_request(PgWriter *w, PgHttpRequest req, PgAllocator *allocator) {
  PgError err = 0;

  err = pg_http_request_write_status_line(w, req, allocator);
  if (err) {
    return err;
  }

  for (u64 i = 0; i < req.headers.len; i++) {
    PgStringKeyValue header = PG_SLICE_AT(req.headers, i);
    err = pg_http_write_header(w, header, allocator);
    if (err) {
      return err;
    }
  }
  err = pg_writer_write_full(w, PG_S("\r\n"), allocator);
  if (err) {
    return err;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_http_request_to_string(PgHttpRequest req, PgAllocator *allocator) {
  u64 cap =
      // TODO: Tweak this number?
      128 + req.url.path_components.len * 64 +
      req.url.query_parameters.len * 64 + req.headers.len * 128;
  PgWriter w = pg_writer_make_string_builder(cap, allocator);

  PG_ASSERT(0 == pg_http_write_request(&w, req, allocator));

  return PG_DYN_SLICE(PgString, w.u.bytes);
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_http_write_response(PgWriter *w, PgHttpResponse res,
                       PgAllocator *allocator) {
  PgError err = 0;

  err = pg_http_response_write_status_line(w, res, allocator);
  if (err) {
    return err;
  }

  for (u64 i = 0; i < res.headers.len; i++) {
    PgStringKeyValue header = PG_SLICE_AT(res.headers, i);
    err = pg_http_write_header(w, header, allocator);
    if (err) {
      return err;
    }
  }
  err = pg_writer_write_full(w, PG_S("\r\n"), allocator);
  if (err) {
    return err;
  }

  err = pg_writer_flush(w, allocator);
  if (err) {
    return err;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_http_content_length(PgStringKeyValueSlice headers) {
  Pgu64Result res = {0};

  for (u64 i = 0; i < headers.len; i++) {
    PgStringKeyValue h = PG_SLICE_AT(headers, i);

    if (!pg_string_ieq_ascii(PG_S("Content-Length"), h.key)) {
      continue;
    }

    PgParseNumberResult res_parse = pg_string_parse_u64(h.value, 10, true);
    if (!res_parse.present) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    if (!pg_string_is_empty(res_parse.remaining)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    res.value = res_parse.n;
    return res;
  }
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgLogger
pg_log_make_logger_stdout(PgLogLevel level, PgLogFormat format,
                          PgAllocator *allocator) {
  PgLogger logger = {
      .level = level,
      .writer = pg_writer_make_from_file_descriptor(pg_os_stdout(), 4 * PG_KiB,
                                                    allocator),
      .format = format,
      // TODO: Consider using `rtdsc` or such.
      .monotonic_epoch = pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC).value,
      .allocator = allocator,
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

#define pg_log_c_u8(k, v) pg_log_u8(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_u8(PgString k, u8 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

#define pg_log_c_u16(k, v) pg_log_u16(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_u16(PgString k, u16 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

#define pg_log_c_u32(k, v) pg_log_u32(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_u32(PgString k, u32 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = (u64)v,
  };
}

#define pg_log_c_u64(k, v) pg_log_u64(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_u64(PgString k, u64 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_U64,
      .value.n64 = v,
  };
}

#define pg_log_c_i32(k, v) pg_log_i32(PG_S(k), v)

[[maybe_unused]] [[nodiscard]] static PgLogEntry pg_log_i32(PgString k, i32 v) {
  return (PgLogEntry){
      .key = k,
      .value.kind = PG_LOG_VALUE_I64,
      .value.s64 = (i64)v,
  };
}

#define pg_log_c_err(k, v) pg_log_i32(PG_S(k), (i32)v)
#define pg_log_err(k, v) pg_log_i32(k, (i32)v)

#define pg_log_c_i64(k, v) pg_log_i64(PG_S(k), v)

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

#define pg_log_c_s(k, v) pg_log_s(PG_S(k), v)

#define pg_log_c_ipv4(k, v) pg_log_ipv4(PG_S(k), v)

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
    if ((logger)->level > (lvl)) {                                             \
      break;                                                                   \
    };                                                                         \
    (void)pg_logger_do_log(logger, lvl, PG_S(msg), (logger)->allocator,        \
                           PG_LOG_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__);       \
  } while (0)

[[nodiscard]] static bool pg_logfmt_byte_needs_escaping(u8 c) {
  return c < ' ' || '\\' == c || '"' == c;
}

static PgError pg_logfmt_write_string_escaped(PgWriter *w, PgString entry,
                                              PgAllocator *allocator) {
  PgError err = 0;

  bool has_spaces = pg_bytes_contains_any_byte(entry, PG_ASCII_SPACES);
  if (has_spaces) {
    err = pg_writer_write_u8(w, '"', allocator);
    if (err) {
      return err;
    }
  }

  for (u64 i = 0; i < entry.len; i++) {
    u8 c = PG_SLICE_AT(entry, i);

    if (pg_logfmt_byte_needs_escaping(c)) {
      err = pg_writer_write_u8(w, '\\', allocator);
      if (err) {
        return err;
      }
    }
    err = pg_writer_write_u8(w, c, allocator);
    if (err) {
      return err;
    }
  }

  if (has_spaces) {
    err = pg_writer_write_u8(w, '"', allocator);
    if (err) {
      return err;
    }
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgArena pg_arena_make_from_mem(u8 *data,
                                                                     u64 len) {
  PgArena arena = {
      .start = data,
      .end = data + len,
      .start_original = data,
  };

  return arena;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_logger_do_log(PgLogger *logger, PgLogLevel level, PgString msg,
                 PgAllocator *allocator, i32 args_count, ...) {
  // Ignore clock errors.
  u64 monotonic_ns =
      pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC).value - logger->monotonic_epoch;
  u64 timestamp_ns = pg_time_ns_now(PG_CLOCK_KIND_REALTIME).value;

  PgError err = 0;

  err = pg_writer_write_full(&logger->writer, PG_S("level="), allocator);
  if (err) {
    return err;
  }
  err = pg_writer_write_full(&logger->writer, pg_log_level_to_string(level),
                             allocator);
  if (err) {
    return err;
  }

  err =
      pg_writer_write_full(&logger->writer, PG_S(" timestamp_ns="), allocator);
  if (err) {
    return err;
  }
  err = pg_writer_write_u64_as_string(&logger->writer, timestamp_ns, allocator);
  if (err) {
    return err;
  }

  err =
      pg_writer_write_full(&logger->writer, PG_S(" monotonic_ns="), allocator);
  if (err) {
    return err;
  }
  err = pg_writer_write_u64_as_string(&logger->writer, monotonic_ns, allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_full(&logger->writer, PG_S(" message="), allocator);
  if (err) {
    return err;
  }
  err = pg_logfmt_write_string_escaped(&logger->writer, msg, allocator);
  if (err) {
    return err;
  }

  va_list argp = {0};
  va_start(argp, args_count);
  for (i32 i = 0; i < args_count; i++) {
    PgLogEntry entry = va_arg(argp, PgLogEntry);
    err = pg_writer_write_u8(&logger->writer, ' ', allocator);
    if (err) {
      return err;
    }
    err = pg_writer_write_full(&logger->writer, entry.key, allocator);
    if (err) {
      return err;
    }
    err = pg_writer_write_u8(&logger->writer, '=', allocator);
    if (err) {
      return err;
    }

    switch (entry.value.kind) {
    case PG_LOG_VALUE_STRING: {
      err = pg_logfmt_write_string_escaped(&logger->writer, entry.value.s,
                                           allocator);
      if (err) {
        return err;
      }
      break;
    }
    case PG_LOG_VALUE_U64:
      err = pg_writer_write_u64_as_string(&logger->writer, entry.value.n64,
                                          allocator);
      if (err) {
        return err;
      }
      break;
    case PG_LOG_VALUE_I64:
      err = pg_writer_write_i64_as_string(&logger->writer, entry.value.s64,
                                          allocator);
      if (err) {
        return err;
      }
      break;
    case PG_LOG_VALUE_IPV4_ADDRESS: {
      PgString ipv4_addr_str =
          pg_net_ipv4_address_to_string(entry.value.ipv4_address, allocator);
      err = pg_writer_write_full(&logger->writer, ipv4_addr_str, allocator);
      if (err) {
        return err;
      }
    } break;
    default:
      PG_ASSERT(0 && "invalid PgLogValueKind");
    }
  }
  va_end(argp);

  err = pg_writer_write_u8(&logger->writer, '\n', allocator);
  if (err) {
    return err;
  }

  return pg_writer_flush(&logger->writer, allocator);
}

typedef int (*PgCmpFn)(const void *a, const void *b);

[[maybe_unused]] static void pg_sort_unique(void *elems, u64 elem_size,
                                            u64 *elems_count, PgCmpFn cmp_fn) {
  // TODO: own sort.
  qsort(elems, *elems_count, elem_size, cmp_fn);

  if (*elems_count <= 1) {
    return;
  }

  for (u64 i = 1; i < *elems_count;) {
    void *previous = (u8 *)elems + elem_size * (i - 1);
    void *current = (u8 *)elems + elem_size * i;
    if (PG_CMP_EQ == cmp_fn(previous, current)) {
      memmove(previous, current, elem_size * (*elems_count - i));
      PG_ASSERT(*elems_count > 0);
      *elems_count -= 1;
    } else {
      i += 1;
    }
  }
}

[[maybe_unused]] [[nodiscard]] static PgUuid pg_uuid_v5(PgUuid namespace,
                                                        PgString name) {
  PG_SHA1_CTX ctx = {0};
  PG_SHA1Init(&ctx);
  PG_SHA1Update(&ctx, namespace.value, PG_STATIC_ARRAY_LEN(namespace.value));
  PG_SHA1Update(&ctx, name.data, name.len);

  PgSha1 digest = {0};
  PG_SHA1Final(digest.data, &ctx);

  PgUuid res = {.version = 5};
  pg_memcpy(res.value, digest.data, 16);

  res.value[6] &= 0x0F;
  res.value[6] |= 0x50;

  res.value[8] &= 0x3F;
  res.value[8] |= 0x80;

  return res;
}

#define PG_UUID_STRING_LENGTH (8 + 4 + 4 + 4 + 12 + 4)

[[maybe_unused]] [[nodiscard]] static PgString
pg_uuid_to_string(PgUuid uuid, PgAllocator *allocator) {
  PgString res = pg_string_make(PG_UUID_STRING_LENGTH, allocator);
  u64 i = 0;

  res.data[i++] = pg_u8_to_hex_rune(uuid.value[0] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[0] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[1] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[1] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[2] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[2] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[3] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[3] & 0xF);
  res.data[i++] = '-';
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[4] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[4] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[5] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[5] & 0xF);
  res.data[i++] = '-';
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[6] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[6] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[7] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[7] & 0xF);
  res.data[i++] = '-';
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[8] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[8] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[9] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[9] & 0xF);
  res.data[i++] = '-';
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[10] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[10] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[11] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[11] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[12] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[12] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[13] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[13] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[14] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[14] & 0xF);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[15] >> 4);
  res.data[i++] = pg_u8_to_hex_rune(uuid.value[15] & 0xF);

  return res;
}

[[nodiscard]] static bool pg_html_tag_is_self_closing(PgString tag) {
  return pg_string_eq(tag, PG_S("area")) || pg_string_eq(tag, PG_S("base")) ||
         pg_string_eq(tag, PG_S("br")) || pg_string_eq(tag, PG_S("col")) ||
         pg_string_eq(tag, PG_S("embed")) || pg_string_eq(tag, PG_S("hr")) ||
         pg_string_eq(tag, PG_S("img")) || pg_string_eq(tag, PG_S("input")) ||
         pg_string_eq(tag, PG_S("link")) || pg_string_eq(tag, PG_S("meta")) ||
         pg_string_eq(tag, PG_S("param")) ||
         pg_string_eq(tag, PG_S("source")) ||
         pg_string_eq(tag, PG_S("track")) || pg_string_eq(tag, PG_S("wbr"));
}

[[nodiscard]] static bool pg_svg_tag_is_self_closing(PgString tag) {
  return pg_string_eq(tag, PG_S("path")) || pg_string_eq(tag, PG_S("line")) ||
         pg_string_eq(tag, PG_S("circle")) || pg_string_eq(tag, PG_S("rect")) ||
         pg_string_eq(tag, PG_S("polygon")) ||
         pg_string_eq(tag, PG_S("polyline")) || pg_string_eq(tag, PG_S("stop"));

  // TODO: more.
}

// FIXME
[[maybe_unused]] [[nodiscard]]
static PgError pg_html_tokenize_attributes(PgString s, u64 *pos,
                                           PgHtmlTokenDyn *tokens,
                                           PgAllocator *allocator) {
  (void)tokens;
  (void)allocator;

  PgRune quote = 0;

  for (;;) {
    PgRuneOption first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (!first_opt.has_value) { // Early EOF.
      return PG_HTML_PARSE_ERROR_EOF_IN_TAG;
    }

    PgRune first = first_opt.value;
    if (pg_rune_ascii_is_space(first)) { // Skip.
      *pos += pg_utf8_rune_bytes_count(first);
      continue;
    }

    if (('"' == first || '\'' == first) && 0 == quote) { // Entering quotes.
      quote = first;
      *pos += pg_utf8_rune_bytes_count(first);
      continue;
    }
    if (quote && first == quote) { // Exiting quotes.
      quote = 0;
      *pos += pg_utf8_rune_bytes_count(first);
      continue;
    }

    if (0 == quote && ('>' == first)) { // End of tag.
      return 0;
    }

    // Other characters, skip.
    *pos += pg_utf8_rune_bytes_count(first);
  }

  PG_ASSERT(0);
}

[[nodiscard]]
static PgError pg_html_tokenize_comment(PgString s, u64 *pos,
                                        PgHtmlTokenDyn *tokens,
                                        PgAllocator *allocator) {
  u32 start = (u32)*pos;

  for (;;) {
    PgRuneOption first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (!first_opt.has_value) {
      return PG_HTML_PARSE_ERROR_EOF_IN_COMMENT;
    }

    PgRune first = first_opt.value;

    // FIXME: More complicated than that in the spec.
    if (pg_string_starts_with(PG_SLICE_RANGE_START(s, *pos), PG_S("-->"))) {
      PgHtmlToken token = {
          .kind = PG_HTML_TOKEN_KIND_COMMENT,
          .comment = pg_string_trim_space(PG_SLICE_RANGE(s, start, *pos)),
          .start = start,
          .end = (u32)*pos,
      };
      PG_DYN_PUSH(tokens, token, allocator);

      *pos += PG_S("-->").len;
      return 0;
    }

    *pos += pg_utf8_rune_bytes_count(first);
  }
}

[[nodiscard]]
static PgError pg_html_tokenize_doctype_name(PgString s, u64 *pos,
                                             PgHtmlTokenDyn *tokens,
                                             PgAllocator *allocator) {

  PgHtmlToken token = {
      .kind = PG_HTML_TOKEN_KIND_DOCTYPE,
      .doctype = {.data = s.data + *pos},
      .start = (u32)*pos,
  };

  for (;;) {
    PgRuneOption first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (!first_opt.has_value) {
      return PG_HTML_PARSE_ERROR_EOF_IN_DOCTYPE;
    }

    PgRune first = first_opt.value;

    if (pg_rune_ascii_is_space(first)) {
      *pos += pg_utf8_rune_bytes_count(first);
      return 0;
    }

    if ('>' == first) {
      token.end = (u32)*pos;
      token.doctype.len = token.end - token.start;
      PG_ASSERT(!pg_string_is_empty(token.doctype));
      PG_DYN_PUSH(tokens, token, allocator);

      *pos += pg_utf8_rune_bytes_count(first);
      return 0;
    }

    *pos += pg_utf8_rune_bytes_count(first);
  }
}

[[nodiscard]]
static PgError pg_html_tokenize_doctype(PgString s, u64 *pos,
                                        PgHtmlTokenDyn *tokens,
                                        PgAllocator *allocator) {
  *pos += PG_S("DOCTYPE").len;

  PgRuneOption first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
  if (!first_opt.has_value) {
    return PG_HTML_PARSE_ERROR_MISSING_WHITESPACE_BEFORE_DOCTYPE_NAME;
  }

  PgRune first = first_opt.value;
  if (!pg_rune_ascii_is_space(first)) {
    return PG_HTML_PARSE_ERROR_MISSING_WHITESPACE_BEFORE_DOCTYPE_NAME;
  }
  *pos += pg_utf8_rune_bytes_count(first);

  for (;;) {
    first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (!first_opt.has_value) {
      return PG_HTML_PARSE_ERROR_EOF_IN_DOCTYPE;
    }

    first = first_opt.value;
    if (pg_rune_ascii_is_space(first)) {
      *pos += pg_utf8_rune_bytes_count(first);
      continue;
    }

    if ('>' == first) {
      return PG_HTML_PARSE_ERROR_MISSING_DOCTYPE_NAME;
    }

    return pg_html_tokenize_doctype_name(s, pos, tokens, allocator);
  }
}

[[nodiscard]]
static PgError pg_html_tokenize_markup(PgString s, u64 *pos,
                                       PgHtmlTokenDyn *tokens,
                                       PgAllocator *allocator) {
  if (pg_string_starts_with(PG_SLICE_RANGE_START(s, *pos), PG_S("--"))) {
    *pos += pg_utf8_rune_bytes_count('-');
    *pos += pg_utf8_rune_bytes_count('-');
    return pg_html_tokenize_comment(s, pos, tokens, allocator);
  }

  if (PG_SLICE_RANGE_START(s, *pos).len >= PG_S("DOCTYPE").len &&
      (('D' == PG_SLICE_AT(s, *pos + 0) || 'd' == PG_SLICE_AT(s, *pos + 0)) ||
       ('O' == PG_SLICE_AT(s, *pos + 0) || 'o' == PG_SLICE_AT(s, *pos + 1)) ||
       ('C' == PG_SLICE_AT(s, *pos + 0) || 'c' == PG_SLICE_AT(s, *pos + 2)) ||
       ('T' == PG_SLICE_AT(s, *pos + 0) || 't' == PG_SLICE_AT(s, *pos + 3)) ||
       ('Y' == PG_SLICE_AT(s, *pos + 0) || 'y' == PG_SLICE_AT(s, *pos + 4)) ||
       ('P' == PG_SLICE_AT(s, *pos + 0) || 'p' == PG_SLICE_AT(s, *pos + 5)) ||
       ('E' == PG_SLICE_AT(s, *pos + 0) || 'e' == PG_SLICE_AT(s, *pos + 6)))) {
    return pg_html_tokenize_doctype(s, pos, tokens, allocator);
  }
  PG_ASSERT(0);

  return 0;
}

[[nodiscard]]
static PgError pg_html_tokenize_tag(PgString s, u64 *pos,
                                    PgHtmlTokenDyn *tokens,
                                    PgAllocator *allocator) {
  PgRuneOption first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
  PG_ASSERT(first_opt.has_value);
  PG_ASSERT('<' == first_opt.value);
  *pos += pg_utf8_rune_bytes_count('<');

  first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
  if (first_opt.has_value && ('!' == first_opt.value)) {
    *pos += pg_utf8_rune_bytes_count('!');
    return pg_html_tokenize_markup(s, pos, tokens, allocator);
  }

  PgHtmlToken token = {
      .start = (u32)*pos,
      .end = 0, // Backpatched,
      .kind = PG_HTML_TOKEN_KIND_TAG_OPENING,
      .tag = {.data = s.data + *pos}, // Length backpatched.
  };

  first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
  if (first_opt.has_value) {
    PgRune first = first_opt.value;
    if ('/' == first) {
      *pos += pg_utf8_rune_bytes_count(first);
      token.start = (u32)*pos;
      token.kind = PG_HTML_TOKEN_KIND_TAG_CLOSING;
      token.tag.data += pg_utf8_rune_bytes_count(first);
    }
  }

  first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
  if (!first_opt.has_value ||
      (!pg_rune_ascii_is_alphabetical(first_opt.value))) {
    return PG_HTML_PARSE_ERROR_INVALID_FIRST_CHARACTER_OF_TAG_NAME;
  }
  *pos += pg_utf8_rune_bytes_count(first_opt.value);

  for (;;) {
    first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));

    if (!first_opt.has_value) { // Early EOF.
      return PG_HTML_PARSE_ERROR_EOF_IN_TAG;
    }

    PgRune first = first_opt.value;

    // End of tag name.
    if ('>' == first || pg_rune_ascii_is_space(first)) {
      if (0 == token.tag.len) {
        token.tag.len = (u64)((s.data + *pos) - token.tag.data);
      }
      PG_ASSERT(token.tag.len <= s.len);
      if (0 == token.end) {
        token.end = (u32)*pos;
      }
    }

    if ('>' == first) {
      token.tag = pg_string_trim_space(token.tag);
      token.tag = pg_string_trim_right(token.tag, '/');

      PG_DYN_PUSH(tokens, token, allocator);
      *pos += pg_utf8_rune_bytes_count(first);
      return 0;
    }

    if (pg_rune_ascii_is_space(first)) {
      *pos += pg_utf8_rune_bytes_count(first);
      PgError err = pg_html_tokenize_attributes(s, pos, tokens, allocator);
      if (err) {
        return err;
      }
      continue;
    }

    // Skip other characters in tag name.
    *pos += pg_utf8_rune_bytes_count(first);
  }
}

// TODO: Decode html entities e.g. `&amp;` ?
[[maybe_unused]] [[nodiscard]]
static PgError pg_html_tokenize_data(PgString s, u64 *pos,
                                     PgHtmlTokenDyn *tokens,
                                     PgAllocator *allocator) {
  u32 start = (u32)*pos;

  PG_ASSERT(tokens->len > 0);
  PG_ASSERT(PG_SLICE_LAST(*tokens).end <= start);

  for (;;) {
    PgRuneOption first_opt = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (!first_opt.has_value) {
      return 0;
    }

    PgRune first = first_opt.value;
    if ('<' == first) { // End of data, start of tag.
      PgHtmlToken token = {
          .start = start,
          .end = (u32)(*pos),
          .kind = PG_HTML_TOKEN_KIND_TEXT,
          .text = PG_SLICE_RANGE(s, start, *pos),
      };
      if (!pg_string_is_empty(pg_string_trim_space(token.text))) {
        PG_DYN_PUSH(tokens, token, allocator);
      }
      return 0;
    }
    // TODO: Ampersand.

    *pos += pg_utf8_rune_bytes_count(first);
  }
}

[[maybe_unused]] [[nodiscard]] static PgHtmlTokenDynResult
pg_html_tokenize(PgString s, PgAllocator *allocator) {
  PgHtmlTokenDynResult res = {0};
  PG_DYN_ENSURE_CAP(&res.value, s.len / 8, allocator);

  /* PgString comment_start = PG_S("<!--"); */
  /* PgString comment_end = PG_S("-->"); */
  PgRune tag_start = '<';
  u64 pos = 0;
  for (;;) {
    PgRuneOption first_opt = pg_string_first(PG_SLICE_RANGE_START(s, pos));
    if (!first_opt.has_value) { // EOF.
      return res;
    }

    PgRune first = first_opt.value;

    // TODO: Doctype.
    // TODO: Comment.

    // Tag.
    if (tag_start == first) {
      res.err = pg_html_tokenize_tag(s, &pos, &res.value, allocator);
      if (res.err) {
        return res;
      }

      res.err = pg_html_tokenize_data(s, &pos, &res.value, allocator);
      if (res.err) {
        return res;
      }
    }
  }
  PG_ASSERT(0);
}

[[maybe_unused]] static void pg_linked_list_init(PgLinkedListNode *node) {
  PG_ASSERT(node);
  node->next = node;
}

[[nodiscard]]
static bool pg_linked_list_is_empty(PgLinkedListNode *node) {
  PG_ASSERT(node);
  return node->next == node;
}

[[maybe_unused]] [[nodiscard]]
static PgLinkedListNode *pg_linked_list_tail(PgLinkedListNode *node) {
  PG_ASSERT(node);
  PgLinkedListNode *it = node;
  for (; !pg_linked_list_is_empty(it); it = it->next) {
  }
  PG_ASSERT(it);
  return it;
}

[[maybe_unused]] static void pg_linked_list_append(PgLinkedListNode *head,
                                                   PgLinkedListNode *elem) {
  PG_ASSERT(head);
  PG_ASSERT(elem);

  pg_linked_list_tail(head)->next = elem;
}

[[nodiscard]] static PgHtmlNode *pg_html_node_get_parent(PgHtmlNode *node) {
  PgLinkedListNode *linked_list_node = node->parent.next;
  return PG_CONTAINER_OF(linked_list_node, PgHtmlNode, parent);
}

[[nodiscard]] static PgHtmlNode *
pg_html_node_get_first_child(PgHtmlNode *node) {
  PgLinkedListNode *linked_list_node = node->first_child.next;
  return PG_CONTAINER_OF(linked_list_node, PgHtmlNode, first_child);
}

[[maybe_unused]] [[nodiscard]] static PgHtmlNode *
pg_html_node_get_next_sibling(PgHtmlNode *node) {
  PgLinkedListNode *linked_list_node = node->next_sibling.next;
  return PG_CONTAINER_OF(linked_list_node, PgHtmlNode, next_sibling);
}

[[maybe_unused]] [[nodiscard]] static PgHtmlNodePtrResult
pg_html_parse(PgString s, PgAllocator *allocator) {
  PgHtmlNodePtrResult res = {0};

  PgHtmlTokenDynResult res_tokens = pg_html_tokenize(s, allocator);
  if (res_tokens.err) {
    res.err = res_tokens.err;
    return res;
  }
  PgHtmlTokenSlice tokens = PG_DYN_SLICE(PgHtmlTokenSlice, res_tokens.value);

  PgHtmlNode *root =
      pg_alloc(allocator, sizeof(PgHtmlNode), _Alignof(PgHtmlNode), 1);
  pg_linked_list_init(&root->parent);
  pg_linked_list_init(&root->first_child);
  pg_linked_list_init(&root->next_sibling);

  PgHtmlNode *parent = root;

  for (u64 i = 0; i < tokens.len; i++) {
    PgHtmlToken token = PG_SLICE_AT(tokens, i);
    PG_ASSERT(parent);

    bool is_self_closing = ((PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind) ||
                            PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind) &&
                           (pg_html_tag_is_self_closing(token.tag) ||
                            pg_svg_tag_is_self_closing(token.tag));

    if (PG_HTML_TOKEN_KIND_TEXT == token.kind ||
        PG_HTML_TOKEN_KIND_COMMENT == token.kind ||
        PG_HTML_TOKEN_KIND_DOCTYPE == token.kind || is_self_closing) {
      PgHtmlNode *node =
          pg_alloc(allocator, sizeof(PgHtmlNode), _Alignof(PgHtmlNode), 1);
      pg_linked_list_init(&node->parent);
      pg_linked_list_init(&node->first_child);
      pg_linked_list_init(&node->next_sibling);
      node->token_start = node->token_end = token;
      node->parent.next = &parent->parent;

      PgLinkedListNode *first_child_of_parent = &parent->first_child;
      if (pg_linked_list_is_empty(first_child_of_parent)) {
        first_child_of_parent->next = &node->first_child;
      } else {
        PgLinkedListNode *next_sibling_of_first_child_of_parent =
            &pg_html_node_get_first_child(parent)->next_sibling;
        pg_linked_list_append(next_sibling_of_first_child_of_parent,
                              &node->next_sibling);
      }
    } else if (PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind) {
      PgHtmlNode *node =
          pg_alloc(allocator, sizeof(PgHtmlNode), _Alignof(PgHtmlNode), 1);
      pg_linked_list_init(&node->parent);
      pg_linked_list_init(&node->first_child);
      pg_linked_list_init(&node->next_sibling);
      node->token_start = node->token_end = token;
      node->parent.next = &parent->parent;

      PgLinkedListNode *first_child_of_parent = &parent->first_child;
      if (pg_linked_list_is_empty(first_child_of_parent)) {
        first_child_of_parent->next = &node->first_child;
      } else {
        PgLinkedListNode *next_sibling_of_first_child_of_parent =
            &pg_html_node_get_first_child(parent)->next_sibling;
        pg_linked_list_append(next_sibling_of_first_child_of_parent,
                              &node->next_sibling);
      }

      if (!is_self_closing) {
        parent = node;
      }
    } else if (PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind) {
      PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == parent->token_start.kind &&
                pg_string_eq(parent->token_start.tag, token.tag));
      parent->token_end = token;

      parent = pg_html_node_get_parent(parent);
    } else {
      PG_ASSERT(0 && "todo");
    }
  }

  // All tags correctly closed.
  PG_ASSERT(parent == root);

  PG_ASSERT(pg_linked_list_is_empty(&root->next_sibling));
  PG_ASSERT(pg_linked_list_is_empty(&root->parent));
  res.value = root;
  PG_ASSERT(root->parent.next == &root->parent);
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_html_node_is_title_opening(PgHtmlNode *node) {
  return (PG_HTML_TOKEN_KIND_TAG_OPENING == node->token_start.kind) &&
         (pg_string_eq(node->token_start.tag, PG_S("h1")) ||
          pg_string_eq(node->token_start.tag, PG_S("h2")) ||
          pg_string_eq(node->token_start.tag, PG_S("h3")) ||
          pg_string_eq(node->token_start.tag, PG_S("h4")) ||
          pg_string_eq(node->token_start.tag, PG_S("h5")) ||
          pg_string_eq(node->token_start.tag, PG_S("h6")));
}

[[maybe_unused]] [[nodiscard]] static u8
pg_html_node_get_title_level(PgHtmlNode *node) {
  if (!pg_html_node_is_title_opening(node)) {
    return 0;
  }

  PgRuneOption last_opt = pg_string_last(node->token_start.tag);
  if (!last_opt.has_value) {
    return 0;
  }

  PgRune last = last_opt.value;
  PG_ASSERT(pg_rune_ascii_is_numeric(last));
  return (u8)last - '0';
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_html_node_get_simple_title_content(PgHtmlNode *node) {
  if (!pg_html_node_is_title_opening(node)) {
    return PG_S("");
  }
  PG_ASSERT(!pg_linked_list_is_empty(&node->first_child));
  PgHtmlNode *child =
      PG_CONTAINER_OF(node->first_child.next, PgHtmlNode, first_child);
  PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == child->token_start.kind);
  PgString res = child->token_start.text;

  PG_ASSERT(!pg_string_is_empty(res));
  return res;
}

// Caller is responsible for proper locking.
[[maybe_unused]] [[nodiscard]]
static PgThreadPoolTask *pg_thread_pool_dequeue_task(PgThreadPool *pool) {
  PG_ASSERT(pool);
  PG_ASSERT(pool->tasks);
  PgThreadPoolTask *res = pool->tasks;
  pool->tasks = pool->tasks->next;
  return res;
}

static i32 pg_pool_worker_start_fn(void *data) {
  PG_ASSERT(data);
  PgThreadPool *pool = data;

  for (;;) {
    PG_ASSERT(0 == pg_mtx_lock(&pool->tasks_mtx));
    while (!pool->tasks) {
      if (pool->done) {
        PG_ASSERT(0 == pg_mtx_unlock(&pool->tasks_mtx));
        return 0;
      }

      PG_ASSERT(0 == pg_cnd_wait(&pool->tasks_cnd, &pool->tasks_mtx));
    }

    PgThreadPoolTask *task = pg_thread_pool_dequeue_task(pool);
    PG_ASSERT(0 == pg_mtx_unlock(&pool->tasks_mtx));

    PG_ASSERT(task);
    PG_ASSERT(task->fn);

    (void)task->fn(task->data);
  }
}

[[maybe_unused]] [[nodiscard]] static PgThreadPoolResult
pg_thread_pool_make(u32 size, PgAllocator *allocator) {
  PgThreadPoolResult res = {0};

  if (0 != pg_mtx_init(&res.value.tasks_mtx, PG_MUTEX_KIND_PLAIN)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  if (0 != pg_cnd_init(&res.value.tasks_cnd)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.value.workers.len = size;
  res.value.workers.data =
      pg_alloc(allocator, sizeof(PgThread), _Alignof(PgThread), size);

  for (u32 i = 0; i < size; i++) {
    PgThread *it = PG_SLICE_AT_PTR(&res.value.workers, i);
    PgThreadResult res_thread = pg_thread_create(pg_pool_worker_start_fn, it);
    if (0 != res_thread.err) {
      res.err = res_thread.err;
      return res;
    }

    PG_SLICE_AT(res.value.workers, i) = res_thread.value;
  }

  return res;
}

[[maybe_unused]]
static void pg_thread_pool_enqueue_task(PgThreadPool *pool, PgThreadFn fn,
                                        void *data, PgAllocator *allocator) {
  PG_ASSERT(pool);
  PG_ASSERT(fn);
  PG_ASSERT(data);
  PG_ASSERT(allocator);

  PgThreadPoolTask *task = PG_NEW(PgThreadPoolTask, allocator);
  task->data = data;
  task->fn = fn;

  PG_ASSERT(0 == pg_mtx_lock(&pool->tasks_mtx));
  if (!pool->tasks) {
    pool->tasks = task;
  } else {
    task->next = pool->tasks;
    pool->tasks = task;
  }
  PG_ASSERT(0 == pg_cnd_signal(&pool->tasks_cnd));
  PG_ASSERT(0 == pg_mtx_unlock(&pool->tasks_mtx));
}

[[maybe_unused]] static void pg_thread_pool_wait(PgThreadPool *pool) {
  PG_ASSERT(0 == pg_mtx_lock(&pool->tasks_mtx));
  pool->done = true;
  PG_ASSERT(0 == pg_cnd_broadcast(&pool->tasks_cnd));
  PG_ASSERT(0 == pg_mtx_unlock(&pool->tasks_mtx));

  for (u32 i = 0; i < pool->workers.len; i++) {
    (void)pg_thread_join(PG_SLICE_AT(pool->workers, i));
  }
}

[[maybe_unused]] [[nodiscard]] static PgElfSymbolType
pg_elf_symbol_get_type(PgElfSymbolTableEntry sym) {
  return sym.info & 0xf;
}

[[maybe_unused]] [[nodiscard]] static PgElfSymbolBind
pg_elf_symbol_get_bind(PgElfSymbolTableEntry sym) {
  return sym.info >> 4;
}

[[nodiscard]] static Pgu8SliceResult
pg_elf_get_section_header_bytes(PgElf elf, u32 section_idx) {
  Pgu8SliceResult res = {0};

  if (section_idx >= elf.section_headers.len) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  PgElfSectionHeader section = PG_SLICE_AT(elf.section_headers, section_idx);

  u64 end = 0;
  if (__builtin_add_overflow(section.offset, section.size, &end)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  if (end >= elf.bytes.len) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.value = PG_SLICE_RANGE(elf.bytes, section.offset, end);

  return res;
}

[[nodiscard]] static PgStringResult pg_elf_get_sh_string_at(PgElf elf,
                                                            u32 offset) {
  PgStringResult res = {0};

  if (offset >= elf.shtrtab_bytes.len) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  Pgu8Slice at = PG_SLICE_RANGE_START(elf.shtrtab_bytes, offset);

  PgBytesCut cut = pg_bytes_cut_byte(at, 0);
  if (!cut.has_value) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.value = cut.left;

  return res;
}

[[nodiscard]] static PgStringResult pg_elf_get_string_at(PgElf elf,
                                                         u32 offset) {
  PgStringResult res = {0};

  if (offset >= elf.strtab_bytes.len) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  Pgu8Slice at = PG_SLICE_RANGE_START(elf.strtab_bytes, offset);

  PgBytesCut cut = pg_bytes_cut_byte(at, 0);
  if (!cut.has_value) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.value = cut.left;

  return res;
}

[[nodiscard]] static PgElfSectionHeader *
pg_elf_section_header_find_ptr_by_name_and_kind(PgElf *elf, PgString name,
                                                PgElfSectionHeaderKind kind) {
  PG_ASSERT(elf);

  for (u32 i = 0; i < elf->section_headers.len; i++) {
    PgElfSectionHeader *section = PG_SLICE_AT_PTR(&elf->section_headers, i);

    if (kind != section->kind) {
      continue;
    }

    PgStringResult res_str = pg_elf_get_sh_string_at(*elf, section->name);
    if (res_str.err) {
      continue;
    }

    if (!pg_string_eq(res_str.value, name)) {
      continue;
    }

    return section;
  }
  return nullptr;
}

[[maybe_unused]] [[nodiscard]] static PgElfResult
pg_elf_parse(Pgu8Slice elf_bytes) {
  PgElfResult res = {0};
  res.value.bytes = elf_bytes;

  if (elf_bytes.len < sizeof(PgElfHeader)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  pg_memcpy(&res.value.header, elf_bytes.data, sizeof(res.value.header));

  // Section headers.
  {
    PgElfHeader h = res.value.header;
    u64 section_headers_size = 0;
    if (__builtin_mul_overflow(h.section_header_entries_count,
                               h.section_header_entry_size,
                               &section_headers_size)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    u64 section_headers_end = 0;
    if (__builtin_add_overflow(h.section_header_offset, section_headers_size,
                               &section_headers_end)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    PG_ASSERT(h.section_header_offset <= section_headers_end);

    Pgu8Slice section_headers_bytes =
        PG_SLICE_RANGE(elf_bytes, h.section_header_offset, section_headers_end);
    res.value.section_headers = (PgElfSectionHeaderDyn){
        .data = (void *)section_headers_bytes.data,
        .len = h.section_header_entries_count,
        .cap = h.section_header_entries_count,
    };

    // Locate `shtrtab` section.
    Pgu8SliceResult res_bytes = pg_elf_get_section_header_bytes(
        res.value, res.value.header.section_header_shstrtab_index);
    if (res_bytes.err) {
      res.err = res_bytes.err;
      return res;
    }
    res.value.shtrtab_bytes = res_bytes.value;

    if (PG_SLICE_IS_EMPTY(res.value.shtrtab_bytes)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    PgElfSectionHeader *section_text =
        pg_elf_section_header_find_ptr_by_name_and_kind(
            &res.value, PG_S(".text"), PG_ELF_SECTION_HEADER_KIND_PROGBITS);
    if (section_text) {
      u32 idx = section_text - res.value.section_headers.data;
      Pgu8SliceResult res_bytes =
          pg_elf_get_section_header_bytes(res.value, idx);
      if (res_bytes.err) {
        res.err = res_bytes.err;
        return res;
      }
      res.value.section_header_text_bytes = res_bytes.value;
      res.value.section_header_text_idx = idx;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu8SliceResult
pg_elf_symbol_get_program_text(PgElf elf, PgElfSymbolTableEntry sym) {
  Pgu8SliceResult res = {0};

  if (elf.section_header_text_idx != sym.section_header_table_index) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  if (sym.value >= elf.section_header_text_bytes.len) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  u64 end = 0;
  if (__builtin_add_overflow(sym.value, sym.size, &end)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.value = PG_SLICE_RANGE(elf.section_header_text_bytes, sym.value, end);
  return res;
}

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/event.h>

[[maybe_unused]] [[nodiscard]] static PgAioResult pg_aio_init() {
  PgAioResult res = {0};

  i32 ret = kqueue();
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value.aio.fd = ret;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_aio_register_interest_fs_name(PgAio *aio, PgString name,
                                 PgAioEventKind interest,
                                 PgAllocator *allocator) {
  PgFileDescriptorResult res =
      pg_file_open(name, PG_FILE_ACCESS_READ, 0666, false, allocator);
  if (res.err) {
    return res;
  }

  res.err = pg_aio_register_interest_fd(*aio, res.value, interest);
  if (res.err) {
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_register_interest_fd(PgAio aio, PgFileDescriptor fd,
                            PgAioEventKind interest) {

  struct kevent changelist[1] = {0};
  changelist[0].ident = (u64)fd.fd;
  changelist[0].flags = EV_ADD;

  if (interest & PG_AIO_EVENT_KIND_READABLE) {
    changelist[0].filter |= EVFILT_READ;
  }
  if (interest & PG_AIO_EVENT_KIND_WRITABLE) {
    changelist[0].filter |= EVFILT_WRITE;
  }
  if (interest & PG_AIO_EVENT_KIND_FILE_CREATED) {
    // TODO
  }
  if (interest & PG_AIO_EVENT_KIND_FILE_MODIFIED) {
    changelist[0].filter |= EVFILT_VNODE;
    changelist[0].fflags |= NOTE_WRITE;
  }
  if (interest & PG_AIO_EVENT_KIND_FILE_DELETED) {
    changelist[0].filter |= EVFILT_VNODE;
    changelist[0].fflags |= NOTE_DELETE;
  }

  i32 ret = kevent(aio.aio.fd, changelist, 1, nullptr, 0, nullptr);
  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_unregister_interest(PgAio aio, PgFileDescriptor fd,
                           PgAioEventKind interest) {
  struct kevent changelist[1] = {0};
  changelist[0].ident = (u64)fd.fd;
  changelist[0].flags = EV_DELETE;

  if (PG_AIO_EVENT_KIND_FILE_MODIFIED & interest) {
    changelist[0].filter = EVFILT_VNODE;
    changelist[0].fflags = NOTE_WRITE;
  }
  if (PG_AIO_EVENT_KIND_FILE_DELETED & interest) {
    changelist[0].filter = EVFILT_VNODE;
    changelist[0].fflags = NOTE_DELETE;
  }
  if (PG_AIO_EVENT_KIND_FILE_CREATED & interest) {
    // TODO
  }
  if (PG_AIO_EVENT_KIND_READABLE & interest) {
    changelist[0].filter = EVFILT_READ;
  }
  if (PG_AIO_EVENT_KIND_WRITABLE & interest) {
    changelist[0].filter = EVFILT_WRITE;
  }

  i32 ret = kevent(aio.aio.fd, changelist, 1, nullptr, 0, nullptr);
  if (-1 == ret && ENOENT != errno) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_aio_wait(PgAio aio, PgAioEventSlice events_out, Pgu32Option timeout_ms) {
  Pgu64Result res = {0};

  struct kevent eventlist[1024] = {0};
  u64 eventlist_len = PG_MIN(PG_STATIC_ARRAY_LEN(eventlist), events_out.len);
  if (0 == eventlist_len) {
    return res;
  }

  struct timespec timeout = {0};
  if (timeout_ms.has_value) {
    timeout.tv_sec = timeout_ms.value / 1000;
    timeout.tv_nsec = (timeout_ms.value % 1000) * 1000 * 1000;
  }

  i32 ret = kevent(aio.aio.fd, nullptr, 0, eventlist, eventlist_len,
                   timeout_ms.has_value ? &timeout : nullptr);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  for (u64 i = 0; i < (u64)ret; i++) {
    struct kevent kevent = PG_C_ARRAY_AT(eventlist, eventlist_len, i);
    PgAioEvent *ev = PG_SLICE_AT_PTR(&events_out, i);
    ev->fd.fd = kevent.ident;

    if (EV_ERROR & kevent.flags) {
      ev->kind |= PG_AIO_EVENT_KIND_ERROR;
    }
    if (EV_EOF & kevent.flags) {
      ev->kind |= PG_AIO_EVENT_KIND_EOF;
    }
    if (EVFILT_READ & kevent.filter) {
      ev->kind |= PG_AIO_EVENT_KIND_READABLE;
    }
    if (EVFILT_WRITE & kevent.filter) {
      ev->kind |= PG_AIO_EVENT_KIND_WRITABLE;
    }
    if ((EVFILT_VNODE & kevent.filter) && (NOTE_DELETE & kevent.fflags)) {
      ev->kind |= PG_AIO_EVENT_KIND_FILE_DELETED;
    }
    if ((EVFILT_VNODE & kevent.filter) && (NOTE_WRITE & kevent.fflags)) {
      ev->kind |= PG_AIO_EVENT_KIND_FILE_MODIFIED;
    }
  }

  res.value = (u64)ret;
  return res;
};

// TODO: Use `pg_aio_wait` ?
[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_aio_wait_cqe(PgAio aio, PgRing *cqe, Pgu32Option timeout_ms) {
  Pgu64Result res = {0};
  u64 can_write_count = pg_ring_can_write_count(*cqe) / sizeof(PgAioEvent);

  struct kevent eventlist[1024] = {0};
  u64 eventlist_len = PG_MIN(PG_STATIC_ARRAY_LEN(eventlist), can_write_count);
  if (0 == eventlist_len) {
    return res;
  }

  struct timespec timeout = {0};
  if (timeout_ms.has_value) {
    timeout.tv_sec = timeout_ms.value / 1000;
    timeout.tv_nsec = (timeout_ms.value % 1000) * 1000 * 1000;
  }

  i32 ret = kevent(aio.aio.fd, nullptr, 0, eventlist, eventlist_len,
                   timeout_ms.has_value ? &timeout : nullptr);
  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  for (u64 i = 0; i < (u64)ret; i++) {
    struct kevent kevent = PG_C_ARRAY_AT(eventlist, eventlist_len, i);
    PgAioEvent ev = {0};
    ev.fd.fd = kevent.ident;

    if (EV_ERROR & kevent.flags) {
      ev.kind |= PG_AIO_EVENT_KIND_ERROR;
    }
    if (EV_EOF & kevent.flags) {
      ev.kind |= PG_AIO_EVENT_KIND_EOF;
    }
    if (EVFILT_READ & kevent.filter) {
      ev.kind |= PG_AIO_EVENT_KIND_READABLE;
    }
    if (EVFILT_WRITE & kevent.filter) {
      ev.kind |= PG_AIO_EVENT_KIND_WRITABLE;
    }
    if ((EVFILT_VNODE & kevent.filter) && (NOTE_DELETE & kevent.fflags)) {
      ev.kind |= PG_AIO_EVENT_KIND_FILE_DELETED;
    }
    if ((EVFILT_VNODE & kevent.filter) && (NOTE_WRITE & kevent.fflags)) {
      ev.kind |= PG_AIO_EVENT_KIND_FILE_MODIFIED;
    }

    Pgu8Slice ev_bytes = {.data = (u8 *)&ev, .len = sizeof(ev)};
    PG_ASSERT(sizeof(ev) == pg_ring_write_bytes(cqe, ev_bytes));
  }

  res.value = (u64)ret;
  return res;
}

#endif

[[maybe_unused]] [[nodiscard]] static bool
pg_aio_is_fs_event_kind(PgAioEventKind kind) {
  switch (kind) {
  case PG_AIO_EVENT_KIND_FILE_MODIFIED:
  case PG_AIO_EVENT_KIND_FILE_DELETED:
  case PG_AIO_EVENT_KIND_FILE_CREATED:
    return true;

  case PG_AIO_EVENT_KIND_READABLE:
  case PG_AIO_EVENT_KIND_WRITABLE:
  case PG_AIO_EVENT_KIND_NONE:
  case PG_AIO_EVENT_KIND_ERROR:
  case PG_AIO_EVENT_KIND_EOF:
    return false;

  default:
    PG_ASSERT(0);
  }
}

#ifdef PG_OS_LINUX

[[maybe_unused]] static void pg_thread_yield() { sched_yield(); }

[[maybe_unused]] [[nodiscard]] static PgString
pg_self_exe_get_path(PgAllocator *allocator) {
  static _Atomic PgOnce once = PG_ONCE_UNINITIALIZED;
  static PgString res = {0};
  if (pg_once_do(&once)) {
    char path_c[PG_PATH_MAX] = {0};
    i32 ret = 0;
    do {
      ret = readlink("/proc/self/exe", path_c, PG_STATIC_ARRAY_LEN(path_c));
    } while (-1 == ret && EINTR == errno);
    if (-1 == errno) {
      return res;
    }

    res = pg_string_clone(pg_cstr_to_string(path_c), allocator);

    pg_once_mark_as_done(&once);
  }

  PG_ASSERT(!pg_string_is_empty(res));
  return res;
}

[[nodiscard]] static u64 pg_self_pie_get_offset() {
  static _Atomic PgOnce once = PG_ONCE_UNINITIALIZED;
  static u64 res = 0;

  if (pg_once_do(&once)) {
    u8 mem[4096] = {0};
    PgArena arena = pg_arena_make_from_mem(mem, PG_STATIC_ARRAY_LEN(mem));
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    PgFileDescriptorResult res_fd = pg_file_open(
        PG_S("/proc/self/maps"), PG_FILE_ACCESS_READ, 0600, false, allocator);
    if (0 != res_fd.err) {
      goto end_once;
    }
    PgFileDescriptor fd = res_fd.value;

    PgReader r = pg_reader_make_from_file(fd, 512, allocator);

    u64 max_lines = 512;

    for (u64 _i = 0; _i < max_lines; _i++) {
      u8 line[512] = {0};
      PgString line_slice = PG_SLICE_FROM_C(line);
      Pgu64OptionResult res_line =
          pg_reader_read_line(&r, PG_NEWLINE_KIND_LF, line_slice);
      if (0 != res_line.err) {
        goto end_file;
      }
      if (!res_line.value.has_value) {
        goto end_file;
      }
      line_slice.len = res_line.value.value;

      PgStringCut cut_space = pg_string_cut_rune(line_slice, ' ');
      if (!cut_space.has_value) {
        continue;
      }

      PgString mem_range = cut_space.left;
      PgString perms = cut_space.right;

      if (!pg_string_contains_rune(perms, 'x')) {
        continue;
      }

      PgStringCut cut_dash = pg_string_cut_rune(mem_range, '-');
      if (!cut_dash.has_value) {
        continue;
      }

      PgParseNumberResult res_mem_start =
          pg_string_parse_u64(cut_dash.left, 16, false);
      if (!res_mem_start.present) {
        continue;
      }

      PgParseNumberResult res_mem_end =
          pg_string_parse_u64(cut_dash.right, 16, false);
      if (!res_mem_end.present) {
        continue;
      }

      u64 start = res_mem_start.n;
      u64 end = res_mem_end.n;
      PG_ASSERT(0 == (start % pg_os_get_page_size()));
      PG_ASSERT(0 == (end % pg_os_get_page_size()));

      if (start <= (u64)&pg_self_pie_get_offset &&
          (u64)&pg_self_pie_get_offset < end) {
        res = start;
        goto end_file;
      }
    }
  end_file:
    (void)pg_file_close(fd);

  end_once:
    pg_once_mark_as_done(&once);
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringResult
pg_self_load_debug_info(PgAllocator *allocator) {
  PgStringResult res = {0};

  PgString exe_path = pg_self_exe_get_path(allocator);
  if (pg_string_is_empty(exe_path)) {
    return res;
  }

  // TODO: Only read the relevant parts.
  // Depending on the size of the executable.
  PgStringResult res_exe = pg_file_read_full_from_path(exe_path, allocator);
  if (res_exe.err) {
    res.err = res_exe.err;
    return res;
  }
  PgString exe = res_exe.value;

  // TODO: Other formats.
  PgElfResult res_elf = pg_elf_parse(exe);
  if (res_elf.err) {
    res.err = res_elf.err;
    return res;
  }
  PgElf elf = res_elf.value;

  PgElfSectionHeader *section_header_debug_abbrev =
      pg_elf_section_header_find_ptr_by_name_and_kind(
          &elf, PG_S(".debug_abbrev"), PG_ELF_SECTION_HEADER_KIND_PROGBITS);

  PgElfSectionHeader *section_header_debug_info =
      pg_elf_section_header_find_ptr_by_name_and_kind(
          &elf, PG_S(".debug_info"), PG_ELF_SECTION_HEADER_KIND_PROGBITS);

  // TODO
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_aio_inotify_init() {
  PgFileDescriptorResult res = {0};

  i32 ret = 0;
  do {
    ret = inotify_init();
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value.fd = ret;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_aio_inotify_register_interest(PgAio aio, PgString name,
                                 PgAioEventKind interest) {
  PgFileDescriptorResult res = {0};

  if (0 == name.len) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  u32 interest_linux = 0;
  if (interest & PG_AIO_EVENT_KIND_FILE_CREATED) {
    interest_linux |= IN_CREATE;
  }
  if (interest & PG_AIO_EVENT_KIND_FILE_MODIFIED) {
    interest_linux |= IN_MODIFY;
  }
  if (interest & PG_AIO_EVENT_KIND_FILE_DELETED) {
    interest_linux |= IN_DELETE;
  }

  u8 name_c[4096] = {0};
  PG_ASSERT(name.data);
  PG_ASSERT(name.len < PG_STATIC_ARRAY_LEN(name_c));
  memcpy(name_c, name.data, name.len);

  i32 ret = 0;
  do {
    ret = inotify_add_watch(aio.inotify.value.fd, (const char *)name_c,
                            interest_linux);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value.fd = ret;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgAioResult pg_aio_init() {
  PgAioResult res = {0};

  i32 ret = 0;
  do {
    ret = epoll_create(1 /*Ignored*/);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value.aio.fd = ret;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_ensure_inotify(PgAio *aio) {
  PG_ASSERT(aio);

  if (aio->inotify.has_value) {
    return 0;
  }

  PgFileDescriptorResult res = pg_aio_inotify_init();
  if (0 != res.err) {
    return res.err;
  }

  aio->inotify.has_value = true;
  aio->inotify.value = res.value;

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_aio_register_interest_fs_name(PgAio *aio, PgString name,
                                 PgAioEventKind interest,
                                 PgAllocator *allocator) {
  (void)allocator; // Unused on Linux.

  PgFileDescriptorResult res = {0};

  PgError err = pg_aio_ensure_inotify(aio);
  if (err) {
    res.err = err;
    return res;
  }

  res = pg_aio_inotify_register_interest(*aio, name, interest);
  if (res.err) {
    return res;
  }

  err = pg_aio_register_interest_fd(*aio, res.value, interest);
  if (err) {
    res.err = err;
    return res;
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_register_interest_fd(PgAio aio, PgFileDescriptor fd,
                            PgAioEventKind interest) {
  struct epoll_event event = {0};
  event.data.fd = fd.fd;

  if (interest & PG_AIO_EVENT_KIND_READABLE) {
    event.events |= EPOLLIN;
  }
  if (interest & PG_AIO_EVENT_KIND_WRITABLE) {
    event.events |= EPOLLOUT;
  }

  i32 ret = 0;
  do {
    ret = epoll_ctl(aio.aio.fd, EPOLL_CTL_ADD, fd.fd, &event);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_aio_unregister_interest(PgAio aio, PgFileDescriptor fd,
                           PgAioEventKind interest) {
  struct epoll_event event = {0};
  event.data.fd = fd.fd;

  if (interest & PG_AIO_EVENT_KIND_READABLE) {
    event.events |= EPOLLIN;
  }
  if (interest & PG_AIO_EVENT_KIND_WRITABLE) {
    event.events |= EPOLLOUT;
  }

  i32 ret = 0;
  do {
    ret = epoll_ctl(aio.aio.fd, EPOLL_CTL_DEL, fd.fd, &event);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    return (PgError)errno;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_aio_wait(PgAio aio, PgAioEventSlice events_out, Pgu32Option timeout_ms) {
  Pgu64Result res = {0};

  struct epoll_event events[1024] = {0};
  u64 events_len = PG_MIN(events_out.len, PG_STATIC_ARRAY_LEN(events));

  i32 ret = 0;
  do {
    ret = epoll_wait(aio.aio.fd, events, (i32)events_len,
                     timeout_ms.has_value ? (i32)timeout_ms.value : -1);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value = (u64)ret;

  for (u64 i = 0; i < (u64)ret; i++) {
    struct epoll_event e =
        PG_C_ARRAY_AT(events, PG_STATIC_ARRAY_LEN(events), i);

    PG_SLICE_AT(events_out, i).fd.fd = e.data.fd;

    if (e.events & EPOLLIN) {
      PG_SLICE_AT(events_out, i).kind |= PG_AIO_EVENT_KIND_READABLE;
    }
    if (e.events & EPOLLOUT) {
      PG_SLICE_AT(events_out, i).kind |= PG_AIO_EVENT_KIND_WRITABLE;
    }
    if (e.events & EPOLLERR) {
      PG_SLICE_AT(events_out, i).kind |= PG_AIO_EVENT_KIND_ERROR;
    }
    if (e.events & EPOLLHUP) {
      PG_SLICE_AT(events_out, i).kind |= PG_AIO_EVENT_KIND_EOF;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_aio_wait_cqe(PgAio aio, PgRing *cqe, Pgu32Option timeout_ms) {
  Pgu64Result res = {0};

  struct epoll_event events[1024] = {0};
  u64 can_write_count = pg_ring_can_write_count(*cqe) / sizeof(PgAioEvent);
  u64 events_len = PG_MIN(can_write_count, PG_STATIC_ARRAY_LEN(events));

  i32 ret = 0;
  do {
    ret = epoll_wait(aio.aio.fd, events, (i32)events_len,
                     timeout_ms.has_value ? (i32)timeout_ms.value : -1);
  } while (-1 == ret && EINTR == errno);

  if (-1 == ret) {
    res.err = (PgError)errno;
    return res;
  }

  res.value = (u64)ret;

  for (u64 i = 0; i < (u64)ret; i++) {
    struct epoll_event e =
        PG_C_ARRAY_AT(events, PG_STATIC_ARRAY_LEN(events), i);

    PgAioEvent ev = {0};
    ev.fd.fd = e.data.fd;

    if (e.events & EPOLLIN) {
      ev.kind |= PG_AIO_EVENT_KIND_READABLE;
    }
    if (e.events & EPOLLOUT) {
      ev.kind |= PG_AIO_EVENT_KIND_WRITABLE;
    }
    if (e.events & EPOLLERR) {
      ev.kind |= PG_AIO_EVENT_KIND_ERROR;
    }
    if (e.events & EPOLLHUP) {
      ev.kind |= PG_AIO_EVENT_KIND_EOF;
    }

    Pgu8Slice ev_bytes = {.data = (u8 *)&ev, .len = sizeof(ev)};
    PG_ASSERT(sizeof(ev) == pg_ring_write_bytes(cqe, ev_bytes));
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgAioEventResult
pg_aio_fs_wait_one(PgAio aio, Pgu32Option timeout_ms, PgAllocator *allocator) {
  PgAioEventResult res = {0};

  PgAioEventSlice events_slice = {
      .data = &res.value,
      .len = 1,
  };

  Pgu64Result res_wait = pg_aio_wait(aio, events_slice, timeout_ms);
  PG_ASSERT(0 == res_wait.err);
  events_slice.len = res_wait.value;

  if (res.value.kind & PG_AIO_EVENT_KIND_READABLE) {
    u8 inev_data[sizeof(struct inotify_event) + 4096 + 1] = {0};
    Pgu8Slice inev_slice = {
        .data = inev_data,
        .len = PG_STATIC_ARRAY_LEN(inev_data),
    };
    Pgu64Result res_read = pg_file_read(res.value.fd, inev_slice);
    if (res_read.err) {
      res.err = res_read.err;
      return res;
    }
    PG_ASSERT(res_read.value >= sizeof(struct inotify_event));

    struct inotify_event inev = *(struct inotify_event *)inev_data;
    res.value = (PgAioEvent){0};
    if (inev.mask & IN_CREATE) {
      res.value.kind |= PG_AIO_EVENT_KIND_FILE_CREATED;
    }
    if (inev.mask & IN_MODIFY) {
      res.value.kind |= PG_AIO_EVENT_KIND_FILE_MODIFIED;
    }
    if (inev.mask & IN_DELETE) {
      res.value.kind |= PG_AIO_EVENT_KIND_FILE_DELETED;
    }

    if (0 != inev.len) {
      res.value.name = pg_string_make(inev.len, allocator);
      memcpy(res.value.name.data, inev_data + sizeof(inev), inev.len);
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_file_send_to_socket(PgFileDescriptor dst, PgFileDescriptor src) {
  Pgu64Result res_size = pg_file_size(src);
  if (res_size.err) {
    return res_size.err;
  }

  i64 offset = 0;
  u64 size = res_size.value;

  for (u64 _i = 0; _i < size; _i++) {
    i64 ret = 0;
    do {
      ret = sendfile(dst.fd, src.fd, &offset, size);
    } while (-1 == ret && EINTR == errno);

    if (-1 == ret) {
      // TODO: Perhaps fallback to `read(2)` + `write(2)` in case of
      // `EINVAL` or `ENOSYS`.
      return (PgError)errno;
    }

    size -= (u64)ret;
    offset += ret;

    if (0 == size) {
      break;
    }
  }
  return 0;
}

#endif

#ifdef PG_OS_APPLE

// TODO: is pthread_yield defined on macos?
[[maybe_unused]] static void pg_thread_yield() {}

#endif

[[maybe_unused]] [[nodiscard]]
static PgError
pg_http_server_handler(PgFileDescriptor sock, PgHttpServerOptions options,
                       PgLogger *logger, PgAllocator *allocator) {
  // TODO: Timeouts.

  PgReader reader =
      pg_reader_make_from_socket(sock, PG_HTTP_LINE_MAX_LEN, allocator);
  PgHttpRequestReadResult res_req = pg_http_read_request(&reader, allocator);

  if (res_req.err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http handler: failed to parse http request",
           pg_log_c_err("err", res_req.err));
    return res_req.err;
  }

  if (!res_req.done) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http handler: failed to read full http request",
           pg_log_c_err("done", res_req.done));
    return PG_ERR_EOF;
  }

  PgHttpRequest req = res_req.req;

  PgWriter writer =
      pg_writer_make_from_socket(sock, PG_HTTP_LINE_MAX_LEN, allocator);

  PG_ASSERT(options.handler);
  options.handler(req, &reader, &writer, logger, allocator, options.ctx);

  return 0;
}

[[maybe_unused]]
static PgError pg_http_server_start(PgHttpServerOptions options,
                                    PgLogger *logger) {
  PgError err = 0;
  err = pg_process_avoid_child_zombies();
  if (err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http server: failed to avoid child zombies",
           pg_log_c_u16("port", options.port), pg_log_c_err("err", err));
    return err;
  }

  PgFileDescriptorResult res_create = pg_net_create_tcp_socket();
  if (res_create.err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http server: failed to create tcp socket",
           pg_log_c_u16("port", options.port), pg_log_c_err("err", err));
    return res_create.err;
  }
  PgFileDescriptor server_socket = res_create.value;

  err = pg_net_socket_enable_reuse(server_socket);
  if (err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http server: failed to enable socket reuse",
           pg_log_c_u16("port", options.port), pg_log_c_err("err", err));
    goto end;
  }

  PgIpv4Address address = {.port = options.port};
  err = pg_net_tcp_bind_ipv4(server_socket, address);
  if (err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR, "http server: failed to bind tcp socket",
           pg_log_c_u16("port", options.port), pg_log_c_err("err", err));
    goto end;
  }

  err = pg_net_tcp_listen(server_socket, options.listen_backlog);
  if (err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http server: failed to listen on tcp socket",
           pg_log_c_u16("port", options.port), pg_log_c_err("err", err));
    goto end;
  }

  pg_log(logger, PG_LOG_LEVEL_INFO, "http server: listening",
         pg_log_c_u16("port", options.port));

  for (;;) {
    PgIpv4AddressAcceptResult res_accept = pg_net_tcp_accept(server_socket);
    if (res_accept.err) {
      // TODO: Some errors are retryable.
      err = res_accept.err;
      pg_log(logger, PG_LOG_LEVEL_ERROR,
             "http server: failed to accept new connection",
             pg_log_c_u16("port", options.port), pg_log_c_err("err", err));
      goto end;
    }

    Pgu32Result res_proc_dup = pg_process_dup();
    if (res_proc_dup.err) {
      pg_log(
          logger, PG_LOG_LEVEL_ERROR,
          "http server: failed to spawn new process to handle new connection",
          pg_log_c_u16("port", options.port),
          pg_log_c_err("err", res_proc_dup.err));
      (void)pg_net_socket_close(res_accept.socket);
      continue;
    }

    if (0 == res_proc_dup.value) { // Child.
      PgArena arena =
          pg_arena_make_from_virtual_mem(options.http_handler_arena_mem);
      PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
      PgAllocator *allocator =
          pg_arena_allocator_as_allocator(&arena_allocator);

      (void)pg_http_server_handler(res_accept.socket, options, logger,
                                   allocator);
      exit(0);
    }

    PG_ASSERT(res_proc_dup.value); // Parent.

    (void)pg_net_socket_close(res_accept.socket);
  }

end:
  (void)pg_net_socket_close(server_socket);
  return err;
}

[[maybe_unused]] [[nodiscard]] static PgAioEventOption
pg_aio_cqe_dequeue(PgRing *cqe) {
  PgAioEventOption res = {0};
  if (pg_ring_can_read_count(*cqe) < sizeof(PgAioEvent)) {
    return res;
  }

  Pgu8Slice bytes = {.data = (u8 *)&res.value, .len = sizeof(PgAioEvent)};
  PG_ASSERT(sizeof(PgAioEvent) == pg_ring_read_bytes(cqe, bytes));

  res.has_value = true;
  return res;
}

typedef struct {
  bool required;
  // Name of the value e.g. `linker` for `-fuse-ld`.
  // If non-empty: the option expects 1 or more value(s).
  // It appears in the help message.
  PgString value_name;
  PgString name_short;
  PgString name_long;
  PgString description;
} PgCliOptionDescription;
PG_OPTION(PgCliOptionDescription) PgCliOptionDescriptionOption;
PG_DYN(PgCliOptionDescription) PgCliOptionDescriptionDyn;
PG_SLICE(PgCliOptionDescription) PgCliOptionDescriptionSlice;

typedef struct {
  PgCliOptionDescription description;
  PgStringDyn values;
  PgError err;
} PgCliOption;
PG_DYN(PgCliOption) PgCliOptionDyn;
PG_SLICE(PgCliOption) PgCliOptionSlice;
PG_RESULT(PgCliOption) PgCliOptionResult;
PG_OPTION(PgCliOption) PgCliOptionOption;
PG_RESULT(PgCliOptionOption) PgCliOptionOptionResult;

typedef struct {
  PgStringDyn plain_arguments;
  PgCliOptionDyn options;

  // Error reporting.
  PgError err;
  PgString err_argv;
} PgCliParseResult;

[[nodiscard]] static bool pg_cli_is_short_option(PgString s) {
  PgStringOption dash_opt = pg_string_consume_rune(s, '-');
  if (!dash_opt.has_value) {
    return false;
  }

  // Is long?
  s = dash_opt.value;
  dash_opt = pg_string_consume_rune(s, '-');
  return !dash_opt.has_value;
}

[[nodiscard]] static bool pg_cli_is_no_option(PgString s) {
  PgStringOption dash_opt = pg_string_consume_rune(s, '-');
  return !dash_opt.has_value;
}

[[nodiscard]] static PgCliOptionDescriptionOption
pg_cli_desc_find_by_name(PgCliOptionDescriptionSlice descs, PgString name) {
  PgCliOptionDescriptionOption res = {0};

  for (u64 i = 0; i < descs.len; i++) {
    PgCliOptionDescription it = PG_SLICE_AT(descs, i);
    if (pg_string_eq(it.name_long, name) || pg_string_eq(it.name_short, name)) {
      res.has_value = true;
      res.value = it;
      return res;
    }
  }

  return res;
}

[[nodiscard]] static PgCliOption *
pg_cli_options_find_by_name(PgCliOptionDyn options, PgString name_short,
                            PgString name_long) {
  for (u64 i = 0; i < options.len; i++) {
    PgCliOption *it = PG_SLICE_AT_PTR(&options, i);
    if (pg_string_eq(it->description.name_long, name_long) ||
        pg_string_eq(it->description.name_short, name_short)) {
      return it;
    }
  }

  return nullptr;
}

[[nodiscard]] static PgError
pg_cli_handle_one_short_option(PgString opt_name, bool with_opt_value_allowed,
                               PgCliOptionDyn *options,
                               PgCliOptionDescriptionSlice descs, char **argv,
                               u64 *argv_idx, PgAllocator *allocator) {

  PgCliOptionDescriptionOption desc_has_value =
      pg_cli_desc_find_by_name(descs, opt_name);
  if (!desc_has_value.has_value) {
    return PG_ERR_CLI_UNKNOWN_OPTION;
  }
  PgCliOptionDescription desc = desc_has_value.value;

  if (!pg_string_is_empty(desc.value_name) && !with_opt_value_allowed) {
    return PG_ERR_CLI_FORBIDEN_OPTION_VALUE;
  }

  PgString opt_value = {0};
  if (!pg_string_is_empty(desc.value_name)) {
    // A value is expected in the next `argv` slot.
    *argv_idx += 1;
    opt_value = pg_cstr_to_string(argv[*argv_idx]);
  }

  if (!pg_string_is_empty(desc.value_name) &&
      (0 == opt_value.len || !pg_cli_is_no_option(opt_value))) {
    return PG_ERR_CLI_MISSING_REQUIRED_OPTION_VALUE;
  }

  // Find by the short name.
  PgCliOption *opt_existing =
      pg_cli_options_find_by_name(*options, desc.name_short, PG_S(""));
  if (opt_existing) {
    if (pg_string_is_empty(desc.value_name)) {
      // Option already exists (without value), do not add it again.
      return 0;
    }

    // With value.

    PG_DYN_PUSH(&opt_existing->values, opt_value, allocator);
    return 0;
  }

  // Add the new option.
  PgCliOption opt = {.description = desc};
  if (!pg_string_is_empty(desc.value_name)) {
    PG_DYN_PUSH(&opt.values, opt_value, allocator);
  }

  PG_DYN_PUSH(options, opt, allocator);
  return 0;
}

[[nodiscard]] static PgError
pg_cli_handle_one_long_option(PgString opt_name, PgCliOptionDyn *options,
                              PgCliOptionDescriptionSlice descs,
                              PgAllocator *allocator) {
  PgString opt_value = {0};
  PgStringCut cut = pg_string_cut_rune(opt_name, '=');
  if (cut.has_value) {
    opt_name = cut.left;
    opt_value = cut.right;
  }

  PgCliOptionDescriptionOption desc_has_value =
      pg_cli_desc_find_by_name(descs, opt_name);
  if (!desc_has_value.has_value) {
    return PG_ERR_CLI_UNKNOWN_OPTION;
  }
  PgCliOptionDescription desc = desc_has_value.value;

  // A value is expected in the same `argv` slot after `=`.
  if (!pg_string_is_empty(desc.value_name) && pg_string_is_empty(opt_value)) {
    return PG_ERR_CLI_MISSING_REQUIRED_OPTION_VALUE;
  }

  // Find by the long name.
  PgCliOption *opt_existing =
      pg_cli_options_find_by_name(*options, PG_S(""), desc.name_long);
  if (opt_existing) {
    if (pg_string_is_empty(desc.value_name)) {
      // Option already exists (without value), do not add it again.
      return 0;
    }

    // With value.

    PG_DYN_PUSH(&opt_existing->values, opt_value, allocator);
    return 0;
  }

  // Add the new option.
  PgCliOption opt = {.description = desc};
  if (!pg_string_is_empty(desc.value_name)) {
    PG_DYN_PUSH(&opt.values, opt_value, allocator);
  }

  PG_DYN_PUSH(options, opt, allocator);
  return 0;
}

static void pg_cli_inject_help_option(PgCliOptionDescriptionDyn *descs,
                                      PgAllocator *allocator) {
  PgCliOptionDescription desc = {
      .name_short = PG_S("h"),
      .name_long = PG_S("help"),
      .description = PG_S("Print this help message"),
  };

  PG_DYN_PUSH(descs, desc, allocator);
}

[[maybe_unused]] [[nodiscard]] static PgCliParseResult
pg_cli_parse(PgCliOptionDescriptionDyn *descs, int argc, char *argv[],
             PgAllocator *allocator) {
  PgCliParseResult res = {0};

  pg_cli_inject_help_option(descs, allocator);
  PgCliOptionDescriptionSlice desc_slice =
      PG_DYN_SLICE(PgCliOptionDescriptionSlice, *descs);

  for (u64 i = 1 /* Skip exe name */; i < (u64)argc; i++) {
    PgString arg = pg_cstr_to_string(argv[i]);

    // Plain argument.
    if (pg_cli_is_no_option(arg)) {
      PG_DYN_PUSH(&res.plain_arguments, arg, allocator);
      continue;
    }

    // Treat all remaining arguments as plain after `--`.
    if (pg_string_eq(arg, PG_S("--"))) {
      for (u64 j = i + 1; j < (u64)argc; j++) {
        PgString arg = pg_cstr_to_string(argv[j]);
        PG_DYN_PUSH(&res.plain_arguments, arg, allocator);
      }
      return res;
    }

    // Error if the option name starts with more than 2 `-` e.g. `---a` or it
    // only contains `-` e.g. `-`, `--`.
    if (pg_string_starts_with(arg, PG_S("---")) ||
        pg_string_eq(arg, PG_S("-"))) {
      res.err = PG_ERR_CLI_MALFORMED_OPTION;
      res.err_argv = arg;
      return res;
    }

    if (pg_cli_is_short_option(arg)) {
      PgString opt_name = pg_string_trim_left(arg, '-');
      PG_ASSERT(opt_name.len > 0);

      if (1 == opt_name.len) {
        PgError err = pg_cli_handle_one_short_option(
            opt_name, true, &res.options, desc_slice, argv, &i, allocator);
        if (0 != err) {
          res.err = err;
          // Best effort reporting.
          res.err_argv = (PG_ERR_CLI_UNKNOWN_OPTION == err) ? arg : opt_name;
          return res;
        }
      } else {
        // Handle coalesced short options, forbid option values.
        // This means that `./a.out -vH foo` will be parsed as
        // options: `-v`, `-H`, plain arguments: `foo`.
        // Assume ASCII.
        for (u64 j = 1; j < arg.len; j++) {
          PgString opt_name = {.data = PG_SLICE_AT_PTR(&arg, j), .len = 1};
          PgError err = pg_cli_handle_one_short_option(
              opt_name, false, &res.options, desc_slice, argv, &i, allocator);
          if (0 != err) {
            res.err = err;
            // Best effort reporting.

            if (PG_ERR_CLI_UNKNOWN_OPTION == err) {
              PgString synth = pg_string_make(2, allocator);
              PG_SLICE_AT(synth, 0) = '-';
              PG_SLICE_AT(synth, 1) = PG_SLICE_AT(arg, j);
              res.err_argv = synth;
            } else {
              res.err_argv = opt_name;
            }
            return res;
          }
        }
      }

      continue;
    }

    // Long option from there.

    PgString opt_name = pg_string_trim_left(arg, '-');
    PG_ASSERT(opt_name.len > 0);
    PgError err = pg_cli_handle_one_long_option(opt_name, &res.options,
                                                desc_slice, allocator);
    if (0 != err) {
      res.err = err;
      res.err_argv = arg;
      return res;
    }
  }

  // Check that all required options are present.
  for (u64 i = 0; i < desc_slice.len; i++) {
    PgCliOptionDescription desc = PG_SLICE_AT(desc_slice, i);
    if (!desc.required) {
      continue;
    }

    // Find by either the short or long name.
    if (!pg_cli_options_find_by_name(res.options, desc.name_short,
                                     desc.name_long)) {
      res.err = PG_ERR_CLI_MISSING_REQUIRED_OPTION;
      res.err_argv = pg_string_is_empty(desc.name_short) ? desc.name_long
                                                         : desc.name_short;
      return res;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_cli_generate_help(PgCliOptionDescriptionDyn descs, PgString exe_name,
                     PgString description, PgString plain_arguments_description,
                     PgAllocator *allocator) {
  Pgu8Dyn sb = {0};
  PG_DYN_ENSURE_CAP(&sb, 1024 + exe_name.len + description.len, allocator);

  pg_string_builder_append_string(&sb, exe_name, allocator);

  for (u64 i = 0; i < descs.len; i++) {
    PgCliOptionDescription desc = PG_SLICE_AT(descs, i);

    pg_string_builder_append_string(&sb, PG_S(" "), allocator);

    pg_string_builder_append_string(&sb, desc.required ? PG_S("(") : PG_S("["),
                                    allocator);

    if (!pg_string_is_empty(desc.name_short)) {
      pg_string_builder_append_string(&sb, PG_S("-"), allocator);
      pg_string_builder_append_string(&sb, desc.name_short, allocator);
    }
    if (!pg_string_is_empty(desc.name_long)) {
      if (!pg_string_is_empty(desc.name_short)) {
        pg_string_builder_append_string(&sb, PG_S("|"), allocator);
      }

      pg_string_builder_append_string(&sb, PG_S("--"), allocator);
      pg_string_builder_append_string(&sb, desc.name_long, allocator);
    }

    if (!pg_string_is_empty(desc.value_name)) {
      pg_string_builder_append_string(&sb, PG_S(" "), allocator);
      pg_string_builder_append_string(&sb, desc.value_name, allocator);
    }

    pg_string_builder_append_string(&sb, desc.required ? PG_S(")") : PG_S("]"),
                                    allocator);
  }

  if (!pg_string_is_empty(plain_arguments_description)) {
    pg_string_builder_append_string(&sb, PG_S(" "), allocator);
    pg_string_builder_append_string(&sb, plain_arguments_description,
                                    allocator);
  }

  pg_string_builder_append_string(&sb, PG_S("\n\n"), allocator);
  pg_string_builder_append_string(&sb, description, allocator);
  if (!pg_string_ends_with(description, PG_S("."))) {
    pg_string_builder_append_string(&sb, PG_S("."), allocator);
  }
  pg_string_builder_append_string(&sb, PG_S("\n"), allocator);

  if (!PG_SLICE_IS_EMPTY(descs)) {
    pg_string_builder_append_string(&sb, PG_S("\nOPTIONS:\n"), allocator);

    for (u64 i = 0; i < descs.len; i++) {
      PgCliOptionDescription desc = PG_SLICE_AT(descs, i);

      pg_string_builder_append_string(&sb, PG_S("    "), allocator);
      if (!pg_string_is_empty(desc.name_short)) {
        pg_string_builder_append_string(&sb, PG_S("-"), allocator);
        pg_string_builder_append_string(&sb, desc.name_short, allocator);

        if (!pg_string_is_empty(desc.value_name)) {
          pg_string_builder_append_string(&sb, PG_S(" "), allocator);
          pg_string_builder_append_string(&sb, desc.value_name, allocator);
        }
      }
      if (!pg_string_is_empty(desc.name_long)) {
        if (!pg_string_is_empty(desc.name_short)) {
          pg_string_builder_append_string(&sb, PG_S(", "), allocator);
        }

        pg_string_builder_append_string(&sb, PG_S("--"), allocator);
        pg_string_builder_append_string(&sb, desc.name_long, allocator);

        if (!pg_string_is_empty(desc.value_name)) {
          pg_string_builder_append_string(&sb, PG_S("="), allocator);
          pg_string_builder_append_string(&sb, desc.value_name, allocator);
        }
      }

      if (desc.required) {
        pg_string_builder_append_string(&sb, PG_S("    [required]"), allocator);
      }

      if (!pg_string_is_empty(desc.description)) {
        pg_string_builder_append_string(&sb, PG_S("\n        "), allocator);
        pg_string_builder_append_string(&sb, desc.description, allocator);
        if (!pg_string_ends_with(desc.description, PG_S("."))) {
          pg_string_builder_append_string(&sb, PG_S("."), allocator);
        }
        pg_string_builder_append_string(&sb, PG_S("\n"), allocator);
      }

      pg_string_builder_append_string(&sb, PG_S("\n"), allocator);
    }

    pg_string_builder_append_string(&sb, PG_S("\n"), allocator);
  }

  return PG_DYN_SLICE(PgString, sb);
}

[[maybe_unused]] static void
pg_cli_print_parse_err(PgCliParseResult res_parse) {
  switch (res_parse.err) {
  case 0:
    return;

  case PG_ERR_CLI_MISSING_REQUIRED_OPTION:
    fprintf(stderr, "Missing required CLI option %.*s.",
            (i32)res_parse.err_argv.len, res_parse.err_argv.data);
    break;
  case PG_ERR_CLI_MISSING_REQUIRED_OPTION_VALUE:
    fprintf(stderr, "Missing required value for CLI option %.*s.",
            (i32)res_parse.err_argv.len, res_parse.err_argv.data);
    break;
  case PG_ERR_CLI_UNKNOWN_OPTION:
    fprintf(stderr, "Encountered unknown CLI option %.*s.",
            (i32)res_parse.err_argv.len, res_parse.err_argv.data);
    break;
  case PG_ERR_CLI_FORBIDEN_OPTION_VALUE:
    fprintf(stderr, "Encountered forbidden value for CLI option %.*s.",
            (i32)res_parse.err_argv.len, res_parse.err_argv.data);
    break;
  case PG_ERR_CLI_MALFORMED_OPTION:
    fprintf(stderr, "Malformed CLI option %.*s.\n", (i32)res_parse.err_argv.len,
            res_parse.err_argv.data);
    break;
  default:
    fprintf(stderr, "Unknown CLI options parse error.");
  }
}
#endif
#pragma GCC diagnostic pop
