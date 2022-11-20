// Copyright (c) 2022 The Mateablecoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <multialgo.h>

#include <chain.h>
#include <util/system.h>

int defaultAlgo;

const int GetAlgoWorkFactor(int height, int algo, const Consensus::Params& params)
{
    if (height < params.nMultiAlgoStartBlock) {
        return 1;
    }

    switch (algo) {
    case ALGO_SCRYPT:
    case ALGO_YESCRYPT:
    case ALGO_WHIRLPOOL:
    case ALGO_GHOSTRIDER:
    case ALGO_BALLOON:
        return 1;
    default:
        return 1;
    }
}

const int GetVersionForAlgo(int algo)
{
    switch (algo) {
    case ALGO_SCRYPT:
        return BLOCK_VERSION_SCRYPT;
    case ALGO_YESCRYPT:
        return BLOCK_VERSION_YESCRYPT;
    case ALGO_WHIRLPOOL:
        return BLOCK_VERSION_WHIRLPOOL;
    case ALGO_GHOSTRIDER:
        return BLOCK_VERSION_GHOSTRIDER;
    case ALGO_BALLOON:
        return BLOCK_VERSION_BALLOON;
    }
    return 0;
}

const int GetAlgo(int nVersion)
{
    switch (nVersion & BLOCK_VERSION_ALGO) {
    case BLOCK_VERSION_SCRYPT:
        return ALGO_SCRYPT;
    case BLOCK_VERSION_YESCRYPT:
        return ALGO_YESCRYPT;
    case BLOCK_VERSION_WHIRLPOOL:
        return ALGO_WHIRLPOOL;
    case BLOCK_VERSION_GHOSTRIDER:
        return ALGO_GHOSTRIDER;
    case BLOCK_VERSION_BALLOON:
        return ALGO_BALLOON;
    }
    return ALGO_UNKNOWN;
}

std::string GetAlgoName(int algo)
{
    switch (algo) {
    case ALGO_SCRYPT:
        return std::string("scrypt");
    case ALGO_YESCRYPT:
        return std::string("yescrypt");
    case ALGO_WHIRLPOOL:
        return std::string("whirlpool");
    case ALGO_GHOSTRIDER:
        return std::string("ghostrider");
    case ALGO_BALLOON:
        return std::string("balloon");
    }
    return std::string("unknown");
}

const int MatchAlgoName(std::string userAlgo)
{
    for (unsigned int i = 0; i < NUM_ALGOS; i++) {
        if (userAlgo == GetAlgoName(i)) {
            return i;
        }
    }

    return ALGO_SCRYPT;
}

const int GetAlgoByIndex(const CBlockIndex* pindex)
{
    return GetAlgo(pindex->GetBlockHeader().nVersion);
}

std::string GetAlgoNameByIndex(const CBlockIndex* pindex)
{
    return GetAlgoName(GetAlgo(pindex->GetBlockHeader().nVersion));
}

bool IsAlgoActive(const CBlockIndex* pindexPrev, int algo, const Consensus::Params& params)
{
    if (!pindexPrev) {
        return algo == ALGO_SCRYPT;
    }

    const int nHeight = pindexPrev->nHeight + 1;
    if (nHeight < params.nMultiAlgoStartBlock) {
        return algo == ALGO_SCRYPT;
    }

    return true;
}

unsigned int InitialDifficulty(int algo, const Consensus::Params& params)
{
    return UintToArith256(params.powLimit).GetCompact();
}

const CBlockIndex* GetLastBlockIndexForAlgo(const CBlockIndex* pindex, int algo, const Consensus::Params& params)
{
    for (; pindex; pindex = pindex->pprev) {
        if (GetAlgoByIndex(pindex) != algo)
            continue;
        // ignore special min-difficulty testnet blocks
        if (params.fPowAllowMinDifficultyBlocks && pindex->pprev && pindex->nTime > pindex->pprev->nTime + params.nPowTargetSpacing * 2) {
            continue;
        }
        return pindex;
    }
    return nullptr;
}
