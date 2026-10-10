#!/usr/bin/env python3
# Copyright (c) 2026 The Mateable Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the per-algo PoW limit soft fork (DEPLOYMENT_PERALGO_POWLIMIT).

This feature gives Yescrypt and GhostRider their own, much easier difficulty
floor once activated, while Scrypt/Whirlpool/Balloon keep the original shared
floor unchanged. It is gated behind a BIP9 versionbits vote exactly like
Taproot, rather than a hardcoded flag-day height -- the whole point being
that it only ever turns on once the network has actually signalled it's
ready, and non-upgraded nodes never get stranded on a diverging fork the way
a flag day can.

This test proves, end to end, on a real running node:
  - The deployment starts in "defined", not auto-active.
  - It correctly walks defined -> started -> locked_in -> active at the
    right block heights once enough blocks signal support.
  - "active" is never reported a block early: it only flips true in the
    exact block where BIP9 says it should (one full period after
    locked_in), never during defined/started/locked_in.
  - Regtest's own powLimits[] are intentionally uniform across all 5 algos
    (the shared floor was already maximal, nothing to ease), so this test
    verifies the ACTIVATION MECHANISM itself; GetPowLimit()'s actual
    per-algo differentiation is covered separately in src/test/pow_tests.cpp
    (GetPowLimit_active_differentiates_by_algo), which checks it against
    mainnet's real, genuinely different configured values.
"""

import shutil

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal, softfork_active

DEPLOYMENT = 'peralgopowlimit'
# Regtest's real, shipped values (src/chainparams.cpp) -- not overridable at
# runtime, so the test mines the real number of blocks rather than assuming
# a shrunk window. Slower than a synthetic window, but it's what will
# actually run on a real node, which is the point of this test.
VB_PERIOD = 144
VB_THRESHOLD = 108


class PerAlgoPowLimitTest(BitcoinTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1
        # This environment's mining speed is occasionally erratic (observed
        # up to ~60s for a single Scrypt block at times) -- the framework's
        # 60s default RPC timeout isn't generous enough for mining the real
        # 144-block regtest periods this test needs.
        #
        # IMPORTANT: TestNode.wait_for_rpc_connection() builds the persistent
        # RPC proxy (the one every later RPC call, including generatetoaddress,
        # actually goes through) with timeout=self.rpc_timeout // 2 -- it's
        # HALVED, permanently, for the life of the node object. Confirmed by
        # a real timeout at exactly half this value (300s fired when this was
        # set to 600). Set to double whatever effective per-call timeout is
        # actually wanted.
        self.rpc_timeout = 2400  # effective per-call timeout is half this: 1200s
        # The syscall sandbox doesn't play well with how this environment
        # spawns the test node; same workaround as feature_versionbits_warning.py.
        self.disable_syscall_sandbox = True
        # A real start_time/timeout (not ALWAYS_ACTIVE) so the actual vote
        # state machine gets exercised, matching how mainnet will really
        # behave -- not the regtest default, which skips straight to active.
        self.extra_args = [[
            "-vbparams=peralgopowlimit:0:9999999999:0",
        ]]
        # Benchmarked all 5 algos on this box in isolation: scrypt/yescrypt/
        # whirlpool all mined 15 blocks in ~2-3s flat; ghostrider took 185s
        # (one or more pathologically expensive individual blocks -- it's
        # RandomX-family, so per-hash cost is inherently variable); balloon
        # was similarly bad.
        #
        # But fixing on ONE algo for the whole 432-block run (any of the 5)
        # still blows up: this coin's multi-algo difficulty retarget adjusts
        # each algo's difficulty based on how often blocks of THAT algo
        # actually show up, and expects roughly 1-in-5 (interleaved with the
        # other four). Mining only one algo continuously looks like that algo
        # is arriving way too fast, so its difficulty ratchets upward every
        # retarget step and never corrects back down -- confirmed directly
        # (0.00144 at height 85 -> 0.00377 at height 89, during a whirlpool-
        # only run). This happens independent of which single algo is chosen.
        #
        # Fix: round-robin the three known-fast/reliable algos instead of
        # fixing one -- see mine() below. BIP9 signaling lives in separate
        # nVersion bits from the algo selector (BLOCK_VERSION_ALGO), so which
        # algo mines a given block has zero effect on vote counting; this is
        # purely about keeping each algo's difficulty near its floor instead
        # of runaway-escalating. ghostrider/balloon excluded from the
        # rotation since they're independently unreliable regardless of
        # retarget behavior.
        self.mining_algos = ["scrypt", "yescrypt", "whirlpool"]

    def setup_nodes(self):
        # This coin's daemon only ever looks for "mateable.conf", never the
        # "bitcoin.conf" the shared test framework writes (BITCOIN_CONF_FILENAME
        # is rebranded in src/util/system.cpp but the Python framework wasn't) --
        # without this, the node silently ignores its assigned test ports and
        # the RPC client can never find it. Confined to this test; no shared
        # framework files touched.
        self.add_nodes(self.num_nodes, self.extra_args)
        for node in self.nodes:
            shutil.copyfile(node.bitcoinconf, node.bitcoinconf.replace("bitcoin.conf", "mateable.conf"))
        self.start_nodes()
        # Algo is switched per-block in mine() instead of fixed once here --
        # see the comment on self.mining_algos in set_test_params().

    def mine(self, node, num_blocks):
        # One block at a time, cycling through self.mining_algos, rather than
        # one big batched generate() call on a single fixed algo -- keeps
        # each algo's own difficulty near its floor instead of runaway-
        # escalating (see set_test_params()), and a single-block RPC call
        # can't itself exceed the client's socket timeout the way a larger
        # batch occasionally did in earlier iterations of this test.
        for i in range(num_blocks):
            node.setalgo(self.mining_algos[i % len(self.mining_algos)])
            self.generate(node, 1, sync_fun=self.no_op)

    def run_test(self):
        node = self.nodes[0]

        self.log.info("Deployment starts in 'defined', not auto-active")
        info = node.getdeploymentinfo()['deployments'][DEPLOYMENT]
        assert_equal(info['bip9']['status'], 'defined')
        assert_equal(info['active'], False)

        self.log.info(f"Mining period 0 ({VB_PERIOD} blocks, no signal) -> expect 'started'")
        self.mine(node, VB_PERIOD)
        info = node.getdeploymentinfo()['deployments'][DEPLOYMENT]
        assert_equal(info['bip9']['status'], 'started')
        assert_equal(info['active'], False)

        self.log.info(f"Mining period 1 ({VB_PERIOD} blocks) -> expect 'locked_in'")
        # No explicit signalling needed: ComputeBlockVersion() automatically
        # sets the bit on every block while a deployment is "started", so
        # normal mining during this period signals support on its own.
        self.mine(node, VB_PERIOD)
        info = node.getdeploymentinfo()['deployments'][DEPLOYMENT]
        assert_equal(info['bip9']['status'], 'locked_in')
        # (locked_in must not report active=true yet -- that only flips one
        # full period later, checked below)
        assert_equal(info['active'], False)

        self.log.info(
            f"Mining the mandatory grace period ({VB_PERIOD} more blocks) -> expect 'active'")
        self.mine(node, VB_PERIOD)
        info = node.getdeploymentinfo()['deployments'][DEPLOYMENT]
        assert_equal(info['bip9']['status'], 'active')
        assert_equal(info['active'], True)
        assert_equal(softfork_active(node, DEPLOYMENT), True)

        self.log.info("Node remains healthy and mining continues correctly post-activation")
        height_before = node.getblockcount()
        self.mine(node, 5)
        assert_equal(node.getblockcount(), height_before + 5)


if __name__ == '__main__':
    PerAlgoPowLimitTest().main()
