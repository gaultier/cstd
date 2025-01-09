#include "lib.c"

static void test_string_indexof_slice() {
  // Empty haystack.
  {
    ASSERT(-1 == string_indexof_string((String){0}, S("fox")));
  }

  // Empty needle.
  {
    ASSERT(-1 == string_indexof_string(S("hello"), (String){0}));
  }

  // Not found.
  {
    ASSERT(-1 == string_indexof_string(S("hello world"), S("foobar")));
  }

  // Found, one occurence.
  {
    ASSERT(6 == string_indexof_string(S("hello world"), S("world")));
  }

  // Found, first occurence.
  {
    ASSERT(6 == string_indexof_string(S("world hello hell"), S("hell")));
  }

  // Found, second occurence.
  {
    ASSERT(10 == string_indexof_string(S("hello fox foxy"), S("foxy")));
  }

  // Almost found, prefix matches.
  {
    ASSERT(-1 == string_indexof_string(S("hello world"), S("worldly")));
  }
}

static void test_string_trim() {
  String trimmed = string_trim(S("   foo "), ' ');
  ASSERT(string_eq(trimmed, S("foo")));
}

static void test_string_split() {
  String s = S("hello..world...foobar");
  SplitIterator it = string_split(s, '.');

  {
    SplitResult elem = string_split_next(&it);
    ASSERT(true == elem.ok);
    ASSERT(string_eq(elem.s, S("hello")));
  }

  {
    SplitResult elem = string_split_next(&it);
    ASSERT(true == elem.ok);
    ASSERT(string_eq(elem.s, S("world")));
  }

  {
    SplitResult elem = string_split_next(&it);
    ASSERT(true == elem.ok);
    ASSERT(string_eq(elem.s, S("foobar")));
  }

  ASSERT(false == string_split_next(&it).ok);
  ASSERT(false == string_split_next(&it).ok);
}

static void test_dyn_ensure_cap() {
  u64 arena_cap = 4 * KiB;

  // Trigger the optimization when the last allocation in the arena gets
  // extended.
  {
    Arena arena = arena_make_from_virtual_mem(arena_cap);

    DynU8 dyn = {0};
    *dyn_push(&dyn, &arena) = 1;
    ASSERT(1 == dyn.len);
    ASSERT(2 == dyn.cap);

    u64 arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    ASSERT(2 == arena_size_expected);
    ASSERT(dyn.cap == arena_size_expected);

    u64 desired_cap = 13;
    dyn_ensure_cap(&dyn, desired_cap, &arena);
    ASSERT(16 == dyn.cap);
    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    ASSERT(16 == arena_size_expected);
  }
  // General case.
  {
    Arena arena = arena_make_from_virtual_mem(arena_cap);

    DynU8 dyn = {0};
    *dyn_push(&dyn, &arena) = 1;
    ASSERT(1 == dyn.len);
    ASSERT(2 == dyn.cap);

    DynU8 dummy = {0};
    *dyn_push(&dummy, &arena) = 2;
    *dyn_push(&dummy, &arena) = 3;

    u64 arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    ASSERT(2 + 2 == arena_size_expected);

    // This triggers a new allocation.
    *dyn_push(&dummy, &arena) = 4;
    ASSERT(3 == dummy.len);
    ASSERT(4 == dummy.cap);

    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    ASSERT(2 + 4 == arena_size_expected);

    u64 desired_cap = 13;
    dyn_ensure_cap(&dyn, desired_cap, &arena);
    ASSERT(16 == dyn.cap);

    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    ASSERT(16 + 6 == arena_size_expected);
  }
}

static void test_slice_range() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  // Empty slice.
  {
    String s = S("");
    ASSERT(slice_is_empty(slice_range(s, 0, 0)));
    ASSERT(slice_is_empty(slice_range_start(s, 0)));
  }
  // Empty range.
  {
    String s = S("a");
    ASSERT(slice_is_empty(slice_range(s, 0, 0)));
    ASSERT(string_eq(s, slice_range_start(s, 0)));
  }

  {
    String s = S("abcd");
    ASSERT(string_eq(slice_range_start(s, 1), S("bcd")));
    ASSERT(string_eq(slice_range(s, 1, 3), S("bc")));
    ASSERT(string_eq(slice_range(s, 1, 4), S("bcd")));
    ASSERT(string_eq(slice_range(s, 1, 5), S("bcd")));
    ASSERT(slice_is_empty(slice_range(s, 100, 300)));
  }

  DynString dyn = {0};
  // Works on empty slices.
  (void)slice_range(dyn_slice(StringSlice, dyn), 0, 0);

  *dyn_push(&dyn, &arena) = S("hello \"world\n\"!");
  *dyn_push(&dyn, &arena) = S("日");
  *dyn_push(&dyn, &arena) = S("本語");

  StringSlice s = dyn_slice(StringSlice, dyn);
  StringSlice range = slice_range_start(s, 1UL);
  ASSERT(2 == range.len);

  ASSERT(string_eq(slice_at(s, 1), slice_at(range, 0)));
  ASSERT(string_eq(slice_at(s, 2), slice_at(range, 1)));
}

static void test_string_consume() {
  {
    StringConsumeResult res = string_consume_byte(S(""), '{');
    ASSERT(!res.consumed);
  }
  {
    StringConsumeResult res = string_consume_byte(S("[1,2]"), '{');
    ASSERT(!res.consumed);
  }
  {
    StringConsumeResult res = string_consume_byte(S("[1,2]"), '[');
    ASSERT(res.consumed);
    ASSERT(string_eq(S("1,2]"), res.remaining));
  }
}

static void test_string_parse_u64() {
  {
    ParseNumberResult num_res = string_parse_u64(S(""));
    ASSERT(!num_res.present);
    ASSERT(0 == num_res.remaining.len);
  }
  {
    ParseNumberResult num_res = string_parse_u64(S("a"));
    ASSERT(!num_res.present);
    ASSERT(string_eq(S("a"), num_res.remaining));
  }
  {
    ParseNumberResult num_res = string_parse_u64(S("a123b"));
    ASSERT(!num_res.present);
    ASSERT(string_eq(S("a123b"), num_res.remaining));
  }
  {
    ParseNumberResult num_res = string_parse_u64(S("0123"));
    ASSERT(!num_res.present);
    ASSERT(string_eq(S("0123"), num_res.remaining));
  }
  {
    ParseNumberResult num_res = string_parse_u64(S("0"));
    ASSERT(num_res.present);
    ASSERT(string_eq(S(""), num_res.remaining));
    ASSERT(0 == num_res.n);
  }
  {
    ParseNumberResult num_res = string_parse_u64(S("0a"));
    ASSERT(num_res.present);
    ASSERT(string_eq(S("a"), num_res.remaining));
    ASSERT(0 == num_res.n);
  }
  {
    ParseNumberResult num_res = string_parse_u64(S("123a"));
    ASSERT(num_res.present);
    ASSERT(string_eq(S("a"), num_res.remaining));
    ASSERT(123 == num_res.n);
  }
}

static void test_string_cmp() {
  {
    StringCompare cmp = string_cmp(S("a"), S("aa"));
    ASSERT(STRING_CMP_LESS == cmp);
  }
  {
    StringCompare cmp = string_cmp(S(""), S("a"));
    ASSERT(STRING_CMP_LESS == cmp);
  }
  {
    StringCompare cmp = string_cmp(S(""), S(""));
    ASSERT(STRING_CMP_EQ == cmp);
  }
  {
    StringCompare cmp = string_cmp(S("a"), S("a"));
    ASSERT(STRING_CMP_EQ == cmp);
  }
  {
    StringCompare cmp = string_cmp(S("a"), S("b"));
    ASSERT(STRING_CMP_LESS == cmp);
  }
  {
    StringCompare cmp = string_cmp(S("b"), S("aa"));
    ASSERT(STRING_CMP_GREATER == cmp);
  }
  {
    StringCompare cmp = string_cmp(S("b"), S("a"));
    ASSERT(STRING_CMP_GREATER == cmp);
  }
  {
    StringCompare cmp = string_cmp(S("announce"), S("comment"));
    ASSERT(STRING_CMP_LESS == cmp);
  }
}

static void test_sha1() {
  {
    u8 hash[20] = {0};
    sha1(S("abc"), hash);

    u8 expected_hash[20] = {
        0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E,
        0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D,
    };
    static_assert(sizeof(hash) == sizeof(expected_hash));

    ASSERT(0 == memcmp(hash, expected_hash, sizeof(hash)));
  }
}

static void test_slice_swap_remove() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);
  {
    String s = string_dup(S("hello world!"), &arena);
    slice_swap_remove(&s, 4);
    ASSERT(string_eq(s, S("hell! world")));
  }
}

static void test_dynu8_append_u8_hex_upper() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  {
    DynU8 sb = {0};
    dynu8_append_u8_hex_upper(&sb, 0xac, &arena);
    dynu8_append_u8_hex_upper(&sb, 0x89, &arena);

    String s = dyn_slice(String, sb);
    ASSERT(string_eq(s, S("AC89")));
  }
}

static void test_ipv4_address_to_string() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);
  {
    Ipv4Address address = {
        .ip = (192UL << 24) | (168UL << 16) | (1UL << 8) | (56UL << 0),
        .port = 6881,
    };

    String s = ipv4_address_to_string(address, &arena);
    ASSERT(string_eq(s, S("192.168.1.56:6881")));
  }
}

static void test_url_encode() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);
  {
    DynU8 sb = {0};
    url_encode_string(&sb, S("日本語"), S("123"), &arena);
    String encoded = dyn_slice(String, sb);

    ASSERT(string_eq(encoded, S("%E6%97%A5%E6%9C%AC%E8%AA%9E=123")));
  }

  {
    DynU8 sb = {0};
    url_encode_string(&sb, S("日本語"), S("foo"), &arena);
    String encoded = dyn_slice(String, sb);

    ASSERT(string_eq(encoded, S("%E6%97%A5%E6%9C%AC%E8%AA%9E=foo")));
  }
}

static void test_string_indexof_any_byte() {
  {
    i64 idx = string_indexof_any_byte(S(""), S(""));
    ASSERT(-1 == idx);
  }
  {
    i64 idx = string_indexof_any_byte(S(""), S(":"));
    ASSERT(-1 == idx);
  }
  {
    i64 idx = string_indexof_any_byte(S("?"), S(":"));
    ASSERT(-1 == idx);
  }
  {
    i64 idx = string_indexof_any_byte(S("abc"), S(":?"));
    ASSERT(-1 == idx);
  }
  {
    i64 idx = string_indexof_any_byte(S("x"), S("yz"));
    ASSERT(-1 == idx);
  }
  {
    i64 idx = string_indexof_any_byte(S(":"), S(":"));
    ASSERT(0 == idx);
  }
  {
    i64 idx = string_indexof_any_byte(S("abc"), S("cd"));
    ASSERT(2 == idx);
  }
}

static void test_log_entry_quote_value() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);
  // Nothing to escape.
  {
    String s = S("hello");
    String expected = S("\"hello\"");
    ASSERT(string_eq(expected, json_escape_string(s, &arena)));
  }
  {
    String s = S("{\"id\": 1}");
    String expected = S("\"{\\\"id\\\": 1}\"");
    ASSERT(string_eq(expected, json_escape_string(s, &arena)));
  }
  {
    String s = S("192.168.1.2:12345");
    String expected = S("\"192.168.1.2:12345\"");
    ASSERT(string_eq(expected, json_escape_string(s, &arena)));
  }
  {
    u8 backslash = 0x5c;
    u8 double_quote = '"';
    u8 data[] = {backslash, double_quote};
    String s = {.data = data, .len = sizeof(data)};

    u8 data_expected[] = {double_quote, backslash,    backslash,
                          backslash,    double_quote, double_quote};
    String expected = {.data = data_expected, .len = sizeof(data_expected)};
    ASSERT(string_eq(expected, json_escape_string(s, &arena)));
  }
}

static void test_make_log_line() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  String log_line = make_log_line(LOG_LEVEL_DEBUG, S("foobar"), &arena, 2,
                                  L("num", 42), L("s", S("hello \"world\"")));

  String expected_suffix =
      S("\"message\":\"foobar\",\"num\":42,\"s\":\"hello \\\"world\\\"\"}\n");
  ASSERT(string_starts_with(log_line,
                            S("{\"level\":\"debug\",\"timestamp_ns\":")));
  ASSERT(string_ends_with(log_line, expected_suffix));
}

static void test_u8x4_be_to_u32_and_back() {
  {
    u32 n = 123'456'789;
    u8 data[sizeof(n)] = {0};
    String s = {.data = data, .len = sizeof(n)};
    u32_to_u8x4_be(n, &s);
    ASSERT(string_eq(s, S("\x07"
                          "\x5b"
                          "\x0cd"
                          "\x15")));

    ASSERT(n == u8x4_be_to_u32(s));
  }
}

static void test_bitfield() {
  {
    String bitfield = S("\x3"
                        "\x2");
    ASSERT(bitfield_get(bitfield, 0));
    ASSERT(bitfield_get(bitfield, 1));
    ASSERT(!bitfield_get(bitfield, 2));
    ASSERT(!bitfield_get(bitfield, 3));
    ASSERT(!bitfield_get(bitfield, 4));
    ASSERT(!bitfield_get(bitfield, 5));
    ASSERT(!bitfield_get(bitfield, 6));
    ASSERT(!bitfield_get(bitfield, 7));
    ASSERT(!bitfield_get(bitfield, 8));
    ASSERT(bitfield_get(bitfield, 9));
    ASSERT(!bitfield_get(bitfield, 10));
    ASSERT(!bitfield_get(bitfield, 11));
    ASSERT(!bitfield_get(bitfield, 12));
    ASSERT(!bitfield_get(bitfield, 13));
    ASSERT(!bitfield_get(bitfield, 14));
    ASSERT(!bitfield_get(bitfield, 15));
  }
}

static void test_ring_buffer_write_slice() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  // Write to empty ring buffer.
  {
    RingBuffer rg = {.data = string_make(12, &arena)};
    ASSERT(ring_buffer_write_space(rg) == rg.data.len - 1);
    ASSERT(ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(5 == rg.idx_write);
    ASSERT(ring_buffer_write_space(rg) == rg.data.len - 1 - 5);

    ASSERT(false == ring_buffer_write_slice(&rg, S(" world!")));
    ASSERT(5 == rg.idx_write);
    ASSERT(ring_buffer_write_space(rg) == rg.data.len - 1 - 5);

    ASSERT(true == ring_buffer_write_slice(&rg, S(" world")));
    ASSERT(11 == rg.idx_write);
    ASSERT(ring_buffer_write_space(rg) == 0);

    ASSERT(0 == rg.idx_read);
    ASSERT(string_eq(S("hello world"),
                     (String){.data = rg.data.data, .len = rg.idx_write}));
  }
  // Write to full ring buffer.
  {
    RingBuffer rg = {
        .data = string_make(12, &arena),
        .idx_write = 1,
        .idx_read = 2,
    };
    ASSERT(ring_buffer_write_space(rg) == 0);
    ASSERT(false == ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(1 == rg.idx_write);
    ASSERT(ring_buffer_write_space(rg) == 0);
  }
  // Write to ring buffer, easy case.
  {
    RingBuffer rg = {
        .data = string_make(12, &arena),
        .idx_read = 1,
        .idx_write = 2,
    };
    ASSERT(ring_buffer_write_space(rg) == 10);
    ASSERT(ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(2 + 5 == rg.idx_write);
    ASSERT(ring_buffer_write_space(rg) == 5);
  }

  // Write to ring buffer, hard case.
  {
    RingBuffer rg = {
        .data = string_make(12, &arena),
        .idx_read = 2,
        .idx_write = 3,
    };
    ASSERT(ring_buffer_write_space(rg) == 10);
    ASSERT(ring_buffer_write_slice(&rg, S("hello worl")));
    ASSERT(1 == rg.idx_write);
    ASSERT(ring_buffer_write_space(rg) == 0);
    ASSERT(string_eq(rg.data, S("l\x0\x0hello wor")));
  }
}

static void test_ring_buffer_read_write_slice() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  // Read from an empty ring buffer.
  {
    RingBuffer rg = {.data = string_make(12, &arena)};
    ASSERT(0 == ring_buffer_read_space(rg));
    ASSERT(true == ring_buffer_read_slice(&rg, (String){0}));

    String dst = string_dup(S("xyz"), &arena);
    ASSERT(false == ring_buffer_read_slice(&rg, dst));
  }

  // Write to empty ring buffer, then read part of it.
  {
    RingBuffer rg = {.data = string_make(12, &arena)};
    ASSERT(ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(5 == rg.idx_write);
    ASSERT(5 == ring_buffer_read_space(rg));

    String dst = string_dup(S("xyz"), &arena);
    ASSERT(ring_buffer_read_slice(&rg, dst));
    ASSERT(string_eq(dst, S("hel")));
    ASSERT(3 == rg.idx_read);
    ASSERT(2 == ring_buffer_read_space(rg));

    ASSERT(true == ring_buffer_write_slice(&rg, S(" world!")));
    ASSERT(0 == rg.idx_write);
    ASSERT(9 == ring_buffer_read_space(rg));

    ASSERT(false == ring_buffer_write_slice(&rg, S("abc")));
    ASSERT(9 == ring_buffer_read_space(rg));
    ASSERT(true == ring_buffer_write_slice(&rg, S("ab")));
    ASSERT(11 == ring_buffer_read_space(rg));
    ASSERT(2 == rg.idx_write);

    dst = string_dup(S("abcdefghijk"), &arena);
    ASSERT(ring_buffer_read_slice(&rg, dst));
    ASSERT(string_eq(dst, S("lo world!ab")));
    ASSERT(2 == rg.idx_read);
    ASSERT(0 == ring_buffer_read_space(rg));
  }
}

static void test_ring_buffer_read_until_excl() {
  Arena arena = arena_make_from_virtual_mem(8 * KiB);
  RingBuffer rg = {.data = string_make(4 * KiB, &arena)};
  ASSERT(ring_buffer_write_slice(
      &rg, S("The quick brown fox jumps over the lazy dog")));

  {
    u64 space_read = ring_buffer_read_space(rg);
    u64 space_write = ring_buffer_write_space(rg);
    StringOk s = ring_buffer_read_until_excl(&rg, S("\r\n"), &arena);
    ASSERT(!s.ok);
    ASSERT(slice_is_empty(s.res));

    // Unmodified.
    ASSERT(ring_buffer_read_space(rg) == space_read);
    ASSERT(ring_buffer_write_space(rg) == space_write);
  }

  {
    StringOk s = ring_buffer_read_until_excl(&rg, S(" "), &arena);
    ASSERT(s.ok);
    ASSERT(string_eq(s.res, S("The")));
  }
  {
    StringOk s = ring_buffer_read_until_excl(&rg, S(" "), &arena);
    ASSERT(s.ok);
    ASSERT(string_eq(s.res, S("quick")));
  }
  {
    StringOk s = ring_buffer_read_until_excl(&rg, S("lazy "), &arena);
    ASSERT(s.ok);
    ASSERT(string_eq(s.res, S("brown fox jumps over the ")));
  }
  {
    StringOk s = ring_buffer_read_until_excl(&rg, S("g"), &arena);
    ASSERT(s.ok);
    ASSERT(string_eq(s.res, S("do")));
  }
  ASSERT(0 == ring_buffer_read_space(rg));
}

static void test_ring_buffer_read_write_fuzz() {
  Arena arena_ring = arena_make_from_virtual_mem(4 * KiB);
  RingBuffer rg = {.data = string_make(4 * KiB, &arena_ring)};

  u64 ROUNDS = 1024;
  Arena arena_strings = arena_make_from_virtual_mem(ROUNDS * 8 * KiB);

  // TODO: Random seed for reproducability?
  for (u64 i = 0; i < ROUNDS; i++) {
    String from =
        string_make(arc4random_uniform((u32)rg.data.len + 1), &arena_strings);
    arc4random_buf(from.data, from.len);

    String to =
        string_make(arc4random_uniform((u32)rg.data.len + 1), &arena_strings);
    arc4random_buf(to.data, to.len);

    bool ok_write = ring_buffer_write_slice(&rg, from);
    (void)ok_write;
    bool ok_read = ring_buffer_read_slice(&rg, to);
    (void)ok_read;

    ASSERT(ring_buffer_write_space(rg) <= rg.data.len - 1);
    ASSERT(ring_buffer_read_space(rg) <= rg.data.len - 1);
  }
}

#if 0
static void test_buffered_reader_read_exactly() {

  // Read from closed remote end: error.
  {
    Arena arena = arena_make_from_virtual_mem(16 * KiB);
    int fd_pipe[2] = {0};
    ASSERT(-1 != pipe(fd_pipe));
    close(fd_pipe[1]);

    BufferedReader br = buffered_reader_make(fd_pipe[0], &arena);
    IoResult res_io = buffered_reader_read_exactly(&br, 128, &arena);
    ASSERT((Error)EOF == res_io.err);

    close(fd_pipe[0]);
  }

  {
    Arena arena = arena_make_from_virtual_mem(16 * KiB);
    int fd_pipe[2] = {0};
    ASSERT(-1 != pipe(fd_pipe));

    u8 value[8] = {0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34};
    ASSERT(write(fd_pipe[1], &value, sizeof(value)) == sizeof(value));

    BufferedReader br = buffered_reader_make(fd_pipe[0], &arena);
    IoResult res_io = buffered_reader_read_exactly(&br, 7, &arena);
    ASSERT(0 == res_io.err);
    ASSERT(7 == res_io.res.len);
    ASSERT(string_eq(res_io.res, S("\x12\x0\x0\x0\x0\x0\x0")));

    close(fd_pipe[0]);
    close(fd_pipe[1]);
  }
  {
    Arena arena = arena_make_from_virtual_mem(4 * KiB);

    TestMemReadContext ctx = {.s = string_dup(S("hello world!"), &arena)};
    BufferedReader reader = test_buffered_reader_make(&ctx, &arena);
    reader.read_fn = test_buffered_reader_read_from_slice_one;

    IoResult res_io = buffered_reader_read_exactly(&reader, 7, &arena);
    ASSERT(0 == res_io.err);
    ASSERT(string_eq(S("hello w"), res_io.res));
  }
}

static void test_buffered_reader_read_until_slice() {

  // Read from closed remote end: error.
  {
    Arena arena = arena_make_from_virtual_mem(4 * KiB);
    int fd_pipe[2] = {0};
    ASSERT(-1 != pipe(fd_pipe));
    close(fd_pipe[1]);

    BufferedReader br = buffered_reader_make(fd_pipe[0], &arena);
    IoResult res_io = buffered_reader_read_until_slice(&br, S("\r\n"), &arena);
    ASSERT((Error)EOF == res_io.err);

    close(fd_pipe[0]);
  }

  // Needle absent.
  {
    Arena arena = arena_make_from_virtual_mem(8 * KiB);
    int fd_pipe[2] = {0};
    ASSERT(-1 != pipe(fd_pipe));

    String value = S("hello");
    ASSERT(write(fd_pipe[1], value.data, value.len) == (i64)value.len);
    close(fd_pipe[1]);

    BufferedReader br = buffered_reader_make(fd_pipe[0], &arena);
    IoResult res_io = buffered_reader_read_until_slice(&br, S("\r\n"), &arena);
    ASSERT((Error)EOF == res_io.err);
    ASSERT(0 == res_io.res.len);

    close(fd_pipe[0]);
  }

  // Needle present.
  {
    Arena arena = arena_make_from_virtual_mem(8 * KiB);
    int fd_pipe[2] = {0};
    ASSERT(-1 != pipe(fd_pipe));

    String value = S("hello\r\nbar");
    ASSERT(write(fd_pipe[1], value.data, value.len) == (i64)value.len);

    BufferedReader br = buffered_reader_make(fd_pipe[0], &arena);
    IoResult res_io = buffered_reader_read_until_slice(&br, S("\r\n"), &arena);
    ASSERT(0 == res_io.err);
    ASSERT(5 + 2 == res_io.res.len);
    ASSERT(value.len == br.buf.len);
    ASSERT(res_io.res.len == br.buf_idx);

    close(fd_pipe[0]);
    close(fd_pipe[1]);
  }
}
#endif

#if 0
static void test_buffered_reader_read_until_end() {
  Arena arena = arena_make_from_virtual_mem(16 * KiB);

  // Read from closed remote end.
  {
    int fd_pipe[2] = {0};
    ASSERT(-1 != pipe(fd_pipe));
    close(fd_pipe[1]);

    BufferedReader br = buffered_reader_make(fd_pipe[0], &arena);
    IoResult res_io = buffered_reader_read_until_end(&br, &arena);
    ASSERT(0 == res_io.err);
    ASSERT(0 == res_io.res.len);

    close(fd_pipe[0]);
  }

  {
    int fd_pipe[2] = {0};
    ASSERT(-1 != pipe(fd_pipe));

    String value = S("hello");
    ASSERT(write(fd_pipe[1], value.data, value.len) == (i64)value.len);
    close(fd_pipe[1]);

    BufferedReader br = buffered_reader_make(fd_pipe[0], &arena);
    IoResult res_io = buffered_reader_read_until_end(&br, &arena);
    ASSERT(0 == res_io.err);
    ASSERT(string_eq(value, res_io.res));

    close(fd_pipe[0]);
  }
}
#endif

static void test_url_parse() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  {
    ParseUrlResult res = url_parse(S(""), &arena);
    ASSERT(0 != res.err);
  }
  {
    ParseUrlResult res = url_parse(S("x"), &arena);
    ASSERT(0 != res.err);
  }
  {
    ParseUrlResult res = url_parse(S("http:"), &arena);
    ASSERT(0 != res.err);
  }
  {
    ParseUrlResult res = url_parse(S("http:/"), &arena);
    ASSERT(0 != res.err);
  }
  {
    ParseUrlResult res = url_parse(S("http://"), &arena);
    ASSERT(0 != res.err);
  }
  {
    ParseUrlResult res = url_parse(S("://"), &arena);
    ASSERT(0 != res.err);
  }
  {
    ParseUrlResult res = url_parse(S("http://a:"), &arena);
    ASSERT(0 == res.err);
    ASSERT(string_eq(S("http"), res.res.scheme));
    ASSERT(0 == res.res.username.len);
    ASSERT(0 == res.res.password.len);
    ASSERT(string_eq(S("a"), res.res.host));
    ASSERT(0 == res.res.path_components.len);
    ASSERT(0 == res.res.port);
  }
  {
    ParseUrlResult res = url_parse(S("http://a:/"), &arena);
    ASSERT(0 == res.err);
    ASSERT(string_eq(S("http"), res.res.scheme));
    ASSERT(0 == res.res.username.len);
    ASSERT(0 == res.res.password.len);
    ASSERT(string_eq(S("a"), res.res.host));
    ASSERT(0 == res.res.path_components.len);
    ASSERT(0 == res.res.port);
  }
  {
    ParseUrlResult res = url_parse(S("http://a:bc"), &arena);
    ASSERT(0 != res.err);
  }
  {
    ParseUrlResult res = url_parse(S("http://abc:0"), &arena);
    ASSERT(0 == res.err);
    ASSERT(string_eq(S("http"), res.res.scheme));
    ASSERT(0 == res.res.username.len);
    ASSERT(0 == res.res.password.len);
    ASSERT(string_eq(S("abc"), res.res.host));
    ASSERT(0 == res.res.path_components.len);
    ASSERT(0 == res.res.port);
  }
  {
    ParseUrlResult res = url_parse(S("http://abc:999999"), &arena);
    ASSERT(0 != res.err);
  }

  // Invalid scheme.
  {
    ParseUrlResult res = url_parse(S("1abc://a:80/"), &arena);
    ASSERT(0 != res.err);
  }
  {
    ParseUrlResult res = url_parse(S("http://a:80"), &arena);
    ASSERT(0 == res.err);
    ASSERT(string_eq(S("http"), res.res.scheme));
    ASSERT(0 == res.res.username.len);
    ASSERT(0 == res.res.password.len);
    ASSERT(string_eq(S("a"), res.res.host));
    ASSERT(0 == res.res.path_components.len);
    ASSERT(80 == res.res.port);
  }
  {
    ParseUrlResult res = url_parse(S("http://a.b.c:80/foo"), &arena);
    ASSERT(0 == res.err);
    ASSERT(string_eq(S("http"), res.res.scheme));
    ASSERT(0 == res.res.username.len);
    ASSERT(0 == res.res.password.len);
    ASSERT(string_eq(S("a.b.c"), res.res.host));
    ASSERT(80 == res.res.port);
    ASSERT(1 == res.res.path_components.len);

    String path_component0 = slice_at(res.res.path_components, 0);
    ASSERT(string_eq(S("foo"), path_component0));
  }
  {
    ParseUrlResult res = url_parse(S("http://a.b.c:80/"), &arena);
    ASSERT(0 == res.err);
    ASSERT(string_eq(S("http"), res.res.scheme));
    ASSERT(0 == res.res.username.len);
    ASSERT(0 == res.res.password.len);
    ASSERT(string_eq(S("a.b.c"), res.res.host));
    ASSERT(80 == res.res.port);
    ASSERT(0 == res.res.path_components.len);
  }
  {
    ParseUrlResult res = url_parse(S("http://a.b.c/foo/bar/baz"), &arena);
    ASSERT(0 == res.err);
    ASSERT(string_eq(S("http"), res.res.scheme));
    ASSERT(0 == res.res.username.len);
    ASSERT(0 == res.res.password.len);
    ASSERT(string_eq(S("a.b.c"), res.res.host));
    ASSERT(0 == res.res.port);
    ASSERT(3 == res.res.path_components.len);

    String path_component0 = slice_at(res.res.path_components, 0);
    ASSERT(string_eq(S("foo"), path_component0));

    String path_component1 = slice_at(res.res.path_components, 1);
    ASSERT(string_eq(S("bar"), path_component1));

    String path_component2 = slice_at(res.res.path_components, 2);
    ASSERT(string_eq(S("baz"), path_component2));
  }
}

#if 0
static void test_read_http_request_without_body() {
  Arena arena = arena_make_from_virtual_mem(8 * KiB);

  String req_slice = S("GET /foo?bar=2 HTTP/1.1\r\nHost: "
                       "localhost:12345\r\nAccept: */*\r\n\r\n");
  TestMemReadContext ctx = {.s = string_dup(req_slice, &arena)};
  BufferedReader reader = test_buffered_reader_make(&ctx, &arena);
  const HttpRequest req = request_read(&reader, &arena);

  ASSERT(reader.buf_idx == req_slice.len); // Read all.
  ASSERT(0 == req.err);
  ASSERT(HM_GET == req.method);
  ASSERT(string_eq(req.path_raw, S("/foo?bar=2")));

  ASSERT(2 == req.headers.len);
  ASSERT(string_eq(dyn_at(req.headers, 0).value, S("localhost:12345")));
  ASSERT(string_eq(dyn_at(req.headers, 1).key, S("Accept")));
  ASSERT(string_eq(dyn_at(req.headers, 1).value, S("*/*")));
}
#endif

typedef enum {
  ALICE_STATE_NONE,
  ALICE_STATE_SENT_HELLO,
  ALICE_STATE_SENT_WORLD,
} AliceState;
static void test_net_socket() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  u16 port = 5678;
  Socket socket_bob = 0;
  {
    CreateSocketResult res_create_socket = net_create_tcp_socket();
    ASSERT(0 == res_create_socket.err);
    socket_bob = res_create_socket.res;

    ASSERT(0 == net_socket_enable_reuse(socket_bob));
    ASSERT(0 == net_socket_set_blocking(socket_bob, false));

    Ipv4Address addr = {0};
    addr.port = port;
    ASSERT(0 == net_tcp_bind_ipv4(socket_bob, addr));
    ASSERT(0 == net_tcp_listen(socket_bob));
  }

  Socket socket_alice = 0;
  {
    DnsResolveIpv4AddressSocketResult res_dns =
        net_dns_resolve_ipv4_tcp(S("localhost"), port, arena);
    ASSERT(0 == res_dns.err);

    ASSERT(port == res_dns.res.address.port);
    ASSERT(0 != res_dns.res.socket);
    socket_alice = res_dns.res.socket;
  }
  ASSERT(0 == net_socket_set_blocking(socket_alice, false));

  Writer writer_alice = writer_make_from_socket(socket_alice);

  AioQueueCreateResult res_queue_create = net_aio_queue_create();
  ASSERT(0 == res_queue_create.err);

  AioQueue queue = res_queue_create.res;
  AioEventSlice events = {0};
  events.len = 2;
  events.data = arena_new(&arena, AioEvent, 3);

  {
    AioEvent *event_bob_listen = slice_at_ptr(&events, 0);
    event_bob_listen->socket = socket_bob;
    event_bob_listen->kind = AIO_EVENT_KIND_IN;
    event_bob_listen->action = AIO_EVENT_ACTION_KIND_ADD;

    ASSERT(0 == net_aio_queue_ctl(queue, events));
  }

  Socket socket_bob_alice = 0;
  Reader reader_bob_alice = {0};
  reader_bob_alice.read_fn = unix_read;

  RingBuffer bob_recv = {0};
  String msg_expected = S("hello world!");
  bob_recv.data = string_make(msg_expected.len + 1, &arena);

  AliceState alice_state = ALICE_STATE_NONE;

  for (;;) {
    ASSERT(0 == net_aio_queue_wait(queue, events, -1, arena));

    for (u64 i = 0; i < events.len; i++) {
      AioEvent event = slice_at(events, i);
      if (event.socket == socket_bob) {
        ASSERT(0 == i); // Only one event thus far.

        Ipv4AddressAcceptResult res_accept = net_tcp_accept(socket_bob);
        ASSERT(0 == res_accept.err);
        ASSERT(0 != res_accept.socket);

        events.len = 3;
        slice_at_ptr(&events, i)->action =
            AIO_EVENT_ACTION_KIND_DEL; // Stop listening.

        AioEvent *event_alice = slice_at_ptr(&events, 1);
        event_alice->socket = socket_alice;
        event_alice->kind = AIO_EVENT_KIND_OUT;
        event_alice->action = AIO_EVENT_ACTION_KIND_ADD;

        socket_bob_alice = res_accept.socket;
        reader_bob_alice.ctx = (void *)(u64)socket_bob_alice;

        AioEvent *event_bob_alice = slice_at_ptr(&events, 2);
        event_bob_alice->socket = res_accept.socket;
        event_bob_alice->kind = AIO_EVENT_KIND_IN;
        event_bob_alice->action = AIO_EVENT_ACTION_KIND_ADD;

        ASSERT(0 == net_aio_queue_ctl(queue, events));
        ASSERT(0 == net_socket_close(socket_bob)); // Stop listening.
        socket_bob = 0;
      } else if (event.socket == socket_alice) {
        ASSERT(AIO_EVENT_KIND_OUT == event.kind);

        switch (alice_state) {
        case ALICE_STATE_NONE: {
          ASSERT(msg_expected.len ==
                 ring_buffer_write_space(
                     bob_recv)); // Nothing received by bob yet.
          ASSERT(0 == writer_write_all_sync(&writer_alice, S("hello")));
          alice_state = ALICE_STATE_SENT_HELLO;
        } break;
        case ALICE_STATE_SENT_HELLO: {
          ASSERT(0 == writer_write_all_sync(&writer_alice, S(" world!")));
          alice_state = ALICE_STATE_SENT_WORLD;
        } break;
        case ALICE_STATE_SENT_WORLD:
          break;
        default:
          ASSERT(0);
        }
      } else if (event.socket == socket_bob_alice) {
        ASSERT(AIO_EVENT_KIND_IN == event.kind);

        if (0 == ring_buffer_write_space(bob_recv)) {
          goto end; // End of test.
        }

        String recv = string_make(ring_buffer_write_space(bob_recv), &arena);

        IoCountResult res_io_count =
            reader_bob_alice.read_fn(reader_bob_alice.ctx, recv.data, recv.len);
        ASSERT(0 == res_io_count.err);
        recv.len = res_io_count.res;
        ASSERT(true == ring_buffer_write_slice(&bob_recv, recv));
      }
    }
  }

end:
  ASSERT(ALICE_STATE_SENT_WORLD == alice_state);

  String msg_bob_received = string_make(msg_expected.len, &arena);
  ASSERT(true == ring_buffer_read_slice(&bob_recv, msg_bob_received));
  ASSERT(string_eq(msg_bob_received, msg_expected));
  ASSERT(0 == net_socket_close(socket_alice));
  ASSERT(0 == net_socket_close(socket_bob_alice));
}

static void test_url_parse_relative_path() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  // Empty.
  {
    DynStringResult res = url_parse_path_components(S(""), &arena);
    ASSERT(0 == res.err);
    ASSERT(0 == res.res.len);
  }
  // Forbidden characters.
  {
    ASSERT(url_parse_path_components(S("/foo?bar"), &arena).err);
    ASSERT(url_parse_path_components(S("/foo:1234"), &arena).err);
    ASSERT(url_parse_path_components(S("/foo#bar"), &arena).err);
  }
  // Must start with slash and it does not.
  {
    DynStringResult res = url_parse_path_components(S("foo"), &arena);
    ASSERT(res.err);
  }
  // Must start with slash and it does.
  {
    DynStringResult res = url_parse_path_components(S("/foo"), &arena);
    ASSERT(0 == res.err);
    ASSERT(1 == res.res.len);
    ASSERT(string_eq(S("foo"), slice_at(res.res, 0)));
  }
  // Simple path with a few components.
  {
    DynStringResult res = url_parse_path_components(S("/foo/bar/baz"), &arena);
    ASSERT(0 == res.err);
    ASSERT(3 == res.res.len);
    ASSERT(string_eq(S("foo"), slice_at(res.res, 0)));
    ASSERT(string_eq(S("bar"), slice_at(res.res, 1)));
    ASSERT(string_eq(S("baz"), slice_at(res.res, 2)));
  }
  // Simple path with a few components with trailing slash.
  {
    DynStringResult res = url_parse_path_components(S("/foo/bar/baz/"), &arena);
    ASSERT(0 == res.err);
    ASSERT(3 == res.res.len);
    ASSERT(string_eq(S("foo"), slice_at(res.res, 0)));
    ASSERT(string_eq(S("bar"), slice_at(res.res, 1)));
    ASSERT(string_eq(S("baz"), slice_at(res.res, 2)));
  }
}

static void test_http_send_request() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);
  {
    HttpRequest req;
    req.method = HTTP_METHOD_GET;

    RingBuffer rg = {.data = string_make(32, &arena)};
    HttpIOState state = HTTP_IO_STATE_NONE;
    u64 header_idx = 0;

    http_write_request(&rg, &state, &header_idx, req, arena);

    ASSERT(HTTP_IO_STATE_DONE == state);
    ASSERT(0 == header_idx);

    String s = string_make(ring_buffer_read_space(rg), &arena);
    ASSERT(true == ring_buffer_read_slice(&rg, s));

    String expected = S("GET / HTTP/1.1\r\n"
                        "\r\n");
    ASSERT(string_eq(s, expected));
  }
  {
    HttpRequest req;
    req.method = HTTP_METHOD_POST;
    http_push_header(&req.headers, S("Host"), S("google.com"), &arena);
    *dyn_push(&req.url.path_components, &arena) = S("foobar");

    RingBuffer rg = {.data = string_make(32, &arena)};
    HttpIOState state = HTTP_IO_STATE_NONE;
    u64 header_idx = 0;

    http_write_request(&rg, &state, &header_idx, req, arena);
    ASSERT(HTTP_IO_STATE_AFTER_STATUS_LINE == state);
    ASSERT(0 == header_idx);

    {
      Arena tmp = arena;
      String s = string_make(ring_buffer_read_space(rg), &tmp);
      ASSERT(true == ring_buffer_read_slice(&rg, s));
      ASSERT(string_eq(s, S("POST /foobar HTTP/1.1\r\n")));
    }

    http_write_request(&rg, &state, &header_idx, req, arena);
    ASSERT(HTTP_IO_STATE_DONE == state);
    ASSERT(1 == header_idx);

    String s = string_make(ring_buffer_read_space(rg), &arena);
    ASSERT(true == ring_buffer_read_slice(&rg, s));

    String expected = S("Host: google.com\r\n"
                        "\r\n");
    ASSERT(string_eq(s, expected));
  }
}

static void test_http_parse_response_status_line() {
  // Empty.
  {
    ASSERT(http_parse_response_status_line(S("")).err);
  }
  // Missing prefix.
  {
    ASSERT(http_parse_response_status_line(S("HTT")).err);
    ASSERT(http_parse_response_status_line(S("abc")).err);
    ASSERT(http_parse_response_status_line(S("/1.1")).err);
  }
  // Missing slash.
  {
    ASSERT(http_parse_response_status_line(S("HTTP1.1 201 Created")).err);
  }
  // Missing major version.
  {
    ASSERT(http_parse_response_status_line(S("HTTP/.1 201 Created")).err);
  }
  // Missing `.`.
  {
    ASSERT(http_parse_response_status_line(S("HTTP/11 201 Created")).err);
  }
  // Missing minor version.
  {
    ASSERT(http_parse_response_status_line(S("HTTP/1. 201 Created")).err);
  }
  // Missing status code.
  {
    ASSERT(http_parse_response_status_line(S("HTTP/1.1 Created")).err);
  }
  // Invalid major version.
  {
    ASSERT(http_parse_response_status_line(S("HTTP/abc.1 201 Created")).err);
    ASSERT(http_parse_response_status_line(S("HTTP/4.1 201 Created")).err);
  }
  // Invalid minor version.
  {
    ASSERT(http_parse_response_status_line(S("HTTP/1.10 201 Created")).err);
  }
  // Invalid status code.
  {
    ASSERT(http_parse_response_status_line(S("HTTP/1.1 99 Created")).err);
    ASSERT(http_parse_response_status_line(S("HTTP/1.1 600 Created")).err);
  }
  // Valid, short.
  {
    HttpResponseStatusLineResult res =
        http_parse_response_status_line(S("HTTP/2.0 201"));
    ASSERT(0 == res.err);
    ASSERT(2 == res.res.version_major);
    ASSERT(0 == res.res.version_minor);
    ASSERT(201 == res.res.status);
  }
  // Valid, short, 0.9.
  {
    HttpResponseStatusLineResult res =
        http_parse_response_status_line(S("HTTP/0.9 201"));
    ASSERT(0 == res.err);
    ASSERT(0 == res.res.version_major);
    ASSERT(9 == res.res.version_minor);
    ASSERT(201 == res.res.status);
  }
  // Valid, long.
  {
    HttpResponseStatusLineResult res =
        http_parse_response_status_line(S("HTTP/1.1 404 Not found"));
    ASSERT(0 == res.err);
    ASSERT(1 == res.res.version_major);
    ASSERT(1 == res.res.version_minor);
    ASSERT(404 == res.res.status);
  }
}

static void test_http_parse_request_status_line() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  // Empty.
  {
    ASSERT(http_parse_request_status_line(S(""), &arena).err);
  }
  // Missing prefix.
  {
    ASSERT(http_parse_request_status_line(S("GE"), &arena).err);
    ASSERT(http_parse_request_status_line(S("abc"), &arena).err);
    ASSERT(http_parse_request_status_line(S("123 "), &arena).err);
  }
  // Missing slash.
  {
    ASSERT(http_parse_request_status_line(S("GET HTTP1.1"), &arena).err);
  }
  // Missing major version.
  {
    ASSERT(http_parse_request_status_line(S("GET / HTTP/.1"), &arena).err);
  }
  // Missing `.`.
  {
    ASSERT(http_parse_request_status_line(S("GET / HTTP/11"), &arena).err);
  }
  // Missing minor version.
  {
    ASSERT(http_parse_request_status_line(S("GET / HTTP/1."), &arena).err);
  }
  // Invalid major version.
  {
    ASSERT(http_parse_request_status_line(S("GET / HTTP/abc.1"), &arena).err);
    ASSERT(http_parse_request_status_line(S("GET / HTTP/4.1"), &arena).err);
  }
  // Invalid minor version.
  {
    ASSERT(http_parse_request_status_line(S("GET / HTTP/1.10"), &arena).err);
  }
  // Valid, short.
  {
    HttpRequestStatusLineResult res =
        http_parse_request_status_line(S("GET / HTTP/2.0"), &arena);
    ASSERT(0 == res.err);
    ASSERT(HTTP_METHOD_GET == res.res.method);
    ASSERT(2 == res.res.version_major);
    ASSERT(0 == res.res.version_minor);
    ASSERT(0 == res.res.url.path_components.len);
  }
  // Valid, short, 0.9.
  {
    HttpRequestStatusLineResult res =
        http_parse_request_status_line(S("GET / HTTP/0.9"), &arena);
    ASSERT(0 == res.err);
    ASSERT(HTTP_METHOD_GET == res.res.method);
    ASSERT(0 == res.res.version_major);
    ASSERT(9 == res.res.version_minor);
    ASSERT(0 == res.res.url.path_components.len);
  }
  // Valid, long.
  {
    HttpRequestStatusLineResult res =
        http_parse_request_status_line(S("GET /foo/bar/baz HTTP/1.1"), &arena);
    ASSERT(0 == res.err);
    ASSERT(HTTP_METHOD_GET == res.res.method);
    ASSERT(1 == res.res.version_major);
    ASSERT(1 == res.res.version_minor);
    ASSERT(3 == res.res.url.path_components.len);
  }
}

static void test_http_parse_header() {
  // Empty.
  {
    ASSERT(http_parse_header(S("")).err);
  }
  // Missing `:`.
  {
    ASSERT(http_parse_header(S("foo bar")).err);
  }
  // Missing key.
  {
    ASSERT(http_parse_header(S(":bcd")).err);
  }
  // Missing value.
  {
    ASSERT(http_parse_header(S("foo:")).err);
  }
  // Multiple colons.
  {
    KeyValueResult res = http_parse_header(S("foo: bar : baz"));
    ASSERT(0 == res.err);
    ASSERT(string_eq(res.res.key, S("foo")));
    ASSERT(string_eq(res.res.value, S("bar : baz")));
  }
  // Valid, one space before the value.
  {
    KeyValueResult res = http_parse_header(S("foo: bar"));
    ASSERT(0 == res.err);
    ASSERT(string_eq(res.res.key, S("foo")));
    ASSERT(string_eq(res.res.value, S("bar")));
  }
  // Valid, no space before the value.
  {
    KeyValueResult res = http_parse_header(S("foo:bar"));
    ASSERT(0 == res.err);
    ASSERT(string_eq(res.res.key, S("foo")));
    ASSERT(string_eq(res.res.value, S("bar")));
  }
  // Valid, multiple spaces before the value.
  {
    KeyValueResult res = http_parse_header(S("foo:   bar"));
    ASSERT(0 == res.err);
    ASSERT(string_eq(res.res.key, S("foo")));
    ASSERT(string_eq(res.res.value, S("bar")));
  }
}

static void test_http_read_response() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  // Empty.
  {
    RingBuffer rg = {.data = string_make(32, &arena)};
    HttpIOState state = HTTP_IO_STATE_NONE;
    HttpResponse res = {0};
    Error err = http_read_response(&rg, &state, &res, &arena);
    ASSERT(0 == err);
    ASSERT(HTTP_IO_STATE_NONE == state);
  }
  // Partial status line.
  {
    RingBuffer rg = {.data = string_make(32, &arena)};
    HttpIOState state = HTTP_IO_STATE_NONE;
    HttpResponse res = {0};

    ASSERT(true == ring_buffer_write_slice(&rg, S("HTTP/1.")));
    Error err = http_read_response(&rg, &state, &res, &arena);
    ASSERT(0 == err);
    ASSERT(HTTP_IO_STATE_NONE == state);
    ASSERT(ring_buffer_read_space(rg) == S("HTTP/1.").len);
  }
  // Status line and some.
  {
    RingBuffer rg = {.data = string_make(32, &arena)};
    HttpIOState state = HTTP_IO_STATE_NONE;
    HttpResponse res = {0};

    ASSERT(true ==
           ring_buffer_write_slice(&rg, S("HTTP/1.1 201 Created\r\nHost:")));
    Error err = http_read_response(&rg, &state, &res, &arena);
    ASSERT(0 == err);
    ASSERT(HTTP_IO_STATE_AFTER_STATUS_LINE == state);
    ASSERT(1 == res.version_major);
    ASSERT(1 == res.version_minor);
    ASSERT(201 == res.status);
  }

  // Full.
  {
    RingBuffer rg = {.data = string_make(32, &arena)};
    HttpIOState state = HTTP_IO_STATE_NONE;
    HttpResponse res = {0};

    {
      ASSERT(true ==
             ring_buffer_write_slice(&rg, S("HTTP/1.1 201 Created\r\nHost:")));
      Error err = http_read_response(&rg, &state, &res, &arena);
      ASSERT(0 == err);
      ASSERT(HTTP_IO_STATE_AFTER_STATUS_LINE == state);
      ASSERT(1 == res.version_major);
      ASSERT(1 == res.version_minor);
      ASSERT(201 == res.status);
    }

    {
      ASSERT(true == ring_buffer_write_slice(&rg, S("google.com\r")));
      Error err = http_read_response(&rg, &state, &res, &arena);
      ASSERT(0 == err);
      ASSERT(HTTP_IO_STATE_AFTER_STATUS_LINE == state);
      ASSERT(0 == res.headers.len);

      ASSERT(true == ring_buffer_write_slice(&rg, S("\n")));
      err = http_read_response(&rg, &state, &res, &arena);
      ASSERT(0 == err);
      ASSERT(HTTP_IO_STATE_AFTER_STATUS_LINE == state);
      ASSERT(1 == res.headers.len);
      KeyValue kv = slice_at(res.headers, 0);
      ASSERT(string_eq(kv.key, S("Host")));
      ASSERT(string_eq(kv.value, S("google.com")));
    }

    {
      ASSERT(true == ring_buffer_write_slice(
                         &rg, S("Authorization: Bearer foo\r\n\r\n")));
      Error err = http_read_response(&rg, &state, &res, &arena);
      ASSERT(0 == err);
      ASSERT(HTTP_IO_STATE_DONE == state);
      ASSERT(2 == res.headers.len);
      KeyValue kv = slice_at(res.headers, 1);
      ASSERT(string_eq(kv.key, S("Authorization")));
      ASSERT(string_eq(kv.value, S("Bearer foo")));
    }
  }
}

static void test_http_request_response() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  u16 port = 5678;
  Socket server_socket = 0;
  {
    CreateSocketResult res_create_socket = net_create_tcp_socket();
    ASSERT(0 == res_create_socket.err);
    server_socket = res_create_socket.res;

    ASSERT(0 == net_socket_enable_reuse(server_socket));
    ASSERT(0 == net_socket_set_blocking(server_socket, false));

    Ipv4Address addr = {0};
    addr.port = port;
    ASSERT(0 == net_tcp_bind_ipv4(server_socket, addr));
    ASSERT(0 == net_tcp_listen(server_socket));
  }

  Socket client_socket = 0;
  {
    DnsResolveIpv4AddressSocketResult res_dns =
        net_dns_resolve_ipv4_tcp(S("localhost"), port, arena);
    ASSERT(0 == res_dns.err);

    ASSERT(port == res_dns.res.address.port);
    ASSERT(0 != res_dns.res.socket);
    client_socket = res_dns.res.socket;
  }
  ASSERT(0 == net_socket_set_blocking(client_socket, false));

  AioQueueCreateResult res_queue_create = net_aio_queue_create();
  ASSERT(0 == res_queue_create.err);

  AioQueue queue = res_queue_create.res;
  AioEventSlice events = {0};
  events.len = 2;
  events.data = arena_new(&arena, AioEvent, 3);

  {
    AioEvent *event_server_listen = slice_at_ptr(&events, 0);
    event_server_listen->socket = server_socket;
    event_server_listen->kind = AIO_EVENT_KIND_IN;
    event_server_listen->action = AIO_EVENT_ACTION_KIND_ADD;

    ASSERT(0 == net_aio_queue_ctl(queue, events));
  }

  Socket server_client_socket = 0;
  Reader server_client_reader = {0};
  server_client_reader.read_fn = unix_read;

  //  Reader client_reader = reader_make_from_socket(client_socket);
  Writer client_writer = writer_make_from_socket(client_socket);

  // RingBuffer client_recv = {.data = string_make(32, &arena)};
  RingBuffer client_send = {.data = string_make(32, &arena)};
  //  RingBuffer server_recv = {.data = string_make(512, &arena)};
  // RingBuffer server_send = {.data = string_make(128, &arena)};

  // HttpIOState client_recv_http_io_state = 0;
  HttpIOState client_send_http_io_state = 0;
  //  HttpIOState server_recv_http_io_state = 0;
  // HttpIOState server_send_http_io_state = 0;

  u64 client_header_idx = 0;
  //  u64 server_header_idx = 0;

  HttpRequest client_req = {0};
  client_req.method = HTTP_METHOD_GET;
  http_push_header(&client_req.headers, S("Host"), S("localhost"), &arena);

  HttpResponse server_res = {0};
  server_res.status = 200;
  server_res.version_major = 1;
  server_res.version_minor = 1;
  http_push_header(&server_res.headers, S("Accept"), S("application/json"),
                   &arena);

  for (;;) {
    ASSERT(0 == net_aio_queue_wait(queue, events, -1, arena));

    for (u64 i = 0; i < events.len; i++) {
      AioEvent event = slice_at(events, i);
      if (event.socket == server_socket) {
        ASSERT(0 == i); // Only one event thus far.

        Ipv4AddressAcceptResult res_accept = net_tcp_accept(server_socket);
        ASSERT(0 == res_accept.err);
        ASSERT(0 != res_accept.socket);

        events.len = 3;
        slice_at_ptr(&events, i)->action =
            AIO_EVENT_ACTION_KIND_DEL; // Stop listening.

        AioEvent *event_client = slice_at_ptr(&events, 1);
        event_client->socket = client_socket;
        event_client->kind = AIO_EVENT_KIND_OUT;
        event_client->action = AIO_EVENT_ACTION_KIND_ADD;

        server_client_socket = res_accept.socket;
        server_client_reader.ctx = (void *)(u64)server_client_socket;

        AioEvent *event_server_client = slice_at_ptr(&events, 2);
        event_server_client->socket = res_accept.socket;
        event_server_client->kind = AIO_EVENT_KIND_IN;
        event_server_client->action = AIO_EVENT_ACTION_KIND_ADD;

        ASSERT(0 == net_aio_queue_ctl(queue, events));
        ASSERT(0 == net_socket_close(server_socket)); // Stop listening.
        server_socket = 0;
      } else if (event.socket == client_socket) {
        if (HTTP_IO_STATE_DONE != client_send_http_io_state) {
          http_write_request(&client_send, &client_send_http_io_state,
                             &client_header_idx, client_req, arena);
          ASSERT(0 == writer_write(&client_writer, &client_send, arena).err);
        } else {
          goto end; // TODO.
        }
      } else if (event.socket == server_client_socket) {
        ASSERT(AIO_EVENT_KIND_IN == event.kind);
      }
    }
  }

end:
  // String msg_server_received = string_make(msg_expected.len, &arena);
  // ASSERT(true == ring_buffer_read_slice(&server_recv, msg_server_received));
  // ASSERT(string_eq(msg_server_received, msg_expected));
  ASSERT(0 == net_socket_close(client_socket));
  ASSERT(0 == net_socket_close(server_client_socket));
}

int main() {
  test_slice_range();
  test_string_indexof_slice();
  test_string_trim();
  test_string_split();
  test_dyn_ensure_cap();
  test_string_consume();
  test_string_parse_u64();
  test_string_cmp();
  test_sha1();
  test_slice_swap_remove();
  test_dynu8_append_u8_hex_upper();
  test_ipv4_address_to_string();
  test_url_encode();
  test_string_indexof_any_byte();
  test_log_entry_quote_value();
  test_make_log_line();
  test_u8x4_be_to_u32_and_back();
  test_bitfield();
#if 0
  test_buffered_reader_read_exactly();
  test_buffered_reader_read_until_end();
  test_buffered_reader_read_until_slice();
#endif
  test_ring_buffer_write_slice();
  test_ring_buffer_read_write_slice();
  test_ring_buffer_read_until_excl();
  test_ring_buffer_read_write_fuzz();
  test_net_socket();
  test_url_parse_relative_path();
  test_url_parse();
  test_http_send_request();
  test_http_parse_response_status_line();
  test_http_parse_request_status_line();
  test_http_parse_header();
  test_http_read_response();
  test_http_request_response();
}
