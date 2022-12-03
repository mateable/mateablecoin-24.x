// Copyright (c) 2022 The Mateablecoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIALGO_H
#define MULTIALGO_H

#include <chainparams.h>
#include <consensus/params.h>
#include <util/system.h>

class CBlockIndex;

extern int defaultAlgo;

enum {
    ALGO_UNKNOWN = -1,
    ALGO_SCRYPT = 0,
    ALGO_YESCRYPT = 1,
    ALGO_WHIRLPOOL = 2,
    ALGO_GHOSTRIDER = 3,
    ALGO_BALLOON = 4,
    NUM_ALGOS
};

enum {
    BLOCK_VERSION_DEFAULT = 2,
    BLOCK_VERSION_ALGO = (15 << 8),
    BLOCK_VERSION_SCRYPT = (0 << 8),
    BLOCK_VERSION_YESCRYPT = (2 << 8),
    BLOCK_VERSION_WHIRLPOOL = (4 << 8),
    BLOCK_VERSION_GHOSTRIDER = (6 << 8),
    BLOCK_VERSION_BALLOON = (8 << 8),
};

const int GetAlgoWorkFactor(int height, int algo, const Consensus::Params& params);
const int GetVersionForAlgo(int algo);
const int GetAlgo(int nVersion);
const int GetAlgoByIndex(const CBlockIndex* pindex);
const int MatchAlgoName(std::string userAlgo);
std::string GetAlgoName(int algo);
std::string GetAlgoNameByIndex(const CBlockIndex* pindex);
bool IsAlgoActive(const CBlockIndex* pindexPrev, int algo, const Consensus::Params& params);
unsigned int InitialDifficulty(int algo, const Consensus::Params& params);
const CBlockIndex* GetLastBlockIndexForAlgo(const CBlockIndex* pindex, int algo, const Consensus::Params& params);

#endif // MULTIALGO_H
