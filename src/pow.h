// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POW_H
#define BITCOIN_POW_H

#include <consensus/params.h>
#include <multialgo.h>

#include <stdint.h>

class CBlockHeader;
class CBlockIndex;
class uint256;

unsigned int GetNextWorkRequiredLegacy(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params&);
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params&);
unsigned int GetNextWorkRequiredPoS(const CBlockIndex* pindexLast, const Consensus::Params& params);
unsigned int GetNextWorkRequiredMultiAlgo(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params&, int algoNum = ALGO_SCRYPT, bool fPerAlgoLimitActive = false);
unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, bool fProofOfStake, const Consensus::Params& params, bool fPerAlgoLimitActive = false);
const CBlockIndex* GetLastPoSBlockIndex(const CBlockIndex* pindex);

/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits.
 *  fPerAlgoLimitActive must be pre-computed by the caller via a real chain-aware
 *  DeploymentActiveAfter(pindexPrev, chainman, Consensus::DEPLOYMENT_PERALGO_POWLIMIT)
 *  check -- this function deliberately cannot compute that itself (no chain context
 *  available this deep), so getting that bool right is entirely on the caller. */
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params&, int algo = ALGO_SCRYPT, bool fPerAlgoLimitActive = false);

/**
 * Return false if the proof-of-work requirement specified by new_nbits at a
 * given height is not possible, given the proof-of-work on the prior block as
 * specified by old_nbits.
 *
 * This function only checks that the new value is within a factor of 4 of the
 * old value for blocks at the difficulty adjustment interval, and otherwise
 * requires the values to be the same.
 *
 * Always returns true on networks where min difficulty blocks are allowed,
 * such as regtest/testnet.
 */
bool PermittedDifficultyTransition(const Consensus::Params& params, int64_t height, uint32_t old_nbits, uint32_t new_nbits);

#endif // BITCOIN_POW_H
