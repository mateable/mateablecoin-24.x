#ifndef CRYPTO_ALGOSANITY_H
#define CRYPTO_ALGOSANITY_H

#include <hash.h>
#include <multialgo.h>
#include <primitives/block.h>
#include <shutdown.h>

CBlockHeader return_test_block();
bool test_algorithm_sanity();

#endif // CRYPTO_ALGOSANITY_H
