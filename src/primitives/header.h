// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_HEADER_H
#define BITCOIN_PRIMITIVES_HEADER_H

#include <primitives/block.h>

/**
 * Custom serializer for CBlockHeader that omits the nNonce, for use as
 * as input to RandomX/ProgPOW.
 */
class CAuxHeader : public CBlockHeader
{
public:
    CAuxHeader(const CBlockHeader &header)
    {
        CBlockHeader::SetNull();
        *((CBlockHeader*)this) = header;
    }

    SERIALIZE_METHODS(CAuxHeader, obj)
    {
        READWRITE(obj.nVersion);
        READWRITE(obj.hashPrevBlock);
        READWRITE(obj.hashMerkleRoot);
        READWRITE(obj.nTime);
        READWRITE(obj.nBits);
    }

    uint256 BuildMoneroHeader() const;
};

/**
 * Custom serializer for CMoneroHeader, suitable for a RandomX hash.
 */
class CMoneroHeader : public CBlockHeader
{
private:
    uint256 headerHash{};
    const uint8_t spacer{0};

public:
    CMoneroHeader(const CAuxHeader &header)
    {
        CMoneroHeader::SetNull();
        headerHash = header.GetAuxHeaderHash();
        hashMerkleRoot = header.hashMerkleRoot;
        nNonce = header.nNonce;
    }

    SERIALIZE_METHODS(CMoneroHeader, obj)
    {
        READWRITE(obj.headerHash);
        for (unsigned int i=0; i<7; i++) {
            READWRITE(obj.spacer);
        }
        READWRITE(obj.nNonce);
        READWRITE(obj.spacer);
        READWRITE(obj.hashMerkleRoot);
    }
};


#endif // BITCOIN_PRIMITIVES_HEADER_H
