# Safe String Handling Implementation

## Overview

This document describes the safe string handling patterns implemented in the Browser Sanity codebase, following Pattern 1 from the SafeStringHandling.md strategy document.

## Implementation Strategy

We implemented **Pattern 1: Named Size Constants** with the following principles:

1. **Use existing constants where appropriate** (e.g., `MAX_PATH` for file system paths)
2. **Create purpose-based constants** only for specific use cases not covered by existing ones
3. **Never write an un-length-checked string into a fixed buffer**

## Buffer Size Constants

All buffer size constants are defined in [`src/include/safe_strings.h`](../src/include/safe_strings.h):

### File System Paths
- `MAX_PATH` - Used for all file system paths (Windows standard: 260 characters)

### Purpose-Based Constants
- `PROG_ID_SIZE` (64) - Program ID strings and registry identifiers
- `VERSION_SIZE` (32) - Version strings
- `MESSAGE_SIZE` (512) - UI messages and status text
- `WINDOW_TITLE_SIZE` (256) - Window titles
- `DETAILS_SIZE` (1024) - Detailed information text
- `URL_SIZE` (512) - URLs and web addresses
- `ARGS_SIZE` (1024) - Command line arguments
- `COMMAND_LINE_SIZE` (4096) - Full command line strings
- `TEXT_BUFFER_SIZE` (1024) - General text operations

## Safe String Operations

### Core Pattern
All unsafe string operations have been replaced with secure alternatives:

```c
// OLD (unsafe)
strcpy(dest, src);
sprintf(buffer, format, args);
strcat(dest, src);

// NEW (safe)
strncpy_s(dest, sizeof(dest), src, _TRUNCATE);
sprintf_s(buffer, sizeof(buffer), format, args);
strcat_s(dest, sizeof(dest), src);
```

### Error Checking Pattern
All string operations include comprehensive error checking:

```c
if (strncpy_s(dest, sizeof(dest), src, _TRUNCATE) != 0) {
    DebugLogError("Failed to copy string");
    dest[0] = '\0';  // Ensure null termination
}

if (sprintf_s(buffer, sizeof(buffer), format, args) < 0) {
    DebugLogError("Failed to format string");
    // Provide fallback behavior
}
```

## Convenience Macros

The [`safe_strings.h`](../src/include/safe_strings.h) header provides convenience macros for common operations:

```c
// Automatic size detection macros
SAFE_SPRINTF(buffer, format, ...)
SAFE_STRNCPY(dest, src)
SAFE_STRCAT(dest, src)

// Error checking versions
SAFE_SPRINTF_CHECK(buffer, format, ...)
SAFE_STRNCPY_CHECK(dest, src)
```

## Files Updated

The following files have been updated to use safe string handling:

### Core Library Files
- [`src/browser-sanity/browser_sanity.c`](../src/browser-sanity/browser_sanity.c)
- [`src/browser-sanity/msedge_redirect_lib.c`](../src/browser-sanity/msedge_redirect_lib.c)

### Action Files
- [`src/browser-sanity/actions/action_install.c`](../src/browser-sanity/actions/action_install.c)
- [`src/browser-sanity/actions/action_uninstall.c`](../src/browser-sanity/actions/action_uninstall.c)

### UI Files
- [`src/browser-sanity/ui/NKWindow_MainLaunch.cpp`](../src/browser-sanity/ui/NKWindow_MainLaunch.cpp)
- [`src/browser-sanity/ui/NKWindow_Settings.cpp`](../src/browser-sanity/ui/NKWindow_Settings.cpp)
- [`src/browser-sanity/ui/NKWindow_Toast.cpp`](../src/browser-sanity/ui/NKWindow_Toast.cpp)

## Key Benefits

1. **Buffer Overflow Prevention**: All string operations are length-checked
2. **Consistent Error Handling**: Standardized error checking and logging
3. **Maintainable Code**: Clear, purpose-based buffer size constants
4. **Debug Support**: Comprehensive logging for string operation failures
5. **Graceful Degradation**: Fallback behavior when string operations fail

## Usage Guidelines

### For New Code
1. Always use named size constants from [`safe_strings.h`](../src/include/safe_strings.h)
2. Use `sprintf_s()`, `strncpy_s()`, and `strcat_s()` instead of unsafe variants
3. Always check return values and provide error handling
4. Include [`safe_strings.h`](../src/include/safe_strings.h) and [`debug_log.h`](../src/include/debug_log.h) for logging

### Buffer Declaration Pattern
```c
#include <safe_strings.h>

// Good: Use named constants
char file_path[MAX_PATH];
char message[MESSAGE_SIZE];
char url[URL_SIZE];

// Bad: Magic numbers
char file_path[260];
char message[512];
```

### String Operation Pattern
```c
// Always check return values
if (sprintf_s(buffer, sizeof(buffer), "Format: %s", value) < 0) {
    DebugLogError("Failed to format string");
    // Provide fallback behavior
    strncpy_s(buffer, sizeof(buffer), "Default value", _TRUNCATE);
}
```

## Testing

The implementation has been tested by:
1. **Compilation**: All files compile without errors or warnings
2. **Build Verification**: Complete application builds successfully
3. **Buffer Size Validation**: All buffer declarations use named constants
4. **Error Path Testing**: String operations handle truncation gracefully

## Compliance

This implementation fully complies with:
- **SafeStringHandling.md Pattern 1**: Named size constants with secure functions
- **Microsoft Security Guidelines**: Use of `_s` secure string functions
- **Buffer Overflow Prevention**: No unchecked string operations remain
- **Error Handling Standards**: Comprehensive error checking and logging