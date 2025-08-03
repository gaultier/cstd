#include "lib.c"
#include <stdatomic.h>

static void test_rune_bytes_count() {
  PG_ASSERT(1 == pg_utf8_rune_bytes_count(0));
  PG_ASSERT(1 == pg_utf8_rune_bytes_count('A'));
  PG_ASSERT(3 == pg_utf8_rune_bytes_count(0x30eb /* ル */));
  PG_ASSERT(3 == pg_utf8_rune_bytes_count(0x805e /* 聞 */));
  PG_ASSERT(4 == pg_utf8_rune_bytes_count(0x1f34c /* 🍌 */));
}

static void test_utf8_count() {
  PG_RESULT(u64, PgError) res = pg_utf8_count_runes(PG_S("🚀🛸"));
  u64 count = PG_UNWRAP(res);
  PG_ASSERT(2 == count);
}

static void test_string_last() {
  // Empty
  {
    PG_ASSERT(!pg_string_last(PG_S("")).has_value);
  }
  {
    PG_OPTION(PgRune) res = pg_string_last(PG_S("朝日新聞デジタル聞"));
    PG_ASSERT(res.has_value);
    PG_ASSERT(0x805e /* 聞 */ == res.value);
  }
}

static void test_string_first() {
  // Empty
  {
    PG_ASSERT(!pg_string_first(PG_S("")).has_value);
  }
  {
    PG_OPTION(PgRune) first_opt = pg_string_first(PG_S("聞デジタル"));
    PG_ASSERT(first_opt.has_value);
    PG_ASSERT(0x805e /* 聞 */ == first_opt.value);
  }
}

static void test_string_index_of_rune() {
  // Empty.
  {
    PG_ASSERT(-1 == pg_string_index_of_rune(PG_S(""), 0x805e /* 聞 */));
  }

  // Unicode
  {
    PgString haystack = PG_S("朝日新聞デジタル聞");
    PG_ASSERT(9 == pg_string_index_of_rune(haystack, 0x805e /* 聞 */));
    PG_ASSERT(-1 == pg_string_index_of_rune(haystack, 0x1f34c /* 🍌 */));
  }
}

static void test_string_last_index_of_rune() {
  // Empty.
  {
    PG_ASSERT(-1 == pg_string_last_index_of_rune(PG_S(""), 0x805e /* 聞 */));
  }

  // Unicode
  {
    PgString haystack = PG_S("朝日新聞デジタ聞ル");
    PG_ASSERT(21 == pg_string_last_index_of_rune(haystack, 0x805e /* 聞 */));
    PG_ASSERT(-1 == pg_string_last_index_of_rune(haystack, 0x1f34c /* 🍌 */));
  }
}

static void test_string_index_of_string() {
  // Empty haystack.
  {
    PG_ASSERT(-1 == pg_string_index_of_string((PgString){0}, PG_S("fox")));
  }

  // Empty needle.
  {
    PG_ASSERT(-1 == pg_string_index_of_string(PG_S("hello"), (PgString){0}));
  }

  // Not found.
  {
    PG_ASSERT(-1 == pg_string_index_of_string(PG_S("hello 🍌 world"),
                                              PG_S("foobar 🍌")));
  }

  // Found, one occurence.
  {
    PG_ASSERT(11 ==
              pg_string_index_of_string(PG_S("hello 🍌 world"), PG_S("world")));
  }

  // Found, one occurence.
  {
    PG_ASSERT(0 == pg_string_index_of_string(PG_S("hello 🍌 world\n"),
                                             PG_S("hello 🍌 world")));
  }

  // Found, first occurence.
  {
    PG_ASSERT(
        6 == pg_string_index_of_string(PG_S("world hello hell"), PG_S("hell")));
  }

  // Found, second occurence.
  {
    PG_ASSERT(10 ==
              pg_string_index_of_string(PG_S("hello fox foxy"), PG_S("foxy")));
  }

  // Almost found, prefix matches.
  {
    PG_ASSERT(-1 ==
              pg_string_index_of_string(PG_S("hello world"), PG_S("worldly")));
  }

  {
    PG_ASSERT(0 == pg_string_index_of_string(PG_S("/"), PG_S("/")));
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

static void test_path_stem() {
  {
    PgString trimmed = pg_path_stem(PG_S("foo/bar.md"));
    PG_ASSERT(pg_string_eq(trimmed, PG_S("foo/bar")));
  }
}

static void test_string_cut() {
  {
    PgStringCut cut = pg_string_cut_string(PG_S("🍌🍌foo🍌"), PG_S("🍌"));
    PG_ASSERT(cut.has_value);
    PG_ASSERT(0 == cut.left.len);
    PG_ASSERT(pg_string_eq(PG_S("🍌foo🍌"), cut.right));
  }
  {
    PgStringCut cut = pg_string_cut_string(PG_S("🍌🍌foo🍌"), PG_S("fo"));
    PG_ASSERT(cut.has_value);
    PG_ASSERT(pg_string_eq(PG_S("🍌🍌"), cut.left));
    PG_ASSERT(pg_string_eq(PG_S("o🍌"), cut.right));
  }
  {
    PgStringCut cut = pg_string_cut_string(PG_S("🍌🍌foo🍌"), PG_S("✨"));
    PG_ASSERT(!cut.has_value);
  }
}

static void test_string_split_byte() {
  PgString s = PG_S("hello..world...foobar");
  PgSplitIterator it = pg_string_split_string(s, PG_S("."));

  {
    PG_OPTION(PgString) split_opt = pg_string_split_next(&it);
    PG_ASSERT(true == split_opt.has_value);
    PG_ASSERT(pg_string_eq(split_opt.value, PG_S("hello")));
  }

  {
    PG_OPTION(PgString) split_opt = pg_string_split_next(&it);
    PG_ASSERT(true == split_opt.has_value);
    PG_ASSERT(pg_string_eq(split_opt.value, PG_S("world")));
  }

  {
    PG_OPTION(PgString) split_opt = pg_string_split_next(&it);
    PG_ASSERT(true == split_opt.has_value);
    PG_ASSERT(pg_string_eq(split_opt.value, PG_S("foobar")));
  }

  PG_ASSERT(false == pg_string_split_next(&it).has_value);
  PG_ASSERT(false == pg_string_split_next(&it).has_value);
}

static void test_string_split_string() {
  PgString s = PG_S("hello🚀🛸world🚀little🚀🛸thing !");
  PgSplitIterator it = pg_string_split_string(s, PG_S("🚀🛸"));

  {
    PG_OPTION(PgString) split_opt = pg_string_split_next(&it);
    PG_ASSERT(true == split_opt.has_value);
    PG_ASSERT(pg_string_eq(split_opt.value, PG_S("hello")));
  }

  {
    PG_OPTION(PgString) split_opt = pg_string_split_next(&it);
    PG_ASSERT(true == split_opt.has_value);
    PG_ASSERT(pg_string_eq(split_opt.value, PG_S("world🚀little")));
  }

  {
    PG_OPTION(PgString) split_opt = pg_string_split_next(&it);
    PG_ASSERT(true == split_opt.has_value);
    PG_ASSERT(pg_string_eq(split_opt.value, PG_S("thing !")));
  }

  PG_ASSERT(false == pg_string_split_next(&it).has_value);
  PG_ASSERT(false == pg_string_split_next(&it).has_value);
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
    PG_DYN_PUSH(&dyn, 1, allocator);
    PG_ASSERT(1 == dyn.len);
    PG_ASSERT(2 == dyn.cap);

    u64 arena_size = arena_cap - ((u64)arena.end - (u64)arena.start);
    PG_ASSERT(2 == arena_size);
    PG_ASSERT(dyn.cap == arena_size);

    u64 desired_cap = 13;
    PG_DYN_ENSURE_CAP(&dyn, desired_cap, allocator);
    PG_ASSERT(16 == dyn.cap);
    PG_ASSERT(16 == pg_arena_mem_use(arena));
  }
  // General case.
  {
    PgArena arena = pg_arena_make_from_virtual_mem(arena_cap);
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    Pgu8Dyn dyn = {0};
    PG_DYN_PUSH(&dyn, 1, allocator);
    PG_ASSERT(1 == dyn.len);
    PG_ASSERT(2 == dyn.cap);

    Pgu8Dyn dummy = {0};
    PG_DYN_PUSH(&dummy, 2, allocator);
    PG_DYN_PUSH(&dummy, 3, allocator);

    PG_ASSERT(2 + 2 == pg_arena_mem_use(arena));

    // This triggers a new allocation.
    PG_DYN_PUSH(&dummy, 4, allocator);
    PG_ASSERT(3 == dummy.len);
    PG_ASSERT(4 == dummy.cap);

    PG_ASSERT(2 + 4 == pg_arena_mem_use(arena));

    u64 desired_cap = 13;
    PG_DYN_ENSURE_CAP(&dyn, desired_cap, allocator);
    PG_ASSERT(16 == dyn.cap);

    PG_ASSERT(16 + 6 == pg_arena_mem_use(arena));
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

  PG_DYN(PgString) dyn = {0};
  // Works on empty slices.
  (void)PG_SLICE_RANGE(PG_DYN_TO_SLICE(PG_SLICE(PgString), dyn), 0, 0);

  PG_DYN_PUSH(&dyn, PG_S("hello \"world\n\"!"), allocator);
  PG_DYN_PUSH(&dyn, PG_S("日"), allocator);
  PG_DYN_PUSH(&dyn, PG_S("本語"), allocator);

  PG_SLICE(PgString) s = PG_DYN_TO_SLICE(PG_SLICE(PgString), dyn);
  PG_SLICE(PgString) range = PG_SLICE_RANGE_START(s, 1UL);
  PG_ASSERT(2 == range.len);

  PG_ASSERT(pg_string_eq(PG_SLICE_AT(s, 1), PG_SLICE_AT(range, 0)));
  PG_ASSERT(pg_string_eq(PG_SLICE_AT(s, 2), PG_SLICE_AT(range, 1)));
}

static void test_utf8_iterator() {
  // Empty.
  {
    PgString s = PG_S("");
    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    u64 count = PG_UNWRAP(res_count);
    PG_ASSERT(0 == count);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);

    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(res.end);
  }
  {
    PgString s = PG_S("2匹の🀅🂣©");
    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    u64 count = PG_UNWRAP(res_count);
    PG_ASSERT(6 == count);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);

    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    PG_ASSERT(0x32 == res.rune);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    PG_ASSERT(0x5339 == res.rune);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    PG_ASSERT(0x306e == res.rune);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    PG_ASSERT(0x1f005 == res.rune);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    PG_ASSERT(0x1f0a3 == res.rune);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    PG_ASSERT(0x00A9 == res.rune);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(res.end);
    PG_ASSERT(0 == res.rune);
  }
  // Null byte.
  {
    PgString s = PG_S("\x1\x0");
    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    u64 count = PG_UNWRAP(res_count);
    PG_ASSERT(2 == count);

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);

    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(res.end);
  }
  // Forbidden byte.
  {
    PgString s = PG_S("\x1\xc0");
    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res_count));

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Forbidden byte.
  {
    PgString s = PG_S("\x1\xf5");
    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res_count));

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Continuation byte but EOF.
  {
    PgString s = PG_S("\x1\x80");
    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res_count));

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // EOF
  {
    PgString s = PG_S("\x1🀅");
    s.len -= 1; // Early EOF.

    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res_count));

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Too high.
  {
    PgString s = PG_S("\x1\xf4\x90\x90\x90");

    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res_count));

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Overlong.
  {
    PgString s = PG_S("\x1\xc0\x80");

    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res_count));

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
  // Overlong.
  {
    PgString s = PG_S("\x2F\xC0\xAE\x2E\x2F");

    PG_RESULT(u64, PgError) res_count = pg_utf8_count_runes(s);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res_count));

    PgUtf8Iterator it = pg_make_utf8_iterator(s);
    PgRuneUtf8Result res = {0};
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(!res.end);
    res = pg_utf8_iterator_next(&it);
    PG_ASSERT(PG_ERR_INVALID_VALUE == res.err);
  }
}

static void test_string_consume() {
  {
    PG_OPTION(PgString) consume_opt = pg_string_consume_rune(PG_S(""), '{');
    PG_ASSERT(!consume_opt.has_value);
  }
  {
    PG_OPTION(PgString)
    consume_opt = pg_string_consume_rune(PG_S("[1,2]"), '{');
    PG_ASSERT(!consume_opt.has_value);
  }
  {
    PG_OPTION(PgString)
    res = pg_string_consume_rune(PG_S("🍌[1,2]"), 0x1f34c /* 🍌 */);
    PG_ASSERT(res.has_value);
    PG_ASSERT(pg_string_eq(PG_S("[1,2]"), res.value));
  }
}

static void test_string_parse_u64() {
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S(""), 10, true);
    PG_ASSERT(!num_res.present);
    PG_ASSERT(0 == num_res.remaining.len);
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("🍌"), 10, true);
    PG_ASSERT(!num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("🍌"), num_res.remaining));
  }
  {
    PgParseNumberResult num_res =
        pg_string_parse_u64(PG_S("🍌123🍌"), 10, true);
    PG_ASSERT(!num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("🍌123🍌"), num_res.remaining));
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("0123"), 10, true);
    PG_ASSERT(!num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("0123"), num_res.remaining));
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("0"), 10, true);
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S(""), num_res.remaining));
    PG_ASSERT(0 == num_res.n);
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("0🍌"), 10, true);
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("🍌"), num_res.remaining));
    PG_ASSERT(0 == num_res.n);
  }
  {
    PgParseNumberResult num_res = pg_string_parse_u64(PG_S("123🍌"), 10, true);
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("🍌"), num_res.remaining));
    PG_ASSERT(123 == num_res.n);
  }
  // Allow leading zero.
  {
    PgParseNumberResult num_res =
        pg_string_parse_u64(PG_S("0123🍌"), 10, false);
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("🍌"), num_res.remaining));
    PG_ASSERT(123 == num_res.n);
  }
  // Base 16, valid.
  {
    PgParseNumberResult num_res =
        pg_string_parse_u64(PG_S("012f🍌"), 16, false);
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("🍌"), num_res.remaining));
    PG_ASSERT(0x12f == num_res.n);
  }
  // Base 16, non hex characters.
  {
    PgParseNumberResult num_res =
        pg_string_parse_u64(PG_S("012g🍌"), 16, false);
    PG_ASSERT(num_res.present);
    PG_ASSERT(pg_string_eq(PG_S("g🍌"), num_res.remaining));
    PG_ASSERT(0x12 == num_res.n);
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
    PgString s = pg_string_clone(PG_S("hello world!"), allocator);
    PG_SLICE_SWAP_REMOVE(&s, 4);
    PG_ASSERT(pg_string_eq(s, PG_S("hell! world")));
  }
}

static void test_dynu8_append_u8_hex_upper() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  {
    PgWriter w = pg_writer_make_string_builder(4, allocator);
    PG_ASSERT(0 == pg_writer_write_u8_hex_upper(&w, 0xac, allocator));
    PG_ASSERT(0 == pg_writer_write_u8_hex_upper(&w, 0x89, allocator));

    PgString s = PG_DYN_TO_SLICE(PgString, w.u.bytes);
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
    PgWriter w = pg_writer_make_string_builder(64, allocator);
    PG_ASSERT(0 ==
              pg_writer_url_encode(&w, PG_S("日本語"), PG_S("123"), allocator));
    PgString encoded = PG_DYN_TO_SLICE(PgString, w.u.bytes);

    PG_ASSERT(pg_string_eq(encoded, PG_S("%E6%97%A5%E6%9C%AC%E8%AA%9E=123")));
  }

  {
    PgWriter w = pg_writer_make_string_builder(64, allocator);
    PG_ASSERT(0 ==
              pg_writer_url_encode(&w, PG_S("日本語"), PG_S("foo"), allocator));
    PgString encoded = PG_DYN_TO_SLICE(PgString, w.u.bytes);

    PG_ASSERT(pg_string_eq(encoded, PG_S("%E6%97%A5%E6%9C%AC%E8%AA%9E=foo")));
  }
}

static void test_string_index_of_any_byte() {
  {
    i64 idx = pg_string_index_of_any_rune(PG_S(""), PG_S(""));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_index_of_any_rune(PG_S(""), PG_S(":"));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_index_of_any_rune(PG_S("?"), PG_S("🚀🛸"));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_index_of_any_rune(PG_S("abc"), PG_S("🚀🛸"));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_index_of_any_rune(PG_S("x"), PG_S("yz"));
    PG_ASSERT(-1 == idx);
  }
  {
    i64 idx = pg_string_index_of_any_rune(PG_S("🛸"), PG_S("🚀🛸"));
    PG_ASSERT(0 == idx);
  }
  {
    i64 idx = pg_string_index_of_any_rune(PG_S("朝日新聞デジタ聞ル🚀🛸A"),
                                          PG_S("🚀🛸"));
    PG_ASSERT(27 == idx);
  }
}

static void test_u8x4_be_to_u32_and_back() {
  {
    u32 n = 123456789;
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
    PgString bitfield = pg_string_clone(PG_S("\x3"
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
    PgString bitfield = pg_string_clone(PG_S("\x20"
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

static void test_ring_buffer_read_write() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Write to empty ring buffer.
  {
    PgString s = PG_S("hello world");

    PgRing rg = pg_ring_make(s.len, allocator);
    PG_ASSERT(pg_ring_is_empty(rg));
    PG_ASSERT(!pg_ring_is_full(rg));
    PG_ASSERT(0 == pg_ring_can_read_count(rg));
    PG_ASSERT(s.len == pg_ring_can_write_count(rg));

    // Full write.
    PG_ASSERT(5 == pg_ring_write_bytes(&rg, PG_S("hello")));
    PG_ASSERT(!pg_ring_is_empty(rg));
    PG_ASSERT(!pg_ring_is_full(rg));
    PG_ASSERT(5 == pg_ring_can_read_count(rg));
    PG_ASSERT(s.len - 5 == pg_ring_can_write_count(rg));

    // Partial write.
    PG_ASSERT(6 == pg_ring_write_bytes(&rg, PG_S(" world!")));
    PG_ASSERT(!pg_ring_is_empty(rg));
    PG_ASSERT(pg_ring_is_full(rg));
    PG_ASSERT(s.len == pg_ring_can_read_count(rg));
    PG_ASSERT(0 == pg_ring_can_write_count(rg));

    // Read all.
    {
      u8 tmp[12] = {0};
      PG_SLICE(u8) tmp_slice = {.data = tmp, PG_STATIC_ARRAY_LEN(tmp)};
      PG_ASSERT(11 == pg_ring_read_bytes(&rg, tmp_slice));
      PG_ASSERT(
          pg_bytes_eq(PG_S("hello world"), PG_SLICE_RANGE(tmp_slice, 0, 11)));

      PG_ASSERT(pg_ring_is_empty(rg));
      PG_ASSERT(!pg_ring_is_full(rg));
      PG_ASSERT(0 == pg_ring_can_read_count(rg));
      PG_ASSERT(s.len == pg_ring_can_write_count(rg));
    }
  }
  // Read from an empty ring buffer.
  {
    PgRing rg = pg_ring_make(12, allocator);
    PG_ASSERT(pg_ring_is_empty(rg));
    PG_ASSERT(!pg_ring_is_full(rg));
    PG_ASSERT(0 == pg_ring_can_read_count(rg));
    PG_ASSERT(12 == pg_ring_can_write_count(rg));

    u8 tmp[1] = {0};
    PG_SLICE(u8) tmp_slice = {.data = tmp, 1};
    PG_ASSERT(0 == pg_ring_read_bytes(&rg, tmp_slice));
  }
  // Write to full ring buffer.
  {
    PgRing rg = pg_ring_make(12, allocator);
    PG_ASSERT(pg_ring_is_empty(rg));
    PG_ASSERT(!pg_ring_is_full(rg));
    PG_ASSERT(0 == pg_ring_can_read_count(rg));
    PG_ASSERT(12 == pg_ring_can_write_count(rg));

    u8 tmp[1] = {99};
    PG_SLICE(u8) tmp_slice = {.data = tmp, 1};

    // Fill.
    for (u64 i = 0; i < 12; i++) {

      PG_ASSERT(1 == pg_ring_write_bytes(&rg, tmp_slice));
      PG_ASSERT(!pg_ring_is_empty(rg));
      PG_ASSERT(pg_ring_can_read_count(rg) > 0);
    }

    PG_ASSERT(0 == pg_ring_write_bytes(&rg, tmp_slice));
    PG_ASSERT(!pg_ring_is_empty(rg));
    PG_ASSERT(pg_ring_is_full(rg));
    PG_ASSERT(12 == pg_ring_can_read_count(rg));
    PG_ASSERT(0 == pg_ring_can_write_count(rg));
  }
}

static void test_ring_buffer_read_write_fuzz() {

  u64 oracle_cap = 0;
  // Get pipe capacity by filling it.
  {
    PG_RESULT(PG_PAIR(PgFileDescriptor), PgError) res_pipe = pg_pipe_make();
    PG_PAIR(PgFileDescriptor) oracle = PG_UNWRAP(res_pipe);

    PG_ASSERT(0 == pg_fd_set_blocking(oracle.second, false));

    u8 buf[512 * PG_KiB] = {0};
    PG_SLICE(u8) buf_slice = {.data = buf, .len = PG_STATIC_ARRAY_LEN(buf)};
    do {
      PG_RESULT(u64, PgError)
      res_pipe_write = pg_file_write(oracle.second, buf_slice);
      u64 write_count = PG_UNWRAP_OR_DEFAULT(res_pipe_write);
      if (0 == write_count) {
        break;
      }
      oracle_cap += write_count;
    } while (1);
    PG_ASSERT(0 == pg_fd_set_blocking(oracle.second, true));

    (void)pg_file_close(oracle.first);
    (void)pg_file_close(oracle.second);
  }
  PG_RESULT(PG_PAIR(PgFileDescriptor), PgError) res_pipe = pg_pipe_make();
  PG_PAIR(PgFileDescriptor) oracle = PG_UNWRAP(res_pipe);

  PgArena arena_ring = pg_arena_make_from_virtual_mem(oracle_cap);
  PgArenaAllocator arena_allocator_ring = pg_make_arena_allocator(&arena_ring);
  PgAllocator *allocator_ring =
      pg_arena_allocator_as_allocator(&arena_allocator_ring);

  PgRing rg = {.data = pg_string_make(oracle_cap, allocator_ring)};

  u64 ROUNDS = 1024;
  PgArena arena_strings =
      pg_arena_make_from_virtual_mem(ROUNDS * 2 * oracle_cap);
  PgArenaAllocator arena_allocator_strings =
      pg_make_arena_allocator(&arena_strings);
  PgAllocator *allocator_strings =
      pg_arena_allocator_as_allocator(&arena_allocator_strings);

  PgRng rng = pg_rand_make();
  // TODO: Print seed for reproducability?
  for (u64 i = 0; i < ROUNDS; i++) {
    u32 len = pg_rand_u32_min_incl_max_incl(&rng, 0, (u32)rg.data.len);
    PgString src = pg_rand_string(&rng, len, allocator_strings);
    PgString dst = pg_rand_string(&rng, len, allocator_strings);

    u64 can_write = pg_ring_can_write_count(rg);
    u64 n_write = pg_ring_write_bytes(&rg, src);
    if (can_write > 0 && src.len > 0) {
      PG_ASSERT(n_write == PG_MIN(can_write, src.len));

      PG_RESULT(u64, PgError) res_write = pg_file_write(oracle.second, src);
      u64 oracle_write_count = PG_UNWRAP(res_write);
      PG_ASSERT(oracle_write_count == n_write);
    }

    u64 can_read = pg_ring_can_read_count(rg);
    u64 n_read = pg_ring_read_bytes(&rg, dst);
    if (can_read > 0 && dst.len > 0) {
      PG_ASSERT(n_read == PG_MIN(can_read, dst.len));

      PG_RESULT(u64, PgError) res_read = pg_file_read(oracle.first, dst);
      u64 oracle_read_count = PG_UNWRAP(res_read);
      PG_ASSERT(oracle_read_count == n_read);
    }
  }
}

static void test_url_parse() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S(""), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("x"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("http:"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("http:/"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("http://"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("://"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("http://a"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(0 == url.port);
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("http://a:"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(0 == url.port);
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("http://a:/"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(0 == url.port);
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a:bc"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://abc:0"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("abc"), url.host));
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(0 == url.port);
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://abc:999999"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }

  // Invalid scheme.
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("1abc://a:80/"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a:80"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(80 == url.port);
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a.b.c:80/foo"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a.b.c"), url.host));
    PG_ASSERT(80 == url.port);
    PG_ASSERT(1 == url.path_components.len);

    PgString path_component0 = PG_SLICE_AT(url.path_components, 0);
    PG_ASSERT(pg_string_eq(PG_S("foo"), path_component0));
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a.b.c:80/"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a.b.c"), url.host));
    PG_ASSERT(80 == url.port);
    PG_ASSERT(0 == url.path_components.len);
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a.b.c/foo/bar/baz"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a.b.c"), url.host));
    PG_ASSERT(0 == url.port);
    PG_ASSERT(3 == url.path_components.len);

    PgString path_component0 = PG_SLICE_AT(url.path_components, 0);
    PG_ASSERT(pg_string_eq(PG_S("foo"), path_component0));

    PgString path_component1 = PG_SLICE_AT(url.path_components, 1);
    PG_ASSERT(pg_string_eq(PG_S("bar"), path_component1));

    PgString path_component2 = PG_SLICE_AT(url.path_components, 2);
    PG_ASSERT(pg_string_eq(PG_S("baz"), path_component2));
  }

  // PgUrl parameters.
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a/?foo"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.port);
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(1 == url.query_parameters.len);

    PgStringKeyValue kv0 = PG_SLICE_AT(url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("")));
  }
  {
    PG_RESULT(PgUrl, PgError) res = pg_url_parse(PG_S("http://a/?"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.port);
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(0 == url.query_parameters.len);
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a/?foo=bar"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.port);
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(1 == url.query_parameters.len);

    PgStringKeyValue kv0 = PG_SLICE_AT(url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a/?foo=bar&"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.port);
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(1 == url.query_parameters.len);

    PgStringKeyValue kv0 = PG_SLICE_AT(url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a/?foo=bar&hello=world"), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.port);
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(2 == url.query_parameters.len);

    PgStringKeyValue kv0 = PG_SLICE_AT(url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));

    PgStringKeyValue kv1 = PG_SLICE_AT(url.query_parameters, 1);
    PG_ASSERT(pg_string_eq(kv1.key, PG_S("hello")));
    PG_ASSERT(pg_string_eq(kv1.value, PG_S("world")));
  }
  {
    PG_RESULT(PgUrl, PgError)
    res = pg_url_parse(PG_S("http://a/?foo=bar&hello=world&a="), allocator);
    PgUrl url = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(PG_S("http"), url.scheme));
    PG_ASSERT(0 == url.username.len);
    PG_ASSERT(0 == url.password.len);
    PG_ASSERT(pg_string_eq(PG_S("a"), url.host));
    PG_ASSERT(0 == url.port);
    PG_ASSERT(0 == url.path_components.len);
    PG_ASSERT(3 == url.query_parameters.len);

    PgStringKeyValue kv0 = PG_SLICE_AT(url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));

    PgStringKeyValue kv1 = PG_SLICE_AT(url.query_parameters, 1);
    PG_ASSERT(pg_string_eq(kv1.key, PG_S("hello")));
    PG_ASSERT(pg_string_eq(kv1.value, PG_S("world")));

    PgStringKeyValue kv2 = PG_SLICE_AT(url.query_parameters, 2);
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
    PG_RESULT(PG_DYN(PgString), PgError)
    res = pg_url_parse_path_components(PG_S(""), allocator);
    PG_DYN(PgString) components = PG_UNWRAP(res);
    PG_ASSERT(0 == components.len);
  }
  // Forbidden characters.
  {

    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(
                  pg_url_parse_path_components(PG_S("/foo?bar"), allocator)));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(
                  pg_url_parse_path_components(PG_S("/foo:1234"), allocator)));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(
                  pg_url_parse_path_components(PG_S("/foo#bar"), allocator)));
  }
  // Must start with slash and it does not.
  {
    PG_RESULT(PG_DYN(PgString), PgError)
    res = pg_url_parse_path_components(PG_S("foo"), allocator);
    PG_ASSERT(PG_ERR_INVALID_VALUE == PG_UNWRAP_ERR(res));
  }
  // Must start with slash and it does.
  {
    PG_RESULT(PG_DYN(PgString), PgError)
    res = pg_url_parse_path_components(PG_S("/foo"), allocator);
    PG_DYN(PgString) components = PG_UNWRAP(res);
    PG_ASSERT(1 == components.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(components, 0)));
  }
  // Simple path with a few components.
  {
    PG_RESULT(PG_DYN(PgString), PgError)
    res = pg_url_parse_path_components(PG_S("/foo/bar/baz"), allocator);
    PG_DYN(PgString) components = PG_UNWRAP(res);
    PG_ASSERT(3 == components.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(components, 0)));
    PG_ASSERT(pg_string_eq(PG_S("bar"), PG_SLICE_AT(components, 1)));
    PG_ASSERT(pg_string_eq(PG_S("baz"), PG_SLICE_AT(components, 2)));
  }
  // Simple path with a few components with trailing slash.
  {
    PG_RESULT(PG_DYN(PgString), PgError)
    res = pg_url_parse_path_components(PG_S("/foo/bar/baz/"), allocator);
    PG_DYN(PgString) components = PG_UNWRAP(res);
    PG_ASSERT(3 == components.len);
    PG_ASSERT(pg_string_eq(PG_S("foo"), PG_SLICE_AT(components, 0)));
    PG_ASSERT(pg_string_eq(PG_S("bar"), PG_SLICE_AT(components, 1)));
    PG_ASSERT(pg_string_eq(PG_S("baz"), PG_SLICE_AT(components, 2)));
  }
}

static void test_http_request_to_string() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
  {
    PgHttpRequest req = {0};
    req.method = PG_HTTP_METHOD_GET;

    PgString s = pg_http_request_to_string(req, allocator);
    PgString expected = PG_S("GET / HTTP/1.1\r\n"
                             "\r\n");
    PG_ASSERT(pg_string_eq(s, expected));
  }
  {
    PgHttpRequest req = {0};
    req.method = PG_HTTP_METHOD_POST;
    pg_http_push_header(&req.headers, PG_S("Host"), PG_S("google.com"),
                        allocator);
    PG_DYN_PUSH(&req.url.path_components, PG_S("foobar"), allocator);

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
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(PG_S(""))));
  }
  // Missing prefix.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(PG_S("HTT"))));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(PG_S("abc"))));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(PG_S("/1.1"))));
  }
  // Missing slash.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP1.1 201 Created"))));
  }
  // Missing major version.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/.1 201 Created"))));
  }
  // Missing `.`.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/11 201 Created"))));
  }
  // Missing minor version.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/1. 201 Created"))));
  }
  // Missing status code.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/1.1 Created"))));
  }
  // Invalid major version.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/abc.1 201 Created"))));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/4.1 201 Created"))));
  }
  // Invalid minor version.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/1.10 201 Created"))));
  }
  // Invalid status code.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/1.1 99 Created"))));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_response_status_line(
                  PG_S("HTTP/1.1 600 Created"))));
  }
  // Valid, short.
  {
    PG_RESULT(PgHttpResponseStatusLine, PgError)
    res = pg_http_parse_response_status_line(PG_S("HTTP/2.0 201"));
    PgHttpResponseStatusLine status_line = PG_UNWRAP(res);
    PG_ASSERT(2 == status_line.version_major);
    PG_ASSERT(0 == status_line.version_minor);
    PG_ASSERT(201 == status_line.status);
  }
  // Valid, short, 0.9.
  {
    PG_RESULT(PgHttpResponseStatusLine, PgError)
    res = pg_http_parse_response_status_line(PG_S("HTTP/0.9 201"));
    PgHttpResponseStatusLine status_line = PG_UNWRAP(res);
    PG_ASSERT(0 == status_line.version_major);
    PG_ASSERT(9 == status_line.version_minor);
    PG_ASSERT(201 == status_line.status);
  }
  // Valid, long.
  {
    PG_RESULT(PgHttpResponseStatusLine, PgError)
    res = pg_http_parse_response_status_line(PG_S("HTTP/1.1 404 Not found"));
    PgHttpResponseStatusLine status_line = PG_UNWRAP(res);
    PG_ASSERT(1 == status_line.version_major);
    PG_ASSERT(1 == status_line.version_minor);
    PG_ASSERT(404 == status_line.status);
  }
}

static void test_http_parse_request_status_line() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Empty.
  {
    PG_ASSERT(
        PG_ERR_INVALID_VALUE ==
        PG_UNWRAP_ERR(pg_http_parse_request_status_line(PG_S(""), allocator)));
  }
  // Missing prefix.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(
                  pg_http_parse_request_status_line(PG_S("GE"), allocator)));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(
                  pg_http_parse_request_status_line(PG_S("abc"), allocator)));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(
                  pg_http_parse_request_status_line(PG_S("123 "), allocator)));
  }
  // Missing slash.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_request_status_line(
                  PG_S("GET HTTP1.1"), allocator)));
  }
  // Missing major version.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_request_status_line(
                  PG_S("GET / HTTP/.1"), allocator)));
  }
  // Missing `.`.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_request_status_line(
                  PG_S("GET / HTTP/11"), allocator)));
  }
  // Missing minor version.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_request_status_line(
                  PG_S("GET / HTTP/1."), allocator)));
  }
  // Invalid major version.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_request_status_line(
                  PG_S("GET / HTTP/abc.1"), allocator)));
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_request_status_line(
                  PG_S("GET / HTTP/4.1"), allocator)));
  }
  // Invalid minor version.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_request_status_line(
                  PG_S("GET / HTTP/1.10"), allocator)));
  }
  // Valid, short.
  {
    PG_RESULT(PgHttpRequestStatusLine, PgError)
    res = pg_http_parse_request_status_line(PG_S("GET / HTTP/2.0"), allocator);
    PgHttpRequestStatusLine status_line = PG_UNWRAP(res);
    PG_ASSERT(PG_HTTP_METHOD_GET == status_line.method);
    PG_ASSERT(2 == status_line.version_major);
    PG_ASSERT(0 == status_line.version_minor);
    PG_ASSERT(0 == status_line.url.path_components.len);
    PG_ASSERT(0 == status_line.url.query_parameters.len);
  }
  // Valid, short with query parameters.
  {
    PG_RESULT(PgHttpRequestStatusLine, PgError)
    res = pg_http_parse_request_status_line(PG_S("GET /?foo=bar& HTTP/2.0"),
                                            allocator);
    PgHttpRequestStatusLine status_line = PG_UNWRAP(res);
    PG_ASSERT(PG_HTTP_METHOD_GET == status_line.method);
    PG_ASSERT(2 == status_line.version_major);
    PG_ASSERT(0 == status_line.version_minor);
    PG_ASSERT(0 == status_line.url.path_components.len);
    PG_ASSERT(1 == status_line.url.query_parameters.len);
    PgStringKeyValue kv0 = PG_SLICE_AT(status_line.url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("bar")));
  }
  // Valid, short, 0.9.
  {
    PG_RESULT(PgHttpRequestStatusLine, PgError)
    res = pg_http_parse_request_status_line(PG_S("GET / HTTP/0.9"), allocator);
    PgHttpRequestStatusLine status_line = PG_UNWRAP(res);
    PG_ASSERT(PG_HTTP_METHOD_GET == status_line.method);
    PG_ASSERT(0 == status_line.version_major);
    PG_ASSERT(9 == status_line.version_minor);
    PG_ASSERT(0 == status_line.url.path_components.len);
    PG_ASSERT(0 == status_line.url.query_parameters.len);
  }
  // Valid, long.
  {
    PG_RESULT(PgHttpRequestStatusLine, PgError)
    res = pg_http_parse_request_status_line(
        PG_S("GET /foo/bar/baz?hey HTTP/1.1"), allocator);
    PgHttpRequestStatusLine status_line = PG_UNWRAP(res);
    PG_ASSERT(PG_HTTP_METHOD_GET == status_line.method);
    PG_ASSERT(1 == status_line.version_major);
    PG_ASSERT(1 == status_line.version_minor);
    PG_ASSERT(3 == status_line.url.path_components.len);
    PG_ASSERT(1 == status_line.url.query_parameters.len);
    PgStringKeyValue kv0 = PG_SLICE_AT(status_line.url.query_parameters, 0);
    PG_ASSERT(pg_string_eq(kv0.key, PG_S("hey")));
    PG_ASSERT(pg_string_eq(kv0.value, PG_S("")));
  }
}

static void test_http_parse_header() {
  // Empty.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_header(PG_S(""))));
  }
  // Missing `:`.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_header(PG_S("foo bar"))));
  }
  // Missing key.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_header(PG_S(":bcd"))));
  }
  // Missing value.
  {
    PG_ASSERT(PG_ERR_INVALID_VALUE ==
              PG_UNWRAP_ERR(pg_http_parse_header(PG_S("foo:"))));
  }
  // Multiple colons.
  {
    PG_RESULT(PgStringKeyValue, PgError)
    res = pg_http_parse_header(PG_S("foo: bar : baz"));
    PgStringKeyValue kv = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(kv.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv.value, PG_S("bar : baz")));
  }
  // Valid, one space before the value.
  {
    PG_RESULT(PgStringKeyValue, PgError)
    res = pg_http_parse_header(PG_S("foo: bar"));
    PgStringKeyValue kv = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(kv.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv.value, PG_S("bar")));
  }
  // Valid, no space before the value.
  {
    PG_RESULT(PgStringKeyValue, PgError)
    res = pg_http_parse_header(PG_S("foo:bar"));
    PgStringKeyValue kv = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(kv.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv.value, PG_S("bar")));
  }
  // Valid, multiple spaces before the value.
  {
    PG_RESULT(PgStringKeyValue, PgError)
    res = pg_http_parse_header(PG_S("foo:   bar"));
    PgStringKeyValue kv = PG_UNWRAP(res);
    PG_ASSERT(pg_string_eq(kv.key, PG_S("foo")));
    PG_ASSERT(pg_string_eq(kv.value, PG_S("bar")));
  }
}

static void test_http_read_request_full_no_content_length() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString req_str =
      PG_S("PUT /info/download/index.mp3?foo=bar&baz HTTP/1.1\r\nAccept: "
           "application/json\r\nContent-Type: "
           "text/html\r\n\r\nHello, world!");
  PG_RESULT(PG_PAIR(PgFileDescriptor), PgError)
  res_sockets = pg_net_make_socket_pair(PG_NET_SOCKET_DOMAIN_LOCAL,
                                        PG_NET_SOCKET_TYPE_TCP,
                                        PG_NET_SOCKET_OPTION_NONE);
  PG_PAIR(PgFileDescriptor) sockets = PG_UNWRAP(res_sockets);

  PG_ASSERT(0 == pg_file_write_full_with_descriptor(sockets.first, req_str));

  PgReader reader = pg_reader_make_from_socket(sockets.second, 512, allocator);
  PgHttpRequestReadResult res_req = pg_http_read_request(&reader, allocator);

  PG_ASSERT(!res_req.err);
  PG_ASSERT(res_req.done);

  PgHttpRequest req = res_req.req;
  PG_ASSERT(PG_HTTP_METHOD_PUT == req.method);
  PG_ASSERT(!req.url.scheme.len);
  PG_ASSERT(!req.url.username.len);
  PG_ASSERT(!req.url.password.len);
  PG_ASSERT(!req.url.host.len);
  PG_ASSERT(!req.url.port);
  PG_ASSERT(1 == req.version_major);
  PG_ASSERT(1 == req.version_minor);
  PG_ASSERT(2 == req.headers.len);
  PG_ASSERT(3 == req.url.path_components.len);
  PG_ASSERT(2 == req.url.query_parameters.len);

  PgStringKeyValue header0 = PG_SLICE_AT(req.headers, 0);
  PgStringKeyValue header1 = PG_SLICE_AT(req.headers, 1);
  PG_ASSERT(pg_string_eq(PG_S("Accept"), header0.key));
  PG_ASSERT(pg_string_eq(PG_S("application/json"), header0.value));
  PG_ASSERT(pg_string_eq(PG_S("Content-Type"), header1.key));
  PG_ASSERT(pg_string_eq(PG_S("text/html"), header1.value));

  PG_ASSERT(
      pg_string_eq(PG_S("info"), PG_SLICE_AT(req.url.path_components, 0)));
  PG_ASSERT(
      pg_string_eq(PG_S("download"), PG_SLICE_AT(req.url.path_components, 1)));
  PG_ASSERT(
      pg_string_eq(PG_S("index.mp3"), PG_SLICE_AT(req.url.path_components, 2)));

  PgStringKeyValue query_param0 = PG_SLICE_AT(req.url.query_parameters, 0);
  PgStringKeyValue query_param1 = PG_SLICE_AT(req.url.query_parameters, 1);
  PG_ASSERT(pg_string_eq(PG_S("foo"), query_param0.key));
  PG_ASSERT(pg_string_eq(PG_S("bar"), query_param0.value));
  PG_ASSERT(pg_string_eq(PG_S("baz"), query_param1.key));
  PG_ASSERT(pg_string_eq(PG_S(""), query_param1.value));

  // Body.
#if 0
  {
    u8 body[128] = {0};
    PG_SLICE(u8) body_slice = {.data = body, .len = PG_STATIC_ARRAY_LEN(body)};
    PG_RESULT(u64) res_read = pg_buf_reader_read(&buf_reader, body_slice);
    PG_ASSERT(!res_read.err);
    body_slice.len = res_read.res;
    PG_ASSERT(pg_string_eq(PG_S("Hello, world!"), body_slice));
  }
#endif
}

static void test_http_read_request_full_without_headers() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString req_str = PG_S("PUT /info/download/index.mp3?foo=bar&baz HTTP/1.1"
                          "\r\n\r\nHello, world!");
  PG_RESULT(PG_PAIR(PgFileDescriptor), PgError)
  res_sockets = pg_net_make_socket_pair(PG_NET_SOCKET_DOMAIN_LOCAL,
                                        PG_NET_SOCKET_TYPE_TCP,
                                        PG_NET_SOCKET_OPTION_NONE);
  PG_PAIR(PgFileDescriptor) sockets = PG_UNWRAP(res_sockets);

  PG_ASSERT(0 == pg_file_write_full_with_descriptor(sockets.first, req_str));

  PgReader reader = pg_reader_make_from_socket(sockets.second, 512, allocator);
  PgHttpRequestReadResult res_req = pg_http_read_request(&reader, allocator);

  PG_ASSERT(!res_req.err);
  PG_ASSERT(res_req.done);

  PgHttpRequest req = res_req.req;
  PG_ASSERT(PG_HTTP_METHOD_PUT == req.method);
  PG_ASSERT(!req.url.scheme.len);
  PG_ASSERT(!req.url.username.len);
  PG_ASSERT(!req.url.password.len);
  PG_ASSERT(!req.url.host.len);
  PG_ASSERT(!req.url.port);
  PG_ASSERT(1 == req.version_major);
  PG_ASSERT(1 == req.version_minor);
  PG_ASSERT(0 == req.headers.len);
  PG_ASSERT(3 == req.url.path_components.len);
  PG_ASSERT(2 == req.url.query_parameters.len);

  PG_ASSERT(
      pg_string_eq(PG_S("info"), PG_SLICE_AT(req.url.path_components, 0)));
  PG_ASSERT(
      pg_string_eq(PG_S("download"), PG_SLICE_AT(req.url.path_components, 1)));
  PG_ASSERT(
      pg_string_eq(PG_S("index.mp3"), PG_SLICE_AT(req.url.path_components, 2)));

  PgStringKeyValue query_param0 = PG_SLICE_AT(req.url.query_parameters, 0);
  PgStringKeyValue query_param1 = PG_SLICE_AT(req.url.query_parameters, 1);
  PG_ASSERT(pg_string_eq(PG_S("foo"), query_param0.key));
  PG_ASSERT(pg_string_eq(PG_S("bar"), query_param0.value));
  PG_ASSERT(pg_string_eq(PG_S("baz"), query_param1.key));
  PG_ASSERT(pg_string_eq(PG_S(""), query_param1.value));

  // Body.
#if 0
  {
    PG_ASSERT(0 == pg_buf_reader_fill_until_full_or_eof(&buf_reader));
    PgString expected = PG_S("Hello, world!");
    PG_ASSERT(expected.len == pg_ring_can_read_count(buf_reader.ring));

    u8 tmp[64] = {0};
    PG_SLICE(u8) tmp_slice = {
        .data = tmp,
        .len = PG_STATIC_ARRAY_LEN(tmp),
    };
    PG_RESULT(u64) res_read = pg_buf_reader_read(&buf_reader, tmp_slice);
    PG_ASSERT(0 == res_read.err);
    PG_ASSERT(expected.len == res_read.res);
    tmp_slice.len = res_read.res;
    PG_ASSERT(pg_string_eq(tmp_slice, expected));
  }
#endif
}

static void test_http_read_request_full_without_body() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString req_str = PG_S("PUT /info/download/index.mp3?foo=bar&baz HTTP/1.1"
                          "\r\n\r\n");
  PG_RESULT(PG_PAIR(PgFileDescriptor), PgError)
  res_sockets = pg_net_make_socket_pair(PG_NET_SOCKET_DOMAIN_LOCAL,
                                        PG_NET_SOCKET_TYPE_TCP,
                                        PG_NET_SOCKET_OPTION_NONE);
  PG_PAIR(PgFileDescriptor) sockets = PG_UNWRAP(res_sockets);

  PG_ASSERT(0 == pg_file_write_full_with_descriptor(sockets.first, req_str));

  PgReader reader = pg_reader_make_from_socket(sockets.second, 512, allocator);
  PgHttpRequestReadResult res_req = pg_http_read_request(&reader, allocator);

  PG_ASSERT(!res_req.err);
  PG_ASSERT(res_req.done);

  PgHttpRequest req = res_req.req;
  PG_ASSERT(PG_HTTP_METHOD_PUT == req.method);
  PG_ASSERT(!req.url.scheme.len);
  PG_ASSERT(!req.url.username.len);
  PG_ASSERT(!req.url.password.len);
  PG_ASSERT(!req.url.host.len);
  PG_ASSERT(!req.url.port);
  PG_ASSERT(1 == req.version_major);
  PG_ASSERT(1 == req.version_minor);
  PG_ASSERT(0 == req.headers.len);
  PG_ASSERT(3 == req.url.path_components.len);
  PG_ASSERT(2 == req.url.query_parameters.len);

  PG_ASSERT(
      pg_string_eq(PG_S("info"), PG_SLICE_AT(req.url.path_components, 0)));
  PG_ASSERT(
      pg_string_eq(PG_S("download"), PG_SLICE_AT(req.url.path_components, 1)));
  PG_ASSERT(
      pg_string_eq(PG_S("index.mp3"), PG_SLICE_AT(req.url.path_components, 2)));

  PgStringKeyValue query_param0 = PG_SLICE_AT(req.url.query_parameters, 0);
  PgStringKeyValue query_param1 = PG_SLICE_AT(req.url.query_parameters, 1);
  PG_ASSERT(pg_string_eq(PG_S("foo"), query_param0.key));
  PG_ASSERT(pg_string_eq(PG_S("bar"), query_param0.value));
  PG_ASSERT(pg_string_eq(PG_S("baz"), query_param1.key));
  PG_ASSERT(pg_string_eq(PG_S(""), query_param1.value));

  // Body.
#if 0
  {
    PG_ASSERT(0 == pg_buf_reader_fill_until_full_or_eof(&buf_reader));
    PG_ASSERT(0 == pg_ring_can_read_count(buf_reader.ring));
  }
#endif
}

static void test_http_read_request_no_body_separator() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString req_str =
      PG_S("PUT /info/download/index.mp3?foo=bar&baz HTTP/1.1\r\nAccept: "
           "application/json\r\nContent-Type: "
           "text/html\r\n");

  PG_RESULT(PG_PAIR(PgFileDescriptor), PgError)
  res_sockets = pg_net_make_socket_pair(PG_NET_SOCKET_DOMAIN_LOCAL,
                                        PG_NET_SOCKET_TYPE_TCP,
                                        PG_NET_SOCKET_OPTION_NONE);
  PG_PAIR(PgFileDescriptor) sockets = PG_UNWRAP(res_sockets);

  PG_ASSERT(0 == pg_file_write_full_with_descriptor(sockets.first, req_str));

  PgReader reader = pg_reader_make_from_socket(sockets.second, 512, allocator);
  // HACK: Since reads are blocking, to be able to test a partial request, and
  // have the parser finish, we set a timeout. In reality the http handler would
  // timeout. Still, this way we can check that parsing the request yields an
  // error.
  PG_ASSERT(0 == pg_net_socket_set_timeout(sockets.second, 0, 1000));
  PgHttpRequestReadResult res_req = pg_http_read_request(&reader, allocator);

  PG_ASSERT(PG_ERR_INVALID_VALUE == res_req.err);
  PG_ASSERT(!res_req.done);
}

#if 0
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

      PgStringKeyValue kv0 = PG_SLICE_AT(res.res.headers, 0);
      PG_ASSERT(pg_string_eq(kv0.key, PG_S("Host")));
      PG_ASSERT(pg_string_eq(kv0.value, PG_S("google.com")));

      PgStringKeyValue kv1 = PG_SLICE_AT(res.res.headers, 1);
      PG_ASSERT(pg_string_eq(kv1.key, PG_S("Authorization")));
      PG_ASSERT(pg_string_eq(kv1.value, PG_S("Bearer foo")));
    }
  }
}

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
  client_req.method = PG_HTTP_METHOD_GET;
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
    PG_RESULT(u64) res_wait = pg_aio_queue_wait(queue, events_watch, -1, arena);
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
  PgArena arena = pg_arena_make_from_virtual_mem(8 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
  // Simple log.
  {
    PgLogger logger = {
        .level = PG_LOG_LEVEL_DEBUG,
        .writer = pg_writer_make_string_builder(4 * PG_KiB, allocator),
        .format = PG_LOG_FORMAT_LOGFMT,
        // TODO: Consider using `rtdsc` or such.
        .monotonic_epoch =
            PG_UNWRAP_OR_DEFAULT(pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC)),
        .allocator = allocator,
    };

    pg_log(&logger, PG_LOG_LEVEL_INFO, "hello world",
           pg_log_c_s("foo", PG_S("bar")), pg_log_c_i64("baz", -317));

    PgString out = PG_DYN_TO_SLICE(PgString, logger.writer.u.bytes);
    PG_ASSERT(pg_string_starts_with(out, PG_S("level=info ")));
  }
  // PgLog but the logger level is higher.
  {
    PgLogger logger = {
        .level = PG_LOG_LEVEL_INFO,
        .writer = pg_writer_make_string_builder(4 * PG_KiB, allocator),
        .format = PG_LOG_FORMAT_LOGFMT,
        // TODO: Consider using `rtdsc` or such.
        .monotonic_epoch =
            PG_UNWRAP_OR_DEFAULT(pg_time_ns_now(PG_CLOCK_KIND_MONOTONIC)),
        .allocator = allocator,
    };

    pg_log(&logger, PG_LOG_LEVEL_DEBUG, "hello world",
           pg_log_s(PG_S("foo"), PG_S("bar")));

    PgString out = PG_DYN_TO_SLICE(PgString, logger.writer.u.bytes);
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

static void test_process_no_capture() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PG_DYN(PgString) args = {0};
  PG_DYN_PUSH(&args, PG_S("ls-files"), allocator);

  PgProcessSpawnOptions options = {0};
  PG_RESULT(PgProcess, PgError)
  res_spawn =
      pg_process_spawn(PG_S("git"), PG_DYN_TO_SLICE(PG_SLICE(PgString), args),
                       options, allocator);

  PgProcess process = PG_UNWRAP(res_spawn);

  PG_RESULT(PgProcessStatus, PgError)
  res_wait = pg_process_wait(process, 0, 0, allocator);

  PgProcessStatus status = PG_UNWRAP(res_wait);
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

  PG_DYN(PgString) args = {0};
  PG_DYN_PUSH(&args, PG_S("ls-files"), allocator);

  PgProcessSpawnOptions options = {
      .stdout_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
      .stderr_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
  };
  PG_RESULT(PgProcess, PgError)
  res_spawn =
      pg_process_spawn(PG_S("git"), PG_DYN_TO_SLICE(PG_SLICE(PgString), args),
                       options, allocator);

  PgProcess process = PG_UNWRAP(res_spawn);

  PG_RESULT(PgProcessStatus, PgError)
  res_wait = pg_process_wait(process, 100, 0, allocator);

  PgProcessStatus status = PG_UNWRAP(res_wait);
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

  PG_DYN(PgString) args = {0};
  PG_DYN_PUSH(&args, PG_S("lo wo"), allocator);

  PgProcessSpawnOptions options = {
      .stdin_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
      .stdout_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
      .stderr_capture = PG_CHILD_PROCESS_STD_IO_PIPE,
  };
  PG_RESULT(PgProcess, PgError)
  res_spawn =
      pg_process_spawn(PG_S("grep"), PG_DYN_TO_SLICE(PG_SLICE(PgString), args),
                       options, allocator);

  PgProcess process = PG_UNWRAP(res_spawn);

  PgString msg = PG_S("hello world");
  PG_RESULT(u64, PgError) res_write = pg_file_write(process.stdin_pipe, msg);
  PG_ASSERT(msg.len == PG_UNWRAP(res_write));

  PG_ASSERT(0 == pg_file_close(process.stdin_pipe));
  process.stdin_pipe.fd = 0;

  PG_RESULT(PgProcessStatus, PgError)
  res_wait = pg_process_wait(process, 0, 0, allocator);

  PgProcessStatus status = PG_UNWRAP(res_wait);
  PG_ASSERT(0 == status.exit_status);
  PG_ASSERT(0 == status.signal);
  PG_ASSERT(status.exited);
  PG_ASSERT(!status.signaled);
  PG_ASSERT(!status.core_dumped);
  PG_ASSERT(!status.stopped);

  PG_ASSERT(pg_string_contains(status.stdout_captured, msg));
  PG_ASSERT(pg_string_is_empty(status.stderr_captured));
}

static void test_linked_list() {
  // Empty.
  {
    PgLinkedListNode head = {0};
    pg_linked_list_init(&head);

    PgLinkedListNode elem = {0};
    pg_linked_list_init(&elem);

    pg_linked_list_append(&head, &elem);

    PG_ASSERT(head.next == &elem);
    PG_ASSERT(elem.next == &elem);
  }
  // Non empty.
  {
    PgLinkedListNode head = {0};
    pg_linked_list_init(&head);

    PgLinkedListNode elem1 = {0};
    pg_linked_list_init(&elem1);

    PgLinkedListNode elem2 = {0};
    pg_linked_list_init(&elem2);

    pg_linked_list_append(&head, &elem1);
    PG_ASSERT(head.next == &elem1);
    PG_ASSERT(elem1.next == &elem1);

    pg_linked_list_append(&head, &elem2);
    PG_ASSERT(head.next == &elem1);
    PG_ASSERT(elem1.next == &elem2);
    PG_ASSERT(elem2.next == &elem2);
  }
}

static void test_html_tokenize_no_attributes() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<html>🍌foo</html>");
  PG_RESULT(PG_DYN(PgHtmlToken), PgError) res = pg_html_tokenize(s, allocator);

  PG_DYN(PgHtmlToken) tokens = PG_UNWRAP(res);
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
    PG_ASSERT(13 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("🍌foo"), token.text));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(15 == token.start);
    PG_ASSERT(19 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
}

static void test_html_tokenize_with_attributes() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<html id=\"bar🍌\" class=\"ba/z\"  > foo  </html>");
  PG_RESULT(PG_DYN(PgHtmlToken), PgError) res = pg_html_tokenize(s, allocator);

  PG_DYN(PgHtmlToken) tokens = PG_UNWRAP(res);
  PG_ASSERT(3 == tokens.len);

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(1 == token.start);
    PG_ASSERT(5 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }

#if 0
  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_ATTRIBUTE == token.kind);
    PG_ASSERT(6 == token.start);
    PG_ASSERT(17 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("id"), token.attribute.key));
    PG_ASSERT(pg_string_eq(PG_S("bar🍌"), token.attribute.value));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_ATTRIBUTE == token.kind);
    PG_ASSERT(19 == token.start);
    PG_ASSERT(30 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("class"), token.attribute.key));
    PG_ASSERT(pg_string_eq(PG_S("ba/z"), token.attribute.value));
  }
#endif

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(34 == token.start);
    PG_ASSERT(40 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("foo"), pg_string_trim_space(token.text)));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(42 == token.start);
    PG_ASSERT(46 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
}

static void test_html_tokenize_with_key_no_value() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<html aria-hidden>foo</html>");
  PG_RESULT(PG_DYN(PgHtmlToken), PgError) res = pg_html_tokenize(s, allocator);

  PG_DYN(PgHtmlToken) tokens = PG_UNWRAP(res);
  PG_ASSERT(3 == tokens.len);

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(1 == token.start);
    PG_ASSERT(5 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }

#if 0
  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_ATTRIBUTE == token.kind);
    PG_ASSERT(6 == token.start);
    PG_ASSERT(17 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("aria-hidden"), token.attribute.key));
    PG_ASSERT(pg_string_eq(PG_S(""), token.attribute.value));
  }
#endif

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(18 == token.start);
    PG_ASSERT(21 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("foo"), pg_string_trim_space(token.text)));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
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
  PG_RESULT(PG_DYN(PgHtmlToken), PgError) res = pg_html_tokenize(s, allocator);

  PG_DYN(PgHtmlToken) tokens = PG_UNWRAP(res);
  /* PG_ASSERT(7 == tokens.len); */

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(1 == token.start);
    PG_ASSERT(5 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }

#if 0
  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_ATTRIBUTE == token.kind);
    PG_ASSERT(6 == token.start);
    PG_ASSERT(13 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("id"), token.attribute.key));
    PG_ASSERT(pg_string_eq(PG_S("bar"), token.attribute.value));
  }
#endif

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(16 == token.start);
    PG_ASSERT(21 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("foo"), pg_string_trim_space(token.text)));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(22 == token.start);
    PG_ASSERT(26 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("span"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 3);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == token.kind);
    PG_ASSERT(27 == token.start);
    PG_ASSERT(30 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("bar"), pg_string_trim_space(token.text)));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 4);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(32 == token.start);
    PG_ASSERT(36 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("span"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 5);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(40 == token.start);
    PG_ASSERT(44 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
}

static void test_html_tokenize_with_doctype() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<!DOCTYPE html> <html></html>");
  PG_RESULT(PG_DYN(PgHtmlToken), PgError) res = pg_html_tokenize(s, allocator);

  PG_DYN(PgHtmlToken) tokens = PG_UNWRAP(res);
  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_DOCTYPE == token.kind);
    PG_ASSERT(10 == token.start);
    PG_ASSERT(14 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(17 == token.start);
    PG_ASSERT(21 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.doctype));
  }
}

static void test_html_tokenize_with_comment() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<html> <!-- hello world --> </html>");
  PG_RESULT(PG_DYN(PgHtmlToken), PgError) res = pg_html_tokenize(s, allocator);

  PG_DYN(PgHtmlToken) tokens = PG_UNWRAP(res);
  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 0);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == token.kind);
    PG_ASSERT(1 == token.start);
    PG_ASSERT(5 == token.end);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 1);
    PG_ASSERT(PG_HTML_TOKEN_KIND_COMMENT == token.kind);
    PG_ASSERT(pg_string_eq(PG_S("hello world"), token.comment));
  }

  {
    PgHtmlToken token = PG_SLICE_AT(tokens, 2);
    PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_CLOSING == token.kind);
    PG_ASSERT(pg_string_eq(PG_S("html"), token.tag));
  }
}

static void test_html_parse() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S(
      "<html>"
      "  <head>"
      "    <title>This is a title</title>"
      "  </head>"
      "  <body>"
      "    <div>"
      "        <p>Hello world!</p>"
      "    </div>"
      "    <img src=\"git_web_ui_link.png\" alt=\"Link in Github's web UI\" />"
      "  </body>"
      "</html>");
  PG_RESULT(PgHtmlNodePtr, PgError) res_parse = pg_html_parse(s, allocator);

  PgHtmlNode *root = PG_UNWRAP(res_parse);
  PG_ASSERT(!pg_linked_list_is_empty(&root->first_child));
  PG_ASSERT(pg_linked_list_is_empty(&root->next_sibling));

  PgHtmlNode *node_html = pg_html_node_get_first_child(root);
  PG_ASSERT(pg_html_node_get_parent(node_html) == root);
  PG_ASSERT(pg_linked_list_is_empty(&node_html->next_sibling));
  PG_ASSERT(!pg_linked_list_is_empty(&node_html->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_html->token_start.kind);
  PG_ASSERT(pg_string_eq(node_html->token_start.tag, PG_S("html")));
  PG_ASSERT(pg_string_eq(node_html->token_end.tag, PG_S("html")));

  PgHtmlNode *node_head = pg_html_node_get_first_child(node_html);
  PG_ASSERT(pg_html_node_get_parent(node_head) == node_html);
  PG_ASSERT(!pg_linked_list_is_empty(&node_head->next_sibling));
  PG_ASSERT(!pg_linked_list_is_empty(&node_head->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_head->token_start.kind);
  PG_ASSERT(pg_string_eq(node_head->token_start.tag, PG_S("head")));
  PG_ASSERT(pg_string_eq(node_head->token_end.tag, PG_S("head")));

  PgHtmlNode *node_title = pg_html_node_get_first_child(node_head);
  PG_ASSERT(pg_html_node_get_parent(node_title) == node_head);
  PG_ASSERT(pg_linked_list_is_empty(&node_title->next_sibling));
  PG_ASSERT(!pg_linked_list_is_empty(&node_title->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_title->token_start.kind);
  PG_ASSERT(pg_string_eq(node_title->token_start.tag, PG_S("title")));
  PG_ASSERT(pg_string_eq(node_title->token_end.tag, PG_S("title")));

  PgHtmlNode *node_title_text = pg_html_node_get_first_child(node_title);
  PG_ASSERT(pg_html_node_get_parent(node_title_text) == node_title);
  PG_ASSERT(pg_linked_list_is_empty(&node_title_text->next_sibling));
  PG_ASSERT(pg_linked_list_is_empty(&node_title_text->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == node_title_text->token_start.kind);
  PG_ASSERT(
      pg_string_eq(node_title_text->token_start.text, PG_S("This is a title")));

  PgHtmlNode *node_body = pg_html_node_get_next_sibling(node_head);
  PG_ASSERT(pg_html_node_get_parent(node_body) == node_html);
  PG_ASSERT(pg_linked_list_is_empty(&node_body->next_sibling));
  PG_ASSERT(!pg_linked_list_is_empty(&node_body->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_body->token_start.kind);
  PG_ASSERT(pg_string_eq(node_body->token_start.tag, PG_S("body")));
  PG_ASSERT(pg_string_eq(node_body->token_end.tag, PG_S("body")));

  PgHtmlNode *node_div = pg_html_node_get_first_child(node_body);
  PG_ASSERT(pg_html_node_get_parent(node_div) == node_body);
  PG_ASSERT(!pg_linked_list_is_empty(&node_div->next_sibling));
  PG_ASSERT(!pg_linked_list_is_empty(&node_div->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_div->token_start.kind);
  PG_ASSERT(pg_string_eq(node_div->token_start.tag, PG_S("div")));
  PG_ASSERT(pg_string_eq(node_div->token_end.tag, PG_S("div")));

  PgHtmlNode *node_p = pg_html_node_get_first_child(node_div);
  PG_ASSERT(pg_html_node_get_parent(node_p) == node_div);
  PG_ASSERT(pg_linked_list_is_empty(&node_p->next_sibling));
  PG_ASSERT(!pg_linked_list_is_empty(&node_p->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_p->token_start.kind);
  PG_ASSERT(pg_string_eq(node_p->token_start.tag, PG_S("p")));
  PG_ASSERT(pg_string_eq(node_p->token_end.tag, PG_S("p")));

  PgHtmlNode *node_p_text = pg_html_node_get_first_child(node_p);
  PG_ASSERT(pg_html_node_get_parent(node_p_text) == node_p);
  PG_ASSERT(pg_linked_list_is_empty(&node_p_text->next_sibling));
  PG_ASSERT(pg_linked_list_is_empty(&node_p_text->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == node_p_text->token_start.kind);
  PG_ASSERT(pg_string_eq(node_p_text->token_start.text, PG_S("Hello world!")));

  PgHtmlNode *node_img = pg_html_node_get_next_sibling(node_div);
  PG_ASSERT(pg_html_node_get_parent(node_img) == node_body);
  PG_ASSERT(pg_linked_list_is_empty(&node_img->next_sibling));
  PG_ASSERT(pg_linked_list_is_empty(&node_img->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_img->token_start.kind);
  PG_ASSERT(pg_string_eq(node_img->token_start.tag, PG_S("img")));
}

static void test_html_parse_title_with_html_content() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgString s = PG_S("<h2>Hello <code>world</code></h2>");
  PG_RESULT(PgHtmlNodePtr, PgError) res_parse = pg_html_parse(s, allocator);

  PgHtmlNode *root = PG_UNWRAP(res_parse);
  PG_ASSERT(!pg_linked_list_is_empty(&root->first_child));
  PG_ASSERT(pg_linked_list_is_empty(&root->next_sibling));

  PgHtmlNode *node_h2 = pg_html_node_get_first_child(root);
  PG_ASSERT(node_h2->parent.next == &root->parent);
  PG_ASSERT(pg_linked_list_is_empty(&node_h2->next_sibling));
  PG_ASSERT(!pg_linked_list_is_empty(&node_h2->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_h2->token_start.kind);
  PG_ASSERT(pg_string_eq(node_h2->token_start.tag, PG_S("h2")));
  PG_ASSERT(pg_string_eq(node_h2->token_end.tag, PG_S("h2")));

  PgHtmlNode *node_h2_text = pg_html_node_get_first_child(node_h2);
  PG_ASSERT(node_h2_text->parent.next == &node_h2->parent);
  PG_ASSERT(!pg_linked_list_is_empty(&node_h2_text->next_sibling));
  PG_ASSERT(pg_linked_list_is_empty(&node_h2_text->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == node_h2_text->token_start.kind);
  PG_ASSERT(pg_string_eq(node_h2_text->token_start.text, PG_S("Hello ")));

  PgHtmlNode *node_code = pg_html_node_get_next_sibling(node_h2_text);
  PG_ASSERT(node_code->parent.next == &node_h2->parent);
  PG_ASSERT(pg_linked_list_is_empty(&node_code->next_sibling));
  PG_ASSERT(!pg_linked_list_is_empty(&node_code->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TAG_OPENING == node_code->token_start.kind);
  PG_ASSERT(pg_string_eq(node_code->token_start.tag, PG_S("code")));
  PG_ASSERT(pg_string_eq(node_code->token_end.tag, PG_S("code")));

  PgHtmlNode *node_code_text = pg_html_node_get_first_child(node_code);
  PG_ASSERT(node_code_text->parent.next == &node_code->parent);
  PG_ASSERT(pg_linked_list_is_empty(&node_code_text->next_sibling));
  PG_ASSERT(pg_linked_list_is_empty(&node_code_text->first_child));
  PG_ASSERT(PG_HTML_TOKEN_KIND_TEXT == node_code_text->token_start.kind);
  PG_ASSERT(pg_string_eq(node_code_text->token_start.text, PG_S("world")));

  PG_ASSERT(node_code_text->token_end.end < node_h2->token_end.start);
}

static void test_string_escape_js() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  {
    PgString s = PG_S("hello\t,\n'world'\r\"\v\"🍌");
    Pgu8Dyn sb = {0};
    pg_string_builder_append_js_string_escaped(&sb, s, allocator);
    PgString out = PG_DYN_TO_SLICE(PgString, sb);
    PgString expected = PG_S("hello\\t,\\n\\'world\\'\\r\\\"\\v\\\"\\u{1f34c}");

    PG_ASSERT(pg_string_eq(out, expected));
  }
}

static void test_string_builder_append_u64() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // Zero.
  {
    Pgu8Dyn sb = {0};
    pg_string_builder_append_u64(&sb, 0, allocator);
    PgString out = PG_DYN_TO_SLICE(PgString, sb);
    PgString expected = PG_S("0");

    PG_ASSERT(pg_string_eq(out, expected));
  }

  // Other numbers.
  {
    Pgu8Dyn sb = {0};
    pg_string_builder_append_u64(&sb, 4'056'123'789, allocator);
    PgString out = PG_DYN_TO_SLICE(PgString, sb);
    PgString expected = PG_S("4056123789");

    PG_ASSERT(pg_string_eq(out, expected));
  }
}

static void test_string_buillder_append_u64_hex() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  Pgu8Dyn sb = {0};
  pg_string_builder_append_u64_hex(&sb, 0x1f34c /* 🍌 */, allocator);
  PgString out = PG_DYN_TO_SLICE(PgString, sb);
  PgString expected = PG_S("1f34c");

  PG_ASSERT(pg_string_eq(out, expected));
}

static void test_adjacency_matrix() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgAdjacencyMatrix matrix = pg_adjacency_matrix_make(6, allocator);
  PG_ASSERT(pg_adjacency_matrix_is_empty(matrix));

  pg_adjacency_matrix_add_edge(&matrix, 2, 0);
  PG_ASSERT(pg_adjacency_matrix_has_edge(matrix, 2, 0));
  pg_adjacency_matrix_add_edge(&matrix, 3, 1);
  PG_ASSERT(pg_adjacency_matrix_has_edge(matrix, 3, 1));
  pg_adjacency_matrix_add_edge(&matrix, 3, 2);
  PG_ASSERT(pg_adjacency_matrix_has_edge(matrix, 3, 2));
  pg_adjacency_matrix_add_edge(&matrix, 5, 0);
  PG_ASSERT(pg_adjacency_matrix_has_edge(matrix, 5, 0));
  pg_adjacency_matrix_add_edge(&matrix, 5, 1);
  PG_ASSERT(pg_adjacency_matrix_has_edge(matrix, 5, 1));
  pg_adjacency_matrix_add_edge(&matrix, 5, 2);
  PG_ASSERT(pg_adjacency_matrix_has_edge(matrix, 5, 2));
  pg_adjacency_matrix_add_edge(&matrix, 5, 3);
  PG_ASSERT(pg_adjacency_matrix_has_edge(matrix, 5, 3));
  pg_adjacency_matrix_add_edge(&matrix, 5, 4);
  PG_ASSERT(pg_adjacency_matrix_has_edge(matrix, 5, 4));

  PG_ASSERT(!pg_adjacency_matrix_is_empty(matrix));

  PG_ASSERT(2 == pg_adjacency_matrix_count_neighbors(matrix, 0));
  PG_ASSERT(2 == pg_adjacency_matrix_count_neighbors(matrix, 1));
  PG_ASSERT(3 == pg_adjacency_matrix_count_neighbors(matrix, 2));
  PG_ASSERT(3 == pg_adjacency_matrix_count_neighbors(matrix, 3));
  PG_ASSERT(1 == pg_adjacency_matrix_count_neighbors(matrix, 4));
  PG_ASSERT(5 == pg_adjacency_matrix_count_neighbors(matrix, 5));
}

static i32 test_thread_fn(void *data) {
  PG_ASSERT(data);

  u64 *n = data;
  *n = 42;

  return 0;
}

static void test_thread() {
  u64 n = 1;
  PG_RESULT(PgThread, PgError)
  res_thread = pg_thread_create(test_thread_fn, &n);

  PgThread thread = PG_UNWRAP(res_thread);

  PG_ASSERT(0 == pg_thread_join(thread));
  PG_ASSERT(42 == n);
}

typedef enum {
  AIO_PEER_STATE_INITIAL,
  AIO_PEER_STATE_SENT_HELLO,
  AIO_PEER_STATE_DONE,
} AioPeerState;

static void test_aio_peer(PgAio aio, PgWriter *w, PgReader *r,
                          AioPeerState *state, PgFileDescriptor fd,
                          PgAllocator *allocator) {
  switch (*state) {
  case AIO_PEER_STATE_INITIAL: {
    PG_ASSERT(0 == pg_writer_write_full(w, PG_S("hello world"), allocator));
    PG_ASSERT(0 == pg_writer_flush(w, allocator));
    PG_ASSERT(0 ==
              pg_aio_unregister_interest(aio, fd, PG_AIO_EVENT_KIND_WRITABLE));
    PG_ASSERT(0 ==
              pg_aio_register_interest_fd(aio, fd, PG_AIO_EVENT_KIND_READABLE));

    *state = AIO_PEER_STATE_SENT_HELLO;
  } break;
  case AIO_PEER_STATE_SENT_HELLO: {
    u8 recv[1024] = {0};
    PG_SLICE(u8)
    recv_slice = {
        .data = recv,
        .len = PG_STATIC_ARRAY_LEN(recv),
    };
    PG_RESULT(u64, PgError) res = pg_reader_read(r, recv_slice);

    recv_slice.len = PG_UNWRAP(res);
    PG_ASSERT(pg_bytes_eq(recv_slice, PG_S("hello world")));

    *state = AIO_PEER_STATE_DONE;
  } break;
  default:
    PG_ASSERT(0);
  }
}

static void test_aio_tcp_sockets() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgRing cqe = pg_ring_make(sizeof(PgAioEvent) * 16, allocator);

  PG_RESULT(PgAio, PgError) res_aio = pg_aio_init();
  PgAio aio = PG_UNWRAP(res_aio);

  PG_RESULT(PG_PAIR(PgFileDescriptor), PgError)
  res_sockets = pg_net_make_socket_pair(PG_NET_SOCKET_DOMAIN_LOCAL,
                                        PG_NET_SOCKET_TYPE_TCP,
                                        PG_NET_SOCKET_OPTION_NONE);
  PG_PAIR(PgFileDescriptor) sockets = PG_UNWRAP(res_sockets);
  PgFileDescriptor client_fd = sockets.first;
  PgFileDescriptor server_fd = sockets.second;

  PG_ASSERT(0 == pg_aio_register_interest_fd(aio, client_fd,
                                             PG_AIO_EVENT_KIND_WRITABLE));
  PG_ASSERT(0 == pg_aio_register_interest_fd(aio, server_fd,
                                             PG_AIO_EVENT_KIND_READABLE));

  AioPeerState client_state = {0};
  AioPeerState server_state = {0};
  PgWriter client_writer = pg_writer_make_from_socket(client_fd, 16, allocator);
  PgWriter server_writer = pg_writer_make_from_socket(server_fd, 16, allocator);
  PgReader client_reader = pg_reader_make_from_socket(client_fd, 16, allocator);
  PgReader server_reader = pg_reader_make_from_socket(server_fd, 16, allocator);

  for (u64 _i = 0; _i < 32; _i++) {
    Pgu32Option timeout_opt = {0};
    PG_RESULT(u64, PgError) res_wait = pg_aio_wait_cqe(aio, &cqe, timeout_opt);
    u64 wait = PG_UNWRAP(res_wait);
    PG_ASSERT(0 != wait);

    for (u64 i = 0; i < wait; i++) {
      PG_OPTION(PgAioEvent) event_opt = pg_aio_cqe_dequeue(&cqe);
      PG_ASSERT(event_opt.has_value);

      PgAioEvent event = event_opt.value;
      if (client_fd.fd == event.fd.fd) {
        test_aio_peer(aio, &client_writer, &client_reader, &client_state,
                      client_fd, allocator);
      } else if (server_fd.fd == event.fd.fd) {
        test_aio_peer(aio, &server_writer, &server_reader, &server_state,
                      server_fd, allocator);
      } else {
        PG_ASSERT(0);
      }
    }

    if (AIO_PEER_STATE_DONE == client_state &&
        AIO_PEER_STATE_DONE == server_state) {
      return;
    }
  }
  PG_ASSERT(0);
}

#if 0
static void test_watch_directory() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgAioResult res_aio = pg_aio_init();
  PG_ASSERT(0 == res_aio.err);
  PgAio aio = res_aio.res;

  PgError err = pg_aio_register_watch_directory(
      &aio, PG_S("."), PG_WALK_DIRECTORY_KIND_FILE, allocator);
  PG_ASSERT(0 == err);

  PgRing cqe = pg_ring_make(sizeof(PgAioEvent) * 16, allocator);

  for (u64 _i = 0; _i < 4; _i++) {
    Pgu32Option timeout_opt = {0};
    PG_RESULT(u64) res_wait = pg_aio_wait_cqe(aio, &cqe, timeout_opt);
    PG_ASSERT(0 == res_wait.err);
    if (0 == res_wait.res) {
      continue;
    }

    for (u64 i = 0; i < res_wait.res; i++) {
      PgAioEventOption event_opt = pg_aio_cqe_dequeue(&cqe);
      PG_ASSERT(event_opt.has_value);

      PgAioEvent event = event_opt.res;
      __builtin_dump_struct(&event, &printf);
    }
  }
}
#endif

static void test_cli_options_parse() {
  PgArena arena = pg_arena_make_from_virtual_mem(16 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  // No options, only plain arguments.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output file"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "out.txt",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(2 == res.plain_arguments.len);
    PG_ASSERT(0 == res.options.len);

    PG_ASSERT(
        pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0), PG_S("out.txt")));
    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 1),
                           PG_S("some_argument")));
  }
  // Some short option with value given.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "-o",
        "out.txt",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.plain_arguments.len);
    PG_ASSERT(1 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_short, PG_S("o")));
    PG_ASSERT(1 == opt0.values.len);
    PG_ASSERT(pg_string_eq(PG_SLICE_AT(opt0.values, 0), PG_S("out.txt")));
  }

  // All short options given.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin", "-o", "out.txt", "some_argument", "-v",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.plain_arguments.len);
    PG_ASSERT(2 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_short, PG_S("o")));
    PG_ASSERT(1 == opt0.values.len);
    PG_ASSERT(pg_string_eq(PG_SLICE_AT(opt0.values, 0), PG_S("out.txt")));

    PgCliOption opt1 = PG_SLICE_AT(res.options, 1);
    PG_ASSERT(0 == opt1.err);
    PG_ASSERT(pg_string_eq(opt1.description.name_short, PG_S("v")));
    PG_ASSERT(0 == opt1.values.len);
  }

  // Short options given, coalesced.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin", "-o", "out.txt", "-vH", "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.plain_arguments.len);
    PG_ASSERT(3 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_short, PG_S("o")));
    PG_ASSERT(1 == opt0.values.len);
    PG_ASSERT(pg_string_eq(PG_SLICE_AT(opt0.values, 0), PG_S("out.txt")));

    PgCliOption opt1 = PG_SLICE_AT(res.options, 1);
    PG_ASSERT(0 == opt1.err);
    PG_ASSERT(0 == opt1.values.len);
    PG_ASSERT(pg_string_eq(opt1.description.name_short, PG_S("v")));

    PgCliOption opt2 = PG_SLICE_AT(res.options, 2);
    PG_ASSERT(0 == opt2.err);
    PG_ASSERT(0 == opt2.values.len);
    PG_ASSERT(pg_string_eq(opt2.description.name_short, PG_S("H")));
  }

  // Short option given without argument but one was expected.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "-o",
        "-vH",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(PG_ERR_CLI_MISSING_REQUIRED_OPTION_VALUE == res.err);
    PG_ASSERT(pg_string_eq(res.err_argv, PG_S("o")));
  }

  // Missing required option.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                    .required = true,
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "-H",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(PG_ERR_CLI_MISSING_REQUIRED_OPTION == res.err);
    PG_ASSERT(pg_string_eq(res.err_argv, PG_S("v")));
  }

  // Malformed option.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "---foo",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(PG_ERR_CLI_MALFORMED_OPTION == res.err);
    PG_ASSERT(pg_string_eq(res.err_argv, PG_S("---foo")));
  }

  // Malformed option.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "-",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(PG_ERR_CLI_MALFORMED_OPTION == res.err);
    PG_ASSERT(pg_string_eq(res.err_argv, PG_S("-")));
  }

  // Treat everything after `--` as plain arguments.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("Hidden mode"),
                }),
                allocator);

    char *argv[] = {
        "main.bin", "-H", "--", "--verbose", "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.options.len);
    PG_ASSERT(2 == res.plain_arguments.len);

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(pg_string_eq(opt0.description.name_long, PG_S("hidden")));
    PG_ASSERT(0 == opt0.values.len);

    PgString plain_arg0 = PG_SLICE_AT(res.plain_arguments, 0);
    PG_ASSERT(pg_string_eq(plain_arg0, PG_S("--verbose")));

    PgString plain_arg1 = PG_SLICE_AT(res.plain_arguments, 1);
    PG_ASSERT(pg_string_eq(plain_arg1, PG_S("some_argument")));
  }

  // Short options given, coalesced, repeated.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin", "-o", "out1.txt", "-vH", "some_argument", "-o", "out2.txt",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.plain_arguments.len);
    PG_ASSERT(3 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_short, PG_S("o")));
    PG_ASSERT(2 == opt0.values.len);
    PgString opt0_value0 = PG_SLICE_AT(opt0.values, 0);
    PgString opt0_value1 = PG_SLICE_AT(opt0.values, 1);
    PG_ASSERT(pg_string_eq(opt0_value0, PG_S("out1.txt")));
    PG_ASSERT(pg_string_eq(opt0_value1, PG_S("out2.txt")));

    PgCliOption opt1 = PG_SLICE_AT(res.options, 1);
    PG_ASSERT(0 == opt1.err);
    PG_ASSERT(pg_string_eq(opt1.description.name_short, PG_S("v")));
    PG_ASSERT(0 == opt1.values.len);

    PgCliOption opt2 = PG_SLICE_AT(res.options, 2);
    PG_ASSERT(0 == opt2.err);
    PG_ASSERT(pg_string_eq(opt2.description.name_short, PG_S("H")));
    PG_ASSERT(0 == opt2.values.len);
  }

  // Short options repeated without value: noop.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "-vH",
        "some_argument",
        "-v",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.plain_arguments.len);
    PG_ASSERT(2 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_short, PG_S("v")));
    PG_ASSERT(0 == opt0.values.len);

    PgCliOption opt1 = PG_SLICE_AT(res.options, 1);
    PG_ASSERT(0 == opt1.err);
    PG_ASSERT(pg_string_eq(opt1.description.name_short, PG_S("H")));
    PG_ASSERT(0 == opt1.values.len);
  }

  // Unknown option passed, coalesced.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "-vx",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(PG_ERR_CLI_UNKNOWN_OPTION == res.err);
    PG_ASSERT(pg_string_eq(res.err_argv, PG_S("-x")));
  }

  // Unknown option passed.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "-x",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(PG_ERR_CLI_UNKNOWN_OPTION == res.err);
    PG_ASSERT(pg_string_eq(res.err_argv, PG_S("-x")));
  }
  // Long option given.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin",
        "--output=out1.txt",
        "some_argument",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.plain_arguments.len);
    PG_ASSERT(1 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_long, PG_S("output")));
    PG_ASSERT(1 == opt0.values.len);
    PgString opt0_value0 = PG_SLICE_AT(opt0.values, 0);
    PG_ASSERT(pg_string_eq(opt0_value0, PG_S("out1.txt")));
  }
  // Long & short options given.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin", "-v", "--output=out1.txt", "some_argument", "--hidden",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.plain_arguments.len);
    PG_ASSERT(3 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_long, PG_S("verbose")));
    PG_ASSERT(0 == opt0.values.len);

    PgCliOption opt1 = PG_SLICE_AT(res.options, 1);
    PG_ASSERT(0 == opt1.err);
    PG_ASSERT(pg_string_eq(opt1.description.name_long, PG_S("output")));
    PG_ASSERT(1 == opt1.values.len);
    PgString opt1_value0 = PG_SLICE_AT(opt1.values, 0);
    PG_ASSERT(pg_string_eq(opt1_value0, PG_S("out1.txt")));

    PgCliOption opt2 = PG_SLICE_AT(res.options, 2);
    PG_ASSERT(0 == opt2.err);
    PG_ASSERT(pg_string_eq(opt2.description.name_long, PG_S("hidden")));
    PG_ASSERT(0 == opt2.values.len);
  }

  // Repeated, same long & short option given.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin", "--output=out1.txt", "some_argument", "-o", "foo.txt",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(1 == res.plain_arguments.len);
    PG_ASSERT(1 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_long, PG_S("output")));
    PG_ASSERT(2 == opt0.values.len);
    PgString opt0_value0 = PG_SLICE_AT(opt0.values, 0);
    PG_ASSERT(pg_string_eq(opt0_value0, PG_S("out1.txt")));
    PgString opt0_value1 = PG_SLICE_AT(opt0.values, 1);
    PG_ASSERT(pg_string_eq(opt0_value1, PG_S("foo.txt")));
  }
  // Help, short form.
  {
    PG_DYN(PgCliOptionDescription) descs = {0};
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("v"),
                    .name_long = PG_S("verbose"),
                    .description = PG_S("Verbose mode"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("H"),
                    .name_long = PG_S("hidden"),
                    .description = PG_S("scan hidden files"),
                }),
                allocator);
    PG_DYN_PUSH(&descs,
                ((PgCliOptionDescription){
                    .name_short = PG_S("o"),
                    .name_long = PG_S("output"),
                    .description = PG_S("Specify an output"),
                    .value_name = PG_S("file"),
                }),
                allocator);

    char *argv[] = {
        "main.bin", "--output=out1.txt", "some_argument", "-h", "foo.txt",
    };
    int argc = PG_STATIC_ARRAY_LEN(argv);

    PgCliParseResult res = pg_cli_parse(&descs, argc, argv, allocator);
    PG_ASSERT(0 == res.err);
    PG_ASSERT(2 == res.plain_arguments.len);
    PG_ASSERT(2 == res.options.len);

    PG_ASSERT(pg_string_eq(PG_SLICE_AT(res.plain_arguments, 0),
                           PG_S("some_argument")));
    PG_ASSERT(
        pg_string_eq(PG_SLICE_AT(res.plain_arguments, 1), PG_S("foo.txt")));

    PgCliOption opt0 = PG_SLICE_AT(res.options, 0);
    PG_ASSERT(0 == opt0.err);
    PG_ASSERT(pg_string_eq(opt0.description.name_long, PG_S("output")));
    PG_ASSERT(1 == opt0.values.len);
    PgString opt0_value0 = PG_SLICE_AT(opt0.values, 0);
    PG_ASSERT(pg_string_eq(opt0_value0, PG_S("out1.txt")));

    PgCliOption opt1 = PG_SLICE_AT(res.options, 1);
    PG_ASSERT(0 == opt1.err);
    PG_ASSERT(pg_string_eq(opt1.description.name_long, PG_S("help")));
    PG_ASSERT(0 == opt1.values.len);
  }
}

static void test_cli_options_help() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PG_DYN(PgCliOptionDescription) descs = {0};
  PG_DYN_PUSH(&descs,
              ((PgCliOptionDescription){
                  .name_short = PG_S("v"),
                  .description = PG_S("Verbose mode"),
              }),
              allocator);
  PG_DYN_PUSH(&descs,
              ((PgCliOptionDescription){
                  .name_long = PG_S("hidden"),
                  .description = PG_S("Scan hidden files"),
              }),
              allocator);
  PG_DYN_PUSH(&descs,
              ((PgCliOptionDescription){
                  .name_short = PG_S("o"),
                  .name_long = PG_S("output"),
                  .description = PG_S("Specify an output file"),
                  .value_name = PG_S("file"),
                  .required = true,
              }),
              allocator);
  PgString exe_name = PG_S("a.out");
  PgString description =
      PG_S("This is an example program. It helps fooizing bars.");
  PgString plain_arguments_description = PG_S("<file1> <file2> <file3>");

  pg_cli_inject_help_option(&descs, allocator);

  PgString help = pg_cli_generate_help(descs, exe_name, description,
                                       plain_arguments_description, allocator);

  PgString expected = PG_S(
      "a.out [-v] [--hidden] (-o|--output file) [-h|--help] <file1> "
      "<file2> <file3>\n\nThis is "
      "an example program. It helps fooizing bars.\n\nOPTIONS:\n    -v\n     "
      "  "
      " Verbose mode.\n\n    --hidden\n        Scan hidden files.\n\n    -o "
      "file, --output=file    [required]\n        Specify an output "
      "file.\n\n    -h, --help\n        Print this help message.\n\n\n");
  PG_ASSERT(pg_string_eq(expected, help));
}

static void test_self() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  {
    u64 offset = pg_self_pie_get_offset();
    PG_ASSERT(offset > 0xffff);
  }
  // Do it twice to check that it works with the `once` mechanism.
  {
    u64 offset = pg_self_pie_get_offset();
    PG_ASSERT(offset >= 0xffffUL);
  }

  {
    PgString exe_path = pg_self_exe_get_path(allocator);
    PG_ASSERT(pg_string_contains(exe_path, PG_S("cstd/test")));
  }

  // Do it twice to check that it works with the `once` mechanism.
  {
    PgString exe_path = pg_self_exe_get_path(nullptr);
    PG_ASSERT(pg_string_contains(exe_path, PG_S("cstd/test")));
  }
}

[[maybe_unused]]
static void test_debug_info() {
  PgArena arena = pg_arena_make_from_virtual_mem(16 * PG_MiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PG_RESULT(PgDebugInfoIterator, PgError)
  res_it = pg_self_debug_info_iterator_make(allocator);
  PgDebugInfoIterator it = PG_UNWRAP(res_it);

  PgDwarfDebugInfoCompilationUnit unit = it.unit;
  PG_ASSERT(PG_DWARF_COMPILATION_UNIT_COMPILE == unit.kind);
  PG_ASSERT(unit.abbrevs.len > 0);

  {
    PgWriter w =
        pg_writer_make_from_file_descriptor(pg_os_stdout(), 1024, allocator);
    (void)pg_dwarf_compilation_unit_print_abbreviations(&w, unit, nullptr);
  }

  PgArena fn_arena = pg_arena_make_from_virtual_mem(512 * PG_KiB);
  PgArenaAllocator fn_arena_allocator = pg_make_arena_allocator(&fn_arena);
  PgAllocator *fn_allocator =
      pg_arena_allocator_as_allocator(&fn_arena_allocator);
  PG_RESULT(PG_DYN(PgDebugFunctionDeclaration), PgError)
  res_fns = pg_dwarf_collect_functions(&it, fn_allocator);
  PG_DYN(PgDebugFunctionDeclaration) fns = PG_UNWRAP(res_fns);
  PG_ASSERT(fns.len > 0);

  PG_ASSERT(0 == pg_arena_release(&arena));
  pg_self_debug_info_iterator_release(it);

  {
    PgWriter w =
        pg_writer_make_from_file_descriptor(pg_os_stderr(), 1024, allocator);
    PG_ASSERT(0 == pg_debug_print_functions(&w, fns, nullptr));
    pg_stack_trace_print_dwarf(1);
  }
}

static void test_u64_leb128() {
  u8 input[] = {0xC0, 0x9c, 0xD2, 0x01};
  PG_SLICE(u8) input_slice = PG_SLICE_FROM_C(input);

  PgReader r = pg_reader_make_from_bytes(input_slice);
  PG_RESULT(u64, PgError) res = pg_reader_read_u64_leb128(&r);
  PG_ASSERT(PG_SLICE_IS_EMPTY(r.u.bytes));
  PG_ASSERT(0x348e40 == PG_UNWRAP(res));
}

static void test_write_u64_hex() {
  PgArena arena = pg_arena_make_from_virtual_mem(4 * PG_KiB);
  PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
  PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

  PgWriter w = pg_writer_make_string_builder(8, allocator);
  PG_ASSERT(0 == pg_writer_write_u64_hex(&w, 0x348e40, allocator));
  PgString s = PG_DYN_TO_SLICE(PgString, w.u.bytes);
  PG_ASSERT(pg_string_eq(PG_S("0x348e40"), s));
}

static void test_arena() {
  // Empty arena.
  {
    PgArenaAllocator arena_allocator = {0};
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);
    u8 *res = pg_alloc(allocator, sizeof(u8), _Alignof(u8), 1);
    PG_ASSERT(nullptr == res);
  }
  // Null allocator.
  {
    u8 *res = pg_alloc(nullptr, sizeof(u8), _Alignof(u8), 1);
    PG_ASSERT(nullptr == res);
  }
  // Arena of size 0.
  {
    PgArena arena = pg_arena_make_from_virtual_mem(0);
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    u8 *res = pg_alloc(allocator, sizeof(u8), _Alignof(u8), 1);
    PG_ASSERT(nullptr == res);
  }
  // Arena too small for allocation.
  {
    PgArena arena = pg_arena_make_from_virtual_mem(2);
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    u8 *res = pg_alloc(allocator, sizeof(u8), _Alignof(u8), 3);
    PG_ASSERT(nullptr == res);
  }
  // Arena just big enough for allocation.
  {
    PgArena arena = pg_arena_make_from_virtual_mem(1);
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    u8 *res = pg_alloc(allocator, sizeof(u8), _Alignof(u8), 1);
    PG_ASSERT(nullptr != res);
    // Prevent optimizer from removing all of this.
    volatile u8 *x = res;
    *x += 1;
  }
  // Arena too small for reallocation.
  {
    PgArena arena = pg_arena_make_from_virtual_mem(1);
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    u8 *res = pg_alloc(allocator, sizeof(u8), _Alignof(u8), 1);
    PG_ASSERT(nullptr != res);

    // 1 -> 2.
    res = pg_realloc(allocator, res, 1, sizeof(u8), _Alignof(u8), 2);
    PG_ASSERT(nullptr == res);
  }
  // Arena just big enough for reallocation, using the bump code path for
  // optimization.
  {
    PgArena arena = pg_arena_make_from_virtual_mem(2);
    PgArenaAllocator arena_allocator = pg_make_arena_allocator(&arena);
    PgAllocator *allocator = pg_arena_allocator_as_allocator(&arena_allocator);

    u8 *res = pg_alloc(allocator, sizeof(u8), _Alignof(u8), 1);
    PG_ASSERT(nullptr != res);

    // 1 -> 2.
    res = pg_realloc(allocator, res, 1, sizeof(u8), _Alignof(u8), 2);

    PG_ASSERT(nullptr != res);
    // Prevent optimizer from removing all of this.
    volatile u8 *x = res + 1;
    *x += 1;
  }
}

int main() {
  test_arena();
  test_u64_leb128();
  test_write_u64_hex();
  test_self();
#if 0
  test_debug_info();
#endif
  test_rune_bytes_count();
  test_utf8_count();
  test_string_last();
  test_string_first();
  test_string_index_of_rune();
  test_string_last_index_of_rune();
  test_slice_range();
  test_utf8_iterator();
  test_string_index_of_string();
  test_string_trim();
  test_string_cut();
  test_string_split_byte();
  test_string_split_string();
  test_dyn_ensure_cap();
  test_path_stem();
  test_string_consume();
  test_string_parse_u64();
  test_string_cmp();
  test_sha1();
  test_slice_swap_remove();
  test_dynu8_append_u8_hex_upper();
  test_ipv4_address_to_string();
  test_url_encode();
  test_string_index_of_any_byte();
  test_u8x4_be_to_u32_and_back();
  test_bitfield();
  test_ring_buffer_read_write();
  test_ring_buffer_read_write_fuzz();
  test_url_parse_relative_path();
  test_url_parse();
  test_http_request_to_string();
  test_http_parse_response_status_line();
  test_http_parse_request_status_line();
  test_http_parse_header();
  test_http_read_request_full_no_content_length();
  test_http_read_request_no_body_separator();
  test_http_read_request_full_without_headers();
  test_http_read_request_full_without_body();
#if 0
  test_http_read_response();
  test_http_request_response();
#endif

  test_log();
  test_div_ceil();
  test_path_base_name();
  test_process_no_capture();
  test_process_capture();
  test_process_stdin();
  test_linked_list();
  test_html_tokenize_no_attributes();
  test_html_tokenize_with_key_no_value();
  test_html_tokenize_with_attributes();
  test_html_tokenize_nested();
  test_html_tokenize_with_doctype();
  test_html_tokenize_with_comment();
  test_html_parse();
  test_html_parse_title_with_html_content();
  test_string_escape_js();
  test_string_builder_append_u64();
  test_string_buillder_append_u64_hex();
  test_adjacency_matrix();
  test_thread();
  test_aio_tcp_sockets();
#if 0
  test_watch_directory();
#endif
  test_cli_options_parse();
  test_cli_options_help();
}
