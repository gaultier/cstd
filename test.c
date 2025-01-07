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

  DynString dyn = {0};
  // Works on empty slices.
  (void)slice_range(dyn_slice(StringSlice, dyn), 0, 0);

  *dyn_push(&dyn, &arena) = S("hello \"world\n\"!");
  *dyn_push(&dyn, &arena) = S("日");
  *dyn_push(&dyn, &arena) = S("本語");

  StringSlice s = dyn_slice(StringSlice, dyn);
  StringSlice range = slice_range(s, 1, 0);
  ASSERT(2 == range.len);

  ASSERT(string_eq(slice_at(s, 1), slice_at(range, 0)));
  ASSERT(string_eq(slice_at(s, 2), slice_at(range, 1)));
}

static void test_string_consume() {
  {
    StringConsumeResult res = string_consume(S(""), '{');
    ASSERT(!res.consumed);
  }
  {
    StringConsumeResult res = string_consume(S("[1,2]"), '{');
    ASSERT(!res.consumed);
  }
  {
    StringConsumeResult res = string_consume(S("[1,2]"), '[');
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
    RingBuffer rg = {
        .data.data = arena_alloc(&arena, 1, 1, 12),
        .data.len = 12,
    };
    ASSERT(ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(5 == rg.idx_write);

    ASSERT(false == ring_buffer_write_slice(&rg, S(" world!")));
    ASSERT(5 == rg.idx_write);

    ASSERT(true == ring_buffer_write_slice(&rg, S(" world")));
    ASSERT(11 == rg.idx_write);

    ASSERT(0 == rg.idx_read);
    ASSERT(string_eq(S("hello world"),
                     (String){.data = rg.data.data, .len = rg.idx_write}));
  }
  // Write to full ring buffer.
  {
    RingBuffer rg = {
        .data.data = arena_alloc(&arena, 1, 1, 12),
        .data.len = 12,
        .idx_write = 1,
        .idx_read = 2,
    };
    ASSERT(false == ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(1 == rg.idx_write);
  }
  // Write to ring buffer, easy case.
  {
    RingBuffer rg = {
        .data.data = arena_alloc(&arena, 1, 1, 12),
        .data.len = 12,
        .idx_read = 1,
        .idx_write = 2,
    };
    ASSERT(ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(2 + 5 == rg.idx_write);
  }

  // Write to ring buffer, hard case.
  {
    RingBuffer rg = {
        .data.data = arena_alloc(&arena, 1, 1, 12),
        .data.len = 12,
        .idx_read = 2,
        .idx_write = 3,
    };
    ASSERT(ring_buffer_write_slice(&rg, S("hello worl")));
    ASSERT(1 == rg.idx_write);
    ASSERT(string_eq(rg.data, S("l\x0\x0hello wor")));
  }
}

static void test_ring_buffer_read_write_slice() {
  Arena arena = arena_make_from_virtual_mem(4 * KiB);

  // Write to empty ring buffer.
  {
    RingBuffer rg = {
        .data.data = arena_alloc(&arena, 1, 1, 12),
        .data.len = 12,
    };
    ASSERT(ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(5 == rg.idx_write);

    String dst = string_dup(S("xyz"), &arena);
    ASSERT(ring_buffer_read_slice(&rg, dst));
    ASSERT(string_eq(dst, S("hel")));
    ASSERT(3 == rg.idx_read);
    ASSERT(string_eq(rg.data, S("\x0\x0\x0lo\x0\x0\x0\x0\x0\x0\x0")));

#if 0
    ASSERT(false == ring_buffer_write_slice(&rg, S(" world!")));
    ASSERT(5 == rg.idx_write);

    ASSERT(true == ring_buffer_write_slice(&rg, S(" world")));
    ASSERT(11 == rg.idx_write);

    ASSERT(0 == rg.idx_read);
    ASSERT(string_eq(S("hello world"),
                     (String){.data = rg.data.data, .len = rg.idx_write}));
#endif
  }
#if 0
  // Write to full ring buffer.
  {
    RingBuffer rg = {
        .data.data = arena_alloc(&arena, 1, 1, 12),
        .data.len = 12,
        .idx_write = 1,
        .idx_read = 2,
    };
    ASSERT(false == ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(1 == rg.idx_write);
  }
  // Write to ring buffer, easy case.
  {
    RingBuffer rg = {
        .data.data = arena_alloc(&arena, 1, 1, 12),
        .data.len = 12,
        .idx_read = 1,
        .idx_write = 2,
    };
    ASSERT(ring_buffer_write_slice(&rg, S("hello")));
    ASSERT(2 + 5 == rg.idx_write);
  }

  // Write to ring buffer, hard case.
  {
    RingBuffer rg = {
        .data.data = arena_alloc(&arena, 1, 1, 12),
        .data.len = 12,
        .idx_read = 2,
        .idx_write = 3,
    };
    ASSERT(ring_buffer_write_slice(&rg, S("hello worl")));
    ASSERT(1 == rg.idx_write);
    ASSERT(string_eq(rg.data, S("l\x0\x0hello wor")));
  }
#endif
}

#if 0
static void test_buffered_reader_read_exactly() {
  Arena arena = arena_make_from_virtual_mem(16 * KiB);

  // Read from closed remote end: error.
  {
    int fd_pipe[2] = {0};
    ASSERT(-1 != pipe(fd_pipe));
    close(fd_pipe[1]);

    BufferedReader br = buffered_reader_make(fd_pipe[0], &arena);
    IoResult res_io = buffered_reader_read_exactly(&br, 128, &arena);
    ASSERT((Error)EOF == res_io.err);

    close(fd_pipe[0]);
  }

  {
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
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("x"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http:"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http:/"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http://"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("://"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http://a:"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http://a:/"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http://a:bc"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http://abc:0"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http://abc:999999"), &arena);
    ASSERT(res.err != 0);
  }
  {
    ParseUrlResult res = url_parse(S("http://a:80"), &arena);
    ASSERT(res.err != 0);
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

int main() {
  test_string_indexof_slice();
  test_string_trim();
  test_string_split();
  test_dyn_ensure_cap();
  test_slice_range();
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
  test_buffered_reader_read_until_slice();
  test_buffered_reader_read_until_end();
  test_read_http_request_without_body();
#endif
  test_url_parse();
  test_ring_buffer_write_slice();
  test_ring_buffer_read_write_slice();
}
