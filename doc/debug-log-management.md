# Debug Log Management

MateableCoin includes automatic debug log file size management to prevent `debug.log` from consuming excessive disk space.

## How It Works

During normal operation (no `-debug` flags), the node automatically monitors the size of `debug.log`. When it exceeds the configured maximum (default: 10 MB), older log entries are trimmed and the most recent 10 MB of entries are kept.

When debug categories are enabled (e.g. `-debug=1`, `-debug=pos`), automatic trimming is **disabled** so that no debug information is lost while troubleshooting.

## Configuration

### Command Line / Config File

```
-maxdebugfilesize=<n>
```

Set the maximum debug log file size in MB. Default is `10`. Set to `0` for unlimited.

Examples:

```bash
# Default 10 MB cap (no config needed)
mateabled

# Set to 100 MB
mateabled -maxdebugfilesize=100

# Unlimited (no trimming)
mateabled -maxdebugfilesize=0

# Debug mode - trimming is automatically disabled regardless of maxdebugfilesize
mateabled -debug=1
```

Add to `mateable.conf` for persistent configuration:

```
maxdebugfilesize=10
```

### Qt Wallet

Go to **Options > Main** tab. The "Max debug log file size" setting allows you to change the limit and shows the current size of `debug.log`.

## Startup Shrink

In addition to runtime management, the existing `-shrinkdebugfile` option trims `debug.log` at startup. This is enabled by default when no `-debug` categories are set.

```
-shrinkdebugfile=1
```

## Behavior Summary

| Configuration | Runtime Trimming | Startup Shrink |
|---|---|---|
| No flags (default) | Yes, at 10 MB | Yes |
| `-maxdebugfilesize=100` | Yes, at 100 MB | Yes |
| `-maxdebugfilesize=0` | No | Yes |
| `-debug=1` or `-debug=pos` | No (auto-disabled) | No (default off) |
| `-debug=1 -shrinkdebugfile=1` | No (auto-disabled) | Yes (forced on) |

## External Log Rotation (Optional)

For advanced users who prefer OS-level log management, `logrotate` can be used instead of or alongside the built-in trimming.

Create `/etc/logrotate.d/mateabled`:

```
/home/username/.mateable/debug.log {
    daily
    rotate 7
    compress
    copytruncate
    missingok
    notifempty
}
```

The `copytruncate` directive is required because the node keeps the log file handle open. It copies the log contents then truncates the original file in-place without requiring a restart.

To disable the built-in trimming when using logrotate:

```
maxdebugfilesize=0
```
