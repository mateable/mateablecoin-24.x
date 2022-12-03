#include <crypto/algo_sanity.h>

uint256 algoResult[5] = { uint256S("1bf52d13e653de7c17d0e0d69e5315b364dea0aa35fd5c5d10f5c23e2ba54c08"),
    uint256S("adf77e0696d36c6729af1c68c737b700757307fcbec8c30fe5560920efb69522"),
    uint256S("8ad3b7e75a7319ef592856e62195b0a06be36ab3030fb1721f4f988ef9abb9c4"),
    uint256S("c0a243a56d03391318a67b38428379c67c2fe547d70766f7050ddd54c9e578c3"),
    uint256S("9192d983c2e2c78e543c20daa1f4869b838457fe00fe3f788b357091b17ab94b") };

CBlockHeader return_test_block()
{
    CBlockHeader block;
    block.nVersion = 0x20000000;
    block.hashPrevBlock = uint256();
    block.hashMerkleRoot = uint256();
    block.nTime = 0x12341234;
    block.nBits = 0x23452345;
    block.nNonce = 0x34563456;
    return block;
}

bool test_algorithm_sanity()
{
    int algo = -1;
    while (++algo < NUM_ALGOS) {
        CBlockHeader test_block = return_test_block();
        test_block.SetAlgo(algo);
        uint256 powHash {};
        for (unsigned int i = 0; i < 3; i++) {
            ++test_block.nNonce;
            powHash = test_block.GetPoWHash();
        }
        if (powHash != algoResult[algo]) {
            LogPrintf("Algorithm %s provided incorrect result\n");
            StartShutdown();
            return false;
        }
    }
    LogPrintf("Algorithm sanity check passed\n");
    return true;
}
