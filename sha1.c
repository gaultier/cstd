/*	$OpenBSD: pg_sha1.c,v 1.27 2019/06/07 22:56:36 dtucker Exp $	*/

/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 *
 * Test Vectors (from FIPS PUB 180-1)
 * "abc"
 *   A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
 * "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *   84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
 * A million repetitions of "a"
 *   34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 */

#include <stddef.h>
#include <stdint.h>

#define PG_SHA1_BLOCK_LENGTH 64
#define PG_SHA1_DIGEST_LENGTH 20
#define PG_SHA1_DIGEST_STRING_LENGTH (PG_SHA1_DIGEST_LENGTH * 2 + 1)

typedef struct {
  uint32_t state[5];
  uint64_t count;
  uint8_t buffer[PG_SHA1_BLOCK_LENGTH];
} PG_SHA1_CTX;

#define pg_rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/*
 * pg_blk0() and pg_blk() perform the initial expand.
 * I got the idea of expanding during the round function from SSLeay
 */
#define pg_blk0(i)                                                             \
  (block->l[i] = (pg_rol(block->l[i], 24) & 0xFF00FF00) |                      \
                 (pg_rol(block->l[i], 8) & 0x00FF00FF))
#define pg_blk(i)                                                              \
  (block->l[i & 15] =                                                          \
       pg_rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^               \
                  block->l[(i + 2) & 15] ^ block->l[i & 15],                   \
              1))

/*
 * (PG_R0+PG_R1), PG_R2, PG_R3, PG_R4 are the different operations (rounds) used
 * in PG_SHA1
 */
#define PG_R0(v, w, x, y, z, i)                                                \
  z += ((w & (x ^ y)) ^ y) + pg_blk0(i) + 0x5A827999 + pg_rol(v, 5);           \
  w = pg_rol(w, 30);
#define PG_R1(v, w, x, y, z, i)                                                \
  z += ((w & (x ^ y)) ^ y) + pg_blk(i) + 0x5A827999 + pg_rol(v, 5);            \
  w = pg_rol(w, 30);
#define PG_R2(v, w, x, y, z, i)                                                \
  z += (w ^ x ^ y) + pg_blk(i) + 0x6ED9EBA1 + pg_rol(v, 5);                    \
  w = pg_rol(w, 30);
#define PG_R3(v, w, x, y, z, i)                                                \
  z += (((w | x) & y) | (w & x)) + pg_blk(i) + 0x8F1BBCDC + pg_rol(v, 5);      \
  w = pg_rol(w, 30);
#define PG_R4(v, w, x, y, z, i)                                                \
  z += (w ^ x ^ y) + pg_blk(i) + 0xCA62C1D6 + pg_rol(v, 5);                    \
  w = pg_rol(w, 30);

typedef union {
  uint8_t c[64];
  uint32_t l[16];
} PG_CHAR64LONG16;

/*
 * Hash a single 512-bit block. This is the core of the algorithm.
 */
static void PG_SHA1Transform(uint32_t state[5],
                             const uint8_t buffer[PG_SHA1_BLOCK_LENGTH]) {
  uint32_t a, b, c, d, e;
  uint8_t workspace[PG_SHA1_BLOCK_LENGTH];
  PG_CHAR64LONG16 *block = (void *)(PG_CHAR64LONG16 *)(void *)workspace;

  (void)__builtin_memcpy(block, buffer, PG_SHA1_BLOCK_LENGTH);

  /* Copy context->state[] to working vars */
  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];

  /* 4 rounds of 20 operations each. Loop unrolled. */
  PG_R0(a, b, c, d, e, 0)
  PG_R0(e, a, b, c, d, 1)
  PG_R0(d, e, a, b, c, 2)
  PG_R0(c, d, e, a, b, 3)
  PG_R0(b, c, d, e, a, 4)
  PG_R0(a, b, c, d, e, 5)
  PG_R0(e, a, b, c, d, 6)
  PG_R0(d, e, a, b, c, 7)
  PG_R0(c, d, e, a, b, 8)
  PG_R0(b, c, d, e, a, 9)
  PG_R0(a, b, c, d, e, 10)
  PG_R0(e, a, b, c, d, 11)
  PG_R0(d, e, a, b, c, 12)
  PG_R0(c, d, e, a, b, 13)
  PG_R0(b, c, d, e, a, 14)
  PG_R0(a, b, c, d, e, 15)
  PG_R1(e, a, b, c, d, 16)
  PG_R1(d, e, a, b, c, 17)
  PG_R1(c, d, e, a, b, 18)
  PG_R1(b, c, d, e, a, 19)
  PG_R2(a, b, c, d, e, 20)
  PG_R2(e, a, b, c, d, 21)
  PG_R2(d, e, a, b, c, 22)
  PG_R2(c, d, e, a, b, 23)
  PG_R2(b, c, d, e, a, 24)
  PG_R2(a, b, c, d, e, 25)
  PG_R2(e, a, b, c, d, 26)
  PG_R2(d, e, a, b, c, 27)
  PG_R2(c, d, e, a, b, 28)
  PG_R2(b, c, d, e, a, 29)
  PG_R2(a, b, c, d, e, 30)
  PG_R2(e, a, b, c, d, 31)
  PG_R2(d, e, a, b, c, 32)
  PG_R2(c, d, e, a, b, 33)
  PG_R2(b, c, d, e, a, 34)
  PG_R2(a, b, c, d, e, 35)
  PG_R2(e, a, b, c, d, 36)
  PG_R2(d, e, a, b, c, 37)
  PG_R2(c, d, e, a, b, 38)
  PG_R2(b, c, d, e, a, 39)
  PG_R3(a, b, c, d, e, 40)
  PG_R3(e, a, b, c, d, 41)
  PG_R3(d, e, a, b, c, 42)
  PG_R3(c, d, e, a, b, 43)
  PG_R3(b, c, d, e, a, 44)
  PG_R3(a, b, c, d, e, 45)
  PG_R3(e, a, b, c, d, 46)
  PG_R3(d, e, a, b, c, 47)
  PG_R3(c, d, e, a, b, 48)
  PG_R3(b, c, d, e, a, 49)
  PG_R3(a, b, c, d, e, 50)
  PG_R3(e, a, b, c, d, 51)
  PG_R3(d, e, a, b, c, 52)
  PG_R3(c, d, e, a, b, 53)
  PG_R3(b, c, d, e, a, 54)
  PG_R3(a, b, c, d, e, 55)
  PG_R3(e, a, b, c, d, 56)
  PG_R3(d, e, a, b, c, 57)
  PG_R3(c, d, e, a, b, 58)
  PG_R3(b, c, d, e, a, 59)
  PG_R4(a, b, c, d, e, 60)
  PG_R4(e, a, b, c, d, 61)
  PG_R4(d, e, a, b, c, 62)
  PG_R4(c, d, e, a, b, 63)
  PG_R4(b, c, d, e, a, 64)
  PG_R4(a, b, c, d, e, 65)
  PG_R4(e, a, b, c, d, 66)
  PG_R4(d, e, a, b, c, 67)
  PG_R4(c, d, e, a, b, 68)
  PG_R4(b, c, d, e, a, 69)
  PG_R4(a, b, c, d, e, 70)
  PG_R4(e, a, b, c, d, 71)
  PG_R4(d, e, a, b, c, 72)
  PG_R4(c, d, e, a, b, 73)
  PG_R4(b, c, d, e, a, 74)
  PG_R4(a, b, c, d, e, 75)
  PG_R4(e, a, b, c, d, 76)
  PG_R4(d, e, a, b, c, 77)
  PG_R4(c, d, e, a, b, 78)
  PG_R4(b, c, d, e, a, 79)

  /* Add the working vars back into context.state[] */
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;

  /* Wipe variables */
  a = b = c = d = e = 0;
}

/*
 * PG_SHA1Init - Initialize new context
 */
static void PG_SHA1Init(PG_SHA1_CTX *context) {

  /* PG_SHA1 initialization constants */
  context->count = 0;
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
}

/*
 * Run your data through this.
 */
static void PG_SHA1Update(PG_SHA1_CTX *context, const uint8_t *data,
                          size_t len) {
  size_t i, j;

  // j = (count * 8) % 64
  j = (size_t)((context->count >> 3) & 63);
  // count += len*8
  context->count += ((uint64_t)len << 3);
  if ((j + len) > 63) { // Too big to fit in 64 bytes => Do a round.
    // i = 64 - j i.e. i = 64 - ((count * 8) % 64)
    (void)__builtin_memcpy(&context->buffer[j], data, (i = 64 - j));
    // Why do we do a first round here?
    PG_SHA1Transform(context->state, context->buffer);
    // Process each chunk, 64 bytes at a time.
    for (; i + 63 < len; i += 64)
      PG_SHA1Transform(context->state, (uint8_t *)&data[i]);
    j = 0;
  } else {
    // Too small too fit in a chunk, just buffer the data in `context->buffer`.
    i = 0;
  }
  // Remainder, smaller than one chunk.
  (void)__builtin_memcpy(&context->buffer[j], &data[i], len - i);
}

/*
 * Add padding and return the message digest.
 */
static void PG_SHA1Pad(PG_SHA1_CTX *context) {
  uint8_t finalcount[8];
  size_t i;

  // Transform the original length in bits (`context->count`) from native to
  // big-endian.
  for (i = 0; i < 8; i++) {
    finalcount[i] = (uint8_t)((context->count >> ((7 - (i & 7)) * 8)) &
                              255); /* Endian independent */
  }
  // Append the bit '1' to the message e.g. by adding 0x80 if message length is
  // a multiple of 8 bits.
  PG_SHA1Update(context, (uint8_t *)"\200", 1);
  // Append 0 ≤ k < 512 bits '0', such that the resulting message length in bits
  // is congruent to −64 ≡ 448 (mod 512).
  while ((context->count & 504) != 448)
    PG_SHA1Update(context, (uint8_t *)"\0", 1);
  // Append ml, the original message length in bits, as a 64-bit big-endian
  // integer. Thus, the total length is a multiple of 512 bits.
  PG_SHA1Update(context, finalcount, 8); /* Should cause a PG_SHA1Transform() */
}

static void PG_SHA1Final(uint8_t digest[PG_SHA1_DIGEST_LENGTH],
                         PG_SHA1_CTX *context) {
  size_t i;

  // Post-process.
  PG_SHA1Pad(context);
  // Extract final hash value from state.
  // Produce the final hash value (big-endian) as a 160-bit number:
  // hh = (h0 leftshift 128) or (h1 leftshift 96) or (h2 leftshift 64) or (h3
  // leftshift 32) or h4
  for (i = 0; i < PG_SHA1_DIGEST_LENGTH; i++) {
    digest[i] =
        (uint8_t)((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
  }
  __builtin_memset(context, 0, sizeof(*context));
}
