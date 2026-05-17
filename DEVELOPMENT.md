# Noctctl Developer Guide

## Building from Source

### Prerequisites

- **GCC** or **Clang** compiler (C99 or later)
- **CMake** 3.10 or later
- **Make** or **Ninja** build tool
- **POSIX-compliant system** (Linux, BSD, macOS)

### Build Steps

```bash
git clone https://github.com/noxwm/noctctl
cd noctctl
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
sudo cmake --install build
```

### Build Options

```bash
# Debug build with symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Custom installation prefix
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/noctctl

# Minimal build (no debug info)
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel
```

### Installation

**System-wide installation**:
```bash
sudo cmake --install build
# Binary installed to /usr/local/bin/noctctl
```

**User installation**:
```bash
cmake --install build --prefix ~/.local
# Add ~/.local/bin to PATH
```

**Manual installation**:
```bash
cp build/noctctl /usr/local/bin/
chmod +x /usr/local/bin/noctctl
```

## Project Structure

```
noctctl/
├── CMakeLists.txt          # CMake build configuration
├── LICENSE                 # MIT License
├── README.md              # User documentation
├── IPC_PROTOCOL.md        # IPC protocol specification
├── DEVELOPMENT.md         # This file
├── include/
│   └── noctctl.h          # Public header file
├── src/
│   ├── main.c             # CLI entry point
│   └── ipc.c              # Socket IPC implementation
└── build/                 # Generated (build artifacts)
```

## Architecture

### Module Overview

```
┌─────────────────────────────────────┐
│       CLI Interface (main.c)        │
│  - Command parsing                  │
│  - Argument validation               │
│  - Error reporting                   │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│    IPC Client Library (ipc.c)       │
│  - Socket management                │
│  - Send/receive operations          │
│  - Error handling                    │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│    Unix Domain Socket (SOCK_STREAM) │
│    $XDG_RUNTIME_DIR/noctis.sock     │
└─────────────────────────────────────┘
                   │
                   ▼
         ┌─────────────────┐
         │  Noctis Server  │
         └─────────────────┘
```

### Key Components

#### `include/noctctl.h`
Public API for IPC operations:
- `noctctl_ipc_init()` - Initialize connection
- `noctctl_ipc_send()` - Send command
- `noctctl_ipc_recv()` - Receive response
- `noctctl_ipc_close()` - Close connection

#### `src/ipc.c`
Low-level socket operations:
- Socket creation and connection
- XDG_RUNTIME_DIR path resolution
- Reliable send/receive with error handling
- Resource cleanup

#### `src/main.c`
Command-line interface:
- Command validation and dispatching
- Argument parsing
- User-facing error messages
- Help and version information

## Development Workflow

### Code Formatting

Follow these style guidelines:

```c
// Use 4 spaces for indentation (no tabs)
// Line length: max 100 characters

// Function naming: snake_case
static int validate_command(const char *cmd, int arg_count) {
    // Implementation
}

// Variable naming: snake_case
int socket_fd = -1;
char *socket_path = NULL;

// Constants: UPPER_SNAKE_CASE
#define NOCTCTL_RESPONSE_SIZE 1024
```

### Adding New Commands

1. **Update `include/noctctl.h`** if adding IPC functions
2. **Add to commands array** in `src/main.c`:

```c
static const command_t commands[] = {
    {"newcmd", "Description of new command", min_args, max_args},
    // ... other commands
};
```

3. **Validate arguments** in `validate_command()`
4. **Test thoroughly** before submitting PR

### Testing

```bash
# Build debug version
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run with test commands
./build/noctctl --help
./build/noctctl --version

# Test error handling
./build/noctctl invalid_command
./build/noctctl reload extra_arg
```

### Memory Safety

- Use ASAN for memory error detection:
```bash
cmake -B build -DCMAKE_C_FLAGS="-fsanitize=address"
cmake --build build
```

- Use Valgrind for leak checking:
```bash
valgrind --leak-check=full ./build/noctctl reload
```

## Integration with Noctis

### Expected Socket Protocol

The compositor must:
1. Listen on `$XDG_RUNTIME_DIR/noctis.sock`
2. Accept plain text commands (no null terminator required)
3. Respond with "ok" on success or "error: <msg>" on failure
4. Close connection after sending response

### Example Noctis Implementation Pseudocode

```rust
// In noctis compositor
fn handle_ipc_connection(mut stream: UnixStream) {
    let mut buffer = vec![0u8; 4096];
    
    match stream.read(&mut buffer) {
        Ok(n) => {
            let command = String::from_utf8_lossy(&buffer[..n]);
            
            match process_command(&command) {
                Ok(_) => {
                    stream.write_all(b"ok").ok();
                }
                Err(e) => {
                    let msg = format!("error: {}", e);
                    stream.write_all(msg.as_bytes()).ok();
                }
            }
        }
        Err(e) => eprintln!("IPC read error: {}", e),
    }
}
```

## Debugging

### Enable Debug Output

Recompile with debug symbols:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Common Issues

**Socket not found**
```bash
# Check if XDG_RUNTIME_DIR is set
echo $XDG_RUNTIME_DIR

# Verify socket exists
ls -la $XDG_RUNTIME_DIR/noctis.sock
```

**Connection refused**
```bash
# Ensure noctis is running
ps aux | grep noctis

# Check socket permissions
stat $XDG_RUNTIME_DIR/noctis.sock
```

**Command parsing issues**
```bash
# Test with explicit arguments
./build/noctctl -- reload
./build/noctctl exec 'complex command with spaces'
```

## Contributing

### Code Review Checklist

- [ ] Code follows project style guidelines
- [ ] Memory is properly allocated and freed
- [ ] Error cases are handled
- [ ] New commands are documented
- [ ] No compiler warnings (gcc -Wall -Wextra -Werror)
- [ ] Tested with both short and long arguments

### Submitting Changes

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request with description

## Performance Considerations

### Optimization Opportunities

1. **Connection pooling** (for high-frequency operations)
2. **Batch operations** (multiple commands in one request)
3. **Async operations** (for long-running commands)
4. **Response caching** (for frequently queried state)

### Current Limitations

- One command per connection
- Synchronous blocking operations
- No timeout mechanism (may hang indefinitely)

## Troubleshooting

### Build Errors

```bash
# CMake not found
sudo apt-get install cmake

# C compiler not found
sudo apt-get install build-essential

# Missing dependencies
ldd ./build/noctctl
```

### Runtime Errors

```bash
# Connection refused
$ noctctl reload
Error: Failed to connect to noctis socket
→ Start noctis compositor

# Invalid command
$ noctctl invalid
Error: unknown command 'invalid'

# Invalid arguments
$ noctctl border
Error: command 'border' requires at least 2 argument(s)
```

## Contact & Support

- **Issues**: GitHub Issues on noxwm/noctctl
- **Discussions**: GitHub Discussions
- **Related**: See noctis compositor repository
