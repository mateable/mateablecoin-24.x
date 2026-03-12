// Copyright (c) 2011-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/transactionrecord.h>

#include <interfaces/wallet.h>
#include <key_io.h>
#include <validation.h>
#include <wallet/ismine.h>

#include <stdint.h>
#include <variant>

#include <QDateTime>

using wallet::isminetype;

/* Return positive answer if transaction should be shown in list.
 */
bool TransactionRecord::showTransaction()
{
    // There are currently no cases where we hide transactions, but
    // we may want to use this in the future for things like RBF.
    return true;
}

/*
 * Decompose CWallet transaction to model transaction records.
 */
QList<TransactionRecord> TransactionRecord::decomposeTransaction(const interfaces::WalletTx& wtx)
{
    QList<TransactionRecord> parts;
    int64_t nTime = wtx.time;
    CAmount nCredit = wtx.credit;
    CAmount nDebit = wtx.debit;
    CAmount nNet = nCredit - nDebit;
    uint256 hash = wtx.tx->GetHash();
    std::map<std::string, std::string> mapValue = wtx.value_map;

    if (wtx.is_coinstake)
    {
        TransactionRecord sub(hash, nTime);
        sub.type = Staked;
        CAmount nTrueCredit = wtx.tx->GetValueOut();
        CAmount nReward = nTrueCredit - nDebit;
        sub.credit = nReward;
        sub.debit = 0;

        for (const isminetype mine : wtx.txin_is_mine) {
            if (mine & wallet::ISMINE_WATCH_ONLY) {
                sub.involvesWatchAddress = true;
                break;
            }
        }
        if (!sub.involvesWatchAddress) {
            for (const isminetype mine : wtx.txout_is_mine) {
                if (mine & wallet::ISMINE_WATCH_ONLY) {
                    sub.involvesWatchAddress = true;
                    break;
                }
            }
        }

        CTxDestination address;
        if (!ExtractDestination(wtx.tx->vout[1].scriptPubKey, address))
        {
            // This is an error, should not happen
        }
        sub.address = EncodeDestination(address);

        parts.append(sub);
    }
    else if (nNet > 0 || wtx.is_coinbase)
    {
        //
        // Credit
        //
        for(unsigned int i = 0; i < wtx.tx->vout.size(); i++)
        {
            const CTxOut& txout = wtx.tx->vout[i];
            isminetype mine = wtx.txout_is_mine[i];
            if(mine)
            {
                TransactionRecord sub(hash, nTime);
                sub.idx = i;
                sub.credit = txout.nValue;
                sub.involvesWatchAddress = mine & wallet::ISMINE_WATCH_ONLY;
                if (wtx.txout_address_is_mine[i])
                {
                    // Received by Bitcoin Address
                    sub.type = RecvWithAddress;
                    sub.address = EncodeDestination(wtx.txout_address[i]);
                }
                else
                {
                    // Received by IP connection (deprecated features), or a multisignature or other script
                    sub.type = RecvFromOther;
                    sub.address = mapValue["from"];
                }
                if (wtx.is_coinbase)
                {
                    // Generated
                    sub.type = Generated;
                }

                parts.append(sub);
            }
        }
    }
    else
    {
        bool involvesWatchAddress = false;
        isminetype fAllFromMe = wallet::ISMINE_SPENDABLE;
        for (const isminetype mine : wtx.txin_is_mine) {
            if(mine & wallet::ISMINE_WATCH_ONLY) involvesWatchAddress = true;
            if(fAllFromMe > mine) fAllFromMe = mine;
        }

        isminetype fAllToMe = wallet::ISMINE_SPENDABLE;
        for (const isminetype mine : wtx.txout_is_mine) {
            if(mine & wallet::ISMINE_WATCH_ONLY) involvesWatchAddress = true;
            if(fAllToMe > mine) fAllToMe = mine;
        }

        if (fAllFromMe && fAllToMe)
        {
            // Payment to self
            CAmount nChange = wtx.change;

            parts.append(TransactionRecord(hash, nTime, SendToSelf, "",
                                            -(nDebit - nChange), nCredit - nChange));
            parts.last().involvesWatchAddress = involvesWatchAddress; // maybe pass to constructor, this is ok for now
        }
        else if (fAllFromMe)
        {
            //
            // Debit
            //
            CAmount nTxFee = nDebit - wtx.tx->GetValueOut();

            for (unsigned int nOut = 0; nOut < wtx.tx->vout.size(); nOut++)
            {
                const CTxOut& txout = wtx.tx->vout[nOut];
                TransactionRecord sub(hash, nTime);
                sub.idx = nOut;
                sub.involvesWatchAddress = involvesWatchAddress;

                if(wtx.txout_is_mine[nOut])
                {
                    // Ignore change
                    continue;
                }

                if (!std::holds_alternative<CNoDestination>(wtx.txout_address[nOut]))
                {
                    // Sent to Bitcoin Address
                    sub.type = SendToAddress;
                    sub.address = EncodeDestination(wtx.txout_address[nOut]);
                }
                else
                {
                    // Sent to IP, or other non-address transaction like OP_RETURN
                    sub.type = SendToOther;
                    sub.address = mapValue["to"];
                }

                CAmount nValue = txout.nValue;
                /* Add fee to first output */
                if (nTxFee > 0)
                {
                    nValue += nTxFee;
                    nTxFee = 0;
                }
                sub.debit = -nValue;

                parts.append(sub);
            }
        }
        else
        {
            //
            // Mixed debit transaction
            //
            for (const isminetype mine : wtx.txin_is_mine) {
                if(mine) {
                    parts.append(TransactionRecord(hash, nTime, RecvFromOther, "", 0, wtx.credit));
                    parts.last().involvesWatchAddress = (mine & wallet::ISMINE_WATCH_ONLY);
                }
            }
            for (const isminetype mine : wtx.txout_is_mine) {
                if(mine) {
                    parts.append(TransactionRecord(hash, nTime, SendToOther, "", wtx.debit, 0));
                    parts.last().involvesWatchAddress = (mine & wallet::ISMINE_WATCH_ONLY);
                }
            }
        }
    }

    return parts;
}

void TransactionRecord::updateStatus(const interfaces::WalletTxStatus& wtx, const uint256& block_hash, int numBlocks, int64_t block_time)
{
    status.cur_block_hash = block_hash;
    status.needsUpdate = false;

    status.depth = wtx.depth_in_main_chain;

    if (wtx.is_coinstake || wtx.is_coinbase) {
        status.required_confirmations = COINBASE_MATURITY + 1;
    } else {
        status.required_confirmations = RecommendedNumConfirmations;
    }

    // Determine transaction status
    bool is_final = true;
    if (wtx.lock_time > 0) {
        // As per CTransaction::IsFinalTx behavior, we need to check lock_time against
        // the current block height or block time.
        if (wtx.lock_time < LOCKTIME_THRESHOLD && (uint32_t)numBlocks < wtx.lock_time) {
            is_final = false;
        } else if (wtx.lock_time >= LOCKTIME_THRESHOLD && block_time < wtx.lock_time) {
            is_final = false;
        }
    }

    if (!is_final) {
        if (wtx.lock_time < LOCKTIME_THRESHOLD) {
            status.status = TransactionStatus::Unconfirmed;
            status.matures_in = 0;
        } else {
            status.status = TransactionStatus::NotAccepted;
        }
    } else {
        if ((wtx.is_coinbase || wtx.is_coinstake) && wtx.blocks_to_maturity > 0)
        {
            status.status = TransactionStatus::Immature;
            if (wtx.is_in_main_chain)
            {
                status.matures_in = wtx.blocks_to_maturity;
            }
            else
            {
                status.status = TransactionStatus::NotAccepted;
            }
        }
        else if (wtx.depth_in_main_chain < 0)
        {
            status.status = TransactionStatus::Conflicted;
        }
        else if (wtx.depth_in_main_chain == 0)
        {
            status.status = TransactionStatus::Unconfirmed;
            if (wtx.is_abandoned)
                status.status = TransactionStatus::Abandoned;
        }
        else if (wtx.depth_in_main_chain < status.required_confirmations)
        {
            status.status = TransactionStatus::Confirming;
        }
        else
        {
            status.status = TransactionStatus::Confirmed;
        }
    }

    status.countsForBalance = status.status == TransactionStatus::Confirmed ||
                              status.status == TransactionStatus::Confirming ||
                              status.status == TransactionStatus::Unconfirmed;

    status.cur_block_hash = block_hash;
}

bool TransactionRecord::isAbandoned() const
{
    return status.status == TransactionStatus::Abandoned;
}


bool TransactionRecord::isConfirmed() const
{
    return status.status == TransactionStatus::Confirmed;
}

bool TransactionRecord::isConflicted() const
{
    return status.status == TransactionStatus::Conflicted;
}

bool TransactionRecord::isImmature() const
{
    return status.status == TransactionStatus::Immature;
}

bool TransactionRecord::isCoinBase() const
{
    return type == Generated;
}

bool TransactionRecord::isCoinStake() const
{
    return type == Staked;
}

bool TransactionRecord::isInMempool() const
{
    return status.depth == 0 && !isAbandoned() && !isConflicted();
}

bool TransactionRecord::isInMainChain() const
{
    return status.depth > 0;
}


QString TransactionRecord::getTxHash() const
{
    return QString::fromStdString(hash.GetHex());
}

int TransactionRecord::getOutputIndex() const
{
    return idx;
}
