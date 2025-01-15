#include "lib.c"

static void test_string_indexof_slice() {
  // Empty haystack.
  {
    PG_ASSERT(-1 == pg_string_indexof_string((PgString){0}, PG_S("fox")));
  }

  // Empty needle.
  {
    PG_ASSERT(-1 == pg_string_indexof_string(PG_S("hello"), (PgString){0}));
  }

  // Not found.
  {
    PG_ASSERT(-1 ==
              pg_string_indexof_string(PG_S("hello world"), PG_S("foobar")));
  }

  // Found, one occurence.
  {
    PG_ASSERT(6 ==
              pg_string_indexof_string(PG_S("hello world"), PG_S("world")));
  }

  // Found, first occurence.
  {
    PG_ASSERT(6 ==
              pg_string_indexof_string(PG_S("world hello hell"), PG_S("hell")));
  }

  // Found, second occurence.
  {
    PG_ASSERT(10 ==
              pg_string_indexof_string(PG_S("hello fox foxy"), PG_S("foxy")));
  }

  // Almost found, prefix matches.
  {
    PG_ASSERT(-1 ==
              pg_string_indexof_string(PG_S("hello world"), PG_S("worldly")));
  }
}

static void test_string_trim() {
  PgString trimmed = pg_string_trim(PG_S("   foo "), ' ');
  PG_ASSERT(pg_string_eq(trimmed, PG_S("foo")));
}

static void test_string_split_byte() {
  PgString s = PG_S("hello..world...foobar");
  PgSplitIterator it = pg_string_split_string(s, PG_S("."));

  {
    PgStringOk elem = pg_string_split_next(&it);
    PG_ASSERT(true == elem.ok);
    PG_ASSERT(pg_string_eq(elem.res, PG_S("hello")));
  }

  {
    PgStringOk elem = pg_string_split_next(&it);
    PG_ASSERT(true == elem.ok);
    PG_ASSERT(pg_string_eq(elem.res, PG_S("world")));
  }

  {
    PgStringOk elem = pg_string_split_next(&it);
    PG_ASSERT(true == elem.ok);
    PG_ASSERT(pg_string_eq(elem.res, PG_S("foobar")));
  }

  PG_ASSERT(false == pg_string_split_next(&it).ok);
  PG_ASSERT(false == pg_string_split_next(&it).ok);
}

static void test_string_split_string() {
  PgString s = PG_S("hello, world,little, thing !");
  PgSplitIterator it = pg_string_split_string(s, PG_S(", "));

  {
    PgStringOk elem = pg_string_split_next(&it);
    PG_ASSERT(true == elem.ok);
    PG_ASSERT(pg_string_eq(elem.res, PG_S("hello")));
  }

  {
    PgStringOk elem = pg_string_split_next(&it);
    PG_ASSERT(true == elem.ok);
    PG_ASSERT(pg_string_eq(elem.res, PG_S("world,little")));
  }

  {
    PgStringOk elem = pg_string_split_next(&it);
    PG_ASSERT(true == elem.ok);
    PG_ASSERT(pg_string_eq(elem.res, PG_S("thing !")));
  }

  PG_ASSERT(false == pg_string_split_next(&it).ok);
  PG_ASSERT(false == pg_string_split_next(&it).ok);
}

static void test_dyn_ensure_cap() {
  u64 arena_cap = 4 * PG_KiB;

  // Trigger the optimization when the last allocation in the arena gets
  // extended.
  {
    Arena arena = arena_make_from_virtual_mem(arena_cap);

    Pgu8Dyn dyn = {0};
    *dyn_push(&dyn, &arena) = 1;
    PG_ASSERT(1 == dyn.len);
    PG_ASSERT(2 == dyn.cap);

    u64 arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(2 == arena_size_expected);
    PG_ASSERT(dyn.cap == arena_size_expected);

    u64 desired_cap = 13;
    dyn_ensure_cap(&dyn, desired_cap, &arena);
    PG_ASSERT(16 == dyn.cap);
    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(16 == arena_size_expected);
  }
  // General case.
  {
    Arena arena = arena_make_from_virtual_mem(arena_cap);

    Pgu8Dyn dyn = {0};
    *dyn_push(&dyn, &arena) = 1;
    PG_ASSERT(1 == dyn.len);
    PG_ASSERT(2 == dyn.cap);

    Pgu8Dyn dummy = {0};
    *dyn_push(&dummy, &arena) = 2;
    *dyn_push(&dummy, &arena) = 3;

    u64 arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(2 + 2 == arena_size_expected);

    // This triggers a new allocation.
    *dyn_push(&dummy, &arena) = 4;
    PG_ASSERT(3 == dummy.len);
    PG_ASSERT(4 == dummy.cap);

    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(2 + 4 == arena_size_expected);

    u64 desired_cap = 13;
    dyn_ensure_cap(&dyn, desired_cap, &arena);
    PG_ASSERT(16 == dyn.cap);

    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(16 + 6 == arena_size_expected);
  }
}

static void test_slice_range() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  // Empty slice.
  {
    PgString s = PG_S("");
    PG_ASSERT(PG_SLICE_IS_EMPTY(PG_SLICE_RANGE(s, 0, 0)));
    PG_ASSERT(PG_SLICE_IS_EMPTY(PG_SLICE_RANGE_START(s, 0)));
  }
  // Empty range.
  {
    PgString s = PG_S("a");
    PG_ASSERT(PG_SLICE_IS_EMPTY(PG_SLICE_RANGE(s, 0, 0)));
    PG_ASSERT(pg_string_eq(s, PG_SLICE_RANGE_START(s, 0)));
  }

  {
    PgString s = PG_S("abcd");
    PG_ASSERT(pg_string_eq(PG_SLICE_RANGE_START(s, 1), PG_S("bcd")));
    PG_ASSERT(pg_string_eq(PG_SLICE_RANGE(s, 1, 3), PG_S("bc")));
    PG_ASSERT(pg_string_eq(PG_SLICE_RANGE(s, 1, 4), PG_S("bcd")));
    PG_ASSERT(pg_string_eq(PG_SLICE_RANGE(s, 1, 5), PG_S("bcd")));
    PG_ASSERT(PG_SLICE_IS_EMPTY(PG_SLICE_RANGE(s, 100, 300)));
  }

  PgStringDyn dyn = {0};
  // Works on empty slices.
  (void)PG_SLICE_RANGE(dyn_slice(PgStringSlice, dyn), 0, 0);

  *dyn_push(&dyn, &arena) = PG_S("hello \"world\n\"!");
  *dyn_push(&dyn, &arena) = PG_S("日");
  *dyn_push(&dyn, &arena) = PG_S("本語");

  PgStringSlice s = dyn_slice(PgStringSlice, dyn);
  PgStringSlice range = PG_SLICE_RANGE_START(s, 1UL);
  PG_ASSERT(2 == range.len);

  PG_ASSERT(pg_string_eq(PG_SLICE_AT(s, 1), PG_SLICE_AT(range, 0)));
  PG_ASSERT(pg_string_eq(PG_SLICE_AT(s, 2), PG_SLICE_AT(range, 1)));
}

static void test_string_consume() {
  {
    PgStringOk res = pg_string_consume_byte(PG_S(""), '{');
    PG_ASSERT(!res.ok);
  }
  {
    PgStringOk res = pg_string_consume_byte(PG_S("[1,2]"), '{');
    PG_ASSERT(!res.ok);
  }
  {
    PgStringOk res = pg_string_consume_byte(PG_S("[1,2]"), '[');
    PG_ASSERT(res.ok);
    PG_ASSERT(pg_string_eq(PG_S("1,2]"), res.res));
  }
}

static void test_string_parse_u64() {
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S(""));
    PG_ASSERT(!num_res.present);
    PG_ASSERT(0 == num_res.remaining.len);
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("a"));
    PG_ASSERT(!num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("a"), num_res.remaining));
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("a123b"));
    PG_ASSERT(!num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("a123b"), num_res.remaining));
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("0123"));
    PG_ASSERT(!num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("0123"), num_res.remaining));
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("0"));
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S(""), num_res.remaining));
    PG_ASSERT(0 == num_res.n);
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("0a"));
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("a"), num_res.remaining));
    PG_ASSERT(0 == num_res.n);
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("123a"));
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("a"), num_res.remaining));
    PG_ASSERT(123 == num_res.n);
  }
}

static void test_string_cmp() {
  {
    StringCompare cmp = pg_string_cmp(PG_S("a"), PG_S("aa"));
    PG_ASSERT(STRING_CMP_LESS == cmp);
  }
  {
    StringCompare cmp = pg_string_cmp(PG_S(""), PG_S("a"));
    PG_ASSERT(STRING_CMP_LESS == cmp);
  }
  {
    StringCompare cmp = pg_string_cmp(PG_S(""), PG_S(""));
    PG_ASSERT(STRING_CMP_EQ == cmp);
  }
  {
    StringCompare cmp = pg_string_cmp(PG_S("a"), PG_S("a"));
    PG_ASSERT(STRING_CMP_EQ == cmp);
  }
  {
    StringCompare cmp = pg_string_cmp(PG_S("a"), PG_S("b"));
    PG_ASSERT(STRING_CMP_LESS == cmp);
  }
  {
    StringCompare cmp = pg_string_cmp(PG_S("b"), PG_S("aa"));
    PG_ASSERT(STRING_CMP_GREATER == cmp);
  }
  {
    StringCompare cmp = pg_string_cmp(PG_S("b"), PG_S("a"));
    PG_ASSERT(STRING_CMP_GREATER == cmp);
  }
  {
    StringCompare cmp = pg_string_cmp(PG_S("announce"), PG_S("comment"));
    PG_ASSERT(STRING_CMP_LESS == cmp);
  }
}

static void test_sha1() {
  {
    u8 hash[20] = {0};
    sha1(PG_S("abc"), hash);

    u8 expected_hash[20] = {
        0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E,
        0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D,
    };
    static_assert(sizeof(hash) == sizeof(expected_hash));

    PG_ASSERT(0 == memcmp(hash, expected_hash, sizeof(hash)));
  }
}

static void test_slice_swap_remove() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);
  {
    PgString s = pg_string_dup(PG_S("hello world!"), &arena);
    PG_SLICE_SWAP_REMOVE(&s, 4);
    PG_ASSERT(pg_string_eq(s, PG_S("hell! world")));
  }
}

static void test_dynu8_append_u8_hex_upper() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  {
    Pgu8Dyn sb = {0};
    dynu8_append_u8_hex_upper(&sb, 0xac, &arena);
    dynu8_append_u8_hex_upper(&sb, 0x89, &arena);

    PgString s = dyn_slice(PgString, sb);
    PG_ASSERT(pg_string_eq(s, PG_S("AC89")));
  }
}

static void test_ipv4_address_to_string() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);
  {
    Ipv4Address address = {
        .ip = (192UL << 24) | (168UL << 16) | (1UL << 8) | (56UL << 0),
        .port = 6881,
    };

    PgString s = ipv4_address_to_string(address, &arena);
    PG_ASSERT(pg_string_eq(s, PG_S("192.168.1.56:6881")));
  }
}

static void test_url_encode() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);
  {
    Pgu8Dyn sb = {0};
    url_encode_string(&sb, PG_S("日本語"), PG_S("123"), &arena);
    PgString encoded = dyn_slice(PgString, sb);

    PG_ASSERT(pg_string_eq(encoded, PG_S("%E6%97%A5%E6%9C%AC%E8%AA%9E=123")));
  }

  {
    Pgu8Dyn sb = {0};
    url_encode_string(&sb, PG_S("日本語"), PG_S("foo"), &arena);
    PgString encoded = dyn_slice(PgString, sb);

    PG_ASSERT(pg_string_eq(encoded, PG_S("%E6%97%A5%E6%9C%AC%E8%AA%9E=foo")));
  }
}

static void test_string_indexof_any_byte() {
  {
    i64 idx = pg_string_indexof_any_byte(PG_S(""), PG_S(""));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_indexof_any_byte(PG_S(""), PG_S(":"));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_indexof_any_byte(PG_S("?"), PG_S(":"));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_indexof_any_byte(PG_S("abc"), PG_S(":?"));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_indexof_any_byte(PG_S("x"), PG_S("yz"));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_indexof_any_byte(PG_S(":"), PG_S(":"));
    PG_ASSERT(0 == idx);
  }
  {
    i64 idx = pg_string_indexof_any_byte(PG_S("abc"), PG_S("cd"));
    PG_ASSERT(2 == idx);
  }
}

static void test_log_entry_quote_value() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);
  // Nothing to escape.
  {
    PgString s = PG_S("hello");
    PgString expected = PG_S("\"hello\"");
    PG_ASSERT(pg_string_eq(expected, json_escape_string(s, &arena)));
  }
  {
    PgString s = PG_S("{\"id\": 1}");
    PgString expected = PG_S("\"{\\\"id\\\": 1}\"");
    PG_ASSERT(pg_string_eq(expected, json_escape_string(s, &arena)));
  }
  {
    PgString s = PG_S("192.168.1.2:12345");
    PgString expected = PG_S("\"192.168.1.2:12345\"");
    PG_ASSERT(pg_string_eq(expected, json_escape_string(s, &arena)));
  }
  {
    u8 backslash = 0x5c;
    u8 double_quote = '"';
    u8 data[] = {backslash, double_quote};
    PgString s = {.data = data, .len = sizeof(data)};

    u8 data_expected[] = {double_quote, backslash,    backslash,
                          backslash,    double_quote, double_quote};
    PgString expected = {.data = data_expected, .len = sizeof(data_expected)};
    PG_ASSERT(pg_string_eq(expected, json_escape_string(s, &arena)));
  }
}

static void test_make_log_line() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  PgString log_line =
      log_make_log_line(LOG_LEVEL_DEBUG, PG_S("foobar"), &arena, 2,
                        L("num", 42), L("s", PG_S("hello \"world\"")));

  PgString expected_suffix = PG_S(
      "\"message\":\"foobar\",\"num\":42,\"s\":\"hello \\\"world\\\"\"}\n");
  PG_ASSERT(pg_string_starts_with(
      log_line, PG_S("{\"level\":\"debug\",\"timestamp_ns\":")));
  PG_ASSERT(pg_string_ends_with(log_line, expected_suffix));
}

static void test_u8x4_be_to_u32_and_back() {
  {
    u32 n = 123'456'789;
    u8 data[sizeof(n)] = {0};
    PgString s = {.data = data, .len = sizeof(n)};
    u32_to_u8x4_be(n, &s);
    PG_ASSERT(pg_string_eq(s, PG_S("\x07"
                                   "\x5b"
                                   "\x0cd"
                                   "\x15")));

    PG_ASSERT(n == u8x4_be_to_u32(s));
  }
}

static void test_bitfield() {
  {
    PgString bitfield = PG_S("\x3"
                             "\x2");
    PG_ASSERT(bitfield_get(bitfield, 0));
    PG_ASSERT(bitfield_get(bitfield, 1));
    PG_ASSERT(!bitfield_get(bitfield, 2));
    PG_ASSERT(!bitfield_get(bitfield, 3));
    PG_ASSERT(!bitfield_get(bitfield, 4));
    PG_ASSERT(!bitfield_get(bitfield, 5));
    PG_ASSERT(!bitfield_get(bitfield, 6));
    PG_ASSERT(!bitfield_get(bitfield, 7));
    PG_ASSERT(!bitfield_get(bitfield, 8));
    PG_ASSERT(bitfield_get(bitfield, 9));
    PG_ASSERT(!bitfield_get(bitfield, 10));
    PG_ASSERT(!bitfield_get(bitfield, 11));
    PG_ASSERT(!bitfield_get(bitfield, 12));
    PG_ASSERT(!bitfield_get(bitfield, 13));
    PG_ASSERT(!bitfield_get(bitfield, 14));
    PG_ASSERT(!bitfield_get(bitfield, 15));
  }
}

static void test_ring_buffer_write_slice() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  // Write to empty ring buffer.
  {
    RingBuffer rg = {.data = pg_string_make(12, &arena)};
    PG_ASSERT(ring_buffer_write_space(rg) == rg.data.len - 1);
    PG_ASSERT(ring_buffer_write_slice(&rg, PG_S("hello")));
    PG_ASSERT(5 == rg.idx_write);
    PG_ASSERT(ring_buffer_write_space(rg) == rg.data.len - 1 - 5);

    PG_ASSERT(false == ring_buffer_write_slice(&rg, PG_S(" world!")));
    PG_ASSERT(5 == rg.idx_write);
    PG_ASSERT(ring_buffer_write_space(rg) == rg.data.len - 1 - 5);

    PG_ASSERT(true == ring_buffer_write_slice(&rg, PG_S(" world")));
    PG_ASSERT(11 == rg.idx_write);
    PG_ASSERT(ring_buffer_write_space(rg) == 0);

    PG_ASSERT(0 == rg.idx_read);
    PG_ASSERT(
        pg_string_eq(PG_S("hello world"),
                     (PgString){.data = rg.data.data, .len = rg.idx_write}));
  }
  // Write to full ring buffer.
  {
    RingBuffer rg = {
        .data = pg_string_make(12, &arena),
        .idx_write = 1,
        .idx_read = 2,
    };
    PG_ASSERT(ring_buffer_write_space(rg) == 0);
    PG_ASSERT(false == ring_buffer_write_slice(&rg, PG_S("hello")));
    PG_ASSERT(1 == rg.idx_write);
    PG_ASSERT(ring_buffer_write_space(rg) == 0);
  }
  // Write to ring buffer, easy case.
  {
    RingBuffer rg = {
        .data = pg_string_make(12, &arena),
        .idx_read = 1,
        .idx_write = 2,
    };
    PG_ASSERT(ring_buffer_write_space(rg) == 10);
    PG_ASSERT(ring_buffer_write_slice(&rg, PG_S("hello")));
    PG_ASSERT(2 + 5 == rg.idx_write);
    PG_ASSERT(ring_buffer_write_space(rg) == 5);
  }

  // Write to ring buffer, hard case.
  {
    RingBuffer rg = {
        .data = pg_string_make(12, &arena),
        .idx_read = 2,
        .idx_write = 3,
    };
    PG_ASSERT(ring_buffer_write_space(rg) == 10);
    PG_ASSERT(ring_buffer_write_slice(&rg, PG_S("hello worl")));
    PG_ASSERT(1 == rg.idx_write);
    PG_ASSERT(ring_buffer_write_space(rg) == 0);
    PG_ASSERT(pg_string_eq(rg.data, PG_S("l\x0\x0hello wor")));
  }
}

static void test_ring_buffer_read_write_slice() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  // Read from an empty ring buffer.
  {
    RingBuffer rg = {.data = pg_string_make(12, &arena)};
    PG_ASSERT(0 == ring_buffer_read_space(rg));
    PG_ASSERT(true == ring_buffer_read_slice(&rg, (PgString){0}));

    PgString dst = pg_string_dup(PG_S("xyz"), &arena);
    PG_ASSERT(false == ring_buffer_read_slice(&rg, dst));
  }

  // Write to empty ring buffer, then read part of it.
  {
    RingBuffer rg = {.data = pg_string_make(12, &arena)};
    PG_ASSERT(ring_buffer_write_slice(&rg, PG_S("hello")));
    PG_ASSERT(5 == rg.idx_write);
    PG_ASSERT(5 == ring_buffer_read_space(rg));

    PgString dst = pg_string_dup(PG_S("xyz"), &arena);
    PG_ASSERT(ring_buffer_read_slice(&rg, dst));
    PG_ASSERT(pg_string_eq(dst, PG_S("hel")));
    PG_ASSERT(3 == rg.idx_read);
    PG_ASSERT(2 == ring_buffer_read_space(rg));

    PG_ASSERT(true == ring_buffer_write_slice(&rg, PG_S(" world!")));
    PG_ASSERT(0 == rg.idx_write);
    PG_ASSERT(9 == ring_buffer_read_space(rg));

    PG_ASSERT(false == ring_buffer_write_slice(&rg, PG_S("abc")));
    PG_ASSERT(9 == ring_buffer_read_space(rg));
    PG_ASSERT(true == ring_buffer_write_slice(&rg, PG_S("ab")));
    PG_ASSERT(11 == ring_buffer_read_space(rg));
    PG_ASSERT(2 == rg.idx_write);

    dst = pg_string_dup(PG_S("abcdefghijk"), &arena);
    PG_ASSERT(ring_buffer_read_slice(&rg, dst));
    PG_ASSERT(pg_string_eq(dst, PG_S("lo world!ab")));
    PG_ASSERT(2 == rg.idx_read);
    PG_ASSERT(0 == ring_buffer_read_space(rg));
  }
}

static void test_ring_buffer_read_until_excl() {
  Arena arena = arena_make_from_virtual_mem(8 * PG_KiB);
  RingBuffer rg = {.data = pg_string_make(4 * PG_KiB, &arena)};
  PG_ASSERT(ring_buffer_write_slice(
      &rg, PG_S("The quick brown fox jumps over the lazy dog")));

  {
    u64 space_read = ring_buffer_read_space(rg);
    u64 space_write = ring_buffer_write_space(rg);
    PgStringOk s = ring_buffer_read_until_excl(&rg, PG_S("\r\n"), &arena);
    PG_ASSERT(!s.ok);
    PG_ASSERT(PG_SLICE_IS_EMPTY(s.res));

    // Unmodified.
    PG_ASSERT(ring_buffer_read_space(rg) == space_read);
    PG_ASSERT(ring_buffer_write_space(rg) == space_write);
  }

  {
    PgStringOk s = ring_buffer_read_until_excl(&rg, PG_S(" "), &arena);
    PG_ASSERT(s.ok);
    PG_ASSERT(pg_string_eq(s.res, PG_S("The")));
  }
  {
    PgStringOk s = ring_buffer_read_until_excl(&rg, PG_S(" "), &arena);
    PG_ASSERT(s.ok);
    PG_ASSERT(pg_string_eq(s.res, PG_S("quick")));
  }
  {
    PgStringOk s = ring_buffer_read_until_excl(&rg, PG_S("lazy "), &arena);
    PG_ASSERT(s.ok);
    PG_ASSERT(pg_string_eq(s.res, PG_S("brown fox jumps over the ")));
  }
  {
    PgStringOk s = ring_buffer_read_until_excl(&rg, PG_S("g"), &arena);
    PG_ASSERT(s.ok);
    PG_ASSERT(pg_string_eq(s.res, PG_S("do")));
  }
  PG_ASSERT(0 == ring_buffer_read_space(rg));
}

static void test_ring_buffer_read_write_fuzz() {
  Arena arena_ring = arena_make_from_virtual_mem(4 * PG_KiB);
  RingBuffer rg = {.data = pg_string_make(4 * PG_KiB, &arena_ring)};

  u64 ROUNDS = 1024;
  Arena arena_strings = arena_make_from_virtual_mem(ROUNDS * 8 * PG_KiB);

  // TODO: Random seed for reproducability?
  for (u64 i = 0; i < ROUNDS; i++) {
    PgString from = pg_string_make(arc4random_uniform((u32)rg.data.len + 1),
                                   &arena_strings);
    arc4random_buf(from.data, from.len);

    PgString to = pg_string_make(arc4random_uniform((u32)rg.data.len + 1),
                                 &arena_strings);
    arc4random_buf(to.data, to.len);

    bool ok_write = ring_buffer_write_slice(&rg, from);
    (void)ok_write;
    bool ok_read = ring_buffer_read_slice(&rg, to);
    (void)ok_read;

    PG_ASSERT(ring_buffer_write_space(rg) <= rg.data.len - 1);
    PG_ASSERT(ring_buffer_read_space(rg) <= rg.data.len - 1);
  }
}

static void test_url_parse() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  {
    PgUrlResult res = url_parse(PG_S(""), &arena);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = url_parse(PG_S("x"), &arena);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = url_parse(PG_S("http:"), &arena);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = url_parse(PG_S("http:/"), &arena);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://"), &arena);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = url_parse(PG_S("://"), &arena);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.port);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a:"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.port);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a:/"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.port);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a:bc"), &arena);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://abc:0"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("abc"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.port);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://abc:999999"), &arena);
    PG_ASSERT(0 != res.err);
  }

  // Invalid scheme.
  {
    PgUrlResult res = url_parse(PG_S("1abc://a:80/"), &arena);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a:80"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(80 == res.res.port);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a.b.c:80/foo"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a.b.c"), res.res.host));
    PG_ASSERT(80 == res.res.port);
    PG_ASSERT(1 == res.res.path_components.len);

    PgString path_component0 = PG_SLICE_AT(res.res.path_components, 0);
    PG_ASSERT(pg_string_eq(PG_S("foo"), path_component0));
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a.b.c:80/"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a.b.c"), res.res.host));
    PG_ASSERT(80 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a.b.c/foo/bar/baz"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a.b.c"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(3 == res.res.path_components.len);

    PgString path_component0 = PG_SLICE_AT(res.res.path_components, 0);
    PG_ASSERT(pg_string_eq(PG_S("foo"), path_component0));

    PgString path_component1 = PG_SLICE_AT(res.res.path_components, 1);
    PG_ASSERT(pg_string_eq(PG_S("bar"), path_component1));

    PgString path_component2 = PG_SLICE_AT(res.res.path_components, 2);
    PG_ASSERT(pg_string_eq(PG_S("baz"), path_component2));
  }

  // Url parameters.
  {
    PgUrlResult res = url_parse(PG_S("http://a/?foo"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(1 == res.res.query_parameters.len);

    KeyValue kv0 = dyn_at(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("")));
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a/?"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.query_parameters.len);
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a/?foo=bar"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(1 == res.res.query_parameters.len);

    KeyValue kv0 = dyn_at(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a/?foo=bar&"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(1 == res.res.query_parameters.len);

    KeyValue kv0 = dyn_at(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  {
    PgUrlResult res = url_parse(PG_S("http://a/?foo=bar&hello=world"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(2 == res.res.query_parameters.len);

    KeyValue kv0 = dyn_at(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));

    KeyValue kv1 = dyn_at(res.res.query_parameters, 1);
    PG_ASSERT(pg_string_eq(kv1.key, PG_S("hello")));
    PG_ASSERT(pg_string_eq(kv1.value, PG_S("world")));
  }
  {
    PgUrlResult res =
        url_parse(PG_S("http://a/?foo=bar&hello=world&a="), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(3 == res.res.query_parameters.len);

    KeyValue kv0 = dyn_at(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));

    KeyValue kv1 = dyn_at(res.res.query_parameters, 1);
    PG_ASSERT(pg_string_eq(kv1.key, PG_S("hello")));
    PG_ASSERT(pg_string_eq(kv1.value, PG_S("world")));

    KeyValue kv2 = dyn_at(res.res.query_parameters, 2);
    PG_ASSERT(pg_string_eq(kv2.key, PG_S("a")));
    PG_ASSERT(pg_string_eq(kv2.value, PG_S("")));
  }
}

typedef enum {
  ALICE_STATE_NONE,
  ALICE_STATE_DONE,
} AliceState;
static void test_net_socket() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  u16 port = 5679;
  Socket socket_listen = 0;
  {
    PgCreateSocketResult res_create_socket = net_create_tcp_socket();
    PG_ASSERT(0 == res_create_socket.err);
    socket_listen = res_create_socket.res;

    PG_ASSERT(0 == net_socket_enable_reuse(socket_listen));
    PG_ASSERT(0 == net_socket_set_blocking(socket_listen, false));

    Ipv4Address addr = {0};
    addr.port = port;
    PG_ASSERT(0 == net_tcp_bind_ipv4(socket_listen, addr));
    PG_ASSERT(0 == net_tcp_listen(socket_listen));
  }

  Socket socket_alice = 0;
  {
    PgDnsResolveIpv4AddressSocketResult res_dns =
        net_dns_resolve_ipv4_tcp(PG_S("localhost"), port, arena);
    PG_ASSERT(0 == res_dns.err);

    PG_ASSERT(port == res_dns.res.address.port);
    PG_ASSERT(0 != res_dns.res.socket);
    socket_alice = res_dns.res.socket;
  }
  PG_ASSERT(0 == net_socket_set_blocking(socket_alice, false));

  PgAioQueueCreateResult res_queue_create = aio_queue_create();
  PG_ASSERT(0 == res_queue_create.err);

  AioQueue queue = res_queue_create.res;
  PgAioEventSlice events_change = PG_SLICE_MAKE(PgAioEvent, 3, &arena);
  events_change.len = 1;

  {
    PgAioEvent *event_bob_listen = PG_SLICE_AT_PTR(&events_change, 0);
    event_bob_listen->socket = socket_listen;
    event_bob_listen->kind = PG_AIO_EVENT_KIND_IN;
    event_bob_listen->action = PG_AIO_EVENT_ACTION_ADD;

    PG_ASSERT(0 == aio_queue_ctl(queue, events_change));
  }

  Socket bob_socket = 0;
  Reader bob_reader = {0};

  Writer alice_writer = writer_make_from_socket(socket_alice);

  RingBuffer bob_recv = {.data = pg_string_make(4 + 1, &arena)};
  RingBuffer alice_send = {.data = pg_string_make(4 + 1, &arena)};

  AliceState alice_state = ALICE_STATE_NONE;

  PgAioEventSlice events_watch = PG_SLICE_MAKE(PgAioEvent, 3, &arena);

  for (;;) {
    Pgu64Result res_wait = aio_queue_wait(queue, events_watch, -1, arena);
    PG_ASSERT(0 == res_wait.err);

    for (u64 i = 0; i < res_wait.res; i++) {
      PgAioEvent event = PG_SLICE_AT(events_watch, i);
      PG_ASSERT(0 == (PG_AIO_EVENT_KIND_ERR & event.kind));

      if (event.socket == socket_listen) {
        Ipv4AddressAcceptResult res_accept = net_tcp_accept(socket_listen);
        PG_ASSERT(0 == res_accept.err);
        PG_ASSERT(0 != res_accept.socket);

        events_change.len = 2;
        PgAioEvent *event_alice = PG_SLICE_AT_PTR(&events_change, 0);
        event_alice->socket = socket_alice;
        event_alice->kind = PG_AIO_EVENT_KIND_OUT;
        event_alice->action = PG_AIO_EVENT_ACTION_ADD;

        bob_socket = res_accept.socket;
        bob_reader = reader_make_from_socket(bob_socket);

        PgAioEvent *event_bob = PG_SLICE_AT_PTR(&events_change, 1);
        event_bob->socket = res_accept.socket;
        event_bob->kind = PG_AIO_EVENT_KIND_IN;
        event_bob->action = PG_AIO_EVENT_ACTION_ADD;

        PG_ASSERT(0 == aio_queue_ctl(queue, events_change));
        events_change.len = 0;
      } else if (event.socket == socket_alice) {
        PG_ASSERT(PG_AIO_EVENT_KIND_OUT & event.kind);

        switch (alice_state) {
        case ALICE_STATE_NONE: {
          PG_ASSERT(true == ring_buffer_write_slice(&alice_send, PG_S("ping")));
          PG_ASSERT(0 == writer_write(&alice_writer, &alice_send, arena).err);
          alice_state = ALICE_STATE_DONE;
        } break;
        case ALICE_STATE_DONE:
          break;
        default:
          PG_ASSERT(0);
        }
      } else if (event.socket == bob_socket) {
        PG_ASSERT(PG_AIO_EVENT_KIND_IN & event.kind);

        PG_ASSERT(0 == reader_read(&bob_reader, &bob_recv, arena).err);

        if (4 == ring_buffer_read_space(bob_recv)) {
          goto end; // End of test.
        }
      }
    }
  }

end:
  PG_ASSERT(ALICE_STATE_DONE == alice_state);

  PgString msg_bob_received = pg_string_make(4, &arena);
  PG_ASSERT(true == ring_buffer_read_slice(&bob_recv, msg_bob_received));
  PG_ASSERT(pg_string_eq(msg_bob_received, PG_S("ping")));

  PG_ASSERT(0 == net_socket_close(socket_alice));
  PG_ASSERT(0 == net_socket_close(bob_socket));
  PG_ASSERT(0 == net_socket_close(socket_listen));
}

static void test_url_parse_relative_path() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  // Empty.
  {
    PgStringDynResult res = url_parse_path_components(PG_S(""), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0 == res.res.len);
  }
  // Forbidden characters.
  {
    PG_ASSERT(url_parse_path_components(PG_S("/foo?bar"), &arena).err);
    PG_ASSERT(url_parse_path_components(PG_S("/foo:1234"), &arena).err);
    PG_ASSERT(url_parse_path_components(PG_S("/foo#bar"), &arena).err);
  }
  // Must start with slash and it does not.
  {
    PgStringDynResult res = url_parse_path_components(PG_S("foo"), &arena);
    PG_ASSERT(res.err);
  }
  // Must start with slash and it does.
  {
    PgStringDynResult res = url_parse_path_components(PG_S("/foo"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.res.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(res.res, 0)));
  }
  // Simple path with a few components.
  {
    PgStringDynResult res =
        url_parse_path_components(PG_S("/foo/bar/baz"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(3 == res.res.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(res.res, 0)));
    PG_ASSERT(pg_string_eq(PG_S("bar"), PG_SLICE_AT(res.res, 1)));
    PG_ASSERT(pg_string_eq(PG_S("baz"), PG_SLICE_AT(res.res, 2)));
  }
  // Simple path with a few components with trailing slash.
  {
    PgStringDynResult res =
        url_parse_path_components(PG_S("/foo/bar/baz/"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(3 == res.res.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(res.res, 0)));
    PG_ASSERT(pg_string_eq(PG_S("bar"), PG_SLICE_AT(res.res, 1)));
    PG_ASSERT(pg_string_eq(PG_S("baz"), PG_SLICE_AT(res.res, 2)));
  }
}

static void test_http_send_request() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);
  {
    HttpRequest req;
    req.method = HTTP_METHOD_GET;

    RingBuffer rg = {.data = pg_string_make(32, &arena)};

    PG_ASSERT(0 == http_write_request(&rg, req, arena));
    PgString s = pg_string_make(ring_buffer_read_space(rg), &arena);
    PG_ASSERT(true == ring_buffer_read_slice(&rg, s));

    PgString expected = PG_S("GET / HTTP/1.1\r\n"
                             "\r\n");
    PG_ASSERT(pg_string_eq(s, expected));
  }
  {
    HttpRequest req;
    req.method = HTTP_METHOD_POST;
    http_push_header(&req.headers, PG_S("Host"), PG_S("google.com"), &arena);
    *dyn_push(&req.url.path_components, &arena) = PG_S("foobar");

    {
      RingBuffer rg = {.data = pg_string_make(32, &arena)};
      PG_ASSERT(ENOMEM == http_write_request(&rg, req, arena));
    }

    RingBuffer rg = {.data = pg_string_make(128, &arena)};

    PG_ASSERT(0 == http_write_request(&rg, req, arena));

    PgString s = pg_string_make(ring_buffer_read_space(rg), &arena);
    PG_ASSERT(true == ring_buffer_read_slice(&rg, s));

    PgString expected = PG_S("POST /foobar HTTP/1.1\r\n"
                             "Host: google.com\r\n"
                             "\r\n");
    PG_ASSERT(pg_string_eq(s, expected));
  }
}

static void test_http_parse_response_status_line() {
  // Empty.
  {
    PG_ASSERT(http_parse_response_status_line(PG_S("")).err);
  }
  // Missing prefix.
  {
    PG_ASSERT(http_parse_response_status_line(PG_S("HTT")).err);
    PG_ASSERT(http_parse_response_status_line(PG_S("abc")).err);
    PG_ASSERT(http_parse_response_status_line(PG_S("/1.1")).err);
  }
  // Missing slash.
  {
    PG_ASSERT(http_parse_response_status_line(PG_S("HTTP1.1 201 Created")).err);
  }
  // Missing major version.
  {
    PG_ASSERT(http_parse_response_status_line(PG_S("HTTP/.1 201 Created")).err);
  }
  // Missing `.`.
  {
    PG_ASSERT(http_parse_response_status_line(PG_S("HTTP/11 201 Created")).err);
  }
  // Missing minor version.
  {
    PG_ASSERT(http_parse_response_status_line(PG_S("HTTP/1. 201 Created")).err);
  }
  // Missing status code.
  {
    PG_ASSERT(http_parse_response_status_line(PG_S("HTTP/1.1 Created")).err);
  }
  // Invalid major version.
  {
    PG_ASSERT(
        http_parse_response_status_line(PG_S("HTTP/abc.1 201 Created")).err);
    PG_ASSERT(
        http_parse_response_status_line(PG_S("HTTP/4.1 201 Created")).err);
  }
  // Invalid minor version.
  {
    PG_ASSERT(
        http_parse_response_status_line(PG_S("HTTP/1.10 201 Created")).err);
  }
  // Invalid status code.
  {
    PG_ASSERT(http_parse_response_status_line(PG_S("HTTP/1.1 99 Created")).err);
    PG_ASSERT(
        http_parse_response_status_line(PG_S("HTTP/1.1 600 Created")).err);
  }
  // Valid, short.
  {
    PgHttpResponseStatusLineResult res =
        http_parse_response_status_line(PG_S("HTTP/2.0 201"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(2 == res.res.version_major);
    PG_ASSERT(0 == res.res.version_minor);
    PG_ASSERT(201 == res.res.status);
  }
  // Valid, short, 0.9.
  {
    PgHttpResponseStatusLineResult res =
        http_parse_response_status_line(PG_S("HTTP/0.9 201"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0 == res.res.version_major);
    PG_ASSERT(9 == res.res.version_minor);
    PG_ASSERT(201 == res.res.status);
  }
  // Valid, long.
  {
    PgHttpResponseStatusLineResult res =
        http_parse_response_status_line(PG_S("HTTP/1.1 404 Not found"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.res.version_major);
    PG_ASSERT(1 == res.res.version_minor);
    PG_ASSERT(404 == res.res.status);
  }
}

static void test_http_parse_request_status_line() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  // Empty.
  {
    PG_ASSERT(http_parse_request_status_line(PG_S(""), &arena).err);
  }
  // Missing prefix.
  {
    PG_ASSERT(http_parse_request_status_line(PG_S("GE"), &arena).err);
    PG_ASSERT(http_parse_request_status_line(PG_S("abc"), &arena).err);
    PG_ASSERT(http_parse_request_status_line(PG_S("123 "), &arena).err);
  }
  // Missing slash.
  {
    PG_ASSERT(http_parse_request_status_line(PG_S("GET HTTP1.1"), &arena).err);
  }
  // Missing major version.
  {
    PG_ASSERT(
        http_parse_request_status_line(PG_S("GET / HTTP/.1"), &arena).err);
  }
  // Missing `.`.
  {
    PG_ASSERT(
        http_parse_request_status_line(PG_S("GET / HTTP/11"), &arena).err);
  }
  // Missing minor version.
  {
    PG_ASSERT(
        http_parse_request_status_line(PG_S("GET / HTTP/1."), &arena).err);
  }
  // Invalid major version.
  {
    PG_ASSERT(
        http_parse_request_status_line(PG_S("GET / HTTP/abc.1"), &arena).err);
    PG_ASSERT(
        http_parse_request_status_line(PG_S("GET / HTTP/4.1"), &arena).err);
  }
  // Invalid minor version.
  {
    PG_ASSERT(
        http_parse_request_status_line(PG_S("GET / HTTP/1.10"), &arena).err);
  }
  // Valid, short.
  {
    PgHttpRequestStatusLineResult res =
        http_parse_request_status_line(PG_S("GET / HTTP/2.0"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(HTTP_METHOD_GET == res.res.method);
    PG_ASSERT(2 == res.res.version_major);
    PG_ASSERT(0 == res.res.version_minor);
    PG_ASSERT(0 == res.res.url.path_components.len);
    PG_ASSERT(0 == res.res.url.query_parameters.len);
  }
  // Valid, short with query parameters.
  {
    PgHttpRequestStatusLineResult res =
        http_parse_request_status_line(PG_S("GET /?foo=bar& HTTP/2.0"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(HTTP_METHOD_GET == res.res.method);
    PG_ASSERT(2 == res.res.version_major);
    PG_ASSERT(0 == res.res.version_minor);
    PG_ASSERT(0 == res.res.url.path_components.len);
    PG_ASSERT(1 == res.res.url.query_parameters.len);
    KeyValue kv0 = dyn_at(res.res.url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  // Valid, short, 0.9.
  {
    PgHttpRequestStatusLineResult res =
        http_parse_request_status_line(PG_S("GET / HTTP/0.9"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(HTTP_METHOD_GET == res.res.method);
    PG_ASSERT(0 == res.res.version_major);
    PG_ASSERT(9 == res.res.version_minor);
    PG_ASSERT(0 == res.res.url.path_components.len);
    PG_ASSERT(0 == res.res.url.query_parameters.len);
  }
  // Valid, long.
  {
    PgHttpRequestStatusLineResult res = http_parse_request_status_line(
        PG_S("GET /foo/bar/baz?hey HTTP/1.1"), &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(HTTP_METHOD_GET == res.res.method);
    PG_ASSERT(1 == res.res.version_major);
    PG_ASSERT(1 == res.res.version_minor);
    PG_ASSERT(3 == res.res.url.path_components.len);
    PG_ASSERT(1 == res.res.url.query_parameters.len);
    KeyValue kv0 = dyn_at(res.res.url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("hey")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("")));
  }
}

static void test_http_parse_header() {
  // Empty.
  {
    PG_ASSERT(http_parse_header(PG_S("")).err);
  }
  // Missing `:`.
  {
    PG_ASSERT(http_parse_header(PG_S("foo bar")).err);
  }
  // Missing key.
  {
    PG_ASSERT(http_parse_header(PG_S(":bcd")).err);
  }
  // Missing value.
  {
    PG_ASSERT(http_parse_header(PG_S("foo:")).err);
  }
  // Multiple colons.
  {
    PgKeyValueResult res = http_parse_header(PG_S("foo: bar : baz"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(res.res.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(res.res.value, PG_S("bar : baz")));
  }
  // Valid, one space before the value.
  {
    PgKeyValueResult res = http_parse_header(PG_S("foo: bar"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(res.res.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(res.res.value, PG_S("bar")));
  }
  // Valid, no space before the value.
  {
    PgKeyValueResult res = http_parse_header(PG_S("foo:bar"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(res.res.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(res.res.value, PG_S("bar")));
  }
  // Valid, multiple spaces before the value.
  {
    PgKeyValueResult res = http_parse_header(PG_S("foo:   bar"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(res.res.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(res.res.value, PG_S("bar")));
  }
}

static void test_http_read_response() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  // Empty.
  {
    RingBuffer rg = {.data = pg_string_make(32, &arena)};
    HttpResponseReadResult res = http_read_response(&rg, 128, &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(false == res.done);
  }
  // Partial status line.
  {
    RingBuffer rg = {.data = pg_string_make(32, &arena)};
    PG_ASSERT(true == ring_buffer_write_slice(&rg, PG_S("HTTP/1.")));
    HttpResponseReadResult res = http_read_response(&rg, 128, &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(false == res.done);
    PG_ASSERT(ring_buffer_read_space(rg) == PG_S("HTTP/1.").len);
  }
  // Status line and some but not full.
  {
    RingBuffer rg = {.data = pg_string_make(32, &arena)};
    PG_ASSERT(true == ring_buffer_write_slice(
                          &rg, PG_S("HTTP/1.1 201 Created\r\nHost:")));
    HttpResponseReadResult res = http_read_response(&rg, 128, &arena);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(false == res.done);
  }

  // Full.
  {
    RingBuffer rg = {.data = pg_string_make(128, &arena)};

    {
      PG_ASSERT(true == ring_buffer_write_slice(
                            &rg, PG_S("HTTP/1.1 201 Created\r\nHost:")));
      HttpResponseReadResult res = http_read_response(&rg, 128, &arena);
      PG_ASSERT(0 == res.err);
      PG_ASSERT(false == res.done);
    }

    {
      PG_ASSERT(true == ring_buffer_write_slice(&rg, PG_S("google.com\r")));
      HttpResponseReadResult res = http_read_response(&rg, 128, &arena);
      PG_ASSERT(0 == res.err);
      PG_ASSERT(false == res.done);

      PG_ASSERT(true == ring_buffer_write_slice(&rg, PG_S("\n")));
      res = http_read_response(&rg, 128, &arena);
      PG_ASSERT(0 == res.err);
      PG_ASSERT(false == res.done);
    }

    {
      PG_ASSERT(true == ring_buffer_write_slice(
                            &rg, PG_S("Authorization: Bearer foo\r\n\r\n")));
      HttpResponseReadResult res = http_read_response(&rg, 128, &arena);
      PG_ASSERT(0 == res.err);
      PG_ASSERT(true == res.done);
      PG_ASSERT(1 == res.res.version_major);
      PG_ASSERT(1 == res.res.version_minor);
      PG_ASSERT(201 == res.res.status);
      PG_ASSERT(2 == res.res.headers.len);

      KeyValue kv0 = PG_SLICE_AT(res.res.headers, 0);
      PG_ASSERT(pg_string_eq(kv0.key, PG_S("Host")));
      PG_ASSERT(pg_string_eq(kv0.value, PG_S("google.com")));

      KeyValue kv1 = PG_SLICE_AT(res.res.headers, 1);
      PG_ASSERT(pg_string_eq(kv1.key, PG_S("Authorization")));
      PG_ASSERT(pg_string_eq(kv1.value, PG_S("Bearer foo")));
    }
  }
}

static void test_http_request_response() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  u16 port = PG_CLAMP(3000, (u16)arc4random_uniform(UINT16_MAX), UINT16_MAX);
  Socket listen_socket = 0;
  {
    PgCreateSocketResult res_create_socket = net_create_tcp_socket();
    PG_ASSERT(0 == res_create_socket.err);
    listen_socket = res_create_socket.res;

    PG_ASSERT(0 == net_socket_enable_reuse(listen_socket));
    PG_ASSERT(0 == net_socket_set_blocking(listen_socket, false));

    Ipv4Address addr = {0};
    addr.port = port;
    PG_ASSERT(0 == net_tcp_bind_ipv4(listen_socket, addr));
    PG_ASSERT(0 == net_tcp_listen(listen_socket));
  }

  Socket client_socket = 0;
  {
    PgDnsResolveIpv4AddressSocketResult res_dns =
        net_dns_resolve_ipv4_tcp(PG_S("localhost"), port, arena);
    PG_ASSERT(0 == res_dns.err);

    PG_ASSERT(port == res_dns.res.address.port);
    PG_ASSERT(0 != res_dns.res.socket);
    client_socket = res_dns.res.socket;
  }
  PG_ASSERT(0 == net_socket_set_blocking(client_socket, false));

  PgAioQueueCreateResult res_queue_create = aio_queue_create();
  PG_ASSERT(0 == res_queue_create.err);

  AioQueue queue = res_queue_create.res;
  PgAioEventSlice events_change = PG_SLICE_MAKE(PgAioEvent, 3, &arena);
  events_change.len = 1;

  {
    PgAioEvent *event_server_listen = PG_SLICE_AT_PTR(&events_change, 0);
    event_server_listen->socket = listen_socket;
    event_server_listen->kind = PG_AIO_EVENT_KIND_IN;
    event_server_listen->action = PG_AIO_EVENT_ACTION_ADD;

    PG_ASSERT(0 == aio_queue_ctl(queue, events_change));
  }

  Socket server_socket = 0;
  Reader server_reader = {0};
  Writer server_writer = {0};

  Writer client_writer = writer_make_from_socket(client_socket);
  Reader client_reader = reader_make_from_socket(client_socket);

  RingBuffer client_recv = {.data = pg_string_make(128, &arena)};
  RingBuffer client_send = {.data = pg_string_make(128, &arena)};
  RingBuffer server_recv = {.data = pg_string_make(128, &arena)};
  RingBuffer server_send = {.data = pg_string_make(128, &arena)};

  bool client_recv_http_io_done = false;
  bool client_send_http_io_done = false;
  bool server_recv_http_io_done = false;
  bool server_send_http_io_done = false;

  HttpRequest client_req = {0};
  client_req.method = HTTP_METHOD_GET;
  http_push_header(&client_req.headers, PG_S("Host"), PG_S("localhost"),
                   &arena);
  *dyn_push(&client_req.url.query_parameters, &arena) = (KeyValue){
      .key = PG_S("uploaded"),
      .value = u64_to_string(123456, &arena),
  };

  HttpResponse server_res = {0};
  server_res.status = 200;
  server_res.version_major = 1;
  server_res.version_minor = 1;
  http_push_header(&server_res.headers, PG_S("Accept"),
                   PG_S("application/json"), &arena);

  HttpRequest server_req = {0};
  HttpResponse client_res = {0};

  PgAioEventSlice events_watch = PG_SLICE_MAKE(PgAioEvent, 3, &arena);

  for (u64 _i = 0; _i <= 128; _i++) {
    Pgu64Result res_wait = aio_queue_wait(queue, events_watch, -1, arena);
    PG_ASSERT(0 == res_wait.err);

    for (u64 i = 0; i < res_wait.res; i++) {
      PgAioEvent event = PG_SLICE_AT(events_watch, i);
      PG_ASSERT(0 == (PG_AIO_EVENT_KIND_ERR & event.kind));

      if (event.socket == listen_socket) {
        Ipv4AddressAcceptResult res_accept = net_tcp_accept(listen_socket);
        PG_ASSERT(0 == res_accept.err);
        PG_ASSERT(0 != res_accept.socket);

        events_change.len = 2;
        PgAioEvent *event_client = PG_SLICE_AT_PTR(&events_change, 0);
        event_client->socket = client_socket;
        event_client->kind = PG_AIO_EVENT_KIND_OUT;
        event_client->action = PG_AIO_EVENT_ACTION_ADD;

        server_socket = res_accept.socket;
        server_reader = reader_make_from_socket(server_socket);
        server_writer = writer_make_from_socket(server_socket);

        PgAioEvent *event_server_client = PG_SLICE_AT_PTR(&events_change, 1);
        event_server_client->socket = res_accept.socket;
        event_server_client->kind = PG_AIO_EVENT_KIND_IN;
        event_server_client->action = PG_AIO_EVENT_ACTION_ADD;

        PG_ASSERT(0 == aio_queue_ctl(queue, events_change));
        events_change.len = 0;
      } else if (event.socket == client_socket) {
        if (PG_AIO_EVENT_KIND_OUT & event.kind) {
          if (!client_send_http_io_done) {
            http_write_request(&client_send, client_req, arena);
            PG_ASSERT(true == ring_buffer_write_slice(
                                  &client_send, PG_S("client request body")));
            PG_ASSERT(0 ==
                      writer_write(&client_writer, &client_send, arena).err);
            client_send_http_io_done = true;

            // Stop subscribing for writing, start subscribing for reading.
            events_change.len = 1;
            PgAioEvent *event_client = PG_SLICE_AT_PTR(&events_change, 0);
            event_client->socket = client_socket;
            event_client->kind = PG_AIO_EVENT_KIND_IN;
            event_client->action = PG_AIO_EVENT_ACTION_MOD;
            PG_ASSERT(0 == aio_queue_ctl(queue, events_change));
            events_change.len = 0;
          }
        }
        if (PG_AIO_EVENT_KIND_IN & event.kind) {
          if (!client_recv_http_io_done) {
            PG_ASSERT(0 ==
                      reader_read(&client_reader, &client_recv, arena).err);
            HttpResponseReadResult res =
                http_read_response(&client_recv, 128, &arena);
            PG_ASSERT(0 == res.err);
            PG_ASSERT(true == res.done);
            client_res = res.res;
            client_recv_http_io_done = true;
            goto end;
          }
        }
      } else if (event.socket == server_socket) {
        if (PG_AIO_EVENT_KIND_IN & event.kind) {
          if (!server_recv_http_io_done) {
            PG_ASSERT(0 ==
                      reader_read(&server_reader, &server_recv, arena).err);
            HttpRequestReadResult res =
                http_read_request(&server_recv, 128, &arena);
            PG_ASSERT(0 == res.err);
            PG_ASSERT(true == res.done);
            server_req = res.res;
            server_recv_http_io_done = true;

            // Stop subscribing for reading, start subscribing for writing.
            events_change.len = 1;
            PgAioEvent *event_server = PG_SLICE_AT_PTR(&events_change, 0);
            event_server->socket = server_socket;
            event_server->kind = PG_AIO_EVENT_KIND_OUT;
            event_server->action = PG_AIO_EVENT_ACTION_MOD;
            PG_ASSERT(0 == aio_queue_ctl(queue, events_change));
            events_change.len = 0;
          }
        }
        if (PG_AIO_EVENT_KIND_OUT & event.kind) {
          if (!server_send_http_io_done) {
            PG_ASSERT(0 ==
                      http_write_response(&server_send, server_res, arena));
            PG_ASSERT(0 ==
                      writer_write(&server_writer, &server_send, arena).err);
            server_send_http_io_done = true;

            // Stop subscribing.
            events_change.len = 1;
            PgAioEvent *event_server = PG_SLICE_AT_PTR(&events_change, 0);
            event_server->socket = server_socket;
            event_server->action = PG_AIO_EVENT_ACTION_DEL;
            PG_ASSERT(0 == aio_queue_ctl(queue, events_change));
            events_change.len = 0;
          }
        }
      } else {
        PG_ASSERT(0);
      }
    }
  }

end:

  PG_ASSERT(client_send_http_io_done);
  PG_ASSERT(client_recv_http_io_done);
  PG_ASSERT(server_recv_http_io_done);
  PG_ASSERT(server_send_http_io_done);

  PG_ASSERT(client_req.method = server_req.method);
  PG_ASSERT(client_req.version_major = server_req.version_major);
  PG_ASSERT(client_req.version_minor = server_req.version_minor);
  PG_ASSERT(client_req.headers.len = server_req.headers.len);
  PG_ASSERT(1 == server_req.url.query_parameters.len);
  KeyValue query0 = PG_SLICE_AT(server_req.url.query_parameters, 0);
  PG_ASSERT(pg_string_eq(query0.key, PG_S("uploaded")));
  PG_ASSERT(pg_string_eq(query0.value, PG_S("123456")));

  PG_ASSERT(client_res.status = server_res.status);
  PG_ASSERT(client_res.version_major = server_res.version_major);
  PG_ASSERT(client_res.version_minor = server_res.version_minor);
  PG_ASSERT(client_res.headers.len = server_res.headers.len);

  PG_ASSERT(0 == net_socket_close(client_socket));
  PG_ASSERT(0 == net_socket_close(server_socket));
  PG_ASSERT(0 == net_socket_close(listen_socket));
}

static void test_log() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);
  // Simple log.
  {
    StringBuilder sb = {.arena = &arena};
    Logger logger = log_logger_make_stdout_json(LOG_LEVEL_DEBUG);
    logger.writer = writer_make_from_string_builder(&sb);

    logger_log(&logger, LOG_LEVEL_INFO, "hello world", arena,
               L("foo", PG_S("bar")));

    PgString out = dyn_slice(PgString, sb.sb);
    PG_ASSERT(pg_string_starts_with(out, PG_S("{\"level\":\"info\"")));
  }
  // Log but the logger level is higher.
  {
    StringBuilder sb = {.arena = &arena};
    Logger logger = log_logger_make_stdout_json(LOG_LEVEL_INFO);
    logger.writer = writer_make_from_string_builder(&sb);

    logger_log(&logger, LOG_LEVEL_DEBUG, "hello world", arena,
               L("foo", PG_S("bar")));

    PgString out = dyn_slice(PgString, sb.sb);
    PG_ASSERT(pg_string_is_empty(out));
  }
}

static void test_timer() {
  Arena arena = arena_make_from_virtual_mem(4 * PG_KiB);

  PgAioQueueCreateResult res_queue_create = aio_queue_create();
  PG_ASSERT(0 == res_queue_create.err);
  AioQueue queue = res_queue_create.res;

  PgTimerResult res_timer =
      pg_timer_create(CLOCK_KIND_MONOTONIC, 10 * PG_Milliseconds);
  PG_ASSERT(0 == res_timer.err);

  Pgu64Result res_start = pg_time_ns_now(CLOCK_KIND_MONOTONIC);
  PG_ASSERT(0 == res_start.err);

  {
    PgAioEvent event_change = {
        .timer = res_timer.res,
        .kind = PG_AIO_EVENT_KIND_IN,
        .action = PG_AIO_EVENT_ACTION_ADD,
    };
    PgError err = aio_queue_ctl_one(queue, event_change);
    PG_ASSERT(0 == err);
  }

  PgAioEventSlice events_watch = PG_SLICE_MAKE(PgAioEvent, 1, &arena);
  Pgu64Result res_wait = aio_queue_wait(queue, events_watch, 1'000, arena);
  PG_ASSERT(0 == res_wait.err);
  PG_ASSERT(1 == res_wait.res);

  PgAioEvent event_watch = PG_SLICE_AT(events_watch, 0);
  PG_ASSERT(0 == (PG_AIO_EVENT_KIND_ERR & event_watch.kind));
  PG_ASSERT(PG_AIO_EVENT_KIND_IN & event_watch.kind);

  Pgu64Result res_end = pg_time_ns_now(CLOCK_KIND_MONOTONIC);
  PG_ASSERT(0 == res_end.err);
  PG_ASSERT(res_end.res > res_start.res);
  PG_ASSERT(res_end.res - res_start.res < 20 * PG_Milliseconds);

  PG_ASSERT(0 == pg_timer_release(res_timer.res));
}

int main() {
  test_slice_range();
  test_string_indexof_slice();
  test_string_trim();
  test_string_split_byte();
  test_string_split_string();
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
  test_timer();
  test_log();
}
