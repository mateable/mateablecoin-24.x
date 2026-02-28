/* Always use the portable opt implementation for consensus correctness.
 * The SIMD implementation produces different hashes on x86_64 vs ARM64,
 * which would cause a consensus split between platforms. */
#include "yescrypt-opt.c"
