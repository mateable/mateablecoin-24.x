#ifndef BITCOIN_QT_STAKINGINFO_H
#define BITCOIN_QT_STAKINGINFO_H

#include <consensus/amount.h>
#include <cstdint>

class QString;

namespace wallet {
class CWallet;
} // namespace wallet

/**
 * Computes a wallet's current stake-eligible balance and a rough estimate of
 * expected time (in seconds) until it finds its next stake, using the exact
 * same target arithmetic as the consensus kernel check (CheckStakeKernelHash).
 * Shared by the staking status bar tooltip and the Overview page so both stay
 * in sync instead of duplicating this math.
 *
 * Returns false if there is no stake-eligible balance (eligible/etaSeconds
 * are still set to 0 in that case, not left uninitialized).
 */
bool GetStakingEligibleBalanceAndETA(wallet::CWallet* pwallet, CAmount& eligible, int64_t& etaSeconds);

/** Formats a staking ETA (seconds) into a short human-readable string, e.g. "~2h 14m". */
QString FormatStakingETA(int64_t nSeconds);

#endif // BITCOIN_QT_STAKINGINFO_H
