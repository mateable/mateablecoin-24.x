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
class CAuxHeader : private CBlockHeader
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
};

#endif // BITCOIN_PRIMITIVES_HEADER_H
