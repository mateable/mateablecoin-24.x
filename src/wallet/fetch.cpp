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

bool fetch_price_data(std::string& outstr)
{
    try
    {
        const char* host = "50.116.31.124";
        const char* port = "80";

        boost::asio::io_context ioc;
        boost::beast::net::ip::tcp::resolver resolver(ioc);
        boost::beast::tcp_stream stream(ioc);

        auto const results = resolver.resolve(host, port);
        
        // Set a timeout for the connection
        stream.expires_after(std::chrono::seconds(5));
        stream.connect(results);

        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/price.json", 11};
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, "mateable/1.0");

        // Set a timeout for the write and read operations
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

bool parse_price_data(std::vector<exchangeEntry>& exchangeList)
{
    std::string outstr;
    if (!fetch_price_data(outstr)) {
        return false;
    }

    UniValue rv;
    if (!rv.read(outstr) || !rv.isArray()) {
        return false;
    }

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
    return true;
}

void return_random_exchange(std::string& exchangeData)
{
    LOCK(cs_fetch);
    exchangeData = cached_exchangeData;
}

bool fetch_wallet_announcement(std::string& outstr)
{
    try
    {
        const char* host = "50.116.31.124";
        const char* port = "80";

        boost::asio::io_context ioc;
        boost::beast::net::ip::tcp::resolver resolver(ioc);
        boost::beast::tcp_stream stream(ioc);

        auto const results = resolver.resolve(host, port);
        stream.expires_after(std::chrono::seconds(5));
        stream.connect(results);

        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/announcement.json", 11};
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
    catch (...)
    {
        return false;
    }
    return true;
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
    while (!ShutdownRequested()) {
        std::vector<exchangeEntry> exchangeList;
        if (parse_price_data(exchangeList)) {
            int totalExchanges = (int)exchangeList.size();
            if (totalExchanges > 0) {
                srand(time(NULL));
                int randExchange = rand() % totalExchanges;
                LOCK(cs_fetch);
                cached_exchangeData = exchangeList[randExchange].first + "    " + exchangeList[randExchange].second;
            }
        }

        std::string outstr_ann;
        if (fetch_wallet_announcement(outstr_ann)) {
            UniValue rv;
            if (rv.read(outstr_ann) && rv.isObject()) {
                LOCK(cs_fetch);
                cached_announcement = rv["message"].getValStr();
            }
        }

        for (int i = 0; i < 600 && !ShutdownRequested(); i++) {
            UninterruptibleSleep(std::chrono::milliseconds{500});
        }
    }
}

void StartWalletFetchThread()
{
    static std::thread fetch_thread;
    if (!fetch_thread.joinable()) {
        fetch_thread = std::thread(ThreadWalletFetch);
        fetch_thread.detach();
    }
}
