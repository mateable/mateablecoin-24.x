// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/header.h>

#include <hash.h>
#include <multialgo.h>
#include <tinyformat.h>

uint256 CBlockHeader::GetAuxHeaderHash() const
{
    CAuxHeader input{*this};
    return SerializeHash(input);
}

uint256 CAuxHeader::BuildMoneroHeader() const
{
    CMoneroHeader input{*this};
    return SerializeHash(input);
}
    
