# Noctctl IPC Protocol Documentation

## Overview

`noctctl` is an IPC (Inter-Process Communication) client that communicates with the `noctis` Wayland compositor through a Unix domain socket. This document describes the protocol, architecture, and command specifications.

## Communication Mechanism

### Socket Location
- **Path**: `$XDG_RUNTIME_DIR/noctis.sock`
- **Type**: Unix Domain Socket (SOCK_STREAM)
- **Protocol**: Plain text commands

### Connection Flow

```
1. Client connects to $XDG_RUNTIME_DIR/noctis.sock
2. Client sends command as plain text string
3. Compositor processes command
4. Compositor sends response ("ok" or "error: <message>")
5. Connection closes
```

## Command Protocol

### Request Format
Commands are sent as plain text with the following format:

```
<command> [arg1] [arg2] ...
```

### Response Format
The compositor responds with one of:

- `ok` - Command executed successfully
- `error: <message>` - Command failed with error message

### Character Encoding
- UTF-8 for all text
- No trailing newline required
- Maximum command length: Implementation dependent (suggest 4096 bytes)

## Commands Specification

### reload
Reloads the compositor configuration file without restarting.

```
Request:  "reload"
Response: "ok" or "error: <reason>"
```

**Use Case**: Apply configuration changes without losing window state.

---

### kill
Closes the currently focused window.

```
Request:  "kill"
Response: "ok" or "error: <reason>"
```

**Use Case**: Close focused window via command/keybind.

---

### exit
Gracefully shuts down the compositor.

```
Request:  "exit"
Response: "ok"
```

**Use Case**: Exit compositor cleanly from scripts or keybinds.

---

### focus
Changes focus to next or previous window.

```
Request:  "focus next" or "focus prev"
Response: "ok" or "error: <reason>"
```

**Arguments**:
- `next`: Focus the next window in the stack
- `prev`: Focus the previous window in the stack

**Use Case**: Window navigation via commands.

---

### exec
Executes an arbitrary command in the compositor's context.

```
Request:  "exec <command>" or "exec <command> <arg1> <arg2> ..."
Response: "ok" or "error: <reason>"
```

**Arguments**:
- `<command>`: The command to execute
- Additional arguments passed to the command

**Examples**:
```
exec firefox
exec kitty --class floating
exec grim -g '$(slurp)'
```

**Use Case**: Launch applications from keybinds or scripts.

---

### gap
Sets the gap (spacing) between windows in pixels.

```
Request:  "gap <pixels>"
Response: "ok" or "error: <reason>"
```

**Arguments**:
- `<pixels>`: Integer value for gap size (0+)

**Examples**:
```
gap 10
gap 0
gap 20
```

**Use Case**: Dynamically adjust window spacing without restarting.

---

### border
Modifies the border color for active or inactive windows.

```
Request:  "border active <color>" or "border inactive <color>"
Response: "ok" or "error: <reason>"
```

**Arguments**:
- `active|inactive`: Which border to modify
- `<color>`: Hex color code (#RRGGBB format)

**Examples**:
```
border active #AB6C6A
border inactive #333333
border active #FF0000
```

**Color Format**: Standard hex color notation
- Valid: `#AB6C6A`, `#000000`, `#FFFFFF`
- Invalid: `AB6C6A`, `rgb(255, 0, 0)`

**Use Case**: Customize border colors on-the-fly for theming.

---

### ratio
Sets the master/slave pane ratio in the layout.

```
Request:  "ratio <value>"
Response: "ok" or "error: <reason>"
```

**Arguments**:
- `<value>`: Float value between 0.0 and 1.0

**Examples**:
```
ratio 0.5
ratio 0.6
ratio 0.33
```

**Value Meaning**:
- `0.5` = Equal split (50% master, 50% slave)
- `0.6` = Master pane takes 60% of space
- `0.33` = Master pane takes 33% of space

**Use Case**: Adjust layout proportions dynamically.

---

## Error Handling

### Common Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `Failed to connect to noctis socket` | Compositor not running | Start noctis compositor |
| `XDG_RUNTIME_DIR not set` | Environment not configured | Set XDG_RUNTIME_DIR variable |
| `error: invalid argument` | Malformed command | Check command syntax |
| `error: window not found` | No focused window | Focus a window first |

### Error Response Examples

```
error: invalid color format
error: invalid ratio value (must be 0.0-1.0)
error: no focused window
error: unknown command
```

## Implementation Details

### Thread Safety
- Each command opens a new socket connection
- No persistent connections (fire-and-forget pattern)
- Safe for concurrent invocations

### Performance Considerations
- Connection overhead minimal for single commands
- Suitable for keybind/hotkey usage
- Not recommended for high-frequency real-time updates

### Resilience
- Timeouts should be implemented (suggest 5 seconds)
- Compositor crash does not leave socket stale
- Client-side socket cleanup on disconnect

## Example Workflows

### Interactive Shell Usage
```bash
# Reload config
noctctl reload

# Launch terminal
noctctl exec kitty

# Adjust gap
noctctl gap 15

# Set theme colors
noctctl border active "#4a90e2"
noctctl border inactive "#666666"

# Adjust layout ratio
noctctl ratio 0.6

# Focus navigation
noctctl focus next
noctctl focus prev

# Close window
noctctl kill

# Exit compositor
noctctl exit
```

### Shell Script Usage
```bash
#!/bin/bash
# Dynamic border color based on time of day

HOUR=$(date +%H)

if [ $HOUR -ge 18 ] || [ $HOUR -lt 6 ]; then
    # Night colors
    noctctl border active "#4a90e2"
else
    # Day colors
    noctctl border active "#e24a4a"
fi
```

### Keybind Configuration
```
# Example configuration
# resize gaps
Mod+equal -> noctctl gap 20
Mod+minus -> noctctl gap 5

# launch apps
Mod+Return -> noctctl exec kitty
Mod+p -> noctctl exec launcher

# window control
Mod+q -> noctctl kill
Mod+Tab -> noctctl focus next

# layout control
Mod+h -> noctctl ratio 0.4
Mod+m -> noctctl ratio 0.5
```

## Future Protocol Extensions

Possible future additions (v2.0+):
- Query current state: `query <state_name>`
- Layout switching: `layout <name>`
- Monitor management: `monitor <action>`
- Workspace switching: `workspace <id>`
- Window information: `window info <id>`

## Version History

- **v0.1.0** (Initial): Basic command set with Unix socket IPC
