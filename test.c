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
    ASSERT(STRING_CMP_LESS == cmp);
  }
  {
    StringCompare cmp = string_cmp(S("b"), S("a"));
    ASSERT(STRING_CMP_GREATER == cmp);
  }
}

int main() {
  test_string_indexof_slice();
  test_string_trim();
  test_string_split();
  test_dyn_ensure_cap();
  test_slice_range();
  test_string_consume();
  test_string_parse_u64();
  test_string_cmp();
}
