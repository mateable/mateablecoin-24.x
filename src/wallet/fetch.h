// Copyright (c) 2024-2025 The Mateable Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FETCH_H
#define FETCH_H

#include <string>

void return_random_exchange(std::string& exchangeData);
bool parse_wallet_announcement(std::string& announcementText);
void StartWalletFetchThread();
#endif // FETCH_H
