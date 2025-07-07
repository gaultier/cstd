#ifndef CSTD_LIB_C
#define CSTD_LIB_C

// TODO: Buffered I/O reader/writer.
// TODO: Document all functions.
// TODO: IPv6.
// TODO: *Pool allocator?*
// TODO: Randomize arena guard pages.
// TODO: HTTP compression (gzip, etc)
// TODO: TLS 1.3
// TODO: Pprof memory profiling.
// TODO: [Unix] Human-readable stacktrace.
// TODO: Get PIE offset for better call stack.
// TODO: Test untested functions.
// Low priority:
// TODO: [Unix] CLI argument parser.

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__) ||        \
    defined(__unix__)
#define PG_OS_UNIX
#endif

#if defined(__linux__)
#define PG_OS_LINUX
#endif

#if defined(__FreeBSD__) || defined(__APPLE__) // TODO: More BSDs.
#define PG_OS_BSD
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
#if 0
#include <threads.h>
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
#define PG_PAD(n) uint8_t PG_PRIVATE_NAME(_padding)[n]

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

// TODO: Separate error type.
typedef u32 PgError;
#define PG_ERR_EOF 4095
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

PG_RESULT(u8) Pgu8Result;
PG_RESULT(u16) Pgu16Result;
PG_RESULT(u32) Pgu32Result;
PG_RESULT(u64) Pgu64Result;

PG_RESULT(i8) Pgi8Result;
PG_RESULT(i16) Pgi16Result;
PG_RESULT(i32) Pgi32Result;
PG_RESULT(i64) Pgi64Result;

PG_RESULT(bool) PgBoolResult;
PG_RESULT(void *) PgVoidPtrResult;

PG_OK(u8) Pgu8Ok;
PG_OK(u16) Pgu16Ok;
PG_OK(u32) Pgu32Ok;
PG_OK(u64) Pgu64Ok;

PG_OK(i8) Pgi8Ok;
PG_OK(i16) Pgi16Ok;
PG_OK(i32) Pgi32Ok;
PG_OK(i64) Pgi64Ok;

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

PG_OK(Pgu8Slice) Pgu8SliceOk;
PG_RESULT(Pgu8Slice) Pgu8SliceResult;

#define PG_STATIC_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

// Clamp a value in the range `[min, max]`.
#define PG_CLAMP(min, n, max) ((n) < (min) ? (min) : (n) > (max) ? (max) : n)

#define PG_SUB_SAT(a, b) ((a) > (b) ? ((a) - (b)) : 0)

[[maybe_unused]] [[nodiscard]] static u64 pg_ns_to_ms(u64 ns) {
  return ns / 1'000'000;
}

#define PG_STACKTRACE_MAX 64
#define PG_LOG_STRING_MAX 256
#define PG_LOG_LINE_MAX_LENGTH 8192

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

typedef enum {
  PG_FILE_ACCESS_NONE = 0,
  PG_FILE_ACCESS_READ = 1 << 0,
  PG_FILE_ACCESS_WRITE = 1 << 1,
  PG_FILE_ACCESS_READ_WRITE = 1 << 2,
} PgFileAccess;

static const u64 PG_FILE_ACCESS_ALL =
    PG_FILE_ACCESS_READ | PG_FILE_ACCESS_WRITE | PG_FILE_ACCESS_READ_WRITE;

typedef u32 PgRune;
PG_RESULT(PgRune) PgRuneResult;

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
  bool ok;
} PgStringCut;

typedef struct {
  Pgu8Slice left, right;
  bool ok;
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

typedef enum {
  PG_READER_KIND_BYTES,
  PG_READER_KIND_FILE,
} PgReaderKind;

typedef struct {
  PgReaderKind kind;
  union {
    PgFileDescriptor file;
    Pgu8Slice bytes;
  } u;
} PgReader;

typedef struct PgWriter PgWriter;

typedef enum {
  PG_WRITER_KIND_NONE, // no-op.
  PG_WRITER_KIND_FILE,
  PG_WRITER_KIND_BYTES,
} PgWriterKind;

struct PgWriter {
  PgWriterKind kind;
  union {
    PgFileDescriptor file;
    Pgu8Dyn bytes;
  } u;
};

// Ring buffer.
// Invariants:
// - Empty: `idx_read == idx_write`.
// - Otherwise: empty slot between `idx_read` and `idx_write`.
typedef struct {
  u64 idx_read, idx_write;
  PgString data;
} PgRing;

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
  // TODO: More?
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
PG_OK(Pgu64Range) Pgu64RangeOk;

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
  PgHttpResponse res;
  PgError err;
} PgHttpResponseReadResult;

typedef struct {
  bool done;
  PgHttpRequest res;
  PgError err;
} PgHttpRequestReadResult;

typedef struct {
  PgReader reader;
  PgRing ring;
} PgBufReader;

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
PG_OK(PgElfSectionHeader) PgElfSectionHeaderOk;

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
  u16 section_header_index;
} PgElfHeader;
static_assert(24 == offsetof(PgElfHeader, entrypoint_address));
static_assert(52 == offsetof(PgElfHeader, header_size));
static_assert(64 == sizeof(PgElfHeader));

typedef struct {
  Pgu8Slice bytes;

  PgElfHeader header;
  PgElfProgramHeaderDyn program_headers;
  PgElfSectionHeaderDyn section_headers;

  // Useful section header data.
  PgElfSymbolTableEntryDyn symtab;
  Pgu8Slice strtab;
  Pgu8Slice program_text;
  u32 program_text_idx;
} PgElf;
PG_RESULT(PgElf) PgElfResult;

// ---------------- Functions.

[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdin();
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stdout();
[[maybe_unused]] [[nodiscard]] static PgFileDescriptor pg_os_stderr();

[[maybe_unused]] static Pgu64Result pg_file_read_at(PgFileDescriptor file,
                                                    PgString buf, u64 offset);

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
[[maybe_unused]] static Pgu64Result pg_file_write(PgFileDescriptor file,
                                                  PgString s);

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
  ((x) ? (0)                                                                   \
       : (pg_stacktrace_print(__FILE__, __LINE__, __FUNCTION__),               \
          fflush(stdout), fflush(stderr), __builtin_trap(), 0))

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

#define PG_SLICE_AT_CAST(T, s, idx) (PG_C_ARRAY_AT((T *)(s).data, (s).len, idx))

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

[[maybe_unused]] [[nodiscard]] static bool pg_rune_ascii_is_space(PgRune c) {
  return ' ' == c || '\t' == c || '\n' == c || '\f' == c;
}

[[maybe_unused]] [[nodiscard]] static bool pg_rune_ascii_is_numeric(PgRune c) {
  return ('0' <= c && c <= '9');
}

[[maybe_unused]] [[nodiscard]] static bool
pg_rune_ascii_is_alphanumeric(PgRune c) {
  return pg_rune_ascii_is_numeric(c) || pg_rune_ascii_is_alphabetical(c);
}

[[maybe_unused]] [[nodiscard]] static PgRune
pg_rune_ascii_to_lower_case(PgRune c) {
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

PG_RESULT(PgString) PgStringResult;
PG_OK(PgString) PgStringOk;
PG_SLICE(PgString) PgStringSlice;

PG_RESULT(PgStringSlice) PgStringSliceResult;

#define PG_SLICE_IS_EMPTY(s)                                                   \
  (((s).len == 0) ? true : (PG_ASSERT(nullptr != (s).data), false))

#define PG_S(s) ((PgString){.data = (u8 *)s, .len = sizeof(s) - 1})

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
// to implement that.
[[maybe_unused]] [[nodiscard]] static PgRuneResult
pg_utf8_iterator_peek_next(PgUtf8Iterator it) {
  PgRuneResult res = {0};

  PgString s = PG_SLICE_RANGE_START(it.s, it.idx);
  if (pg_string_is_empty(s)) {
    return res;
  }

  u8 c0 = PG_SLICE_AT(s, 0);
  if (0 == c0) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  // One byte.
  if (0b0000'0000 == (c0 & 0b1000'0000) && c0 != 0) {
    res.res = c0 & 0x7F;
    return res;
  }

  // 2 bytes.
  if (0b1100'0000 == (c0 & 0b1110'0000)) {
    if (s.len < 2) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    u8 c1 = PG_SLICE_AT(s, 1);
    PgRune rune0 = (PgRune)c0 & 0b0001'1111;
    PgRune rune1 = ((PgRune)c1 & 0b0011'1111);
    res.res = (rune0 << 6) | rune1;
    if (res.res < 0x80) { // Overlong.
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    return res;
  }

  // 3 bytes.
  if (0b1110'0000 == (c0 & 0b1111'0000)) {
    if (s.len < 3) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    u8 c1 = PG_SLICE_AT(s, 1);
    u8 c2 = PG_SLICE_AT(s, 2);
    PgRune rune0 = ((PgRune)c0 & 0b0000'1111);
    PgRune rune1 = ((PgRune)c1 & 0b0011'1111);
    PgRune rune2 = ((PgRune)c2 & 0b0011'1111);

    res.res = (rune0 << 12) | (rune1 << 6) | rune2;
    if (res.res < 0x0800) { // Overlong.
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    return res;
  }

  // 4 bytes.
  if (0b1111'0000 == (c0 & 0b1111'1000)) {
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
    res.res = (rune0 << 18) | (rune1 << 12) | (rune2 << 6) | rune3;
    if (res.res < 0x01'00'00) { // Overlong.
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    if (res.res >= 0x10FFFF) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    return res;
  }

  res.err = PG_ERR_INVALID_VALUE;
  return res;
}

[[maybe_unused]] [[nodiscard]] static u64 pg_utf8_rune_bytes_count(PgRune c) {
  if (0 == c) {
    return 0;
  }

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

[[maybe_unused]] [[nodiscard]] static PgRuneResult
pg_utf8_iterator_next(PgUtf8Iterator *it) {
  PgRuneResult res = pg_utf8_iterator_peek_next(*it);
  it->idx += pg_utf8_rune_bytes_count(res.res);

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_utf8_count_runes(PgString s) {
  Pgu64Result res = {0};

  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  u64 len = 0;
  for (;; len++) {
    PgRuneResult res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err) {
      res.err = res_rune.err;
      return res;
    }
    if (0 == res_rune.res) {
      break;
    }
  }

  res.res = len;
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool
pg_string_is_ascii_alphabetical(PgString s) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  PgRuneResult res_rune = {0};

  for (res_rune = pg_utf8_iterator_next(&it); !res_rune.err && res_rune.res;
       res_rune = pg_utf8_iterator_next(&it)) {
    if (!pg_rune_ascii_is_alphabetical(res_rune.res)) {
      return false;
    }
  }
  PG_ASSERT(0 == res_rune.err);
  return true;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_trim_left(PgString s,
                                                                   PgRune c) {
  if (pg_string_is_empty(s)) {
    return s;
  }

  PgString res = s;

  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  PgRuneResult res_rune = {0};

  u64 bytes_idx = 0;
  for (res_rune = pg_utf8_iterator_next(&it); !res_rune.err && res_rune.res;
       res_rune = pg_utf8_iterator_next(&it)) {
    if (res_rune.res != c) {
      break;
    }
    bytes_idx = it.idx;
  }

  res.data += bytes_idx;
  res.len -= bytes_idx;

  return res;
}

[[nodiscard]] static PgRune pg_string_last(PgString s) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  PgRune res = 0;
  for (;;) {
    PgRuneResult res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || 0 == res_rune.res) {
      return res;
    }
    res = res_rune.res;
  }
  PG_ASSERT(0);
}
[[maybe_unused]] [[nodiscard]] static PgRune pg_string_first(PgString s) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  PgRuneResult res_rune = pg_utf8_iterator_next(&it);
  return res_rune.res;
}

[[maybe_unused]] [[nodiscard]] static PgString pg_string_trim_right(PgString s,
                                                                    PgRune c) {
  while (!pg_string_is_empty(s)) {
    PgRune last = pg_string_last(s);
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
  res = pg_string_trim_left(res, ' ');
  res = pg_string_trim_left(res, '\n');
  res = pg_string_trim_left(res, '\t');
  res = pg_string_trim_left(res, '\r');
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_trim_space_right(PgString s) {
  PgString res = s;
  res = pg_string_trim_right(res, ' ');
  res = pg_string_trim_right(res, '\n');
  res = pg_string_trim_right(res, '\t');
  res = pg_string_trim_right(res, '\r');
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgString
pg_string_trim_space(PgString s) {
  PgString res = s;
  res = pg_string_trim(res, ' ');
  res = pg_string_trim(res, '\n');
  res = pg_string_trim(res, '\t');
  res = pg_string_trim(res, '\r');
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
    PgRuneResult res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || !res_rune.res) {
      return -1;
    }

    if (needle == res_rune.res) {
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
    PgRuneResult res_rune = pg_utf8_iterator_next(&it);
    if (res_rune.err || !res_rune.res) {
      return res;
    }

    if (needle == res_rune.res) {
      res.left = PG_SLICE_RANGE(s, 0, idx);
      res.right = PG_SLICE_RANGE_START(s, it.idx);
      res.ok = true;
      return res;
    }
  }

  PG_ASSERT(0);
}

[[nodiscard]] static i64 pg_string_last_index_of_rune(PgString haystack,
                                                      PgRune needle) {
  while (!pg_string_is_empty(haystack)) {
    PgRune last = pg_string_last(haystack);
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
static Pgu64Ok pg_bytes_index_of_byte(Pgu8Slice haystack, u8 needle) {
  Pgu64Ok res = {0};

  for (u64 i = 0; i < haystack.len; i++) {
    u8 it = PG_SLICE_AT(haystack, i);
    if (needle == it) {
      res.res = i;
      res.ok = true;
      return res;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]]
static Pgu64Ok pg_bytes_last_index_of_byte(Pgu8Slice haystack, u8 needle) {
  Pgu64Ok res = {0};

  for (i64 i = (i64)haystack.len - 1; i >= 0; i--) {
    u8 it = PG_SLICE_AT(haystack, i);
    if (needle == it) {
      res.res = (u64)i;
      res.ok = true;
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
static Pgu64Ok pg_bytes_index_of_bytes(Pgu8Slice haystack, Pgu8Slice needle) {
  Pgu64Ok res = {0};

  if (PG_SLICE_IS_EMPTY(needle)) {
    return res;
  }

  if (needle.len > haystack.len) {
    return res;
  }

  for (u64 i = 0; i < haystack.len; i++) {
    if (pg_bytes_starts_with(PG_SLICE_RANGE_START(haystack, (u64)i), needle)) {
      res.res = (u64)i;
      res.ok = true;
      return res;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]]
static Pgu64Ok pg_bytes_last_index_of_bytes(Pgu8Slice haystack,
                                            Pgu8Slice needle) {
  Pgu64Ok res = {0};

  if (PG_SLICE_IS_EMPTY(needle)) {
    return res;
  }

  if (needle.len > haystack.len) {
    return res;
  }

  for (i64 i = (i64)haystack.len - 1; i >= 0; i--) {
    if (pg_bytes_ends_with(PG_SLICE_RANGE_START(haystack, (u64)i), needle)) {
      res.res = (u64)i;
      res.ok = true;
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

  res.ok = true;
  res.left.data = haystack.data;
  res.left.len = (u64)(ret - haystack.data);
  res.right.data = ret + 1;
  res.right.len = haystack.len - res.left.len - 1;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgBytesCut
pg_bytes_cut_bytes_excl(Pgu8Slice haystack, Pgu8Slice needle) {
  PgBytesCut res = {0};

  Pgu64Ok search = pg_bytes_index_of_bytes(haystack, needle);
  if (!search.ok) {
    return res;
  }

  res.ok = true;
  res.left = PG_SLICE_RANGE(haystack, 0, search.res);
  res.right = PG_SLICE_RANGE_START(haystack, search.res);
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
  res.ok = true;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_string_split_next(PgSplitIterator *it) {
  if (PG_SLICE_IS_EMPTY(it->s)) {
    return (PgStringOk){0};
  }

  for (u64 _i = 0; _i < it->s.len; _i++) {
    i64 idx = pg_string_index_of_string(it->s, it->sep);
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
  PgRuneResult res_rune = {0};
  for (res_rune = pg_utf8_iterator_next(&it);
       0 == res_rune.err && 0 != res_rune.res;
       res_rune = pg_utf8_iterator_next(&it)) {
    PgRune needle = res_rune.res;
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
  PgRuneResult res_rune = {0};
  for (res_rune = pg_utf8_iterator_next(&it);
       0 == res_rune.err && 0 != res_rune.res;
       res_rune = pg_utf8_iterator_next(&it)) {
    PgRune needle = res_rune.res;
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
  PgRuneResult res_rune = {0};
  for (res_rune = pg_utf8_iterator_next(&it);
       0 == res_rune.err && 0 != res_rune.res;
       res_rune = pg_utf8_iterator_next(&it)) {
    PgRune needle = res_rune.res;

    i64 idx = pg_string_index_of_rune(haystack, needle);
    if (-1 != idx) {
      return idx;
    }
  }
  return -1;
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

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_string_consume_rune(PgString haystack, PgRune needle) {
  PgStringOk res = {0};

  PgUtf8Iterator it = pg_make_utf8_iterator(haystack);
  PgRuneResult res_rune = pg_utf8_iterator_next(&it);

  if (needle != res_rune.res) {
    return res;
  }

  res.res.data = haystack.data + it.idx;
  res.res.len = haystack.len - it.idx;
  res.ok = true;
  return res;
}

[[maybe_unused]] [[nodiscard]] static PgStringOk
pg_string_consume_string(PgString haystack, PgString needle) {
  PgStringOk res = {0};
  res.res = haystack;

  for (u64 i = 0; i < needle.len; i++) {
    res = pg_string_consume_rune(res.res, PG_SLICE_AT(needle, i));
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

[[maybe_unused]] [[nodiscard]] static PgParseNumberResult
pg_string_parse_u64(PgString s) {
  PgParseNumberResult res = {0};
  res.remaining = s;

  // Forbid leading zero(es) if there is more than one digit.
  if (pg_string_starts_with(s, PG_S("0")) && s.len >= 2 &&
      pg_rune_ascii_is_numeric(PG_SLICE_AT(s, 1))) {
    return res;
  }

  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  u64 last_idx = 0;
  for (;;) {
    PgRuneResult res_rune = pg_utf8_iterator_next(&it);
    if (0 != res_rune.err || 0 == res_rune.res) {
      break;
    }

    PgRune c = res_rune.res;
    if (!pg_rune_ascii_is_numeric(c)) { // End of numbers sequence.
      break;
    }

    res.n *= 10;
    res.n += (u8)c - '0';
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

[[maybe_unused]]
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
  // TODO: Consider always allocating one more byte to make the conversion to c
  // string non-allocating.
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
  memcpy(res, s.data, s.len);

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
  ((new_cap > 0) && (dyn)->cap < (new_cap))                                    \
      ? PG_DYN_GROW(dyn, sizeof(*(dyn)->data),                                 \
                    _Alignof(typeof((dyn)->data[0])), new_cap, allocator),     \
      PG_ASSERT((dyn)->cap >= (new_cap)), PG_ASSERT((dyn)->data), 0 : 0

#define PG_DYN_SPACE(T, dyn)                                                   \
  ((T){.data = (dyn)->data + (dyn)->len, .len = (dyn)->cap - (dyn)->len})

#define PG_DYN_PUSH(s, allocator)                                              \
  (PG_DYN_ENSURE_CAP(s, (s)->len + 1, allocator),                              \
   (s)->len > 0 ? PG_ASSERT((s)->data) : 0, (s)->data + (s)->len++)

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

[[maybe_unused]] [[nodiscard]] static Pgu8Dyn
pg_string_builder_make(u64 cap, PgAllocator *allocator) {
  Pgu8Dyn res = {0};
  PG_DYN_ENSURE_CAP(&res, cap, allocator);
  PG_ASSERT(res.data);
  return res;
}

[[maybe_unused]] static PgRing pg_ring_make(u64 cap, PgAllocator *allocator) {
  return (PgRing){.data = pg_string_make(cap + 1, allocator)};
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_is_empty(PgRing rg) {
  return rg.idx_write == rg.idx_read;
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_can_write(PgRing rg) {
  if (rg.idx_write == rg.idx_read) { // Empty.
    return true;
  } else if (rg.idx_write < rg.idx_read) {
    return (1 /* Empty slot */ + rg.idx_write) < rg.idx_read;
  } else if (rg.idx_write > rg.idx_read) {
    return (1 + rg.idx_write == rg.data.len) ? rg.idx_read != 0 : true;
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_can_read(PgRing rg) {
  if (rg.idx_write == rg.idx_read) { // Empty.
    return false;
  } else {
    return true;
  }
  PG_ASSERT(0);
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_try_write_byte(PgRing *rg,
                                                                  u8 byte) {
  PG_ASSERT(rg);
  PG_ASSERT(rg->idx_read <= rg->data.len);
  PG_ASSERT(rg->idx_write <= rg->data.len);
  PG_ASSERT(rg->data.len);

  if (!pg_ring_can_write(*rg)) {
    return false;
  }

  PG_SLICE_AT(rg->data, rg->idx_write) = byte;
  rg->idx_write = (1 + rg->idx_write) % rg->data.len;

  PG_ASSERT(rg->idx_write < rg->data.len);
  PG_ASSERT(!pg_ring_is_empty(*rg));
  PG_ASSERT(pg_ring_can_read(*rg));

  return true;
}

[[maybe_unused]] [[nodiscard]] static Pgu8Ok pg_ring_try_read_byte(PgRing *rg) {
  PG_ASSERT(rg);
  PG_ASSERT(rg->idx_read <= rg->data.len);
  PG_ASSERT(rg->idx_write <= rg->data.len);
  PG_ASSERT(rg->data.len);

  Pgu8Ok res = {0};

  if (!pg_ring_can_read(*rg)) {
    return res;
  }
  PG_ASSERT(!pg_ring_is_empty(*rg));

  res.ok = true;
  res.res = PG_SLICE_AT(rg->data, rg->idx_read);
  rg->idx_read = (1 + rg->idx_read) % rg->data.len;

  PG_ASSERT(rg->idx_read < rg->data.len);
  PG_ASSERT(pg_ring_can_write(*rg));

  return res;
}

[[maybe_unused]] [[nodiscard]] static u64 pg_ring_write_space(PgRing rg) {
  u64 res = 0;
  while (pg_ring_can_write(rg)) {
    PG_ASSERT(pg_ring_try_write_byte(&rg, 0));
    res += 1;
  }

  PG_ASSERT(res < rg.data.len);

  return res;
}

[[maybe_unused]] [[nodiscard]] static u64
pg_ring_try_write_bytes(PgRing *rg, Pgu8Slice src) {
  PG_ASSERT(nullptr != rg->data.data);
  PG_ASSERT(rg->idx_read <= rg->data.len);
  PG_ASSERT(rg->idx_write <= rg->data.len);
  PG_ASSERT(rg->data.len > 0);

  for (u64 i = 0; i < src.len; i++) {
    if (!pg_ring_try_write_byte(rg, PG_SLICE_AT(src, i))) {
      return i;
    }
  }

  return src.len;
}

[[maybe_unused]] [[nodiscard]] static u64
pg_ring_try_read_bytes(PgRing *rg, Pgu8Slice dst) {
  PG_ASSERT(nullptr != rg->data.data);
  PG_ASSERT(rg->idx_read <= rg->data.len);
  PG_ASSERT(rg->idx_write <= rg->data.len);
  PG_ASSERT(rg->data.len > 0);

  for (u64 i = 0; i < dst.len; i++) {
    Pgu8Ok byte_opt = pg_ring_try_read_byte(rg);
    if (!byte_opt.ok) {
      return i;
    }
    PG_SLICE_AT(dst, i) = byte_opt.res;
  }

  return dst.len;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Ok
pg_ring_index_of_bytes(PgRing rg, Pgu8Slice needle) {
(void)rg;(void)needle;
  Pgu64Ok res = {0};
  // TODO
  return res;
}

[[maybe_unused]] [[nodiscard]] static bool pg_ring_try_read_u32(PgRing *rg,
                                                                u32 *val) {
  PgString s = {.len = sizeof(*val), .data = (u8 *)val};
  return pg_ring_try_read_bytes(rg, s);
}

[[maybe_unused]] [[nodiscard]] static PgError pg_writer_close(PgWriter *w) {
  PG_ASSERT(w);

  switch (w->kind) {
  case PG_WRITER_KIND_FILE:
    return pg_file_close(w->u.file);
  case PG_WRITER_KIND_NONE:
  case PG_WRITER_KIND_BYTES:
    return 0;
  default:
    PG_ASSERT(0);
  }
}

[[nodiscard]] [[maybe_unused]] static PgWriter
pg_writer_make_string_builder(u64 cap, PgAllocator *allocator) {
  PgWriter w = {0};
  w.kind = PG_WRITER_KIND_BYTES;
  PG_DYN_ENSURE_CAP(&w.u.bytes, cap, allocator);
  return w;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_write(PgWriter *w, Pgu8Slice src, PgAllocator *allocator) {
  Pgu64Result res = {0};

  switch (w->kind) {
  case PG_WRITER_KIND_NONE:
    return res;
  case PG_WRITER_KIND_FILE:
    return pg_file_write(w->u.file, src);
  case PG_WRITER_KIND_BYTES: {
    PG_DYN_APPEND_SLICE(&w->u.bytes, src, allocator);
    res.res = src.len;
    return res;
  }
  default:
    PG_ASSERT(0);
  }
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_write_u8(PgWriter *w, u8 c, PgAllocator *allocator) {
  Pgu8Slice src = {.data = &c, .len = 1};
  Pgu64Result res = pg_writer_write(w, src, allocator);
  if (res.err) {
    return res.err;
  }

  return res.res == 1 ? 0 : PG_ERR_IO;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_writer_write_string_full(PgWriter *w, PgString s, PgAllocator *allocator) {
  PgString remaining = s;
  for (u64 _i = 0; _i < s.len; _i++) {
    if (pg_string_is_empty(remaining)) {
      break;
    }

    Pgu64Result res = pg_writer_write(w, remaining, allocator);
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

[[nodiscard]] [[maybe_unused]] static Pgu64Result
pg_reader_read(PgReader *r, Pgu8Slice dst) {
  PG_ASSERT(dst.data);

  switch (r->kind) {
  case PG_READER_KIND_BYTES: {
    Pgu64Result res = {0};

    u64 n = PG_MIN(dst.len, r->u.bytes.len);
    if (n > 0) {
      PG_ASSERT(dst.data);
      PG_ASSERT(r->u.bytes.data);
      memcpy(dst.data, r->u.bytes.data, n);
    }
    res.res = n;

    r->u.bytes = PG_SLICE_RANGE_START(r->u.bytes, n);

    return res;
  }
  case PG_READER_KIND_FILE:
    return pg_file_read(r->u.file, dst);
  default:
    PG_ASSERT(0);
  }
}

[[nodiscard]] [[maybe_unused]] static PgStringResult
pg_reader_read_until_full_or_eof(PgReader *r, Pgu8Slice buf) {
  PgStringResult res = {.res.data = buf.data};

  do {
    u64 idx = res.res.len;
    PG_ASSERT(idx < buf.len);

    Pgu8Slice dst = PG_SLICE_RANGE_START(buf, idx);
    Pgu64Result read_res = pg_reader_read(r, dst);

    if (read_res.err) {
      res.err = read_res.err;
      return res;
    }

    res.res.len += read_res.res;

    if (0 == read_res.res) {
      return res;
    }
  } while (1); // FIXME: Bounded;
}

[[nodiscard]] [[maybe_unused]] static Pgu64Result
pg_writer_write_from_reader(PgWriter *w, PgReader *r, PgAllocator *allocator) {
  Pgu64Result res = {0};

  // TODO: Get a hint from the reader?
  u8 tmp[4096] = {0};
  PgString dst = {.data = tmp, .len = PG_STATIC_ARRAY_LEN(tmp)};

  res = pg_reader_read(r, dst);
  if (res.err) {
    return res;
  }
  dst.len = res.res;

  res = pg_writer_write(w, dst, allocator);
  if (res.err) {
    return res;
  }

  // WARN: In that case, there is data loss.
  // Not all readers support putting back data that could not be written out.
  if (res.res != dst.len) {
    res.err = PG_ERR_IO;
    return res;
  }

  return res;
}

[[nodiscard]] [[maybe_unused]] static PgError
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

  return pg_writer_write_string_full(w, s, allocator);
}

[[nodiscard]] [[maybe_unused]] static PgError
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

  return pg_writer_write_string_full(w, s, allocator);
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
    PgRuneResult res_rune = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res_rune.err);
    PgRune rune = res_rune.res;
    if (0 == rune) {
      break;
    }

    if (-1 != pg_string_index_of_rune(runes_to_escape, rune)) {
      pg_string_builder_append_rune(sb, rune_escape, allocator);
    }
    pg_string_builder_append_rune(sb, rune, allocator);
  }
}

[[maybe_unused]] static void pg_string_builder_append_string_escaped(
    Pgu8Dyn *sb, PgString s, PgRune rune_to_escape, PgRune rune_escape,
    PgAllocator *allocator) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);
  for (;;) {
    PgRuneResult res_rune = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res_rune.err);
    PgRune rune = res_rune.res;
    if (0 == rune) {
      break;
    }

    if (rune_to_escape == rune) {
      pg_string_builder_append_rune(sb, rune_escape, allocator);
    }
    pg_string_builder_append_rune(sb, rune, allocator);
  }
}

// Should `pg_string_builder` take `PgRune[]` as argument?
// I.e. UTF32.
[[maybe_unused]] static void
pg_string_builder_append_js_string_escaped(Pgu8Dyn *sb, PgString s,
                                           PgAllocator *allocator) {
  PgUtf8Iterator it = pg_make_utf8_iterator(s);

  for (u64 i = 0; i < s.len; i++) {
    PgRuneResult res_rune = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res_rune.err);
    PgRune rune = res_rune.res;
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
        *PG_DYN_PUSH(sb, allocator) = '\\';
        *PG_DYN_PUSH(sb, allocator) = 'u';
        *PG_DYN_PUSH(sb, allocator) = '{';
        pg_string_builder_append_u64_hex(sb, rune, allocator);
        *PG_DYN_PUSH(sb, allocator) = '}';
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

  *PG_DYN_PUSH(dyn, allocator) = n;
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
    if (escape != pg_string_first(remaining)) {
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
  PgRuneResult res_rune = {0};
  for (res_rune = pg_utf8_iterator_next(&it);
       0 == res_rune.err && 0 != res_rune.res;
       res_rune = pg_utf8_iterator_next(&it)) {
    PgRune needle = res_rune.res;
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

[[nodiscard]] [[maybe_unused]]
static PgAdjacencyMatrixNeighborIterator
pg_adjacency_matrix_make_neighbor_iterator(PgAdjacencyMatrix matrix, u64 node) {
  PgAdjacencyMatrixNeighborIterator it = {0};
  it.matrix = matrix;
  it.row = node;
  return it;
}

[[nodiscard]] [[maybe_unused]]
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

[[nodiscard]] [[maybe_unused]]
static bool pg_adjacency_matrix_is_empty(PgAdjacencyMatrix matrix) {
  bool set = false;
  for (u64 i = 0; i < matrix.bitfield.len; i++) {
    set |= PG_SLICE_AT(matrix.bitfield, i);
  }
  return set == 0;
}

[[nodiscard]] [[maybe_unused]]
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

[[nodiscard]] [[maybe_unused]] static PgRng pg_rand_make() {
  PgRng rng = {0};
  // Rely on ASLR.
  Pgu64Result now = pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC);
  PG_ASSERT(0 == now.err);
  rng.state = (u64)(&pg_rand_make) ^ now.res;

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

[[maybe_unused]] [[nodiscard]] static Pgu64RangeOk
pg_u64_range_search(Pgu64Slice haystack, u64 needle) {
  Pgu64RangeOk res = {0};

  if (0 == haystack.len) {
    return res;
  }

  for (u64 i = 1; i < haystack.len; i++) {
    u64 elem = PG_SLICE_AT(haystack, i);
    if (needle < elem) {
      res.ok = true;
      res.res.idx = i - 1;
      res.res.start_incl = PG_SLICE_AT(haystack, i - 1);
      res.res.end_excl = elem;

      PG_ASSERT(res.res.start_incl <= needle);
      PG_ASSERT(needle < res.res.end_excl);
      return res;
    }
  }

  return res;
}

#ifdef PG_OS_UNIX
#define PG_PATH_SEPARATOR '/'
#define PG_PATH_SEPARATOR_S "/"
#else
#define PG_PATH_SEPARATOR '\\'
#define PG_PATH_SEPARATOR_S "\\"
#endif

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

  memcpy(res.data, left.data, left.len);
  memcpy(res.data + left.len, right.data, right.len);

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgWriter
pg_writer_make_from_file_descriptor(PgFileDescriptor file) {
  PgWriter w = {0};
  w.kind = PG_WRITER_KIND_FILE;
  w.u.file = file;

  return w;
}

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
    Pgu64Result res_read = pg_file_read(file, space);
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

[[maybe_unused]] static PgStringResult
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

    u64 read_n = res_read.res;
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

  res.res = PG_DYN_SLICE(PgString, sb);
  return res;
}

[[nodiscard]] [[maybe_unused]] static PgReader
pg_reader_make_from_file_descriptor(PgFileDescriptor file) {
  PgReader r = {0};
  r.kind = PG_READER_KIND_FILE;
  r.u.file = file;
  return r;
}

[[nodiscard]] [[maybe_unused]] static PgReader
pg_reader_make_from_bytes(Pgu8Slice bytes) {
  PgReader r = {0};
  r.kind = PG_READER_KIND_BYTES;
  r.u.bytes = bytes;
  return r;
}

[[nodiscard]] [[maybe_unused]] static PgError
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
      pg_file_open(path, PG_FILE_ACCESS_READ, 0600, false, allocator);
  if (res_file.err) {
    res.err = res_file.err;
    return res;
  }

  PgFileDescriptor file = res_file.res;

  Pgu64Result res_size = pg_file_size(file);
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

[[maybe_unused]] static PgError pg_file_write_full(PgString path,
                                                   PgString content, u64 mode,
                                                   PgAllocator *allocator) {
  PgError err = 0;

  PgFileDescriptorResult res_file =
      pg_file_open(path, PG_FILE_ACCESS_WRITE, mode, true, allocator);
  if (res_file.err) {
    err = res_file.err;
    return err;
  }

  PgFileDescriptor file = res_file.res;

  err = pg_file_write_full_with_descriptor(file, content);

  (void)pg_file_close(file);

  return err;
}

[[nodiscard]] [[maybe_unused]]
static PgProcessResult pg_process_spawn(PgString path, PgStringSlice args,
                                        PgProcessSpawnOptions options,
                                        PgAllocator *allocator);

[[nodiscard]] [[maybe_unused]]
static PgProcessExitResult pg_process_wait(PgProcess process,
                                           PgAllocator *allocator);

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_send_to_socket(PgFileDescriptor dst, PgFileDescriptor src);

// This works from any kind of file descriptor to any kind of file descriptor.
// But this may be slower than OS-specific syscalls.
[[nodiscard]] [[maybe_unused]] static PgError
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
    if (0 == res_read.res) {
      return (PgError)0;
    }

    if (offset > res_read.res) {
      // Keep reading and do not write yet.

      offset -= res_read.res;
      continue;
    }

    PG_ASSERT(offset <= res_read.res);

    Pgu8Slice write_slice = {.data = read_slice.data + offset,
                             .len = res_read.res - offset};
    PgError err = pg_file_write_full_with_descriptor(dst, write_slice);
    if (err) {
      return err;
    }

    if (offset > 0) {
      offset = 0;
    }
  }
}

[[maybe_unused]] [[nodiscard]] static PgFileDescriptorResult
pg_net_create_tcp_socket();
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_socket_close(PgFileDescriptor sock);
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_set_nodelay(PgFileDescriptor sock, bool enabled);
[[maybe_unused]] [[nodiscard]] static PgError
pg_net_connect_ipv4(PgFileDescriptor sock, PgIpv4Address address);

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

[[maybe_unused]] [[nodiscard]] static PgIpv4AddressAcceptResult
pg_net_tcp_accept(PgFileDescriptor sock);

[[maybe_unused]] [[nodiscard]] static PgError
pg_net_get_socket_error(PgFileDescriptor socket);

[[maybe_unused]] [[nodiscard]] static Pgu32Result pg_process_dup();

[[nodiscard]] static PgError pg_process_avoid_child_zombies();

#ifdef PG_OS_UNIX
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define PG_PIPE_READ 0
#define PG_PIPE_WRITE 1

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

  res.res = (u32)pid;
  return res;
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

  res.res.fd = sock_fd;

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
      ret = connect(res_create_socket.res.fd, rp->ai_addr, rp->ai_addrlen);
    } while (-1 == ret && EINTR == errno);

    if (-1 == ret) {
      if (EINPROGRESS != errno) {
        (void)pg_net_socket_close(res_create_socket.res);
        continue;
      }
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
    res.res = (u64)n;
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
    res.res = (u64)n;
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

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_rewind_start(PgFileDescriptor f) {
  off_t ret = lseek(f.fd, 0, SEEK_SET);
  if (-1 == ret) {
    return (PgError)errno;
  }
  return 0;
}

// TODO: Review in the context of multiple threads spawning and reaping
// processes.
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
[[nodiscard]] [[maybe_unused]]
static PgProcessCaptureStdResult
pg_process_capture_std_io(PgProcess process, PgAllocator *allocator) {
  PgProcessCaptureStdResult res = {0};
  Pgu8Dyn stdout_sb = {0};
  Pgu8Dyn stderr_sb = {0};

  if (process.stdout_pipe.fd) {
    stdout_sb = pg_string_builder_make(4096, allocator);
  }

  if (process.stderr_pipe.fd) {
    stderr_sb = pg_string_builder_make(4096, allocator);
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
  if ((void *)-1 == res.res) {
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

    // `ip` points to the return instruction in the caller, once this call
    // is done. But: We want the location of the call i.e. the `call xxx`
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

[[maybe_unused]] static Pgu64Result pg_file_read(PgFileDescriptor file,
                                                 PgString buf) {
  Pgu64Result res = {0};

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

[[maybe_unused]] static Pgu64Result pg_file_write(PgFileDescriptor file,
                                                  PgString s) {
  Pgu64Result res = {0};

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

[[maybe_unused]] static Pgu64Result pg_file_read_at(PgFileDescriptor file,
                                                    PgString buf, u64 offset) {
  Pgu64Result res = {0};

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

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_writer_win32_write(PgWriter *w, Pgu8Slice src) {
  PG_ASSERT(nullptr != w);
  if (PG_SLICE_IS_EMPTY(src)) {
    return (Pgu64Result){0};
  }

  PgFileDescriptor file = w->ctx;
  HANDLE handle = file.ptr;
  DWORD n = 0;
  bool ok = WriteFile(handle, src.data, (DWORD)src.len, &n, nullptr);
  Pgu64Result res = {0};
  if (!ok) {
    res.err = (PgError)pg_os_get_last_error();
  } else {
    res.res = (u64)n;
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

[[nodiscard]] static Pgu64Result pg_file_size(PgFileDescriptor file) {
  Pgu64Result res = {0};

  LARGE_INTEGER size;
  if (0 == GetFileSizeEx(file.ptr, &size)) {
    res.err = (PgError)pg_os_get_last_error();
    return res;
  }

  res.res = (u64)size.QuadPart;
  return res;
}

[[nodiscard]] static Pgu64Result pg_file_read(PgFileDescriptor file,
                                              PgString dst) {
  Pgu64Result res = {0};

  if (0 == ReadFile(file.ptr, dst.data, (DWORD)dst.len, (LPDWORD)&res.res,
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
      WriteFile(file.ptr, s.data, (DWORD)s.len, (LPDWORD)&res.res, nullptr)) {
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
    PgStringOk consume = pg_string_consume_rune(remaining, '.');
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
    PgStringOk consume = pg_string_consume_rune(remaining, ' ');
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
static void pg_http_push_header(PgStringKeyValueDyn *headers, PgString key,
                                PgString value, PgAllocator *allocator) {
  *PG_DYN_PUSH(headers, allocator) =
      (PgStringKeyValue){.key = key, .value = value};
}

[[nodiscard]] [[maybe_unused]] static PgError
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

  err = pg_writer_write_string_full(w, pg_http_method_to_string(req.method),
                                    allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_string_full(w, PG_S(" /"), allocator);
  if (err) {
    return err;
  }

  for (u64 i = 0; i < req.url.path_components.len; i++) {
    PgString path_component = PG_SLICE_AT(req.url.path_components, i);
    err = pg_writer_write_string_full(w, path_component, allocator);
    if (err) {
      return err;
    }

    if (i < req.url.path_components.len - 1) {
      err = pg_writer_write_string_full(w, PG_S("/"), allocator);
      if (err) {
        return err;
      }
    }
  }

  if (req.url.query_parameters.len > 0) {
    err = pg_writer_write_string_full(w, PG_S("?"), allocator);
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
        err = pg_writer_write_string_full(w, PG_S("&"), allocator);
        if (err) {
          return err;
        }
      }
    }
  }

  err = pg_writer_write_string_full(w, PG_S(" HTTP/1.1\r\n"), allocator);
  if (err) {
    return err;
  }
  return 0;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_http_response_write_status_line(PgWriter *w, PgHttpResponse res,
                                   PgAllocator *allocator) {
  PgError err = 0;

  err = pg_writer_write_string_full(w, PG_S("HTTP/"), allocator);
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

  err = pg_writer_write_string_full(w, PG_S("\r\n"), allocator);
  if (err) {
    return err;
  }

  return 0;
}

[[nodiscard]] [[maybe_unused]] static PgError
pg_http_write_header(PgWriter *w, PgStringKeyValue header,
                     PgAllocator *allocator) {
  PgError err = 0;

  err = pg_writer_write_string_full(w, header.key, allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_string_full(w, PG_S(": "), allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_string_full(w, header.value, allocator);
  if (err) {
    return err;
  }

  err = pg_writer_write_string_full(w, PG_S("\r\n"), allocator);
  if (err) {
    return err;
  }

  return 0;
}

// NOTE: Only sanitation for including the string inside an HTML tag e.g.:
// `<div>...ESCAPED_STRING..</div>`.
// To include the string inside other context (e.g. JS, CSS, HTML
// attributes, etc), more advance sanitation is required.
[[maybe_unused]] [[nodiscard]] static PgString
pg_html_sanitize(PgString s, PgAllocator *allocator) {
  Pgu8Dyn res = {0};
  PG_DYN_ENSURE_CAP(&res, s.len, allocator);
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
      *PG_DYN_PUSH(&res, allocator) = c;
    }
  }

  return PG_DYN_SLICE(PgString, res);
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

[[maybe_unused]] [[nodiscard]] static PgStringDynKeyValueResult
pg_url_parse_query_parameters(PgString s, PgAllocator *allocator) {
  PgStringDynKeyValueResult res = {0};

  PgString remaining = s;
  {
    PgStringOk res_consume_question = pg_string_consume_rune(s, '?');
    if (!res_consume_question.ok) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
    remaining = res_consume_question.res;
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
      *PG_DYN_PUSH(&res.res, allocator) =
          (PgStringKeyValue){.key = k, .value = v};
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
        pg_string_consume_until_rune_incl(remaining, '@');
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
      pg_string_consume_until_rune_incl(remaining, ':');
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
    res.res.path_components = res_path_components.res;
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

  PgStringCut cut = pg_string_cut_rune(status_line, ' ');
  if (!cut.ok) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  // Method.
  {
    PgString method = pg_string_trim_space(cut.left);
    if (pg_string_eq(method, PG_S("OPTIONS"))) {
      res.res.method = PG_HTTP_METHOD_OPTIONS;
    } else if (pg_string_eq(method, PG_S("GET"))) {
      res.res.method = PG_HTTP_METHOD_GET;
    } else if (pg_string_eq(method, PG_S("HEAD"))) {
      res.res.method = PG_HTTP_METHOD_HEAD;
    } else if (pg_string_eq(method, PG_S("POST"))) {
      res.res.method = PG_HTTP_METHOD_POST;
    } else if (pg_string_eq(method, PG_S("PUT"))) {
      res.res.method = PG_HTTP_METHOD_PUT;
    } else if (pg_string_eq(method, PG_S("DELETE"))) {
      res.res.method = PG_HTTP_METHOD_DELETE;
    } else if (pg_string_eq(method, PG_S("TRACE"))) {
      res.res.method = PG_HTTP_METHOD_TRACE;
    } else if (pg_string_eq(method, PG_S("CONNECT"))) {
      res.res.method = PG_HTTP_METHOD_CONNECT;
    } else {
      res.res.method = PG_HTTP_METHOD_EXTENSION;
    }
  }

  // Path.
  cut = pg_string_cut_rune(cut.right, ' ');
  {
    PgString path = pg_string_trim_space(cut.left);
    PgUrlResult res_url = pg_url_parse_after_authority(path, allocator);
    if (res_url.err) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }

    res.res.url = res_url.res;
  }

  PgString remaining = pg_string_trim_space(cut.right);
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
    PgStringOk consume = pg_string_consume_rune(remaining, '.');
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

[[maybe_unused]] [[nodiscard]] static PgStringKeyValueResult
pg_http_parse_header(PgString s) {
  PgStringKeyValueResult res = {0};

  PgStringCut cut = pg_string_cut_rune(s, ':');
  if (!cut.ok) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.res.key = pg_string_trim_space(cut.left);
  if (pg_string_is_empty(res.res.key)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.res.value = pg_string_trim_space(cut.right);
  if (pg_string_is_empty(res.res.value)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  return res;
}

#if 0
[[maybe_unused]] [[nodiscard]] static PgHttpResponseReadResult
pg_http_read_response(PgRing *rg, PgAllocator *allocator) {
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

  for (;;) {
    res_split = pg_string_split_next(&it);
    if (!res_split.ok) {
      break;
    }
    PgStringKeyValueResult res_kv = pg_http_parse_header(res_split.res);
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
#endif

[[maybe_unused]] [[nodiscard]] static PgBufReader
pg_buf_reader_make(PgReader reader, u64 buf_size, PgAllocator *allocator) {
  PgBufReader res = {0};
  res.reader = reader;
  res.ring = pg_ring_make(buf_size, allocator);

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_buf_reader_read(PgBufReader *r, Pgu8Slice dst) {
  PG_ASSERT(r);

  Pgu64Result res = {0};

  u64 n_read = pg_ring_try_read_bytes(&r->ring, dst);
  if (n_read > 0) {
    res.res = n_read;
    return res;
  }

  // Time to call the underlying reader.

  u8 tmp[4096] = {0};
  Pgu8Slice tmp_slice = {
      .data = tmp,
      .len = PG_MIN(PG_STATIC_ARRAY_LEN(tmp), pg_ring_write_space(r->ring)),
  };
  Pgu64Result res_read = pg_reader_read(&r->reader, tmp_slice);
  if (res_read.err) {
    res.err = res_read.err;
    return res;
  }

  Pgu8Slice read_data = PG_SLICE_RANGE(tmp_slice, 0, res_read.res);
  PG_ASSERT(read_data.len == pg_ring_try_write_bytes(&r->ring, read_data));

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgError
pg_buf_reader_try_fill_internal_buffer(PgBufReader *r) {
  PG_ASSERT(r);

  u64 ring_write_space = pg_ring_write_space(r->ring);
  u8 tmp[4096] = {0};
  Pgu8Slice tmp_slice = {
      .data = tmp,
      .len = PG_MIN(ring_write_space, PG_STATIC_ARRAY_LEN(tmp)),
  };
  // No more space.
  if (0 == tmp_slice.len) {
    return 0;
  }

  Pgu64Result res_read = pg_reader_read(&r->reader, tmp_slice);
  if (res_read.err) {
    return res_read.err;
  }
  if (0 == res_read.res) {
    return PG_ERR_EOF;
  }

  Pgu8Slice read_data = PG_SLICE_RANGE(tmp_slice, 0, res_read.res);
  PG_ASSERT(read_data.len == pg_ring_try_write_bytes(&r->ring, read_data));

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_buf_reader_read_mem_until_bytes_incl(PgBufReader *r, Pgu8Slice dst,
                                        Pgu8Slice needle) {
  PG_ASSERT(dst.data);

  for (;;) {
    Pgu64Result res = {0};
    PgError err = pg_buf_reader_try_fill_internal_buffer(r);
    if (err) {
      res.err = err;
      return res;
    }

    // NOTE: We want to only read until the `needle`, and not one byte more.
    Pgu64Ok search = pg_ring_index_of_bytes(r->ring, needle);
    if (!search.ok) {
      return res;
    }

    // Do the real read out of the real ring.
    u64 n_read = search.res + needle.len;
    PG_ASSERT(n_read ==
              pg_ring_try_read_bytes(&r->ring, PG_SLICE_RANGE(dst, 0, n_read)));
    res.res = n_read;
    return res;
  }
}

[[maybe_unused]] [[nodiscard]] static PgHttpRequestReadResult
pg_http_read_request(PgBufReader *reader, PgAllocator *allocator) {
  PgHttpRequestReadResult res = {0};
  PgString sep = PG_S("\r\n\r\n");

  Pgu8Slice header_bytes = pg_bytes_make(reader->ring.data.len, allocator);
  Pgu64Result res_read_headers =
      pg_buf_reader_read_mem_until_bytes_incl(reader, header_bytes, sep);
  if (res_read_headers.err) {
    res.err = res_read_headers.err;
    return res;
  }

  header_bytes.len = res_read_headers.res;

  PgSplitIterator it = pg_string_split_string(header_bytes, PG_S("\r\n"));
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

  for (;;) {
    res_split = pg_string_split_next(&it);
    if (!res_split.ok) {
      break;
    }
    PgStringKeyValueResult res_kv = pg_http_parse_header(res_split.res);
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
  err = pg_writer_write_string_full(w, PG_S("\r\n"), allocator);
  if (err) {
    return err;
  }

  return 0;
}

[[maybe_unused]] static PgString
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
  err = pg_writer_write_string_full(w, PG_S("\r\n"), allocator);
  if (err) {
    return err;
  }

  return 0;
}

[[maybe_unused]] [[nodiscard]] static Pgu64Result
pg_http_headers_parse_content_length(PgStringKeyValueSlice headers,
                                     PgArena arena) {
  Pgu64Result res = {0};

  for (u64 i = 0; i < headers.len; i++) {
    PgStringKeyValue h = PG_SLICE_AT(headers, i);

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
      // TODO: Consider using `rtdsc` or such.
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
    PG_ASSERT(nullptr != (logger)->make_log_line);                             \
    if ((logger)->level > (lvl)) {                                             \
      break;                                                                   \
    };                                                                         \
    u8 mem[PG_LOG_LINE_MAX_LENGTH] = {0};                                      \
    PgString xxx_log_line = (logger)->make_log_line(                           \
        mem, PG_STATIC_ARRAY_LEN(mem), logger, lvl, PG_S(msg),                 \
        PG_LOG_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__);                          \
    (void)pg_writer_write(&(logger)->writer, xxx_log_line, nullptr);           \
  } while (0)

[[maybe_unused]] static void pg_logfmt_escape_u8(Pgu8Dyn *sb, u8 c,
                                                 PgAllocator *allocator) {
  if (' ' == c || c == '-' || c == '_' || c == ':' || c == ',' || c == '.' ||
      c == '/' || c == '(' || c == ')' || pg_rune_ascii_is_alphanumeric(c)) {
    *PG_DYN_PUSH(sb, allocator) = c;
  } else {
    u8 c1 = c % 16;
    u8 c2 = c / 16;
    PG_DYN_APPEND_SLICE(sb, PG_S("\\x"), allocator);
    *PG_DYN_PUSH(sb, allocator) = pg_u8_to_hex_rune(c2);
    *PG_DYN_PUSH(sb, allocator) = pg_u8_to_hex_rune(c1);
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
  PgWriter w = pg_writer_make_string_builder(256, allocator);

  PG_ASSERT(0 == pg_writer_write_string_full(&w, PG_S("level="), allocator));
  PG_ASSERT(0 == pg_writer_write_string_full(&w, pg_log_level_to_string(level),
                                             allocator));

  PG_ASSERT(0 ==
            pg_writer_write_string_full(&w, PG_S(" timestamp_ns="), allocator));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, timestamp_ns, allocator));

  PG_ASSERT(0 ==
            pg_writer_write_string_full(&w, PG_S(" monotonic_ns="), allocator));
  PG_ASSERT(0 == pg_writer_write_u64_as_string(&w, monotonic_ns, allocator));

  PG_ASSERT(0 == pg_writer_write_string_full(&w, PG_S(" message="), allocator));
  PG_ASSERT(0 == pg_writer_write_string_full(
                     &w, pg_logfmt_escape_string(msg, allocator), allocator));

  va_list argp = {0};
  va_start(argp, args_count);
  for (i32 i = 0; i < args_count; i++) {
    PgLogEntry entry = va_arg(argp, PgLogEntry);
    PG_ASSERT(0 == pg_writer_write_u8(&w, ' ', allocator));
    PG_ASSERT(0 == pg_writer_write_string_full(&w, entry.key, allocator));
    PG_ASSERT(0 == pg_writer_write_u8(&w, '=', allocator));

    switch (entry.value.kind) {
    case PG_LOG_VALUE_STRING: {
      PG_ASSERT(0 == pg_writer_write_string_full(
                         &w, pg_logfmt_escape_string(entry.value.s, allocator),
                         allocator));
      break;
    }
    case PG_LOG_VALUE_U64:
      PG_ASSERT(0 ==
                pg_writer_write_u64_as_string(&w, entry.value.n64, allocator));
      break;
    case PG_LOG_VALUE_I64:
      PG_ASSERT(0 ==
                pg_writer_write_i64_as_string(&w, entry.value.s64, allocator));
      break;
    case PG_LOG_VALUE_IPV4_ADDRESS: {
      PgString ipv4_addr_str =
          pg_net_ipv4_address_to_string(entry.value.ipv4_address, allocator);
      PG_ASSERT(0 == pg_writer_write_string_full(&w, ipv4_addr_str, allocator));
    } break;
    default:
      PG_ASSERT(0 && "invalid PgLogValueKind");
    }
  }
  va_end(argp);

  PG_ASSERT(0 == pg_writer_write_u8(&w, '\n', allocator));

  return PG_DYN_SLICE(PgString, w.u.bytes);
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
  memcpy(res.value, digest.data, 16);

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

#if 0
[[maybe_unused]] [[nodiscard]]
static PgHtmlTokenResult pg_html_tokenize_attribute_key_value(PgString s,
                                                              u64 *pos) {
  PgHtmlTokenResult res = {0};

  for (;;) {
    PgRune first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (pg_rune_is_space(first)) {
      *pos += 1;
      continue;
    }

    if ('/' == first) {
      *pos += 1;
      if ('>' != pg_string_first(PG_SLICE_RANGE_START(s, *pos))) {
        res.err = PG_HTML_PARSE_ERROR_EOF_IN_TAG;
        return res;
      }
      *pos += 1;
      return res;
    }

    if ('=' == first) {
      // TODO
    }

    res.res.kind = PG_HTML_TOKEN_KIND_ATTRIBUTE;
    res.res.start = (u32)*pos;
    res.res.attribute.key.data = s.data + *pos;

    if ('=' != pg_string_first(PG_SLICE_RANGE_START(s, *pos))) {
      return res;
    }

    *pos += 1;

    PgRune quote = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (!quote) {
      res.err = PG_HTML_PARSE_ERROR_EOF_IN_TAG;
      return res;
    }

    idx = pg_string_index_of_rune(PG_SLICE_RANGE_START(s, *pos), quote);
    if (-1 == idx) {
      res.err = PG_HTML_PARSE_ERROR_EOF_IN_TAG;
      return res;
    }

    res.res.attribute.value = PG_SLICE_RANGE(s, *pos, *pos + (u64)idx);

    return res;
  }
}
#endif

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
    PgRune first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));

    if (0 == first) { // Early EOF.
      return PG_HTML_PARSE_ERROR_EOF_IN_TAG;
    }

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
    PgRune first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (0 == first) {
      return PG_HTML_PARSE_ERROR_EOF_IN_COMMENT;
    }

    // FIXME: More complicated than that in the spec.
    if (pg_string_starts_with(PG_SLICE_RANGE_START(s, *pos), PG_S("-->"))) {
      PgHtmlToken token = {
          .kind = PG_HTML_TOKEN_KIND_COMMENT,
          .comment = pg_string_trim_space(PG_SLICE_RANGE(s, start, *pos)),
          .start = start,
          .end = (u32)*pos,
      };
      *PG_DYN_PUSH(tokens, allocator) = token;

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
    PgRune first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));

    if (0 == first) {
      return PG_HTML_PARSE_ERROR_EOF_IN_DOCTYPE;
    }

    if (pg_rune_ascii_is_space(first)) {
      *pos += pg_utf8_rune_bytes_count(first);
      return 0;
    }

    if ('>' == first) {
      token.end = (u32)*pos;
      token.doctype.len = token.end - token.start;
      PG_ASSERT(!pg_string_is_empty(token.doctype));
      *PG_DYN_PUSH(tokens, allocator) = token;

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

  PgRune first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
  if (!pg_rune_ascii_is_space(first)) {
    return PG_HTML_PARSE_ERROR_MISSING_WHITESPACE_BEFORE_DOCTYPE_NAME;
  }
  *pos += pg_utf8_rune_bytes_count(first);

  for (;;) {
    first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    if (0 == first) {
      return PG_HTML_PARSE_ERROR_EOF_IN_DOCTYPE;
    }

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
  PG_ASSERT('<' == pg_string_first(PG_SLICE_RANGE_START(s, *pos)));
  *pos += pg_utf8_rune_bytes_count('<');

  if ('!' == pg_string_first(PG_SLICE_RANGE_START(s, *pos))) {
    *pos += pg_utf8_rune_bytes_count('!');
    return pg_html_tokenize_markup(s, pos, tokens, allocator);
  }

  PgHtmlToken token = {
      .start = (u32)*pos,
      .end = 0, // Backpatched,
      .kind = PG_HTML_TOKEN_KIND_TAG_OPENING,
      .tag = {.data = s.data + *pos}, // Length backpatched.
  };

  PgRune first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
  if ('/' == first) {
    *pos += pg_utf8_rune_bytes_count(first);
    token.start = (u32)*pos;
    token.kind = PG_HTML_TOKEN_KIND_TAG_CLOSING;
    token.tag.data += pg_utf8_rune_bytes_count(first);
  }

  first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
  if (!pg_rune_ascii_is_alphabetical(first)) {
    return PG_HTML_PARSE_ERROR_INVALID_FIRST_CHARACTER_OF_TAG_NAME;
  }
  *pos += pg_utf8_rune_bytes_count(first);

  for (;;) {
    first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));

    if (0 == first) { // Early EOF.
      return PG_HTML_PARSE_ERROR_EOF_IN_TAG;
    }

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

      *PG_DYN_PUSH(tokens, allocator) = token;
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
    PgRune first = pg_string_first(PG_SLICE_RANGE_START(s, *pos));
    // TODO: Check about null byte.
    if (0 == first) {
      return 0;
    }
    if ('<' == first) { // End of data, start of tag.
      PgHtmlToken token = {
          .start = start,
          .end = (u32)(*pos),
          .kind = PG_HTML_TOKEN_KIND_TEXT,
          .text = PG_SLICE_RANGE(s, start, *pos),
      };
      if (!pg_string_is_empty(pg_string_trim_space(token.text))) {
        *PG_DYN_PUSH(tokens, allocator) = token;
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

  /* PgString comment_start = PG_S("<!--"); */
  /* PgString comment_end = PG_S("-->"); */
  PgRune tag_start = '<';
  u64 pos = 0;
  for (;;) {
    PgRune first = pg_string_first(PG_SLICE_RANGE_START(s, pos));
    if (0 == first) { // EOF.
      return res;
    }

    // TODO: Doctype.
    // TODO: Comment.

    // Tag.
    if (tag_start == first) {
      res.err = pg_html_tokenize_tag(s, &pos, &res.res, allocator);
      if (res.err) {
        return res;
      }

      res.err = pg_html_tokenize_data(s, &pos, &res.res, allocator);
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
  PgHtmlTokenSlice tokens = PG_DYN_SLICE(PgHtmlTokenSlice, res_tokens.res);

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
  res.res = root;
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
  return pg_html_node_is_title_opening(node)
             ? ((u8)pg_string_last(node->token_start.tag) - '0')
             : 0;
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

#if 0
PG_SLICE(thrd_t) PgThreadSlice;
PG_DYN(thrd_t) PgThreadDyn;

typedef struct PgThreadPoolTask PgThreadPoolTask;
struct PgThreadPoolTask {
  void *data;
  PgThreadPoolTask *next;
  thrd_start_t fn;
};

typedef struct {
  PgThreadSlice workers;

  // Linked list.
  PgThreadPoolTask *tasks;
  mtx_t tasks_mtx;
  cnd_t tasks_cnd;

  bool done;
} PgThreadPool;

typedef struct {
  PgError err;
  PgThreadPool *pool;
} PgThreadPoolResult;

// Caller is responsible for proper locking.
[[maybe_unused]] [[nodiscard]]
static PgThreadPoolTask *pg_thread_pool_dequeue_task(PgThreadPool *pool) {
  PG_ASSERT(pool);
  PG_ASSERT(pool->tasks);
  PgThreadPoolTask *res = pool->tasks;
  pool->tasks = pool->tasks->next;
  return res;
}

[[nodiscard]] static int pg_pool_worker_start_fn(void *data) {
  PG_ASSERT(data);
  PgThreadPool *pool = data;

  for (;;) {
    PG_ASSERT(thrd_success == mtx_lock(&pool->tasks_mtx));
    while (!pool->tasks) {
      if (pool->done) {
        PG_ASSERT(thrd_success == mtx_unlock(&pool->tasks_mtx));
        return thrd_success;
      }

      PG_ASSERT(thrd_success == cnd_wait(&pool->tasks_cnd, &pool->tasks_mtx));
    }

    PgThreadPoolTask *task = pg_thread_pool_dequeue_task(pool);
    PG_ASSERT(thrd_success == mtx_unlock(&pool->tasks_mtx));

    PG_ASSERT(task);
    PG_ASSERT(task->fn);

    int ret = task->fn(task->data);
    if (thrd_success != ret) {
      return ret;
    }
  }
}

[[nodiscard]] [[maybe_unused]] static PgThreadPoolResult
pg_thread_pool_make(u32 size, PgAllocator *allocator) {
  PgThreadPoolResult res = {0};
  res.pool = PG_NEW(PgThreadPool, allocator);

  if (thrd_success != mtx_init(&res.pool->tasks_mtx, mtx_plain)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  if (thrd_success != cnd_init(&res.pool->tasks_cnd)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.pool->workers.len = size;
  res.pool->workers.data =
      pg_alloc(allocator, sizeof(thrd_t), _Alignof(thrd_t), size);

  for (u32 i = 0; i < size; i++) {
    thrd_t *thread = PG_SLICE_AT_PTR(&res.pool->workers, i);
    int ret = thrd_create(thread, pg_pool_worker_start_fn, res.pool);
    if (ret != thrd_success) {
      res.err = PG_ERR_INVALID_VALUE; // FIXME: return exact error.
      return res;
    }
  }

  return res;
}

[[maybe_unused]]
static void pg_thread_pool_enqueue_task(PgThreadPool *pool, thrd_start_t fn,
                                        void *data, PgAllocator *allocator) {
  PG_ASSERT(pool);
  PG_ASSERT(fn);
  PG_ASSERT(data);
  PG_ASSERT(allocator);

  PgThreadPoolTask *task = PG_NEW(PgThreadPoolTask, allocator);
  task->data = data;
  task->fn = fn;

  PG_ASSERT(thrd_success == mtx_lock(&pool->tasks_mtx));
  if (!pool->tasks) {
    pool->tasks = task;
  } else {
    task->next = pool->tasks;
    pool->tasks = task;
  }
  PG_ASSERT(thrd_success == cnd_signal(&pool->tasks_cnd));
  PG_ASSERT(thrd_success == mtx_unlock(&pool->tasks_mtx));
}

[[maybe_unused]] static void pg_thread_pool_wait(PgThreadPool *pool) {
  PG_ASSERT(thrd_success == mtx_lock(&pool->tasks_mtx));
  pool->done = true;
  PG_ASSERT(thrd_success == cnd_broadcast(&pool->tasks_cnd));
  PG_ASSERT(thrd_success == mtx_unlock(&pool->tasks_mtx));

  for (u32 i = 0; i < pool->workers.len; i++) {
    thrd_join(PG_SLICE_AT(pool->workers, i), nullptr);
  }
}
#endif

[[nodiscard]] [[maybe_unused]] static PgElfSymbolType
pg_elf_symbol_get_type(PgElfSymbolTableEntry sym) {
  return sym.info & 0xf;
}

[[nodiscard]] [[maybe_unused]] static PgElfSymbolBind
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

  res.res = PG_SLICE_RANGE(elf.bytes, section.offset, end);

  return res;
}

[[nodiscard]] static PgStringResult pg_elf_get_string_at(PgElf elf,
                                                         u32 offset) {
  PgStringResult res = {0};

  if (offset >= elf.strtab.len) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  Pgu8Slice at = PG_SLICE_RANGE_START(elf.strtab, offset);

  PgBytesCut cut = pg_bytes_cut_byte(at, 0);
  if (!cut.ok) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.res = cut.left;

  return res;
}

[[maybe_unused]] [[nodiscard]] static PgElfResult
pg_elf_parse(Pgu8Slice elf_bytes, PgAllocator *allocator) {
  PgElfResult res = {0};
  res.res.bytes = elf_bytes;

  if (elf_bytes.len < sizeof(PgElfHeader)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }
  memcpy(&res.res.header, elf_bytes.data, sizeof(res.res.header));

  // Section headers.
  {
    PgElfHeader h = res.res.header;
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
    res.res.section_headers = (PgElfSectionHeaderDyn){
        .data = (void *)section_headers_bytes.data,
        .len = h.section_header_entries_count,
        .cap = h.section_header_entries_count,
    };
    for (u32 i = 0; i < res.res.section_headers.len; i++) {
      PgElfSectionHeader section = PG_SLICE_AT(res.res.section_headers, i);

      switch (section.kind) {
      case PG_ELF_SECTION_HEADER_KIND_SYMTAB: {
        Pgu8SliceResult res_bytes = pg_elf_get_section_header_bytes(res.res, i);
        if (res_bytes.err) {
          res.err = res_bytes.err;
          return res;
        }

        Pgu8Slice bytes = res_bytes.res;

        if (PG_SLICE_IS_EMPTY(bytes)) {
          res.err = res_bytes.err;
          return res;
        }

        PG_DYN_ENSURE_CAP(&res.res.symtab, section.size, allocator);
        memcpy(res.res.symtab.data, bytes.data, bytes.len);
        res.res.symtab.len = bytes.len / sizeof(PgElfSymbolTableEntry);
      } break;
      case PG_ELF_SECTION_HEADER_KIND_STRTAB: {
        Pgu8SliceResult res_bytes = pg_elf_get_section_header_bytes(res.res, i);
        if (res_bytes.err) {
          res.err = res_bytes.err;
          return res;
        }

        if (PG_SLICE_IS_EMPTY(res_bytes.res)) {
          res.err = res_bytes.err;
          return res;
        }

        res.res.strtab = res_bytes.res;
      } break;

      case PG_ELF_SECTION_HEADER_KIND_NULL:
      case PG_ELF_SECTION_HEADER_KIND_PROGBITS:
      case PG_ELF_SECTION_HEADER_KIND_RELA:
      case PG_ELF_SECTION_HEADER_KIND_HASH:
      case PG_ELF_SECTION_HEADER_KIND_DYNAMIC:
      case PG_ELF_SECTION_HEADER_KIND_NOTE:
      case PG_ELF_SECTION_HEADER_KIND_NOBITS:
      case PG_ELF_SECTION_HEADER_KIND_REL:
      case PG_ELF_SECTION_HEADER_KIND_SHLIB:
      case PG_ELF_SECTION_HEADER_KIND_DYNSYM:
      default: {
      }
      }
    }

    // Second pass to find the `.text` section since section header might be
    // in any order.
    for (u32 i = 0; i < res.res.section_headers.len; i++) {
      PgElfSectionHeader section = PG_SLICE_AT(res.res.section_headers, i);

      if (PG_ELF_SECTION_HEADER_KIND_PROGBITS != section.kind) {
        continue;
      }

      PgStringResult res_str = pg_elf_get_string_at(res.res, section.name);
      if (res_str.err) {
        res.err = res_str.err;
        return res;
      }

      if (!pg_string_eq(res_str.res, PG_S(".text"))) {
        continue;
      }

      // Found.

      Pgu8SliceResult res_bytes = pg_elf_get_section_header_bytes(res.res, i);
      if (res_bytes.err) {
        res.err = res_bytes.err;
        return res;
      }

      res.res.program_text = res_bytes.res;
      res.res.program_text_idx = i;
      break;
    }

    if (PG_SLICE_IS_EMPTY(res.res.program_text)) {
      res.err = PG_ERR_INVALID_VALUE;
      return res;
    }
  }

  return res;
}

[[maybe_unused]] [[nodiscard]] static Pgu8SliceResult
pg_elf_symbol_get_program_text(PgElf elf, PgElfSymbolTableEntry sym) {
  Pgu8SliceResult res = {0};

  if (elf.program_text_idx != sym.section_header_table_index) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  if (sym.value >= elf.program_text.len) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  u64 end = 0;
  if (__builtin_add_overflow(sym.value, sym.size, &end)) {
    res.err = PG_ERR_INVALID_VALUE;
    return res;
  }

  res.res = PG_SLICE_RANGE(elf.program_text, sym.value, end);
  return res;
}

#ifdef PG_OS_LINUX
#include <sys/sendfile.h>

[[nodiscard]] [[maybe_unused]] static PgError
pg_file_send_to_socket(PgFileDescriptor dst, PgFileDescriptor src) {
  Pgu64Result res_size = pg_file_size(src);
  if (res_size.err) {
    return res_size.err;
  }

  i64 offset = 0;
  u64 size = res_size.res;

  for (u64 _i = 0; _i < size; _i++) {
    i64 ret = sendfile(dst.fd, src.fd, &offset, size);
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

// TODO: sendfile on BSD.

[[maybe_unused]] [[nodiscard]]
static PgError pg_http_server_handler(PgFileDescriptor sock, PgLogger *logger,
                                      PgAllocator *allocator) {
  PgReader reader = pg_reader_make_from_file_descriptor(sock);
  PgBufReader buf_reader = pg_buf_reader_make(reader, 12 * PG_KiB, allocator);
  PgHttpRequestReadResult res_req =
      pg_http_read_request(&buf_reader, allocator);

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

  PgHttpRequest req = res_req.res;
  __builtin_dump_struct(&req, printf);

  // TODO: Read body depending on the HTTP method.

  // Response.
  {
    PgHttpResponse resp = {0};
    resp.status = 200;
    resp.version_major = 1;
    resp.version_minor = 1;

    pg_http_push_header(&resp.headers, PG_S("Content-Type"),
                        PG_S("application/html"), allocator);

    PgWriter w = pg_writer_make_from_file_descriptor(sock);

    PgError err = pg_http_write_response(&w, resp, allocator);
    if (err) {
      pg_log(logger, PG_LOG_LEVEL_ERROR,
             "http handler: failed to write http response",
             pg_log_c_err("err", err));
      return err;
    }

    Pgu64Result res_write =
        pg_writer_write(&w, PG_S("<html>hello</html>"), allocator);
    if (res_write.err) {
      pg_log(logger, PG_LOG_LEVEL_ERROR,
             "http handler: failed to write http response body",
             pg_log_c_err("err", res_write.err));
      return res_write.err;
    }
  }
  return 0;
}

[[maybe_unused]]
static PgError pg_http_server_start(u16 port, u64 listen_backlog,
                                    u64 http_handler_arena_mem,
                                    PgLogger *logger) {
  PgError err = 0;
  err = pg_process_avoid_child_zombies();
  if (err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http server: failed to avoid child zombies",
           pg_log_c_u16("port", port), pg_log_c_err("err", err));
    return err;
  }

  PgFileDescriptorResult res_create = pg_net_create_tcp_socket();
  if (res_create.err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http server: failed to create tcp socket",
           pg_log_c_u16("port", port), pg_log_c_err("err", err));
    return res_create.err;
  }
  PgFileDescriptor server_socket = res_create.res;

  err = pg_net_socket_enable_reuse(server_socket);
  if (err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http server: failed to enable socket reuse",
           pg_log_c_u16("port", port), pg_log_c_err("err", err));
    goto end;
  }

  PgIpv4Address address = {.port = port};
  err = pg_net_tcp_bind_ipv4(server_socket, address);
  if (err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR, "http server: failed to bind tcp socket",
           pg_log_c_u16("port", port), pg_log_c_err("err", err));
    goto end;
  }

  err = pg_net_tcp_listen(server_socket, listen_backlog);
  if (err) {
    pg_log(logger, PG_LOG_LEVEL_ERROR,
           "http server: failed to listen on tcp socket",
           pg_log_c_u16("port", port), pg_log_c_err("err", err));
    goto end;
  }

  pg_log(logger, PG_LOG_LEVEL_INFO, "http server: listening",
         pg_log_c_u16("port", port));

  for (;;) {
    PgIpv4AddressAcceptResult res_accept = pg_net_tcp_accept(server_socket);
    if (res_accept.err) {
      // TODO: Some errors are retryable.
      err = res_accept.err;
      pg_log(logger, PG_LOG_LEVEL_ERROR,
             "http server: failed to accept new connection",
             pg_log_c_u16("port", port), pg_log_c_err("err", err));
      goto end;
    }

    Pgu32Result res_proc_dup = pg_process_dup();
    if (res_proc_dup.err) {
      pg_log(
          logger, PG_LOG_LEVEL_ERROR,
          "http server: failed to spawn new process to handle new connection",
          pg_log_c_u16("port", port), pg_log_c_err("err", res_proc_dup.err));
      (void)pg_net_socket_close(res_accept.socket);
      continue;
    }

    if (0 == res_proc_dup.res) { // Child.
      PgArena arena = pg_arena_make_from_virtual_mem(http_handler_arena_mem);
      PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
      PgAllocator *allocator =
          pg_arena_allocator_as_allocator(&arena_allocator);

      (void)pg_http_server_handler(res_accept.socket, logger, allocator);
      exit(0);
    }

    PG_ASSERT(res_proc_dup.res); // Parent.

    (void)pg_net_socket_close(res_accept.socket);
  }

end:
  (void)pg_net_socket_close(server_socket);
  return err;
}
#endif
