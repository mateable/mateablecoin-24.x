// Copyright (c) 2024-2025 The Mateable Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <wallet/fetch.h>
#include <stdlib.h>
#include <time.h>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <ostream>
#include <sstream>
#include <sync.h>
#include <util/time.h>
#include <thread>
#include <shutdown.h>
#include <univalue.h>

typedef std::pair<std::string, std::string> exchangeEntry;

/** Dummy ShutdownRequested for wallet tool */
bool __attribute__((weak)) ShutdownRequested() { return false; }

static GlobalMutex cs_fetch;
static std::string cached_exchangeData;
static std::string cached_announcement;
static std::thread fetch_thread;
static std::atomic<bool> fStopFetch(false);

bool perform_http_get(boost::asio::io_context& ioc, const std::string& path, std::string& outstr)
{
    try
    {
        const char* host = "50.116.31.124";
        const char* port = "80";

        boost::beast::net::ip::tcp::resolver resolver(ioc);
        boost::beast::tcp_stream stream(ioc);

        auto const results = resolver.resolve(host, port);
        
        stream.expires_after(std::chrono::seconds(5));
        stream.connect(results);

        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, path, 11};
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, "mateable/1.0");

        stream.expires_after(std::chrono::seconds(5));
        boost::beast::http::write(stream, req);
        
        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::string_body> res;
        boost::beast::http::read(stream, buffer, res);

        outstr = res.body();

        boost::beast::error_code ec;
        stream.socket().shutdown(boost::beast::net::ip::tcp::socket::shutdown_both, ec);
    } 
    catch(...)
    {
        return false;
    }

    return true;
}

bool fetch_price_data(boost::asio::io_context& ioc, std::string& outstr)
{
    return perform_http_get(ioc, "/price.json", outstr);
}

bool fetch_wallet_announcement(boost::asio::io_context& ioc, std::string& outstr)
{
    return perform_http_get(ioc, "/announcement.json", outstr);
}

bool parse_price_data(boost::asio::io_context& ioc, std::vector<exchangeEntry>& exchangeList)
{
    std::string outstr;
    if (!fetch_price_data(ioc, outstr)) {
        return false;
    }

    UniValue rv;
    if (!rv.read(outstr)) {
        return false;
    }

    if (rv.isArray()) {
        for (unsigned int i = 0; i < rv.size(); i++) {
            const UniValue& val = rv[i];
            if (val.isObject()) {
                exchangeEntry entry;
                entry.first = val["exchange"].getValStr();
                entry.second = val["price"].getValStr();
                if (!entry.first.empty() && !entry.second.empty()) {
                    exchangeList.push_back(entry);
                }
            }
        }
    } else if (rv.isObject()) {
        if (rv.exists("usd_value")) {
            exchangeEntry entry;
            entry.first = "Market";
            entry.second = "$" + rv["usd_value"].getValStr();
            exchangeList.push_back(entry);
        }
    }
    return true;
}

void return_random_exchange(std::string& exchangeData)
{
    LOCK(cs_fetch);
    exchangeData = cached_exchangeData;
}

bool parse_wallet_announcement(std::string& announcementText)
{
    LOCK(cs_fetch);
    if (cached_announcement.empty()) return false;
    announcementText = cached_announcement;
    return true;
}

void ThreadWalletFetch()
{
    boost::asio::io_context ioc;
    while (!ShutdownRequested() && !fStopFetch) {
        std::vector<exchangeEntry> exchangeList;
        if (parse_price_data(ioc, exchangeList)) {
            int totalExchanges = (int)exchangeList.size();
            if (totalExchanges > 0) {
                srand(time(NULL));
                int randExchange = rand() % totalExchanges;
                LOCK(cs_fetch);
                cached_exchangeData = exchangeList[randExchange].first + "    " + exchangeList[randExchange].second;
            }
        }

        std::string outstr_ann;
        if (fetch_wallet_announcement(ioc, outstr_ann)) {
            UniValue rv;
            if (rv.read(outstr_ann) && rv.isObject()) {
                LOCK(cs_fetch);
                cached_announcement = rv["message"].getValStr();
            }
        }

        for (int i = 0; i < 600 && !ShutdownRequested() && !fStopFetch; i++) {
            UninterruptibleSleep(std::chrono::milliseconds{500});
        }
    }
}


void StartWalletFetchThread()
{
    if (fetch_thread.joinable()) return;
    fStopFetch = false;
    fetch_thread = std::thread(ThreadWalletFetch);
}

void StopWalletFetchThread()
{
    fStopFetch = true;
    if (fetch_thread.joinable()) {
        fetch_thread.join();
    }
}

