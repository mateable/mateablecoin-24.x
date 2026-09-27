#include <qt/stakinginfo.h>

#include <arith_uint256.h>
#include <chainparams.h>
#include <node/context.h>
#include <pos/minter.h>
#include <pow.h>
#include <sync.h>
#include <validation.h>
#include <wallet/receive.h>
#include <wallet/wallet.h>

#include <QObject>
#include <QString>

#include <cmath>
#include <set>
#include <utility>

bool GetStakingEligibleBalanceAndETA(wallet::CWallet* pwallet, CAmount& eligible, int64_t& etaSeconds)
{
    eligible = 0;
    etaSeconds = 0;
    if (!pwallet) return false;

    CAmount nBalance = wallet::GetSpendableBalance(*pwallet);
    std::set<std::pair<const wallet::CWalletTx*, unsigned int>> setCoins;
    SelectCoinsForStaking(pwallet, nBalance - pwallet->nReserveBalance, setCoins, eligible);
    if (eligible <= 0) return false;

    node::NodeContext* node_ctx = pwallet->chain().context();
    if (!node_ctx || !node_ctx->chainman) return false;

    unsigned int nBitsPoS;
    {
        LOCK(cs_main);
        const CBlockIndex* pindex = node_ctx->chainman->ActiveChain().Tip();
        nBitsPoS = GetNextWorkRequiredPoS(pindex, node_ctx->chainman->GetConsensus());
    }

    arith_uint256 bnTarget;
    bnTarget.SetCompact(nBitsPoS);
    arith_uint256 bnWeight(eligible);
    arith_uint256 bnWeightedTarget = bnTarget * bnWeight;
    double dProbability = bnWeightedTarget.getdouble() / std::pow(2.0, 256);
    if (dProbability <= 0) return false;

    double dAttemptIntervalSeconds = (double)(Params().GetStakeTimestampMask() + 1);
    etaSeconds = (int64_t)(dAttemptIntervalSeconds / dProbability);
    return true;
}

QString FormatStakingETA(int64_t nSeconds)
{
    if (nSeconds <= 0) return QObject::tr("calculating…");
    int64_t days = nSeconds / 86400;
    int64_t hours = (nSeconds % 86400) / 3600;
    int64_t minutes = (nSeconds % 3600) / 60;
    if (days > 0) return QObject::tr("~%1d %2h").arg(days).arg(hours);
    if (hours > 0) return QObject::tr("~%1h %2m").arg(hours).arg(minutes);
    if (minutes > 0) return QObject::tr("~%1m").arg(minutes);
    return QObject::tr("< 1m");
}
