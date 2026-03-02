#ifndef CRYPTO_BALLOON_BALLOON_H
#define CRYPTO_BALLOON_BALLOON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#define BITSTREAM_BUF_SIZE ((32) * (AES_BLOCK_SIZE))
#define N_NEIGHBORS (3)
#define SALT_LEN (32)
#define INLEN_MAX (1ull << 20)
#define TCOST_MIN 1ull
#define SCOST_MIN (1)
#define SCOST_MAX (UINT32_MAX)
#define BLOCKS_MIN (1ull)
#define THREADS_MAX 4096
#define BALLOON_BLOCK_SIZE (32)
#define UNUSED __attribute__((unused))

struct balloon_options {
    int64_t s_cost;
    int32_t t_cost;
};

struct bitstream {
    bool initialized;
    uint8_t* zeros;
    SHA256_CTX c;
    EVP_CIPHER_CTX* ctx;
};

struct hash_state {
    uint64_t counter;
    uint64_t n_blocks;
    bool has_mixed;
    uint8_t* buffer;
    struct bitstream bstream;
    const struct balloon_options* opts;
};

void balloon_hash(const unsigned char* input, unsigned char* output, int32_t len);

#endif // CRYPTO_BALLOON_BALLOON_H
