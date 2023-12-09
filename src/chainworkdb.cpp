// Copyright (c) 2023 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainworkdb.h>

ChainworkDB::ChainworkDB() : initialized(false) {}

void ChainworkDB::Initialize(const fs::path& ldb_path, size_t nCacheSize, bool fMemory, bool fWipe)
{
    if (!initialized) {
        this->ldb_path = ldb_path;
        this->nCacheSize = nCacheSize;
        this->fMemory = fMemory;
        this->fWipe = fWipe;
        db = std::make_unique<CDBWrapper>(ldb_path, nCacheSize, fMemory, fWipe, true);
        initialized = true;
    }
}

ChainworkDB chainwork_db;

bool ChainworkDB::HaveEntry(const int& height) const {
    if (!initialized) throw std::runtime_error("ChainworkDB not initialized");
    return db->Exists(height);
}

void ChainworkDB::GetEntry(const int& height, arith_uint256& chainWork) const {
    if (!initialized) throw std::runtime_error("ChainworkDB not initialized");
    uint256 temp{};
    db->Read(height, temp);
    chainWork = UintToArith256(temp);
}

void ChainworkDB::WriteEntry(const int& height, arith_uint256& chainWork) {
    if (!initialized) throw std::runtime_error("ChainworkDB not initialized");
    uint256 temp = ArithToUint256(chainWork);
    db->Write(height, temp);
}
