#ifndef CRYPTO_GHOSTRIDER_GHOSTRIDER_H
#define CRYPTO_GHOSTRIDER_GHOSTRIDER_H

#include <crypto/ghostrider/hash_selection.h>

template <typename T1>
uint256 ghostrider_hash(const T1 pbegin, const T1 pend, const uint256 PrevBlockHash)
{
    static unsigned char pblank[1];

    uint512 hash[18];
    HashSelection hashSelection(PrevBlockHash, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}, {0, 1, 2, 3, 4, 5});
    std::vector<int> randomCNs(hashSelection.getCnIndexes());
    std::vector<int> coreHashIndexes(hashSelection.getAlgoIndexes());
    for (int i = 0; i < 18; ++i) {
        const void* toHash;
        int lenToHash;
        if (i == 0) {
            toHash = (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0]));
            lenToHash = (pend - pbegin) * sizeof(pbegin[0]);
        } else { 
            toHash = static_cast<const void*>(&hash[i - 1]);
            lenToHash = 64;
        }
        int coreSelection = -1;
        int cnSelection   = -1;
        if (i < 5) {
            coreSelection = coreHashIndexes[i];
        } else if (i == 5) {
            cnSelection = randomCNs[0];
        } else if (i < 11) {
            coreSelection = coreHashIndexes[i - 1];
        } else if (i == 11) {
            cnSelection = randomCNs[1];
        } else if (i < 17) {
            coreSelection = coreHashIndexes[i - 2];
        } else if (i == 17) {
            cnSelection = randomCNs[2];
        }

        coreHash(toHash, &hash[i], lenToHash, coreSelection);
        cnHash(&hash[i - 1], &hash[i], lenToHash, cnSelection);
    }
    return hash[17].trim256();
}   

#endif // CRYPTO_GHOSTRIDER_GHOSTRIDER_H
