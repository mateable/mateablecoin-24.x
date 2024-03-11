<h1 align="center">
Mateable Core [MTBC]
<br>
v24.x
<br/><br/>
<img src="https://64.media.tumblr.com/230b42ccd0bbc026d5b1258bb8a8f92c/4fe9f521c4196e49-e7/s540x810/4e419bef2010b24459e969c46eaa4b9e20e710fd.pnj" alt="mateablecoin" width="300"/>
</h1>


<div align="center">

[![mateablecoinBadge](https://img.shields.io/badge/Mateable-coin-blueviolet.svg)](https://coin.mateable.com) [![mateablecoinBadge](https://img.shields.io/badge/MateableCoin-MTBC-blueviolet.svg)](https://coin.mateable.com) 

[![mateablecoinBadge](https://img.shields.io/badge/PoS-blueviolet.svg)](https://coin.mateable.com) [![mateablecoinBadge](https://img.shields.io/badge/scrypt-blueviolet.svg)](https://coin.mateable.com) [![mateablecoinBadge](https://img.shields.io/badge/yescrypt-R8-blueviolet.svg)](https://coin.mateable.com) [![mateablecoinBadge](https://img.shields.io/badge/whirlpool-blueviolet.svg)](https://coin.mateable.com) [![mateablecoinBadge](https://img.shields.io/badge/ghostrider-blueviolet.svg)](https://coin.mateable.com) [![mateablecoinBadge](https://img.shields.io/badge/balloon-blueviolet.svg)](https://coin.mateable.com)

</div>




MateableCoin is a community-driven cryptocurrency. The Mateable Core 24.x software allows anyone to operate a node in the MateableCoin blockchain networks and uses the Scrypt hashing method for Proof of Work. It is adapted from Bitcoin Core and other cryptocurrencies.

Mateable Core 24.x is the first multi-algorithm (5 algorithms) currency with proof of stake. These algorithms are scrypt, yescrypt, whirlpool, ghostrider and balloon. The proof of stake system is based on Particl's PoSv3 system.

For information about the default fees used on the MateableCoin network, please
refer to the [fee recommendation](doc/fee-recommendation.md).

**Website:** [coin.mateable.com](https://coin.mateable.com) 

## Usage 💻

To start your journey with Mateable Core 24.x, see the [installation guide](INSTALL.md) and the [getting started](doc/getting-started.md) tutorial.

The JSON-RPC API provided by Mateable Core is self-documenting and can be browsed with `mateable-cli help`, while detailed information for each command can be viewed with `mateable-cli help <command>`. Alternatively, see the [Bitcoin Core documentation](https://developer.bitcoin.org/reference/rpc/) - which implement a similar protocol - to get a browsable version.

### Such ports

Mateable Core 24.x by default uses port `6969` for peer-to-peer communication that
is needed to synchronize the "mainnet" blockchain and stay informed of new
transactions and blocks. Additionally, a JSONRPC port can be opened, which
defaults to port `6966` for mainnet nodes. It is strongly recommended to not
expose RPC ports to the public internet.

| Function | mainnet | 
| :------- | ------: | 
| P2P      |   6969 |   
| RPC      |   6966 |  

## Ongoing development - Moon plan 🌒

MateableCoin is an open source and community driven software. The development
process is open and publicly visible; anyone can see, discuss and work on the
software.

Main development resources:

* [Github Projects](https://github.com/mateable) is used to
  follow planned and in-progress work for upcoming releases.
* [Github Discussion](https://github.com/mateable/mateablecoin-24.x/discussions) is used
  to discuss features, planned and unplanned, related to both the development of
  the Mateable Core software, the underlying protocols and the MATEABLE asset.  
* [MateableCoin]  subreddit](https://www.reddit.com/r/Mateable/)
* [Discord](https://discord.com/invite/9TF5QkTwFA)

### Version strategy
Version numbers are following ```major.minor.patch``` semantics.

### Branches
There are 3 types of branches in this repository:

- **master:** Stable, contains the latest version of the latest *major.minor* release.
- **maintenance:** Stable, contains the latest version of previous releases, which are still under active maintenance. Format: ```<version>-maint```
- **development:** Unstable, contains new code for planned releases. Format: ```<version>-dev```

*Master and maintenance branches are exclusively mutable by release. Planned*
*releases will always have a development branch and pull requests should be*
*submitted against those. Maintenance branches are there for **bug fixes only,***
*please submit new features against the development branch with the highest version.*

## Contributing 🤝

If you find a bug or experience issues with this software, please report it
using the [issue system](https://github.com/mateable/mateablecoin-24.x/issues/new?assignees=&labels=bug&template=bug_report.md&title=%5Bbug%5D+).

Please see [the contribution guide](CONTRIBUTING.md) to see how you can
participate in the development of Mateable Core. There are often
[topics seeking help](https://github.com/mateable/mateablecoin-24.x/labels/help%20wanted)
where your contributions will have high impact and get very appreciation. wow.

## Communities 🚀🍾

You can join the communities on different social media.
To see what's going on, meet people & discuss, find the lastest meme, learn
about MateableCoin, give or ask for help, to share your project.

Here are some places to visit:

* [MateableCoin subreddit](https://www.reddit.com/r/mateable/)
* [Discord](https://discord.com/invite/9TF5QkTwFA)
* [MateableCoin Twitter](https://twitter.com/mateable)

## Very Much Frequently Asked Questions ❓

Do you have a question regarding MateableCoin? An answer is perhaps already in the
[FAQ](doc/FAQ.md) or the
[Q&A section](https://github.com/mateable/mateablecoin-24.x/discussions/categories/q-a)
of the discussion board!

## License - Much license ⚖️
Mateablecoin Core is released under the terms of the MIT license. See
[COPYING](COPYING) for more information or see
[opensource.org](https://opensource.org/licenses/MIT)

