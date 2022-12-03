/*
 * Copyright (c) 2015-2016, Henry Corrigan-Gibbs (https://github.com/henrycg/balloon)
 * Copyright (c) 2018-2022, barrystyle (https://github.com/barrystyle/balloon)
 *
 * balloon hashing
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <crypto/balloon/balloon.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

uint64_t bytes_to_littleend_uint64(const uint8_t* bytes, size_t n_bytes)
{
    if (n_bytes > 8)
        n_bytes = 8;
    uint64_t out = 0;
    for (int i = n_bytes - 1; i >= 0; i--) {
        out <<= 8;
        out |= bytes[i];
    }
    return out;
}

static uint64_t options_n_blocks(const struct balloon_options* opts)
{
    const uint32_t bsize = BALLOON_BLOCK_SIZE;
    uint64_t ret = (opts->s_cost * 1024) / bsize;
    return (ret < BLOCKS_MIN) ? BLOCKS_MIN : ret;
}

void bitstream_init(struct bitstream* b)
{
    SHA256_Init(&b->c);
    b->initialized = false;
    b->ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(b->ctx);
    b->zeros = (uint8_t*)malloc(BITSTREAM_BUF_SIZE * sizeof(uint8_t));
    memset(b->zeros, 0, BITSTREAM_BUF_SIZE);
}

void bitstream_free(struct bitstream* b)
{
    uint8_t out[AES_BLOCK_SIZE];
    int outl;
    EVP_EncryptFinal(b->ctx, out, &outl);
    EVP_CIPHER_CTX_cleanup(b->ctx);
    EVP_CIPHER_CTX_free(b->ctx);
    free(b->zeros);
}

void bitstream_seed_add(struct bitstream* b, const void* seed, size_t seedlen)
{
    SHA256_Update(&b->c, seed, seedlen);
}

void bitstream_seed_finalize(struct bitstream* b)
{
    uint8_t key_bytes[SHA256_DIGEST_LENGTH];
    SHA256_Final(key_bytes, &b->c);
    uint8_t iv[AES_BLOCK_SIZE];
    memset(iv, 0, AES_BLOCK_SIZE);
    EVP_CIPHER_CTX_set_padding(b->ctx, 1);
    EVP_EncryptInit(b->ctx, EVP_aes_128_ctr(), key_bytes, iv);
    b->initialized = true;
}

void compress(uint64_t* counter, uint8_t* out, const uint8_t* blocks[], size_t blocks_to_comp)
{
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    uint8_t* block_buf = (uint8_t*)malloc(blocks_to_comp * BALLOON_BLOCK_SIZE);
    memcpy(&block_buf[0], counter, 8);
    for (unsigned int i = 0; i < blocks_to_comp; i++) {
        memcpy(&block_buf[8 + i * 32], blocks[i], 32);
    }
    SHA256_Update(&ctx, block_buf, 8 + blocks_to_comp * 32);
    SHA256_Final(out, &ctx);
    *counter += 1;
    free(block_buf);
}

void expand(uint64_t* counter, uint8_t* buf, size_t blocks_in_buf)
{
    const uint8_t* blocks[1] = { buf };
    uint8_t* cur = buf + BALLOON_BLOCK_SIZE;
    for (size_t i = 1; i < blocks_in_buf; i++) {
        compress(counter, cur, blocks, 1);
        blocks[0] += BALLOON_BLOCK_SIZE;
        cur += BALLOON_BLOCK_SIZE;
    }
}

void* block_index(const struct hash_state* s, size_t i)
{
    return s->buffer + (BALLOON_BLOCK_SIZE * i);
}

void* block_last(const struct hash_state* s)
{
    return block_index(s, s->n_blocks - 1);
}

static void encrypt_partial(struct bitstream* b, void* outp, int to_encrypt)
{
    int encl;
    EVP_EncryptUpdate(b->ctx, (unsigned char*)outp, &encl, b->zeros, to_encrypt);
}

void bitstream_fill_buffer(struct bitstream* b, void* out, size_t outlen)
{
    size_t total = 0;
    while (total < outlen) {
        const int to_encrypt = MIN(outlen - total, BITSTREAM_BUF_SIZE);
        encrypt_partial(b, out + total, to_encrypt);
        total += to_encrypt;
    }
}

void hash_state_init(struct hash_state* s, const struct balloon_options* opts, const uint8_t salt[SALT_LEN])
{
    s->counter = 0;
    s->n_blocks = options_n_blocks(opts);
    if (s->n_blocks % 2 != 0)
        s->n_blocks++;
    s->has_mixed = false;
    s->opts = opts;
    s->buffer = (uint8_t*)malloc(s->n_blocks * BALLOON_BLOCK_SIZE);
    bitstream_init(&s->bstream);
    bitstream_seed_add(&s->bstream, salt, SALT_LEN);
    bitstream_seed_add(&s->bstream, &opts->s_cost, 8);
    bitstream_seed_add(&s->bstream, &opts->t_cost, 4);
    bitstream_seed_finalize(&s->bstream);
}

void hash_state_free(struct hash_state* s)
{
    bitstream_free(&s->bstream);
    free(s->buffer);
}

void hash_state_fill(struct hash_state* s, const uint8_t salt[SALT_LEN], const uint8_t* in, size_t inlen)
{
    SHA256_CTX c;
    SHA256_Init(&c);
    SHA256_Update(&c, &s->counter, 8);
    SHA256_Update(&c, salt, SALT_LEN);
    SHA256_Update(&c, in, inlen);
    SHA256_Update(&c, &s->opts->s_cost, 8);
    SHA256_Update(&c, &s->opts->t_cost, 4);
    SHA256_Final(s->buffer, &c);
    s->counter++;
    expand(&s->counter, s->buffer, s->n_blocks);
}

void hash_state_mix(struct hash_state* s, int32_t mixrounds)
{
    uint8_t buf[8];
    uint64_t neighbor;
    for (int32_t rounds = 0; rounds < mixrounds; rounds++) {
        for (size_t i = 0; i < s->n_blocks; i++) {
            uint8_t* cur_block = (uint8_t*)block_index(s, i);
            const size_t n_blocks_to_hash = 3;
            const uint8_t* blocks[2 + n_blocks_to_hash];
            const uint8_t* prev_block = i ? (const uint8_t*)cur_block - BALLOON_BLOCK_SIZE : (const uint8_t*)block_last(s);
            blocks[0] = prev_block;
            blocks[1] = cur_block;
            for (size_t n = 2; n < 2 + n_blocks_to_hash; n++) {
                bitstream_fill_buffer(&s->bstream, buf, 8);
                neighbor = bytes_to_littleend_uint64(buf, 8);
                blocks[n] = (const uint8_t*)block_index(s, neighbor % s->n_blocks);
            }
            compress(&s->counter, cur_block, blocks, 2 + n_blocks_to_hash);
        }
        s->has_mixed = true;
    }
}

void hash_state_extract(const struct hash_state* s, uint8_t out[BALLOON_BLOCK_SIZE])
{
    uint8_t* b = (uint8_t*)block_last(s);
    memcpy((char*)out, (const char*)b, BALLOON_BLOCK_SIZE);
}

void balloon(const unsigned char* input, unsigned char* output, int32_t len, int64_t s_cost, int32_t t_cost)
{
    struct hash_state s;
    struct balloon_options opts{s_cost, t_cost};
    hash_state_init(&s, &opts, input);
    hash_state_fill(&s, input, input, len);
    hash_state_mix(&s, t_cost);
    hash_state_extract(&s, output);
    hash_state_free(&s);
}

void balloon_hash(const unsigned char* input, unsigned char* output, int32_t len)
{
    balloon(input, output, len, 128, 4);
}
