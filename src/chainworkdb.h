// Copyright (c) 2023 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CHAINWORKDB_H
#define CHAINWORKDB_H

#include <arith_uint256.h>
#include <dbwrapper.h>
#include <uint256.h>
#include <validation.h>

class ChainworkDB
{
protected:
    CDBWrapper db;

public:
    explicit ChainworkDB(fs::path ldb_path, size_t nCacheSize, bool fMemory, bool fWipe);

    bool HaveEntry(const int& height) const;
    void GetEntry(const int& height, arith_uint256& chainWork) const;
    void WriteEntry(const int& height, arith_uint256& chainWork);
};

extern ChainworkDB chainwork_db;

#endif // CHAINWORKDB_H
