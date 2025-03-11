#include "lib.c"

static void test_rune_bytes_count() {
  PG_ASSERT(0 == pg_utf8_rune_bytes_count(0));
  PG_ASSERT(1 == pg_utf8_rune_bytes_count('A'));
  PG_ASSERT(3 == pg_utf8_rune_bytes_count(0x30eb /* ル */));
  PG_ASSERT(3 == pg_utf8_rune_bytes_count(0x805e /* 聞 */));
  PG_ASSERT(4 == pg_utf8_rune_bytes_count(0x1f34c /* 🍌 */));
}

static void test_utf8_count() {
  PG_ASSERT(2 == pg_utf8_count(PG_S("🚀🛸")).res);
}

static void test_string_last() {
  // Empty
  {
    PG_ASSERT(0 == pg_string_last(PG_S("")));
  }
  {
    PG_ASSERT(0x805e /* 聞 */ == pg_string_last(PG_S("朝日新聞デジタル聞")));
  }
}

static void test_string_first() {
  // Empty
  {
    PG_ASSERT(0 == pg_string_first(PG_S("")));
  }
  {
    PG_ASSERT(0x805e /* 聞 */ == pg_string_first(PG_S("聞デジタル")));
  }
}

static void test_string_indexof_rune() {
  // Empty.
  {
    PG_ASSERT(-1 == pg_string_indexof_rune(PG_S(""), 0x805e /* 聞 */));
  }

  // Unicode
  {
    PgString haystack = PG_S("朝日新聞デジタル聞");
    PG_ASSERT(9 == pg_string_indexof_rune(haystack, 0x805e /* 聞 */));
    PG_ASSERT(-1 == pg_string_indexof_rune(haystack, 0x1f34c /* 🍌 */));
  }
}

static void test_string_last_indexof_rune() {
  // Empty.
  {
    PG_ASSERT(-1 == pg_string_last_indexof_rune(PG_S(""), 0x805e /* 聞 */));
  }

  // Unicode
  {
    PgString haystack = PG_S("朝日新聞デジタ聞ル");
    PG_ASSERT(21 == pg_string_last_indexof_rune(haystack, 0x805e /* 聞 */));
    PG_ASSERT(-1 == pg_string_last_indexof_rune(haystack, 0x1f34c /* 🍌 */));
  }
}

static void test_string_indexof_string() {
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
    PG_ASSERT(-1 == pg_string_indexof_string(PG_S("hello 🍌 world"),
                                             PG_S("foobar 🍌")));
  }

  // Found, one occurence.
  {
    PG_ASSERT(11 ==
              pg_string_indexof_string(PG_S("hello 🍌 world"), PG_S("world")));
  }

  // Found, one occurence.
  {
    PG_ASSERT(0 == pg_string_indexof_string(PG_S("hello 🍌 world\n"),
                                            PG_S("hello 🍌 world")));
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

  {
    PG_ASSERT(0 == pg_string_indexof_string(PG_S("/"), PG_S("/")));
  }
}

static void test_string_trim() {
  {
    PgString trimmed = pg_string_trim(PG_S("   foo "), ' ');
    PG_ASSERT(pg_string_eq(trimmed, PG_S("foo")));
  }
  {
    PgString trimmed = pg_string_trim(PG_S("🍌🍌foo🍌"), 0x1f34c /* 🍌 */);
    PG_ASSERT(pg_string_eq(trimmed, PG_S("foo")));
  }
}

static void test_string_cut() {
  {
    PgStringCut cut = pg_string_cut_string(PG_S("🍌🍌foo🍌"), PG_S("🍌"));
    PG_ASSERT(cut.ok);
    PG_ASSERT(0 == cut.left.len);
    PG_ASSERT(pg_string_eq(PG_S("🍌foo🍌"), cut.right));
  }
  {
    PgStringCut cut = pg_string_cut_string(PG_S("🍌🍌foo🍌"), PG_S("fo"));
    PG_ASSERT(cut.ok);
    PG_ASSERT(pg_string_eq(PG_S("🍌🍌"), cut.left));
    PG_ASSERT(pg_string_eq(PG_S("o🍌"), cut.right));
  }
  {
    PgStringCut cut = pg_string_cut_string(PG_S("🍌🍌foo🍌"), PG_S("✨"));
    PG_ASSERT(!cut.ok);
  }
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
  PgString s = PG_S("hello🚀🛸world🚀little🚀🛸thing !");
  PgSplitIterator it = pg_string_split_string(s, PG_S("🚀🛸"));

  {
    PgStringOk elem = pg_string_split_next(&it);
    PG_ASSERT(true == elem.ok);
    PG_ASSERT(pg_string_eq(elem.res, PG_S("hello")));
  }

  {
    PgStringOk elem = pg_string_split_next(&it);
    PG_ASSERT(true == elem.ok);
    PG_ASSERT(pg_string_eq(elem.res, PG_S("world🚀little")));
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
    PgArena arena = pg_arena_make_from_virtual_mem(arena_cap);
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    Pgu8Dyn dyn = {0};
    *PG_DYN_PUSH(&dyn, allocator) = 1;
    PG_ASSERT(1 == dyn.len);
    PG_ASSERT(2 == dyn.cap);

    u64 arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(2 == arena_size_expected);
    PG_ASSERT(dyn.cap == arena_size_expected);

    u64 desired_cap = 13;
    PG_DYN_ENSURE_CAP(&dyn, desired_cap, allocator);
    PG_ASSERT(16 == dyn.cap);
    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(16 == arena_size_expected);
  }
  // General case.
  {
    PgArena arena = pg_arena_make_from_virtual_mem(arena_cap);
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    Pgu8Dyn dyn = {0};
    *PG_DYN_PUSH(&dyn, allocator) = 1;
    PG_ASSERT(1 == dyn.len);
    PG_ASSERT(2 == dyn.cap);

    Pgu8Dyn dummy = {0};
    *PG_DYN_PUSH(&dummy, allocator) = 2;
    *PG_DYN_PUSH(&dummy, allocator) = 3;

    u64 arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(2 + 2 == arena_size_expected);

    // This triggers a new allocation.
    *PG_DYN_PUSH(&dummy, allocator) = 4;
    PG_ASSERT(3 == dummy.len);
    PG_ASSERT(4 == dummy.cap);

    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(2 + 4 == arena_size_expected);

    u64 desired_cap = 13;
    PG_DYN_ENSURE_CAP(&dyn, desired_cap, allocator);
    PG_ASSERT(16 == dyn.cap);

    arena_size_expected = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(16 + 6 == arena_size_expected);
  }
}

static void test_slice_range() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

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
  (void)PG_SLICE_RANGE(PG_DYN_SLICE(PgStringSlice, dyn), 0, 0);

  *PG_DYN_PUSH(&dyn, allocator) = PG_S("hello \"world\n\"!");
  *PG_DYN_PUSH(&dyn, allocator) = PG_S("日");
  *PG_DYN_PUSH(&dyn, allocator) = PG_S("本語");

  PgStringSlice s = PG_DYN_SLICE(PgStringSlice, dyn);
  PgStringSlice range = PG_SLICE_RANGE_START(s, 1UL);
  PG_ASSERT(2 == range.len);

  PG_ASSERT(pg_string_eq(PG_SLICE_AT(s, 1), PG_SLICE_AT(range, 0)));
  PG_ASSERT(pg_string_eq(PG_SLICE_AT(s, 2), PG_SLICE_AT(range, 1)));
}

static void test_utf8_iterator() {
  // Empty.
  {
    PgString s = PG_S("");
    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(0 == res_count.err);
    PG_ASSERT(0 == res_count.res);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);

    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0 == res.res);
  }
  {
    PgString s = PG_S("2匹の🀅🂣©");
    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(0 == res_count.err);
    PG_ASSERT(6 == res_count.res);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);

    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0x32 == res.res);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0x5339 == res.res);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0x306e == res.res);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0x1f005 == res.res);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0x1f0a3 == res.res);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0x00A9 == res.res);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0 == res.res);
  }
  // Null byte.
  {
    PgString s = PG_S("\x1\x0");
    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res_count.err);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Forbidden byte.
  {
    PgString s = PG_S("\x1\xc0");
    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res_count.err);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Forbidden byte.
  {
    PgString s = PG_S("\x1\xf5");
    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res_count.err);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Continuation byte but EOF.
  {
    PgString s = PG_S("\x1\x80");
    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res_count.err);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // EOF
  {
    PgString s = PG_S("\x1🀅");
    s.len -= 1; // Early EOF.

    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res_count.err);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Too high.
  {
    PgString s = PG_S("\x1\xf4\x90\x90\x90");

    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res_count.err);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Overlong.
  {
    PgString s = PG_S("\x1\xc0\x80");

    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res_count.err);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Overlong.
  {
    PgString s = PG_S("\x2F\xC0\xAE\x2E\x2F");

    PgU64Result res_count = pg_utf8_count(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res_count.err);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneResult res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
}

static void test_string_consume() {
  {
    PgStringOk res = pg_string_consume_rune(PG_S(""), '{');
    PG_ASSERT(!res.ok);
  }
  {
    PgStringOk res = pg_string_consume_rune(PG_S("[1,2]"), '{');
    PG_ASSERT(!res.ok);
  }
  {
    PgStringOk res = pg_string_consume_rune(PG_S("🍌[1,2]"), 0x1f34c /* 🍌 */);
    PG_ASSERT(res.ok);
    PG_ASSERT(pg_string_eq(PG_S("[1,2]"), res.res));
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
    PgCompare cmp = pg_string_cmp(PG_S("a"), PG_S("aa"));
    PG_ASSERT(PG_CMP_LESS == cmp);
  }
  {
    PgCompare cmp = pg_string_cmp(PG_S(""), PG_S("a"));
    PG_ASSERT(PG_CMP_LESS == cmp);
  }
  {
    PgCompare cmp = pg_string_cmp(PG_S(""), PG_S(""));
    PG_ASSERT(PG_CMP_EQ == cmp);
  }
  {
    PgCompare cmp = pg_string_cmp(PG_S("a"), PG_S("a"));
    PG_ASSERT(PG_CMP_EQ == cmp);
  }
  {
    PgCompare cmp = pg_string_cmp(PG_S("a"), PG_S("b"));
    PG_ASSERT(PG_CMP_LESS == cmp);
  }
  {
    PgCompare cmp = pg_string_cmp(PG_S("b"), PG_S("aa"));
    PG_ASSERT(PG_CMP_GREATER == cmp);
  }
  {
    PgCompare cmp = pg_string_cmp(PG_S("b"), PG_S("a"));
    PG_ASSERT(PG_CMP_GREATER == cmp);
  }
  {
    PgCompare cmp = pg_string_cmp(PG_S("announce"), PG_S("comment"));
    PG_ASSERT(PG_CMP_LESS == cmp);
  }
}

static void test_sha1() {
  {
    PgSha1 hash = pg_sha1(PG_S("abc"));

    PgSha1 expected_hash = {.data = {
                                0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81,
                                0x6A, 0xBA, 0x3E, 0x25, 0x71, 0x78, 0x50,
                                0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D,
                            }};
    static_assert(sizeof(hash) == sizeof(expected_hash));

    PG_ASSERT(0 == memcmp(hash.data, expected_hash.data, sizeof(hash)));
  }
  {
    PgSha1 hash = pg_sha1(
        PG_S("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoi"
             "jklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"));

    PgSha1 expected_hash = {.data = {
                                0xa4, 0x9b, 0x24, 0x46, 0xa0, 0x2c, 0x64,
                                0x5b, 0xf4, 0x19, 0xf9, 0x95, 0xb6, 0x70,
                                0x91, 0x25, 0x3a, 0x04, 0xa2, 0x59,
                            }};
    static_assert(sizeof(hash) == sizeof(expected_hash));

    PG_ASSERT(0 == memcmp(hash.data, expected_hash.data, sizeof(hash)));
  }
}

static void test_slice_swap_remove() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
  {
    PgString s = pg_string_dup(PG_S("hello world!"), allocator);
    PG_SLICE_SWAP_REMOVE(&s, 4);
    PG_ASSERT(pg_string_eq(s, PG_S("hell! world")));
  }
}

static void test_dynu8_append_u8_hex_upper() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  {
    Pgu8Dyn sb = {0};
    PgWriter w = pg_writer_make_from_string_builder(&sb, allocator);
    PG_ASSERT(0 == pg_writer_write_u8_hex_upper(&w, 0xac));
    PG_ASSERT(0 == pg_writer_write_u8_hex_upper(&w, 0x89));

    PgString s = PG_DYN_SLICE(PgString, sb);
    PG_ASSERT(pg_string_eq(s, PG_S("AC89")));
  }
}

static void test_ipv4_address_to_string() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
  {
    PgIpv4Address address = {
        .ip = (192UL << 24) | (168UL << 16) | (1UL << 8) | (56UL << 0),
        .port = 6881,
    };

    PgString s = pg_net_ipv4_address_to_string(address, allocator);
    PG_ASSERT(pg_string_eq(s, PG_S("192.168.1.56:6881")));
  }
}

static void test_url_encode() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
  {
    Pgu8Dyn sb = {0};
    PgWriter w = pg_writer_make_from_string_builder(&sb, allocator);
    PG_ASSERT(0 == pg_writer_url_encode(&w, PG_S("日本語"), PG_S("123")));
    PgString encoded = PG_DYN_SLICE(PgString, sb);

    PG_ASSERT(pg_string_eq(encoded, PG_S("%E6%97%A5%E6%9C%AC%E8%AA%9E=123")));
  }

  {
    Pgu8Dyn sb = {0};
    PgWriter w = pg_writer_make_from_string_builder(&sb, allocator);
    PG_ASSERT(0 == pg_writer_url_encode(&w, PG_S("日本語"), PG_S("foo")));
    PgString encoded = PG_DYN_SLICE(PgString, sb);

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

static void test_u8x4_be_to_u32_and_back() {
  {
    u32 n = 123'456'789;
    u8 data[sizeof(n)] = {0};
    PgString s = {.data = data, .len = sizeof(n)};
    pg_u32_to_u8x4_be(n, &s);
    PG_ASSERT(pg_string_eq(s, PG_S("\x07"
                                   "\x5b"
                                   "\x0cd"
                                   "\x15")));

    PG_ASSERT(n == pg_u8x4_be_to_u32(s));
  }
}

static void test_bitfield() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
  {
    PgString bitfield = PG_S("\x3"
                             "\x2");
    PG_ASSERT(3 == pg_bitfield_count(bitfield));
    PG_ASSERT(pg_bitfield_get(bitfield, 0));
    PG_ASSERT(pg_bitfield_get(bitfield, 1));
    PG_ASSERT(!pg_bitfield_get(bitfield, 2));
    PG_ASSERT(!pg_bitfield_get(bitfield, 3));
    PG_ASSERT(!pg_bitfield_get(bitfield, 4));
    PG_ASSERT(!pg_bitfield_get(bitfield, 5));
    PG_ASSERT(!pg_bitfield_get(bitfield, 6));
    PG_ASSERT(!pg_bitfield_get(bitfield, 7));
    PG_ASSERT(!pg_bitfield_get(bitfield, 8));
    PG_ASSERT(pg_bitfield_get(bitfield, 9));
    PG_ASSERT(!pg_bitfield_get(bitfield, 10));
    PG_ASSERT(!pg_bitfield_get(bitfield, 11));
    PG_ASSERT(!pg_bitfield_get(bitfield, 12));
    PG_ASSERT(!pg_bitfield_get(bitfield, 13));
    PG_ASSERT(!pg_bitfield_get(bitfield, 14));
    PG_ASSERT(!pg_bitfield_get(bitfield, 15));
  }
  {
    PgString bitfield = pg_string_dup(PG_S("\x3"
                                           "\x2"),
                                      allocator);
    PG_ASSERT(3 == pg_bitfield_count(bitfield));

    pg_bitfield_set(bitfield, 0, true);
    PG_ASSERT(3 == pg_bitfield_count(bitfield));
    PG_ASSERT(pg_bitfield_get(bitfield, 0));

    pg_bitfield_set(bitfield, 1, true);
    PG_ASSERT(3 == pg_bitfield_count(bitfield));
    PG_ASSERT(pg_bitfield_get(bitfield, 1));

    pg_bitfield_set(bitfield, 2, true);
    PG_ASSERT(4 == pg_bitfield_count(bitfield));
    PG_ASSERT(pg_bitfield_get(bitfield, 2));

    pg_bitfield_set(bitfield, 8, true);
    PG_ASSERT(5 == pg_bitfield_count(bitfield));
    PG_ASSERT(pg_bitfield_get(bitfield, 0));
    PG_ASSERT(pg_bitfield_get(bitfield, 1));
    PG_ASSERT(pg_bitfield_get(bitfield, 2));
    PG_ASSERT(!pg_bitfield_get(bitfield, 3));
    PG_ASSERT(!pg_bitfield_get(bitfield, 4));
    PG_ASSERT(!pg_bitfield_get(bitfield, 5));
    PG_ASSERT(!pg_bitfield_get(bitfield, 6));
    PG_ASSERT(!pg_bitfield_get(bitfield, 7));
    PG_ASSERT(pg_bitfield_get(bitfield, 8));
    PG_ASSERT(pg_bitfield_get(bitfield, 9));
    PG_ASSERT(!pg_bitfield_get(bitfield, 10));
    PG_ASSERT(!pg_bitfield_get(bitfield, 11));
    PG_ASSERT(!pg_bitfield_get(bitfield, 12));
    PG_ASSERT(!pg_bitfield_get(bitfield, 13));
    PG_ASSERT(!pg_bitfield_get(bitfield, 14));
    PG_ASSERT(!pg_bitfield_get(bitfield, 15));
  }
  {
    PgString bitfield = pg_string_dup(PG_S("\x20"
                                           "\x01"
                                           "\x80"
                                           "\x90"),
                                      allocator);
    PG_ASSERT(5 == pg_bitfield_count(bitfield));
    pg_bitfield_set(bitfield, 28, true);
    PG_ASSERT(5 == pg_bitfield_count(bitfield));
    pg_bitfield_set(bitfield, 29, true);
    PG_ASSERT(6 == pg_bitfield_count(bitfield));
  }
}

static void test_ring_buffer_write_slice() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Write to empty ring buffer.
  {
    PgRing rg = {.data = pg_string_make(12, allocator)};
    PG_ASSERT(pg_ring_write_space(rg) == rg.data.len - 1);
    PG_ASSERT(pg_ring_write_slice(&rg, PG_S("hello")));
    PG_ASSERT(5 == rg.idx_write);
    PG_ASSERT(pg_ring_write_space(rg) == rg.data.len - 1 - 5);

    PG_ASSERT(false == pg_ring_write_slice(&rg, PG_S(" world!")));
    PG_ASSERT(5 == rg.idx_write);
    PG_ASSERT(pg_ring_write_space(rg) == rg.data.len - 1 - 5);

    PG_ASSERT(true == pg_ring_write_slice(&rg, PG_S(" world")));
    PG_ASSERT(11 == rg.idx_write);
    PG_ASSERT(pg_ring_write_space(rg) == 0);

    PG_ASSERT(0 == rg.idx_read);
    PG_ASSERT(
        pg_string_eq(PG_S("hello world"),
                     (PgString){.data = rg.data.data, .len = rg.idx_write}));
  }
  // Write to full ring buffer.
  {
    PgRing rg = {
        .data = pg_string_make(12, allocator),
        .idx_write = 1,
        .idx_read = 2,
    };
    PG_ASSERT(pg_ring_write_space(rg) == 0);
    PG_ASSERT(false == pg_ring_write_slice(&rg, PG_S("hello")));
    PG_ASSERT(1 == rg.idx_write);
    PG_ASSERT(pg_ring_write_space(rg) == 0);
  }
  // Write to ring buffer, easy case.
  {
    PgRing rg = {
        .data = pg_string_make(12, allocator),
        .idx_read = 1,
        .idx_write = 2,
    };
    PG_ASSERT(pg_ring_write_space(rg) == 10);
    PG_ASSERT(pg_ring_write_slice(&rg, PG_S("hello")));
    PG_ASSERT(2 + 5 == rg.idx_write);
    PG_ASSERT(pg_ring_write_space(rg) == 5);
  }

  // Write to ring buffer, hard case.
  {
    PgRing rg = {
        .data = pg_string_make(12, allocator),
        .idx_read = 2,
        .idx_write = 3,
    };
    PG_ASSERT(pg_ring_write_space(rg) == 10);
    PG_ASSERT(pg_ring_write_slice(&rg, PG_S("hello worl")));
    PG_ASSERT(1 == rg.idx_write);
    PG_ASSERT(pg_ring_write_space(rg) == 0);
    PG_ASSERT(pg_string_eq(rg.data, PG_S("l\x0\x0hello wor")));
  }
}

static void test_ring_buffer_read_write_slice() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Read from an empty ring buffer.
  {
    PgRing rg = {.data = pg_string_make(12, allocator)};
    PG_ASSERT(0 == pg_ring_read_space(rg));
    PG_ASSERT(true == pg_ring_read_slice(&rg, (PgString){0}));

    PgString dst = pg_string_dup(PG_S("xyz"), allocator);
    PG_ASSERT(false == pg_ring_read_slice(&rg, dst));
  }

  // Write to empty ring buffer, then read part of it.
  {
    PgRing rg = {.data = pg_string_make(12, allocator)};
    PG_ASSERT(pg_ring_write_slice(&rg, PG_S("hello")));
    PG_ASSERT(5 == rg.idx_write);
    PG_ASSERT(5 == pg_ring_read_space(rg));

    PgString dst = pg_string_dup(PG_S("xyz"), allocator);
    PG_ASSERT(pg_ring_read_slice(&rg, dst));
    PG_ASSERT(pg_string_eq(dst, PG_S("hel")));
    PG_ASSERT(3 == rg.idx_read);
    PG_ASSERT(2 == pg_ring_read_space(rg));

    PG_ASSERT(true == pg_ring_write_slice(&rg, PG_S(" world!")));
    PG_ASSERT(0 == rg.idx_write);
    PG_ASSERT(9 == pg_ring_read_space(rg));

    PG_ASSERT(false == pg_ring_write_slice(&rg, PG_S("abc")));
    PG_ASSERT(9 == pg_ring_read_space(rg));
    PG_ASSERT(true == pg_ring_write_slice(&rg, PG_S("ab")));
    PG_ASSERT(11 == pg_ring_read_space(rg));
    PG_ASSERT(2 == rg.idx_write);

    dst = pg_string_dup(PG_S("abcdefghijk"), allocator);
    PG_ASSERT(pg_ring_read_slice(&rg, dst));
    PG_ASSERT(pg_string_eq(dst, PG_S("lo world!ab")));
    PG_ASSERT(2 == rg.idx_read);
    PG_ASSERT(0 == pg_ring_read_space(rg));
  }
}

static void test_ring_buffer_read_until_excl() {
  PgArena arena = pg_arena_make_from_virtual_mem(8 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgRing rg = {.data = pg_string_make(4 * PG_KiB, allocator)};
  PG_ASSERT(pg_ring_write_slice(
      &rg, PG_S("The quick brown fox jumps over the lazy dog")));

  {
    u64 space_read = pg_ring_read_space(rg);
    u64 space_write = pg_ring_write_space(rg);
    PgStringOk s = pg_ring_read_until_excl(&rg, PG_S("\r\n"), allocator);
    PG_ASSERT(!s.ok);
    PG_ASSERT(PG_SLICE_IS_EMPTY(s.res));

    // Unmodified.
    PG_ASSERT(pg_ring_read_space(rg) == space_read);
    PG_ASSERT(pg_ring_write_space(rg) == space_write);
  }

  {
    PgStringOk s = pg_ring_read_until_excl(&rg, PG_S(" "), allocator);
    PG_ASSERT(s.ok);
    PG_ASSERT(pg_string_eq(s.res, PG_S("The")));
  }
  {
    PgStringOk s = pg_ring_read_until_excl(&rg, PG_S(" "), allocator);
    PG_ASSERT(s.ok);
    PG_ASSERT(pg_string_eq(s.res, PG_S("quick")));
  }
  {
    PgStringOk s = pg_ring_read_until_excl(&rg, PG_S("lazy "), allocator);
    PG_ASSERT(s.ok);
    PG_ASSERT(pg_string_eq(s.res, PG_S("brown fox jumps over the ")));
  }
  {
    PgStringOk s = pg_ring_read_until_excl(&rg, PG_S("g"), allocator);
    PG_ASSERT(s.ok);
    PG_ASSERT(pg_string_eq(s.res, PG_S("do")));
  }
  PG_ASSERT(0 == pg_ring_read_space(rg));
}

static void test_ring_buffer_read_write_fuzz() {
  PgArena arena_ring = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator_ring = pg_make_arena_allocator(&arena_ring);
  PgAllocator *allocator_ring =
      pg_arena_allocator_as_allocator(&arena_allocator_ring);

  PgRing rg = {.data = pg_string_make(4 * PG_KiB, allocator_ring)};

  u64 ROUNDS = 1024;
  PgArena arena_strings = pg_arena_make_from_virtual_mem(ROUNDS * 8 * PG_KiB);
  PgArenaAllocator arena_allocator_strings =
      pg_make_arena_allocator(&arena_strings);
  PgAllocator *allocator_strings =
      pg_arena_allocator_as_allocator(&arena_allocator_strings);

  PgRng rng = pg_rand_make();
  // TODO: Print seed for reproducability?
  for (u64 i = 0; i < ROUNDS; i++) {
    u32 len = pg_rand_u32_min_incl_max_incl(&rng, 0, (u32)rg.data.len + 1);
    PgString from = pg_string_make(len, allocator_strings);
    pg_rand_string_mut(&rng, from);

    PgString to = pg_string_make(len, allocator_strings);
    pg_rand_string_mut(&rng, to);

    bool ok_write = pg_ring_write_slice(&rg, from);
    (void)ok_write;
    bool ok_read = pg_ring_read_slice(&rg, to);
    (void)ok_read;

    PG_ASSERT(pg_ring_write_space(rg) <= rg.data.len - 1);
    PG_ASSERT(pg_ring_read_space(rg) <= rg.data.len - 1);
  }
}

static void test_url_parse() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  {
    PgUrlResult res = pg_url_parse(PG_S(""), allocator);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("x"), allocator);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http:"), allocator);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http:/"), allocator);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://"), allocator);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("://"), allocator);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.port);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a:"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.port);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a:/"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.port);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a:bc"), allocator);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://abc:0"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("abc"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(0 == res.res.port);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://abc:999999"), allocator);
    PG_ASSERT(0 != res.err);
  }

  // Invalid scheme.
  {
    PgUrlResult res = pg_url_parse(PG_S("1abc://a:80/"), allocator);
    PG_ASSERT(0 != res.err);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a:80"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(80 == res.res.port);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a.b.c:80/foo"), allocator);
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
    PgUrlResult res = pg_url_parse(PG_S("http://a.b.c:80/"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a.b.c"), res.res.host));
    PG_ASSERT(80 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a.b.c/foo/bar/baz"), allocator);
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

  // PgUrl parameters.
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a/?foo"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(1 == res.res.query_parameters.len);

    PgKeyValue kv0 = PG_SLICE_AT(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("")));
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a/?"), allocator);
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
    PgUrlResult res = pg_url_parse(PG_S("http://a/?foo=bar"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(1 == res.res.query_parameters.len);

    PgKeyValue kv0 = PG_SLICE_AT(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  {
    PgUrlResult res = pg_url_parse(PG_S("http://a/?foo=bar&"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(1 == res.res.query_parameters.len);

    PgKeyValue kv0 = PG_SLICE_AT(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  {
    PgUrlResult res =
        pg_url_parse(PG_S("http://a/?foo=bar&hello=world"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(2 == res.res.query_parameters.len);

    PgKeyValue kv0 = PG_SLICE_AT(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));

    PgKeyValue kv1 = PG_SLICE_AT(res.res.query_parameters, 1);
    PG_ASSERT(pg_string_eq(kv1.key, PG_S("hello")));
    PG_ASSERT(pg_string_eq(kv1.value, PG_S("world")));
  }
  {
    PgUrlResult res =
        pg_url_parse(PG_S("http://a/?foo=bar&hello=world&a="), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(PG_S("http"), res.res.scheme));
    PG_ASSERT(0 == res.res.username.len);
    PG_ASSERT(0 == res.res.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), res.res.host));
    PG_ASSERT(0 == res.res.port);
    PG_ASSERT(0 == res.res.path_components.len);
    PG_ASSERT(3 == res.res.query_parameters.len);

    PgKeyValue kv0 = PG_SLICE_AT(res.res.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));

    PgKeyValue kv1 = PG_SLICE_AT(res.res.query_parameters, 1);
    PG_ASSERT(pg_string_eq(kv1.key, PG_S("hello")));
    PG_ASSERT(pg_string_eq(kv1.value, PG_S("world")));

    PgKeyValue kv2 = PG_SLICE_AT(res.res.query_parameters, 2);
    PG_ASSERT(pg_string_eq(kv2.key, PG_S("a")));
    PG_ASSERT(pg_string_eq(kv2.value, PG_S("")));
  }
}

static void test_url_parse_relative_path() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Empty.
  {
    PgStringDynResult res = pg_url_parse_path_components(PG_S(""), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0 == res.res.len);
  }
  // Forbidden characters.
  {
    PG_ASSERT(pg_url_parse_path_components(PG_S("/foo?bar"), allocator).err);
    PG_ASSERT(pg_url_parse_path_components(PG_S("/foo:1234"), allocator).err);
    PG_ASSERT(pg_url_parse_path_components(PG_S("/foo#bar"), allocator).err);
  }
  // Must start with slash and it does not.
  {
    PgStringDynResult res =
        pg_url_parse_path_components(PG_S("foo"), allocator);
    PG_ASSERT(res.err);
  }
  // Must start with slash and it does.
  {
    PgStringDynResult res =
        pg_url_parse_path_components(PG_S("/foo"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.res.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(res.res, 0)));
  }
  // Simple path with a few components.
  {
    PgStringDynResult res =
        pg_url_parse_path_components(PG_S("/foo/bar/baz"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(3 == res.res.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(res.res, 0)));
    PG_ASSERT(pg_string_eq(PG_S("bar"), PG_SLICE_AT(res.res, 1)));
    PG_ASSERT(pg_string_eq(PG_S("baz"), PG_SLICE_AT(res.res, 2)));
  }
  // Simple path with a few components with trailing slash.
  {
    PgStringDynResult res =
        pg_url_parse_path_components(PG_S("/foo/bar/baz/"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(3 == res.res.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(res.res, 0)));
    PG_ASSERT(pg_string_eq(PG_S("bar"), PG_SLICE_AT(res.res, 1)));
    PG_ASSERT(pg_string_eq(PG_S("baz"), PG_SLICE_AT(res.res, 2)));
  }
}

static void test_http_request_to_string() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
  {
    PgHttpRequest req = {0};
    req.method = HTTP_METHOD_GET;

    PgString s = pg_http_request_to_string(req, allocator);
    PgString expected = PG_S("GET / HTTP/1.1\r\n"
                             "\r\n");
    PG_ASSERT(pg_string_eq(s, expected));
  }
  {
    PgHttpRequest req = {0};
    req.method = HTTP_METHOD_POST;
    pg_http_push_header(&req.headers, PG_S("Host"), PG_S("google.com"), &arena);
    *PG_DYN_PUSH(&req.url.path_components, allocator) = PG_S("foobar");

    PgString s = pg_http_request_to_string(req, allocator);
    PgString expected = PG_S("POST /foobar HTTP/1.1\r\n"
                             "Host: google.com\r\n"
                             "\r\n");
    PG_ASSERT(pg_string_eq(s, expected));
  }
}

static void test_http_parse_response_status_line() {
  // Empty.
  {
    PG_ASSERT(pg_http_parse_response_status_line(PG_S("")).err);
  }
  // Missing prefix.
  {
    PG_ASSERT(pg_http_parse_response_status_line(PG_S("HTT")).err);
    PG_ASSERT(pg_http_parse_response_status_line(PG_S("abc")).err);
    PG_ASSERT(pg_http_parse_response_status_line(PG_S("/1.1")).err);
  }
  // Missing slash.
  {
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP1.1 201 Created")).err);
  }
  // Missing major version.
  {
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP/.1 201 Created")).err);
  }
  // Missing `.`.
  {
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP/11 201 Created")).err);
  }
  // Missing minor version.
  {
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP/1. 201 Created")).err);
  }
  // Missing status code.
  {
    PG_ASSERT(pg_http_parse_response_status_line(PG_S("HTTP/1.1 Created")).err);
  }
  // Invalid major version.
  {
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP/abc.1 201 Created")).err);
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP/4.1 201 Created")).err);
  }
  // Invalid minor version.
  {
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP/1.10 201 Created")).err);
  }
  // Invalid status code.
  {
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP/1.1 99 Created")).err);
    PG_ASSERT(
        pg_http_parse_response_status_line(PG_S("HTTP/1.1 600 Created")).err);
  }
  // Valid, short.
  {
    PgHttpResponseStatusLineResult res =
        pg_http_parse_response_status_line(PG_S("HTTP/2.0 201"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(2 == res.res.version_major);
    PG_ASSERT(0 == res.res.version_minor);
    PG_ASSERT(201 == res.res.status);
  }
  // Valid, short, 0.9.
  {
    PgHttpResponseStatusLineResult res =
        pg_http_parse_response_status_line(PG_S("HTTP/0.9 201"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(0 == res.res.version_major);
    PG_ASSERT(9 == res.res.version_minor);
    PG_ASSERT(201 == res.res.status);
  }
  // Valid, long.
  {
    PgHttpResponseStatusLineResult res =
        pg_http_parse_response_status_line(PG_S("HTTP/1.1 404 Not found"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.res.version_major);
    PG_ASSERT(1 == res.res.version_minor);
    PG_ASSERT(404 == res.res.status);
  }
}

static void test_http_parse_request_status_line() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Empty.
  {
    PG_ASSERT(pg_http_parse_request_status_line(PG_S(""), allocator).err);
  }
  // Missing prefix.
  {
    PG_ASSERT(pg_http_parse_request_status_line(PG_S("GE"), allocator).err);
    PG_ASSERT(pg_http_parse_request_status_line(PG_S("abc"), allocator).err);
    PG_ASSERT(pg_http_parse_request_status_line(PG_S("123 "), allocator).err);
  }
  // Missing slash.
  {
    PG_ASSERT(
        pg_http_parse_request_status_line(PG_S("GET HTTP1.1"), allocator).err);
  }
  // Missing major version.
  {
    PG_ASSERT(
        pg_http_parse_request_status_line(PG_S("GET / HTTP/.1"), allocator)
            .err);
  }
  // Missing `.`.
  {
    PG_ASSERT(
        pg_http_parse_request_status_line(PG_S("GET / HTTP/11"), allocator)
            .err);
  }
  // Missing minor version.
  {
    PG_ASSERT(
        pg_http_parse_request_status_line(PG_S("GET / HTTP/1."), allocator)
            .err);
  }
  // Invalid major version.
  {
    PG_ASSERT(
        pg_http_parse_request_status_line(PG_S("GET / HTTP/abc.1"), allocator)
            .err);
    PG_ASSERT(
        pg_http_parse_request_status_line(PG_S("GET / HTTP/4.1"), allocator)
            .err);
  }
  // Invalid minor version.
  {
    PG_ASSERT(
        pg_http_parse_request_status_line(PG_S("GET / HTTP/1.10"), allocator)
            .err);
  }
  // Valid, short.
  {
    PgHttpRequestStatusLineResult res =
        pg_http_parse_request_status_line(PG_S("GET / HTTP/2.0"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(HTTP_METHOD_GET == res.res.method);
    PG_ASSERT(2 == res.res.version_major);
    PG_ASSERT(0 == res.res.version_minor);
    PG_ASSERT(0 == res.res.url.path_components.len);
    PG_ASSERT(0 == res.res.url.query_parameters.len);
  }
  // Valid, short with query parameters.
  {
    PgHttpRequestStatusLineResult res = pg_http_parse_request_status_line(
        PG_S("GET /?foo=bar& HTTP/2.0"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(HTTP_METHOD_GET == res.res.method);
    PG_ASSERT(2 == res.res.version_major);
    PG_ASSERT(0 == res.res.version_minor);
    PG_ASSERT(0 == res.res.url.path_components.len);
    PG_ASSERT(1 == res.res.url.query_parameters.len);
    PgKeyValue kv0 = PG_SLICE_AT(res.res.url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  // Valid, short, 0.9.
  {
    PgHttpRequestStatusLineResult res =
        pg_http_parse_request_status_line(PG_S("GET / HTTP/0.9"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(HTTP_METHOD_GET == res.res.method);
    PG_ASSERT(0 == res.res.version_major);
    PG_ASSERT(9 == res.res.version_minor);
    PG_ASSERT(0 == res.res.url.path_components.len);
    PG_ASSERT(0 == res.res.url.query_parameters.len);
  }
  // Valid, long.
  {
    PgHttpRequestStatusLineResult res = pg_http_parse_request_status_line(
        PG_S("GET /foo/bar/baz?hey HTTP/1.1"), allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(HTTP_METHOD_GET == res.res.method);
    PG_ASSERT(1 == res.res.version_major);
    PG_ASSERT(1 == res.res.version_minor);
    PG_ASSERT(3 == res.res.url.path_components.len);
    PG_ASSERT(1 == res.res.url.query_parameters.len);
    PgKeyValue kv0 = PG_SLICE_AT(res.res.url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("hey")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("")));
  }
}

static void test_http_parse_header() {
  // Empty.
  {
    PG_ASSERT(pg_http_parse_header(PG_S("")).err);
  }
  // Missing `:`.
  {
    PG_ASSERT(pg_http_parse_header(PG_S("foo bar")).err);
  }
  // Missing key.
  {
    PG_ASSERT(pg_http_parse_header(PG_S(":bcd")).err);
  }
  // Missing value.
  {
    PG_ASSERT(pg_http_parse_header(PG_S("foo:")).err);
  }
  // Multiple colons.
  {
    PgKeyValueResult res = pg_http_parse_header(PG_S("foo: bar : baz"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(res.res.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(res.res.value, PG_S("bar : baz")));
  }
  // Valid, one space before the value.
  {
    PgKeyValueResult res = pg_http_parse_header(PG_S("foo: bar"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(res.res.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(res.res.value, PG_S("bar")));
  }
  // Valid, no space before the value.
  {
    PgKeyValueResult res = pg_http_parse_header(PG_S("foo:bar"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(res.res.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(res.res.value, PG_S("bar")));
  }
  // Valid, multiple spaces before the value.
  {
    PgKeyValueResult res = pg_http_parse_header(PG_S("foo:   bar"));
    PG_ASSERT(0 == res.err);
    PG_ASSERT(pg_string_eq(res.res.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(res.res.value, PG_S("bar")));
  }
}

static void test_http_read_response() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Empty.
  {
    PgRing rg = {.data = pg_string_make(32, allocator)};
    PgHttpResponseReadResult res = pg_http_read_response(&rg, 128, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(false == res.done);
  }
  // Partial status line.
  {
    PgRing rg = {.data = pg_string_make(32, allocator)};
    PG_ASSERT(true == pg_ring_write_slice(&rg, PG_S("HTTP/1.")));
    PgHttpResponseReadResult res = pg_http_read_response(&rg, 128, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(false == res.done);
    PG_ASSERT(pg_ring_read_space(rg) == PG_S("HTTP/1.").len);
  }
  // Status line and some but not full.
  {
    PgRing rg = {.data = pg_string_make(32, allocator)};
    PG_ASSERT(true ==
              pg_ring_write_slice(&rg, PG_S("HTTP/1.1 201 Created\r\nHost:")));
    PgHttpResponseReadResult res = pg_http_read_response(&rg, 128, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(false == res.done);
  }

  // Full.
  {
    PgRing rg = {.data = pg_string_make(128, allocator)};

    {
      PG_ASSERT(true == pg_ring_write_slice(
                            &rg, PG_S("HTTP/1.1 201 Created\r\nHost:")));
      PgHttpResponseReadResult res = pg_http_read_response(&rg, 128, allocator);
      PG_ASSERT(0 == res.err);
      PG_ASSERT(false == res.done);
    }

    {
      PG_ASSERT(true == pg_ring_write_slice(&rg, PG_S("google.com\r")));
      PgHttpResponseReadResult res = pg_http_read_response(&rg, 128, allocator);
      PG_ASSERT(0 == res.err);
      PG_ASSERT(false == res.done);

      PG_ASSERT(true == pg_ring_write_slice(&rg, PG_S("\n")));
      res = pg_http_read_response(&rg, 128, allocator);
      PG_ASSERT(0 == res.err);
      PG_ASSERT(false == res.done);
    }

    {
      PG_ASSERT(true == pg_ring_write_slice(
                            &rg, PG_S("Authorization: Bearer foo\r\n\r\n")));
      PgHttpResponseReadResult res = pg_http_read_response(&rg, 128, allocator);
      PG_ASSERT(0 == res.err);
      PG_ASSERT(true == res.done);
      PG_ASSERT(1 == res.res.version_major);
      PG_ASSERT(1 == res.res.version_minor);
      PG_ASSERT(201 == res.res.status);
      PG_ASSERT(2 == res.res.headers.len);

      PgKeyValue kv0 = PG_SLICE_AT(res.res.headers, 0);
      PG_ASSERT(pg_string_eq(kv0.key, PG_S("Host")));
      PG_ASSERT(pg_string_eq(kv0.value, PG_S("google.com")));

      PgKeyValue kv1 = PG_SLICE_AT(res.res.headers, 1);
      PG_ASSERT(pg_string_eq(kv1.key, PG_S("Authorization")));
      PG_ASSERT(pg_string_eq(kv1.value, PG_S("Bearer foo")));
    }
  }
}

#if 0
static void test_http_request_response() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);

  u16 port = (u16)pg_rand_u32(3000, UINT16_MAX);
  PgSocket listen_socket = 0;
  {
    PgSocketResult res_create_socket = pg_net_create_tcp_socket();
    PG_ASSERT(0 == res_create_socket.err);
    listen_socket = res_create_socket.res;

    PG_ASSERT(0 == pg_net_socket_enable_reuse(listen_socket));
    PG_ASSERT(0 == pg_net_socket_set_blocking(listen_socket, false));

    PgIpv4Address addr = {0};
    addr.port = port;
    PG_ASSERT(0 == pg_net_tcp_bind_ipv4(listen_socket, addr));
    PG_ASSERT(0 == pg_net_tcp_listen(listen_socket, 1));
  }

  PgSocket client_socket = 0;
  {
    PgDnsResolveIpv4AddressSocketResult res_dns =
        pg_net_dns_resolve_ipv4_tcp(PG_S("localhost"), port, arena);
    PG_ASSERT(0 == res_dns.err);

    PG_ASSERT(port == res_dns.res.address.port);
    PG_ASSERT(0 != res_dns.res.socket);
    client_socket = res_dns.res.socket;
  }
  PG_ASSERT(0 == pg_net_socket_set_blocking(client_socket, false));

  PgAioQueueResult res_queue_create = pg_aio_queue_create();
  PG_ASSERT(0 == res_queue_create.err);

  PgAioQueue queue = res_queue_create.res;
  PgAioEventSlice events_change = PG_SLICE_MAKE(PgAioEvent, 3, &arena);
  events_change.len = 1;

  {
    PgAioEvent *event_server_listen = PG_SLICE_AT_PTR(&events_change, 0);
    event_server_listen->os_handle = (u64)listen_socket;
    event_server_listen->kind = PG_AIO_EVENT_KIND_IN;
    event_server_listen->action = PG_AIO_EVENT_ACTION_ADD;

    PG_ASSERT(0 == pg_aio_queue_ctl(queue, events_change));
  }

  PgSocket server_socket = 0;
  PgReader server_reader = {0};
  PgWriter server_writer = {0};

  PgWriter client_writer = pg_writer_make_from_socket(client_socket);
  PgReader client_reader = pg_reader_make_from_socket(client_socket);

  PgRing client_recv = {.data = pg_string_make(128, &arena)};
  PgRing client_send = {.data = pg_string_make(128, &arena)};
  PgRing server_recv = {.data = pg_string_make(128, &arena)};
  PgRing server_send = {.data = pg_string_make(128, &arena)};

  PgWriter client_send_writer = pg_writer_make_from_ring(&client_send);
  PgWriter server_send_writer = pg_writer_make_from_ring(&server_send);

  bool client_recv_http_io_done = false;
  bool client_send_http_io_done = false;
  bool server_recv_http_io_done = false;
  bool server_send_http_io_done = false;

  PgHttpRequest client_req = {0};
  client_req.method = HTTP_METHOD_GET;
  pg_http_push_header(&client_req.headers, PG_S("Host"), PG_S("localhost"),
                      &arena);
  *PG_DYN_PUSH(&client_req.url.query_parameters, &arena) = (PgKeyValue){
      .key = PG_S("uploaded"),
      .value = pg_u64_to_string(123456, &arena),
  };

  PgHttpResponse server_res = {0};
  server_res.status = 200;
  server_res.version_major = 1;
  server_res.version_minor = 1;
  pg_http_push_header(&server_res.headers, PG_S("Accept"),
                      PG_S("application/json"), &arena);

  PgHttpRequest server_req = {0};
  PgHttpResponse client_res = {0};

  PgAioEventSlice events_watch = PG_SLICE_MAKE(PgAioEvent, 3, &arena);

  for (u64 _i = 0; _i <= 128; _i++) {
    Pgu64Result res_wait = pg_aio_queue_wait(queue, events_watch, -1, arena);
    PG_ASSERT(0 == res_wait.err);

    for (u64 i = 0; i < res_wait.res; i++) {
      PgAioEvent event = PG_SLICE_AT(events_watch, i);
      PG_ASSERT(0 == (PG_AIO_EVENT_KIND_ERR & event.kind));

      if (event.os_handle == (u64)listen_socket) {
        PgIpv4AddressAcceptResult res_accept = pg_net_tcp_accept(listen_socket);
        PG_ASSERT(0 == res_accept.err);
        PG_ASSERT(0 != res_accept.socket);

        events_change.len = 2;
        PgAioEvent *event_client = PG_SLICE_AT_PTR(&events_change, 0);
        event_client->os_handle = (u64)client_socket;
        event_client->kind = PG_AIO_EVENT_KIND_OUT;
        event_client->action = PG_AIO_EVENT_ACTION_ADD;

        server_socket = res_accept.socket;
        server_reader = pg_reader_make_from_socket(server_socket);
        server_writer = pg_writer_make_from_socket(server_socket);

        PgAioEvent *event_server_client = PG_SLICE_AT_PTR(&events_change, 1);
        event_server_client->os_handle = (u64)res_accept.socket;
        event_server_client->kind = PG_AIO_EVENT_KIND_IN;
        event_server_client->action = PG_AIO_EVENT_ACTION_ADD;

        PG_ASSERT(0 == pg_aio_queue_ctl(queue, events_change));
        events_change.len = 0;
      } else if (event.os_handle == (u64)client_socket) {
        if (PG_AIO_EVENT_KIND_OUT & event.kind) {
          if (!client_send_http_io_done) {
            pg_http_write_request(&client_send_writer, client_req);
            PG_ASSERT(true == pg_ring_write_slice(&client_send,
                                                  PG_S("client request body")));
            PG_ASSERT(0 ==
                      pg_writer_write(&client_send_writer, &client_send, arena).err);
            client_send_http_io_done = true;

            // Stop subscribing for writing, start subscribing for reading.
            events_change.len = 1;
            PgAioEvent *event_client = PG_SLICE_AT_PTR(&events_change, 0);
            event_client->os_handle = (u64)client_socket;
            event_client->kind = PG_AIO_EVENT_KIND_IN;
            event_client->action = PG_AIO_EVENT_ACTION_MOD;
            PG_ASSERT(0 == pg_aio_queue_ctl(queue, events_change));
            events_change.len = 0;
          }
        }
        if (PG_AIO_EVENT_KIND_IN & event.kind) {
          if (!client_recv_http_io_done) {
            PG_ASSERT(0 ==
                      pg_reader_read(&client_reader, &client_recv, arena).err);
            PgHttpResponseReadResult res =
                pg_http_read_response(&client_recv, 128, &arena);
            PG_ASSERT(0 == res.err);
            PG_ASSERT(true == res.done);
            client_res = res.res;
            client_recv_http_io_done = true;
            goto end;
          }
        }
      } else if (event.os_handle == (u64)server_socket) {
        if (PG_AIO_EVENT_KIND_IN & event.kind) {
          if (!server_recv_http_io_done) {
            PG_ASSERT(0 ==
                      pg_reader_read(&server_reader, &server_recv, arena).err);
            PgHttpRequestReadResult res =
                pg_http_read_request(&server_recv, 128, &arena);
            PG_ASSERT(0 == res.err);
            PG_ASSERT(true == res.done);
            server_req = res.res;
            server_recv_http_io_done = true;

            // Stop subscribing for reading, start subscribing for writing.
            events_change.len = 1;
            PgAioEvent *event_server = PG_SLICE_AT_PTR(&events_change, 0);
            event_server->os_handle = (u64)server_socket;
            event_server->kind = PG_AIO_EVENT_KIND_OUT;
            event_server->action = PG_AIO_EVENT_ACTION_MOD;
            PG_ASSERT(0 == pg_aio_queue_ctl(queue, events_change));
            events_change.len = 0;
          }
        }
        if (PG_AIO_EVENT_KIND_OUT & event.kind) {
          if (!server_send_http_io_done) {
            PG_ASSERT(0 == pg_http_write_response(&server_send, server_res));
            PG_ASSERT(
                0 ==
                pg_writer_write_from_reader(&server_writer, &server_send).err);
            server_send_http_io_done = true;

            // Stop subscribing.
            events_change.len = 1;
            PgAioEvent *event_server = PG_SLICE_AT_PTR(&events_change, 0);
            event_server->os_handle = (u64)server_socket;
            event_server->action = PG_AIO_EVENT_ACTION_DEL;
            PG_ASSERT(0 == pg_aio_queue_ctl(queue, events_change));
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
  PgKeyValue query0 = PG_SLICE_AT(server_req.url.query_parameters, 0);
  PG_ASSERT(pg_string_eq(query0.key, PG_S("uploaded")));
  PG_ASSERT(pg_string_eq(query0.value, PG_S("123456")));

  PG_ASSERT(client_res.status = server_res.status);
  PG_ASSERT(client_res.version_major = server_res.version_major);
  PG_ASSERT(client_res.version_minor = server_res.version_minor);
  PG_ASSERT(client_res.headers.len = server_res.headers.len);

  PG_ASSERT(0 == pg_net_socket_close(client_socket));
  PG_ASSERT(0 == pg_net_socket_close(server_socket));
  PG_ASSERT(0 == pg_net_socket_close(listen_socket));
}
#endif

static void test_log() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
  // Simple log.
  {
    Pgu8Dyn sb = {0};
    PG_DYN_ENSURE_CAP(&sb, 256, allocator);

    PgLogger logger = pg_log_make_logger_stdout_logfmt(PG_LOG_LEVEL_DEBUG);
    logger.writer = pg_writer_make_from_string_builder(&sb, allocator);

    pg_log(&logger, PG_LOG_LEVEL_INFO, "hello world",
           pg_log_cs("foo", PG_S("bar")), pg_log_ci64("baz", -317));

    PgString out = PG_DYN_SLICE(PgString, sb);
    PG_ASSERT(pg_string_starts_with(out, PG_S("level=info ")));
  }
  // PgLog but the logger level is higher.
  {
    Pgu8Dyn sb = {0};
    PG_DYN_ENSURE_CAP(&sb, 256, allocator);
    PgLogger logger = pg_log_make_logger_stdout_logfmt(PG_LOG_LEVEL_INFO);
    logger.writer = pg_writer_make_from_string_builder(&sb, allocator);

    pg_log(&logger, PG_LOG_LEVEL_DEBUG, "hello world",
           pg_log_s(PG_S("foo"), PG_S("bar")));

    PgString out = PG_DYN_SLICE(PgString, sb);
    PG_ASSERT(pg_string_is_empty(out));
  }
}

static void test_div_ceil() {
  PG_ASSERT(1 == pg_div_ceil(1, 1));
  PG_ASSERT(1 == pg_div_ceil(1, 2));
  PG_ASSERT(2 == pg_div_ceil(2, 1));
  PG_ASSERT(1 == pg_div_ceil(2, 2));
  PG_ASSERT(1 == pg_div_ceil(2, 3));
  PG_ASSERT(2 == pg_div_ceil(5, 4));
}

static void test_path_base_name() {
  PG_ASSERT(pg_string_eq(PG_S(""), pg_path_base_name(PG_S(""))));
  PG_ASSERT(
      pg_string_eq(PG_S(""), pg_path_base_name(PG_S("" PG_PATH_SEPARATOR_S))));
  PG_ASSERT(pg_string_eq(PG_S("foo"), pg_path_base_name(PG_S("foo"))));
  PG_ASSERT(pg_string_eq(PG_S("foo.mp3"), pg_path_base_name(PG_S("foo.mp3"))));
  PG_ASSERT(
      pg_string_eq(PG_S("foo.mp3"),
                   pg_path_base_name(PG_S(PG_PATH_SEPARATOR_S
                                          "a" PG_PATH_SEPARATOR_S
                                          "b" PG_PATH_SEPARATOR_S "foo.mp3"))));
  PG_ASSERT(pg_string_eq(
      PG_S("b foo.mp3"),
      pg_path_base_name(
          PG_S("" PG_PATH_SEPARATOR_S "a" PG_PATH_SEPARATOR_S "b foo.mp3"))));
  PG_ASSERT(pg_string_eq(
      PG_S("foo.mp3"),
      pg_path_base_name(PG_S("/a" PG_PATH_SEPARATOR_S "b" PG_PATH_SEPARATOR_S
                             ".." PG_PATH_SEPARATOR_S "foo.mp3"))));
  PG_ASSERT(
      pg_string_eq(PG_S("foo.mp3"),
                   pg_path_base_name(PG_S("." PG_PATH_SEPARATOR_S "foo.mp3"))));
}

typedef struct {
  u64 value;
  PgHeapNode heap;
} u64Node;

static bool u64_node_less_than(PgHeapNode *a, PgHeapNode *b) {
  PG_ASSERT(a);
  PG_ASSERT(b);

  u64Node *una = PG_CONTAINER_OF(a, u64Node, heap);
  u64Node *unb = PG_CONTAINER_OF(b, u64Node, heap);

  return una->value < unb->value;
}

static void test_heap_insert_dequeue() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);

  PgHeap heap = {0};
  u64 values[] = {100, 19, 36, 17, 3, 25, 1, 2, 7};

  for (u64 i = 0; i < PG_STATIC_ARRAY_LEN(values); i++) {
    u64 value = PG_C_ARRAY_AT(values, PG_STATIC_ARRAY_LEN(values), i);
    u64Node *node = pg_arena_new(&arena, u64Node, 1);
    node->value = value;
    pg_heap_insert(&heap, &node->heap, u64_node_less_than);

    pg_heap_node_sanity_check(heap.root, u64_node_less_than);
  }
  PG_ASSERT(PG_STATIC_ARRAY_LEN(values) == heap.count);

  u64Node *n1 = PG_CONTAINER_OF(heap.root, u64Node, heap);
  PG_ASSERT(1 == n1->value);

  u64Node *n2 = PG_CONTAINER_OF(n1->heap.left, u64Node, heap);
  PG_ASSERT(2 == n2->value);

  u64Node *n3 = PG_CONTAINER_OF(n1->heap.right, u64Node, heap);
  PG_ASSERT(3 == n3->value);

  u64 last_min = 0;

  for (u64 i = 0; i < PG_STATIC_ARRAY_LEN(values); i++) {
    u64Node *root = PG_CONTAINER_OF(heap.root, u64Node, heap);
    PG_ASSERT(root);
    u64 min = root->value;
    // Check that we see values in ascending order.
    PG_ASSERT(last_min < min);

    pg_heap_dequeue(&heap, u64_node_less_than);
    pg_heap_node_sanity_check(heap.root, u64_node_less_than);
  }

  PG_ASSERT(0 == heap.count);
  PG_ASSERT(!heap.root);
}

static void test_heap_remove_in_the_middle() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);

  PgHeap heap = {0};
  u64 values[] = {100, 19, 36, 17, 3, 25, 1, 2, 7};

  for (u64 i = 0; i < PG_STATIC_ARRAY_LEN(values); i++) {
    u64 value = PG_C_ARRAY_AT(values, PG_STATIC_ARRAY_LEN(values), i);
    u64Node *node = pg_arena_new(&arena, u64Node, 1);
    node->value = value;

    pg_heap_insert(&heap, &node->heap, u64_node_less_than);
    pg_heap_node_sanity_check(heap.root, u64_node_less_than);
  }
  PG_ASSERT(PG_STATIC_ARRAY_LEN(values) == heap.count);

  PgHeapNode *rm = heap.root->left->right;
  PG_ASSERT(19 == PG_CONTAINER_OF(rm, u64Node, heap)->value);

  pg_heap_node_remove(&heap, rm, u64_node_less_than);
  pg_heap_node_sanity_check(heap.root, u64_node_less_than);

  PG_ASSERT(PG_STATIC_ARRAY_LEN(values) - 1 == heap.count);
  PG_ASSERT(heap.root);
}

static void test_process_no_capture() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgStringDyn args = {0};
  *PG_DYN_PUSH(&args, allocator) = PG_S("ls-files");

  PgProcessSpawnOptions options = {0};
  PgProcessResult res_spawn = pg_process_spawn(
      PG_S("git"), PG_DYN_SLICE(PgStringSlice, args), options, allocator);
  PG_ASSERT(0 == res_spawn.err);

  PgProcess process = res_spawn.res;

  PgProcessExitResult res_wait = pg_process_wait(process, allocator);
  PG_ASSERT(0 == res_wait.err);

  PgProcessStatus status = res_wait.res;
  PG_ASSERT(0 == status.exit_status);
  PG_ASSERT(0 == status.signal);
  PG_ASSERT(status.exited);
  PG_ASSERT(!status.signaled);
  PG_ASSERT(!status.core_dumped);
  PG_ASSERT(!status.stopped);

  // Not captured.
  PG_ASSERT(pg_string_is_empty(status.stdout_captured));
  PG_ASSERT(pg_string_is_empty(status.stderr_captured));
}

static void test_process_capture() {
  PgArena arena = pg_arena_make_from_virtual_mem(16 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgStringDyn args = {0};
  *PG_DYN_PUSH(&args, allocator) = PG_S("ls-files");

  PgProcessSpawnOptions options = {
      .stdout_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
      .stderr_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
  };
  PgProcessResult res_spawn = pg_process_spawn(
      PG_S("git"), PG_DYN_SLICE(PgStringSlice, args), options, allocator);
  PG_ASSERT(0 == res_spawn.err);

  PgProcess process = res_spawn.res;

  PgProcessExitResult res_wait = pg_process_wait(process, allocator);
  PG_ASSERT(0 == res_wait.err);

  PgProcessStatus status = res_wait.res;
  PG_ASSERT(0 == status.exit_status);
  PG_ASSERT(0 == status.signal);
  PG_ASSERT(status.exited);
  PG_ASSERT(!status.signaled);
  PG_ASSERT(!status.core_dumped);
  PG_ASSERT(!status.stopped);

  PG_ASSERT(pg_string_contains(status.stdout_captured, PG_S("test.c")));
  PG_ASSERT(pg_string_is_empty(status.stderr_captured));
}

static void test_process_stdin() {
  PgArena arena = pg_arena_make_from_virtual_mem(16 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgStringDyn args = {0};
  *PG_DYN_PUSH(&args, allocator) = PG_S("lo wo");

  PgProcessSpawnOptions options = {
      .stdin_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
      .stdout_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
      .stderr_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
  };
  PgProcessResult res_spawn = pg_process_spawn(
      PG_S("grep"), PG_DYN_SLICE(PgStringSlice, args), options, allocator);
  PG_ASSERT(0 == res_spawn.err);

  PgProcess process = res_spawn.res;

  PgString msg = PG_S("hello world");
  PgU64Result res_write = pg_file_write(process.stdin_pipe, msg);
  PG_ASSERT(0 == res_write.err);
  PG_ASSERT(msg.len == res_write.res);

  PG_ASSERT(0 == pg_file_close(process.stdin_pipe));
  process.stdin_pipe.fd = 0;

  PgProcessExitResult res_wait = pg_process_wait(process, allocator);
  PG_ASSERT(0 == res_wait.err);

  PgProcessStatus status = res_wait.res;
  PG_ASSERT(0 == status.exit_status);
  PG_ASSERT(0 == status.signal);
  PG_ASSERT(status.exited);
  PG_ASSERT(!status.signaled);
  PG_ASSERT(!status.core_dumped);
  PG_ASSERT(!status.stopped);

  PG_ASSERT(pg_string_contains(status.stdout_captured, msg));
  PG_ASSERT(pg_string_is_empty(status.stderr_captured));
}

static void test_html_tokenize_no_attributes() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<html>foo</html>");
  PgHtmlTokenDynResult res = pg_html_tokenize(s, allocator);
  PG_ASSERT(0 == res.err);

  PgHtmlTokenDyn tokens = res.res;
  PG_ASSERT(3 == tokens.len);

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(1 == token.start);
    PG_ASSERT(5 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(6 == token.start);
    PG_ASSERT(9 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("foo"), token.text));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(11 == token.start);
    PG_ASSERT(15 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
}

static void test_html_tokenize_with_attributes() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<html id=\"bar\" class=\"ba/z\"  > foo  </html>");
  PgHtmlTokenDynResult res = pg_html_tokenize(s, allocator);
  PG_ASSERT(0 == res.err);

  PgHtmlTokenDyn tokens = res.res;
  PG_ASSERT(5 == tokens.len);

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(1 == token.start);
    PG_ASSERT(5 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_ATTRIBUTE == token.kind);
    PG_ASSERT(6 == token.start);
    PG_ASSERT(13 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("id"), token.attribute.key));
    PG_ASSERT(pg_string_eq(PG_S("bar"), token.attribute.value));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_ATTRIBUTE == token.kind);
    PG_ASSERT(15 == token.start);
    PG_ASSERT(26 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("class"), token.attribute.key));
    PG_ASSERT(pg_string_eq(PG_S("ba/z"), token.attribute.value));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 3);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(30 == token.start);
    PG_ASSERT(36 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("foo"), pg_string_trim_space(token.text)));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 4);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(38 == token.start);
    PG_ASSERT(42 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
}

static void test_html_tokenize_with_key_no_value() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<html aria-hidden>foo</html>");
  PgHtmlTokenDynResult res = pg_html_tokenize(s, allocator);
  PG_ASSERT(0 == res.err);

  PgHtmlTokenDyn tokens = res.res;
  PG_ASSERT(4 == tokens.len);

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(1 == token.start);
    PG_ASSERT(5 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_ATTRIBUTE == token.kind);
    PG_ASSERT(6 == token.start);
    PG_ASSERT(17 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("aria-hidden"), token.attribute.key));
    PG_ASSERT(pg_string_eq(PG_S(""), token.attribute.value));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(18 == token.start);
    PG_ASSERT(21 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("foo"), pg_string_trim_space(token.text)));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 3);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(23 == token.start);
    PG_ASSERT(27 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
}

static void test_html_tokenize_nested() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<html id=\"bar\" > foo <span>bar</span> </html>");
  PgHtmlTokenDynResult res = pg_html_tokenize(s, allocator);
  PG_ASSERT(0 == res.err);

  PgHtmlTokenDyn tokens = res.res;
  PG_ASSERT(7 == tokens.len);

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(1 == token.start);
    PG_ASSERT(5 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_ATTRIBUTE == token.kind);
    PG_ASSERT(6 == token.start);
    PG_ASSERT(13 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("id"), token.attribute.key));
    PG_ASSERT(pg_string_eq(PG_S("bar"), token.attribute.value));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(16 == token.start);
    PG_ASSERT(21 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("foo"), pg_string_trim_space(token.text)));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 3);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(22 == token.start);
    PG_ASSERT(26 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("span"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 4);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(27 == token.start);
    PG_ASSERT(30 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("bar"), pg_string_trim_space(token.text)));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 5);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(32 == token.start);
    PG_ASSERT(36 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("span"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 6);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(40 == token.start);
    PG_ASSERT(44 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
}

int main() {
  test_rune_bytes_count();
  test_utf8_count();
  test_string_last();
  test_string_first();
  test_string_indexof_rune();
  test_string_last_indexof_rune();
  test_slice_range();
  test_utf8_iterator();
  test_string_indexof_string();
  test_string_trim();
  test_string_cut();
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
#if 0
  test_log_entry_quote_value();
  test_make_log_line();
#endif
  test_u8x4_be_to_u32_and_back();
  test_bitfield();
  test_ring_buffer_write_slice();
  test_ring_buffer_read_write_slice();
  test_ring_buffer_read_until_excl();
  test_ring_buffer_read_write_fuzz();
#if 0
  test_url_parse_relative_path();
  test_url_parse();
  test_http_request_to_string();
  test_http_parse_response_status_line();
  test_http_parse_request_status_line();
  test_http_parse_header();
  test_http_read_response();
  // test_http_request_response();
  test_log();
#endif
  test_div_ceil();
#if 0
  test_path_base_name();
#endif
  test_heap_insert_dequeue();
  test_heap_remove_in_the_middle();
#if 0
  test_process_no_capture();
  test_process_capture();
  test_process_stdin();
  test_html_tokenize_no_attributes();
  test_html_tokenize_with_key_no_value();
  test_html_tokenize_with_attributes();
  test_html_tokenize_nested();
#endif
}
