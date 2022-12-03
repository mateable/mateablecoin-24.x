#include <string.h>
#include <crypto/whirlpool/sph_whirlpool.h>

void whirlpool_hash(const char* input, char* output, uint32_t len)
{
    unsigned char hash[64];
    memset(hash, 0, sizeof(hash));

    sph_whirlpool1_context ctx_whirlpool;

    sph_whirlpool1_init(&ctx_whirlpool);
    sph_whirlpool1(&ctx_whirlpool, input, len);
    sph_whirlpool1_close(&ctx_whirlpool, (void*)hash);

    sph_whirlpool1_init(&ctx_whirlpool);
    sph_whirlpool1(&ctx_whirlpool, (const void*)hash, 64);
    sph_whirlpool1_close(&ctx_whirlpool, (void*)hash);

    sph_whirlpool1_init(&ctx_whirlpool);
    sph_whirlpool1(&ctx_whirlpool, (const void*)hash, 64);
    sph_whirlpool1_close(&ctx_whirlpool, (void*)hash);

    sph_whirlpool1_init(&ctx_whirlpool);
    sph_whirlpool1(&ctx_whirlpool, (const void*)hash, 64);
    sph_whirlpool1_close(&ctx_whirlpool, (void*)hash);

    memcpy(output, hash, 32);
}
