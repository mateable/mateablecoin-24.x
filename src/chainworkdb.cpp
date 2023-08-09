// Copyright (c) 2023 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainworkdb.h>

ChainworkDB chainwork_db(GetDefaultDataDir() / "chainworkdb", 4194304, false, false);

ChainworkDB::ChainworkDB(fs::path ldb_path, size_t nCacheSize, bool fMemory, bool fWipe) :
    db(ldb_path, nCacheSize, fMemory, fWipe, true)
{}

bool ChainworkDB::HaveEntry(const int& height) const {
    return db.Exists(height);
}

void ChainworkDB::GetEntry(const int& height, arith_uint256& chainWork) const {
    uint256 temp{};
    db.Read(height, temp);
    chainWork = UintToArith256(temp);
}

void ChainworkDB::WriteEntry(const int& height, arith_uint256& chainWork) {
    uint256 temp{};
    temp = ArithToUint256(chainWork);
    db.Write(height, temp);
}
