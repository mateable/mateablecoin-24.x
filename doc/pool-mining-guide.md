# Mateablecoin Pool Mining Guide

This guide is for pool operators who want to mine Mateablecoin (MTBC). Mateablecoin
is a multi-algorithm coin with 5 PoW algorithms and Proof of Stake. A single node
and wallet can be used to mine all algorithms.

## Table of Contents

1. [Supported Algorithms](#supported-algorithms)
2. [Block Reward Schedule](#block-reward-schedule)
3. [Node Setup](#node-setup)
4. [Full mateable.conf Example](#full-mateableconf-example)
5. [Security Recommendations](#security-recommendations)
6. [Getting Block Templates](#getting-block-templates)
7. [Long Polling](#long-polling)
8. [Block Template Response](#block-template-response)
9. [Per-Algo Difficulty](#per-algo-difficulty)
10. [Coinbase Transaction Structure](#coinbase-transaction-structure)
11. [Submitting Blocks](#submitting-blocks)
12. [Multi-Algo Pool Architecture](#multi-algo-pool-architecture)
13. [Proof of Stake](#proof-of-stake)
14. [Compatible Pool Software](#compatible-pool-software)
15. [YIIMP Setup](#yiimp-setup)
16. [SegWit and Taproot Fork (Block 4,000,000)](#segwit-and-taproot-fork-block-4000000)
17. [Useful RPC Commands](#useful-rpc-commands)
18. [Network Information](#network-information)

---

## Supported Algorithms

Mateablecoin supports 5 PoW algorithms. Each algorithm has its own independent
difficulty that adjusts separately.

| ID | Algorithm  | Notes                        |
|----|------------|------------------------------|
| 0  | scrypt     | Standard Scrypt (N=1024)     |
| 1  | yescrypt   | Yescrypt-R8                  |
| 2  | whirlpool  | Whirlpool hash               |
| 3  | ghostrider | GhostRider (Raptoreum algo)  |
| 4  | balloon    | Balloon hashing              |

Multi-algo mining starts at block 150,000. PoS also starts at block 150,000.

---

## Block Reward Schedule

| Block Range              | PoW Reward   | Notes                                        |
|--------------------------|--------------|----------------------------------------------|
| 1                        | 100,000,000  | Premine block                                |
| 2 - 199,999              | 214 MTBC     | Single algo (scrypt); multi-algo at 150,000  |
| 200,000 - 399,999        | 107 MTBC     | PoS starts at block 150,000                  |
| 400,000 - 599,999        | 53.5 MTBC    |                                              |
| 600,000 - 799,999        | 26.75 MTBC   |                                              |
| 800,000 - 999,999        | 13.375 MTBC  |                                              |
| 1,000,000 - 1,999,999    | 6.6875 MTBC  |                                              |
| 2,000,000 - 3,999,999    | 3.34375 MTBC |                                              |
| 4,000,000 - 4,599,999    | 1 MTBC       | SegWit + Taproot activate at block 4,000,000 |
| 4,600,000 - 5,599,999    | 3 MTBC       |                                              |
| 5,600,000 - 6,599,999    | 6 MTBC       |                                              |
| 6,600,000 - 7,599,999    | 3 MTBC       |                                              |
| 7,600,000 - 8,599,999    | 1 MTBC       |                                              |
| 8,600,000+               | 1 MTBC       | Minimum perpetual reward                     |

- Coinbase maturity: **59 blocks** (must wait 59 confirmations before spending)
- Block time: **~1 minute** per block across all algos combined
- Each algo targets roughly equal share of blocks

---

## Node Setup

### Requirements

- Ubuntu 20.04 or 22.04 (recommended)
- 4+ GB RAM
- 50+ GB disk space (for blockchain)
- Stable internet connection

### Install and run the daemon

```bash
# Start the daemon
mateablecoind -daemon

# Check sync status
mateable-cli getblockchaininfo

# Wait until "initialblockdownload" is false before mining
```

---

## Full mateable.conf Example

Create `~/.mateable/mateable.conf` with these settings for a pool node:

```ini
# Network
server=1
listen=1
maxconnections=125

# RPC - use rpcauth instead of rpcuser/rpcpassword (more secure)
# Generate with: python3 share/rpcauth/rpcauth.py yourusername
rpcauth=yourusername:yourhash
rpcport=6966
rpcallowip=127.0.0.1

# If your pool stratum runs on a different machine, add its IP:
# rpcallowip=192.168.1.100

# Transaction index (required for some pool software)
txindex=1

# Block notification - tells stratum a new block was found
# Replace PORT with your stratum's block notify port (e.g. 8888)
blocknotify=curl -s http://127.0.0.1:PORT/block-notify/%s > /dev/null

# Or use the blocknotify binary if your pool software provides one:
# blocknotify=/var/stratum/blocknotify 127.0.0.1:PORT %s

# Performance
dbcache=512
par=4

# Default mining algorithm (scrypt, yescrypt, whirlpool, ghostrider, balloon)
# algo=scrypt
```

### Generate rpcauth (recommended over rpcuser/rpcpassword)

```bash
python3 share/rpcauth/rpcauth.py yourusername
```

Copy the output line into `mateable.conf`. The script also shows the password
to use in your pool software's RPC config.

---

## Security Recommendations

- **Use `rpcauth`** instead of plain `rpcuser`/`rpcpassword` - it stores a
  hashed credential instead of plaintext
- **Never expose RPC port 6966 to the public internet** - use firewall rules:
  ```bash
  ufw allow 6969/tcp   # P2P - public
  ufw deny 6966/tcp    # RPC - keep private
  ```
- **Bind RPC to localhost only** with `rpcallowip=127.0.0.1`
- **Use a dedicated wallet** for the pool payout address - do not use your
  personal wallet on the pool node
- **Regular backups** of `wallet.dat` - store encrypted copies offsite
- **Keep the node updated** - subscribe to release notifications on GitHub
- **Run behind a reverse proxy** (nginx) if exposing any web interface

---

## Getting Block Templates

Use `getblocktemplate` to request a block template for mining. The algorithm
is passed as the second parameter.

### Before block 4,000,000 (no SegWit yet)

```bash
mateable-cli getblocktemplate '{}' "scrypt"
mateable-cli getblocktemplate '{}' "yescrypt"
mateable-cli getblocktemplate '{}' "whirlpool"
mateable-cli getblocktemplate '{}' "ghostrider"
mateable-cli getblocktemplate '{}' "balloon"
```

### After block 4,000,000 (SegWit active - rules required)

```bash
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "scrypt"
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "yescrypt"
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "whirlpool"
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "ghostrider"
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "balloon"
```

Most modern pool software already passes `{"rules": ["segwit"]}` - it works
before and after the fork.

### Set or check the default algorithm

```bash
# Set the default algorithm
mateable-cli setalgo "yescrypt"

# Check the current default algorithm
mateable-cli getalgo

# Set default on startup via config or flag
# Add to mateable.conf:  algo=yescrypt
# Or start with:         mateablecoind -algo=yescrypt
```

---

## Long Polling

Long polling allows pool software to be notified immediately when a new block
is found, rather than polling repeatedly. The GBT response includes a
`longpollid` field. Pass it back in the next request to block until the chain
tip changes:

```bash
# First request - get longpollid from response
LPID=$(mateable-cli getblocktemplate '{"rules":["segwit"]}' "scrypt" | jq -r '.longpollid')

# Second request - blocks until new block found or mempool changes
mateable-cli getblocktemplate "{\"rules\":[\"segwit\"],\"longpollid\":\"$LPID\"}" "scrypt"
```

Most stratum software handles long polling automatically. Just make sure
`blocknotify` is configured in `mateable.conf` so the node notifies your
stratum when a new block arrives.

---

## Block Template Response

The `getblocktemplate` response includes all fields a pool needs:

```json
{
  "version": 536871426,
  "previousblockhash": "000000000000abc123...",
  "transactions": [...],
  "coinbasevalue": 5000000000,
  "target": "00000fffffff...",
  "bits": "1e0ffff0",
  "height": 3500001,
  "curtime": 1709123456,
  "mintime": 1709123400,
  "pow_algo": "scrypt",
  "pow_algo_id": 0,
  "sigoplimit": 80000,
  "sizelimit": 4000000,
  "weightlimit": 4000000,
  "mutable": ["time", "transactions", "prevblock"],
  "noncerange": "00000000ffffffff",
  "rules": ["csv", "!segwit"],
  "vbavailable": {},
  "vbrequired": 0,
  "longpollid": "000000000000abc123...1234",
  "default_witness_commitment": "6a24aa21a9ed..."
}
```

Key fields for pool operators:

| Field | Description |
|-------|-------------|
| `pow_algo` | Algorithm name for this template (e.g. "scrypt") |
| `pow_algo_id` | Numeric algorithm ID (0-4) |
| `bits` | Compact target for this algo's current difficulty |
| `target` | Full 256-bit target hash |
| `coinbasevalue` | Maximum block reward in satoshis (fees + subsidy) |
| `height` | Block height being mined |
| `version` | Block version (encodes the algorithm) |
| `previousblockhash` | Hash of the current chain tip |
| `mintime` | Minimum valid timestamp for the new block |
| `longpollid` | Use for long polling to detect new blocks |
| `default_witness_commitment` | Witness commitment for coinbase (after block 4M) |
| `transactions` | Transactions to include in the block |

---

## Per-Algo Difficulty

Each algorithm has its own independent difficulty that adjusts every block.
The `bits` field in the GBT response is specific to the algorithm you
requested. A scrypt template and a yescrypt template for the same height will
have **different** `bits` values.

The node tracks the last block for each algorithm separately and adjusts
difficulty based on the time since that algo's last block.

When displaying hashrate or difficulty stats for your pool, always use the
`bits`/`target` from the GBT response for that specific algorithm. Do not
mix difficulties across algorithms.

```bash
# Get current difficulty for each algorithm
mateable-cli getmininginfo

# Get per-algo network hashrate (last 120 blocks of that algo)
mateable-cli getnetworkhashps 120 -1 "scrypt"
mateable-cli getnetworkhashps 120 -1 "yescrypt"
mateable-cli getnetworkhashps 120 -1 "whirlpool"
mateable-cli getnetworkhashps 120 -1 "ghostrider"
mateable-cli getnetworkhashps 120 -1 "balloon"
```

---

## Coinbase Transaction Structure

Your pool must construct a valid coinbase transaction. The node provides
`coinbasevalue` (the maximum reward) but your pool software builds the
actual coinbase transaction.

### Before block 4,000,000

Standard coinbase:
```
Input:  coinbase scriptSig = block height (BIP34) + extra nonce
Output: pool payout address, value = coinbasevalue
```

### After block 4,000,000 (SegWit active)

After SegWit activates you must include the witness commitment output in the
coinbase. The node provides this in the `default_witness_commitment` field:

```
Input:  coinbase scriptSig = block height + extra nonce
        coinbase witness   = 32-byte nonce (all zeros is fine)
Output 0: pool payout address, value = coinbasevalue
Output 1: OP_RETURN <default_witness_commitment>  (witness commitment)
```

Most modern pool software handles this automatically when SegWit is enabled.
Check your stratum software docs for how to enable witness commitment support.

### Coinbase maturity

The coinbase output cannot be spent until **59 confirmations** after the block
is included in the chain. Do not pay out miners until this is confirmed.

---

## Submitting Blocks

When a miner finds a valid nonce:

```bash
mateable-cli submitblock "hexencodedblockdata"
```

Returns `null` on success, or an error string on failure. Common errors:

| Error | Meaning |
|-------|---------|
| `high-hash` | Block hash does not meet target |
| `duplicate` | Block already submitted |
| `inconclusive` | Block may be valid, need more data |
| `bad-cb-missing` | Missing coinbase transaction |
| `bad-witness-merkle-match` | Witness commitment mismatch (after fork) |

---

## Multi-Algo Pool Architecture

A single Mateablecoin node serves templates for all 5 algorithms. Run one
stratum process per algorithm, each on its own port:

```
                     +------------------------+
                     |   Mateablecoin Node    |
                     |   RPC port 6966        |
                     |   P2P port 6969        |
                     +----------+-------------+
                                |
              +-----------------+-----------------+
              |        blocknotify to all stratums |
              |                                   |
    +---------+---------+             +-----------+---------+
    |  getblocktemplate |             | getblocktemplate    |
    |  algo=scrypt      |    ...      | algo=balloon        |
    +---------+---------+             +-----------+---------+
              |                                   |
    +---------+---------+             +-----------+---------+
    | Stratum  port 3333|             | Stratum  port 3337  |
    | algo: scrypt      |             | algo: balloon       |
    +---------+---------+             +-----------+---------+
              |                                   |
          Miners                              Miners
```

All algo stratums pay to the same wallet address. The pool operator runs
one wallet, one node, and one stratum process per algorithm.

The template is automatically regenerated when:
- A new block is found on the chain
- The mempool changes significantly (new transactions)
- A different algorithm is requested

---

## Proof of Stake

Mateablecoin uses Proof of Stake (PoSv3, based on Particl's system) starting
at block 150,000. PoS blocks are minted by wallets holding coins, not by
miners. Pool operators should be aware of the following:

- **PoS blocks do not go through `getblocktemplate`** - they are created by
  the wallet's staking thread automatically
- **PoS blocks appear in the chain between PoW blocks** - this is normal
- **Staking requires coins in the wallet** with a minimum age of 10 minutes
  and a maximum age of 30 days
- **Pool wallets can stake** - if the pool wallet holds MTBC it will
  automatically stake and earn rewards
- **To enable staking** in the wallet GUI: click the staking toggle. In the
  daemon: staking starts automatically when the wallet is unlocked

Pool operators do not need to do anything special for PoS - the node handles
it automatically if the wallet is running and funded.

---

## Compatible Pool Software

The following pool software is known to work with Mateablecoin's GBT and
multi-algo setup:

| Software | Multi-Algo | SegWit | Notes |
|----------|-----------|--------|-------|
| **YIIMP** | Yes | Yes | Most popular for altcoin multi-algo pools |
| **miningcore** | Yes | Yes | .NET based, modern, good multi-coin support |
| **NOMP** | Yes | Partial | Node Open Mining Portal, older but widely used |
| **CoiniumServ** | Yes | Yes | C# based pool server |

---

## YIIMP Setup

YIIMP (Yet Another Implementation of a Mining Pool) by
[tpruvot](https://github.com/tpruvot/yiimp) is the most widely used pool
software for multi-algo altcoin pools and works well with Mateablecoin.

### mateable.conf for YIIMP

Add `blocknotify` to your `mateable.conf` so YIIMP is notified instantly
when a new block is found:

```ini
server=1
txindex=1
rpcauth=yourusername:yourhash
rpcport=6966
rpcallowip=127.0.0.1
blocknotify=/var/stratum/blocknotify 127.0.0.1:NOTIFYPORT %s
```

Replace `NOTIFYPORT` with the port YIIMP's stratum listens on for block
notifications (typically a port like 8888 or similar - check your YIIMP
stratum config).

### Adding Mateablecoin to YIIMP (MySQL)

Add a coin entry to the YIIMP database for each algorithm you want to mine.
Run this once per algorithm:

```sql
-- Scrypt port
INSERT INTO `coins` (
  name, symbol, algo, port,
  rpchost, rpcport, rpcuser, rpcpassword,
  enable, auto_ready, use_getblocktemplate
) VALUES (
  'Mateablecoin', 'MTBC', 'scrypt', 3333,
  '127.0.0.1', 6966, 'yourusername', 'yourpassword',
  1, 1, 1
);

-- Yescrypt port
INSERT INTO `coins` (
  name, symbol, algo, port,
  rpchost, rpcport, rpcuser, rpcpassword,
  enable, auto_ready, use_getblocktemplate
) VALUES (
  'Mateablecoin', 'MTBC', 'yescrypt', 3334,
  '127.0.0.1', 6966, 'yourusername', 'yourpassword',
  1, 1, 1
);

-- Repeat for whirlpool (3335), ghostrider (3336), balloon (3337)
```

Set `use_getblocktemplate = 1` to use GBT (required for SegWit support after
block 4,000,000).

### YIIMP stratum algo names

YIIMP uses its own algo name strings. Make sure the `algo` field in the
database matches what YIIMP's stratum binary expects:

| Mateablecoin Algo | YIIMP Algo Name |
|-------------------|----------------|
| scrypt            | `scrypt`        |
| yescrypt          | `yescrypt`      |
| whirlpool         | `whirlpool`     |
| ghostrider        | `ghostrider`    |
| balloon           | `balloon`       |

### Starting YIIMP stratum processes

```bash
# Start one stratum screen per algorithm
screen -dmS mtbc_scrypt     /var/stratum/stratum scrypt
screen -dmS mtbc_yescrypt   /var/stratum/stratum yescrypt
screen -dmS mtbc_whirlpool  /var/stratum/stratum whirlpool
screen -dmS mtbc_ghostrider /var/stratum/stratum ghostrider
screen -dmS mtbc_balloon    /var/stratum/stratum balloon
```

### Miner connection string for YIIMP

```
stratum+tcp://yourpool.com:3333
Username: YOUR_MTBC_ADDRESS
Password: c=MTBC
```

---

## SegWit and Taproot Fork (Block 4,000,000)

Mateablecoin activates SegWit, Taproot, and CSV at block 4,000,000. This is a
mandatory hard fork. All nodes and pools must update to v24.x before this block.

### Before block 4,000,000
- The `{"rules": ["segwit"]}` parameter is optional in `getblocktemplate`
- Old protocol (70017) and new protocol (70018) nodes can communicate
- Mining works exactly as before

### At block 4,000,000
- SegWit (BIP141/143/147), Taproot (BIPs 340-342), and CSV (BIP68/112/113) activate
- `{"rules": ["segwit"]}` becomes required in `getblocktemplate`
- Nodes running old protocol version 70017 are disconnected by updated nodes
- Witness transactions and Taproot (Schnorr signatures, Tapscript) are enabled
- The `default_witness_commitment` field appears in GBT and must be included
  in the coinbase transaction

### What pool operators need to do

1. **Update your node to v24.x** before block 4,000,000
2. **Pass `{"rules": ["segwit"]}`** in all `getblocktemplate` calls - works
   before and after the fork
3. **Enable SegWit in your pool software** - most modern software supports
   this. Look for a `segwit=true` or similar setting
4. **Handle `default_witness_commitment`** - most modern pool software does
   this automatically when SegWit is enabled
5. **Test on testnet first** - SegWit and Taproot are already active on
   testnet so you can test your pool setup before the mainnet fork

### Checking fork status

```bash
# Check current deployment status
mateable-cli getblockchaininfo

# Look for "segwit" and "taproot" in the "softforks" section
# Before block 4M: segwit will show "defined" or "locked_in"
# After block 4M:  segwit and taproot will show "active"
```

---

## Useful RPC Commands

| Command | Description |
|---------|-------------|
| `getblocktemplate '{"rules":["segwit"]}' "algo"` | Get a block template |
| `submitblock "hex"` | Submit a mined block |
| `setalgo "name"` | Set the default mining algorithm |
| `getalgo` | Get the current default algorithm |
| `getmininginfo` | Mining status, difficulty, and current algo |
| `getnetworkhashps 120 -1 "algo"` | Network hashrate for a specific algorithm |
| `getblockchaininfo` | Blockchain status and fork activation info |
| `getpeerinfo` | Connected peers and protocol versions |
| `getwalletinfo` | Wallet balance and staking status |
| `getstakinginfo` | Staking status and expected time to stake |
| `getblock "hash"` | Full block details |
| `getblockhash height` | Get block hash at a given height |
| `validateaddress "addr"` | Validate a payout address |

---

## Network Information

| Parameter | Value |
|-----------|-------|
| P2P Port | 6969 |
| RPC Port | 6966 |
| Protocol Version | 70018 |
| Minimum Peer Version | 70017 (70018 after block 4,000,000) |
| Coin Ticker | MTBC |
| Address Prefix | M (base58) |
| Bech32 Prefix | mtbc (SegWit addresses, after block 4,000,000) |
| Block Time | ~1 minute (all algos combined) |
| Coinbase Maturity | 59 blocks |
| SegWit/Taproot/CSV Activation | Block 4,000,000 |
| Staking Min Age | 10 minutes |
| Staking Max Age | 30 days |
