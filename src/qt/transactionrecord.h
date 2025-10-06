// Copyright (c) 2011-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_TRANSACTIONRECORD_H
#define BITCOIN_QT_TRANSACTIONRECORD_H

#include <interfaces/wallet.h>
#include <uint256.h>

#include <QList>
#include <QString>

class CWallet;

namespace interfaces {
class Node;
class Wallet;
struct WalletTx;
struct WalletTxStatus;
}

/** UI model for transaction status. The transaction status is the part of a transaction that may change over time.
 */
struct TransactionStatus
{
    TransactionStatus():
        status(Unconfirmed),
        depth(0),
        matures_in(0),
        needsUpdate(true)
    { }

    //! Transaction status in enum
    enum Status {
        Unconfirmed,
        Abandoned,
        Confirming,
        Confirmed,
        ConfirmedImmature,
        Conflicted,
        Immature,
        NotAccepted
    };

    Status status;
    //! Depth of transaction in blockchain
    int depth;
    //! For coinbase transactions, number of blocks left before maturity
    int matures_in;

    //! Number of confirmations required for transaction to be considered mature
    int required_confirmations;

    //! Whether this transaction counts for balance
    bool countsForBalance;

    //! Last block hash this status was updated against
    uint256 cur_block_hash;

    bool needsUpdate;

    /**
     * Determine whether this transaction status is up-to-date.
     *
     * @param[in] cur_block_hash The hash of the best block in the chain.
     */
    bool statusUpdateNeeded(const uint256& cur_block_hash) const
    {
        return needsUpdate || this->cur_block_hash != cur_block_hash;
    }
};

/** UI model for a transaction. A core transaction can be represented by multiple UI transactions (one for each payment within the transaction).
 */
class TransactionRecord
{
public:
    enum Type
    {
        Other,
        Generated,
        Staked,
        SendToAddress,
        SendToOther,
        RecvWithAddress,
        RecvFromOther,
        SendToSelf
    };

    /** Number of confirmation recommended for accepting a transaction */
    static const int RecommendedNumConfirmations = 6;

    TransactionRecord():
            hash(), time(0), type(Other), address(""), debit(0), credit(0), idx(0)
    {
    }

    TransactionRecord(uint256 _hash, qint64 _time):
            hash(_hash), time(_time), type(Other), address(""), debit(0),
            credit(0), idx(0)
    {
    }

    TransactionRecord(uint256 _hash, qint64 _time,
                    Type _type, const std::string& _address,
                    const CAmount& _debit, const CAmount& _credit):
            hash(_hash), time(_time), type(_type), address(_address), debit(_debit),
            credit(_credit), idx(0)
    {
    }

    /** Decompose CWallet transaction to model transaction records.
     */
    static QList<TransactionRecord> decomposeTransaction(const interfaces::WalletTx& wtx);

    /** @name Immutable transaction data
        @*/
    uint256 hash;
    qint64 time;
    Type type;
    std::string address;
    CAmount debit;
    CAmount credit;

    /** Subtransaction index, for sort key */
    int idx;

    /** Status: can change with block chain update */
    TransactionStatus status;

    /** Whether the transaction was sent/received to/from a watch-only address. */
    bool involvesWatchAddress;

    /** Return the unique identifier for this transaction record.
     */
    QString getTxHash() const;

    /** Return the output index of the subtransaction.
     */
    int getOutputIndex() const;

    /** Update status from core wallet transaction.
     */
    void updateStatus(const interfaces::WalletTxStatus& wtx, const uint256& block_hash, int numBlocks, int64_t block_time);

    /** Return whether a transaction is abandoned.
     */
    bool isAbandoned() const;

    /** Return whether a transaction is confirmed.
     */
    bool isConfirmed() const;

    /** Return whether we consider transaction status safe.
     */
    bool isStatusOk() const;

    /** Return whether this transaction is part of a conflicted block.
     */
    bool isConflicted() const;

    /** Return whether this transaction is immature.
     */
    bool isImmature() const;

    /** Return whether transaction is a coinbase transaction.
     */
    bool isCoinBase() const;

    /** Return whether transaction is a coinstake transaction.
     */
    bool isCoinStake() const;

    /** Send computed status to UI.
     */
    void showStatus(bool showTransaction, bool mature) const;

    /** Return true if transaction can be abandoned.
     */
    bool canAbandon() const;

    /** Update transaction status to abandoned.
     */
    bool abandon() const;

    /** Return whether transaction has status in memory pool.
     */
    bool isInMempool() const;

    /** Return whether transaction has status in chain.
     */
    bool isInMainChain() const;

    /** Only show transactions that are not coinbase or coinstake.
     */
    static bool showTransaction();
};

#endif // BITCOIN_QT_TRANSACTIONRECORD_H