#ifndef CRYPTO_WHIRLPOOL_WHIRLPOOL_H
#define CRYPTO_WHIRLPOOL_WHIRLPOOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void whirlpool_hash(const char* input, char* output, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // CRYPTO_WHIRLPOOL_WHIRLPOOL_H
