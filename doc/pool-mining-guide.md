# Mateablecoin Pool Mining Guide

This guide is for pool operators who want to mine Mateablecoin (MTBC). Mateablecoin
is a multi-algorithm coin with 5 PoW algorithms and Proof of Stake. A single node
and wallet can be used to mine all algorithms.

## Supported Algorithms

| ID | Algorithm   | Block Version Bits |
|----|-------------|-------------------|
| 0  | scrypt      | `0 << 8`          |
| 1  | yescrypt    | `2 << 8`          |
| 2  | whirlpool   | `4 << 8`          |
| 3  | ghostrider  | `6 << 8`          |
| 4  | balloon     | `8 << 8`          |

## Node Setup

1. Build or download Mateablecoin Core v24.x
2. Start the daemon:
```bash
mateablecoind -daemon -server -rpcuser=yourusername -rpcpassword=yourpassword
```

3. Wait for the blockchain to fully sync before mining.

## RPC Configuration

Add to your `mateable.conf`:
```
server=1
rpcuser=yourusername
rpcpassword=yourpassword
rpcallowip=127.0.0.1
rpcport=6966
```

Default RPC port is `6966` for mainnet.

## Getting Block Templates

Use `getblocktemplate` to request a block template for mining. The algorithm is
passed as the second parameter.

### Request a template for a specific algorithm

```bash
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "scrypt"
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "yescrypt"
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "whirlpool"
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "ghostrider"
mateable-cli getblocktemplate '{"rules": ["segwit"]}' "balloon"
```

### Request a template using the default algorithm

If you don't pass an algorithm, the current default is used:
```bash
mateable-cli getblocktemplate '{"rules": ["segwit"]}'
```

### Set or check the default algorithm

```bash
# Set the default algorithm
mateable-cli setalgo "yescrypt"

# Check the current default algorithm
mateable-cli getalgo
```

You can also set the default algorithm on startup:
```bash
mateablecoind -algo=yescrypt
```

## Block Template Response

The `getblocktemplate` response includes all the fields a pool needs:

```json
{
  "version": 536871426,
  "previousblockhash": "00000000...",
  "transactions": [...],
  "coinbasevalue": 5000000000,
  "target": "00000fff...",
  "bits": "1e0ffff0",
  "height": 3500001,
  "curtime": 1709123456,
  "pow_algo": "scrypt",
  "pow_algo_id": 0,
  "default_witness_commitment": "6a24aa21a9ed...",
  "sigoplimit": 80000,
  "sizelimit": 4000000,
  "weightlimit": 4000000,
  "mutable": ["time", "transactions", "prevblock"],
  "noncerange": "00000000ffffffff",
  "rules": ["csv", "!segwit"],
  ...
}
```

Key fields for pool operators:

| Field | Description |
|-------|-------------|
| `pow_algo` | The algorithm name for this template (e.g. "scrypt") |
| `pow_algo_id` | The numeric algorithm ID (0-4) |
| `bits` | Compact target for this algorithm's current difficulty |
| `target` | Full 256-bit target hash |
| `coinbasevalue` | Maximum block reward in satoshis |
| `height` | Block height being mined |
| `default_witness_commitment` | SegWit witness commitment for the coinbase (after block 4,000,000) |
| `version` | Block version (includes algorithm encoding) |
| `previousblockhash` | Hash of the previous block |

## Submitting Blocks

When a miner finds a valid block:
```bash
mateable-cli submitblock "hexencodedblockdata"
```

## Multi-Algo Pool Architecture

A single Mateablecoin node can serve templates for all 5 algorithms. A typical
multi-algo pool setup looks like:

```
                     +---------------------+
                     |  Mateablecoin Node  |
                     |   (single wallet)   |
                     +----------+----------+
                                | RPC (port 6966)
           +--------------------+--------------------+
           |          |          |          |         |
      GBT scrypt  GBT yescrypt  GBT whirlpool  GBT ghostrider  GBT balloon
           |          |          |          |         |
     +-----+----+  +--+------+  +--+------+  +--+--------+  +--+-----+
     | Stratum  |  | Stratum |  | Stratum |  | Stratum   |  | Stratum|
     | :3333    |  | :3334   |  | :3335   |  | :3336     |  | :3337  |
     +-----+----+  +--+------+  +--+------+  +--+--------+  +--+-----+
           |          |          |          |         |
       Miners      Miners     Miners     Miners    Miners
```

Each stratum port handles one algorithm. The pool software calls
`getblocktemplate` with the appropriate algorithm for each port and distributes
work to connected miners. All payouts go to the same wallet address.

The template is automatically regenerated when:
- A new block is found (tip changes)
- The mempool is updated with new transactions
- A different algorithm is requested

## SegWit and Taproot Fork (Block 4,000,000)

Mateablecoin activates SegWit, Taproot, and CSV at block 4,000,000. This is a
mandatory upgrade. All nodes and pools must update to v24.x before this block.

### Before block 4,000,000
- The `rules` parameter is optional in `getblocktemplate`
- Old protocol (70017) and new protocol (70018) nodes can communicate
- Mining works as before

### At block 4,000,000
- SegWit, Taproot, and CSV activate
- `{"rules": ["segwit"]}` becomes required in `getblocktemplate`
- Old protocol (70017) nodes are disconnected from updated nodes
- Witness transactions and Taproot (Schnorr/Tapscript) transactions are enabled
- The `default_witness_commitment` field appears in the GBT response

### What pool operators need to do
1. Update your Mateablecoin node to v24.x before block 4,000,000
2. Make sure your pool software passes `{"rules": ["segwit"]}` in GBT calls
3. Handle the `default_witness_commitment` field in the coinbase transaction
4. Most modern pool software (e.g. NOMP, MPOS, CoiniumServ, miningcore) already
   supports SegWit - just make sure it's enabled in your pool config

## Useful RPC Commands

| Command | Description |
|---------|-------------|
| `getblocktemplate '{"rules":["segwit"]}' "algo"` | Get a block template for mining |
| `submitblock "hex"` | Submit a mined block |
| `setalgo "name"` | Set the default mining algorithm |
| `getalgo` | Get the current default algorithm |
| `getmininginfo` | Get mining status and current difficulty |
| `getnetworkhashps 120 -1 "algo"` | Get network hashrate for a specific algorithm |
| `getblockchaininfo` | Get blockchain status and fork activation info |
| `getpeerinfo` | Check connected peers |

## Network Information

| Parameter | Value |
|-----------|-------|
| P2P Port | 6969 |
| RPC Port | 6966 |
| Protocol Version | 70018 |
| Coin Ticker | MTBC |
| Bech32 Prefix | mtbc |
| SegWit/Taproot Activation | Block 4,000,000 |
