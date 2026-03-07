---
name: "libtcc-integration"
description: "Integrates TinyC (libtcc) dynamic compilation engine into host applications for real-time C script execution. Invoke when user needs to add JIT compilation capability for geometric modeling or scriptable features."
---

# LibTCC Integration Skill

This skill provides a comprehensive guide for integrating TinyC (libtcc) dynamic compilation engine into host applications, enabling real-time C script execution for geometric modeling and other scriptable features.

## Overview

LibTCC integration allows you to:
- Enable just-in-time (JIT) compilation of C scripts
- Create a secure sandbox for script execution
- Implement parameterized geometry generation through C scripts
- Establish a safe communication channel between C scripts and C++ geometry kernel

## Environment Setup

### Required Components
- **Host Compiler**: MinGW-w64 (GCC 8.1+) or MSVC 2019+ (C++17 compatible)
- **TinyC Version**: Latest stable version or mob branch
- **Dependencies**:
  - libtcc.h (header file)
  - libtcc.a / libtcc.dll (library file)
  - Runtime support files: libtcc1.c, crt1.o, crti.o, crtn.o
  - Standard headers (stdio.h, math.h, etc.)

### Directory Structure

```
MyApp/
├── MyApp.exe          # Host application
├── libtcc.dll         # TCC dynamic library
└── tcc_libs/          # TCC resource directory
    ├── include/       # Standard headers
    └── lib/           # Runtime objects
```

## Core Architecture

### Security Sandbox Model

| Level | Strategy | Implementation |
|-------|----------|----------------|
| L1: Symbol Whitelist | Only expose safe APIs | Use tcc_add_symbol to register secure functions |
| L2: Memory Isolation | No direct memory access | Use integer handles instead of pointers |
| L3: Exception Firewall | Prevent script crashes from affecting host | Wrap all exposed functions in try-catch |
| L4: Resource Limits | Prevent infinite loops and memory exhaustion | Set execution timeouts and memory pools |

### Handle System for Data Interaction

- **Host Side**: Maintain a global map `std::map<int, std::shared_ptr<GeoFeature>>`
- **API Design**: Create, modify, and delete objects using integer handles
- **Script Side**: Only interact with integer handles, no knowledge of underlying pointers

## Implementation Guide

### TCCScriptEngine Class

```cpp
class TCCScriptEngine {
    TCCState *state;

public:
    bool initialize() {
        state = tcc_new();
        if (!state) return false;

        // Set output type to memory (JIT mode)
        tcc_set_output_type(state, TCC_OUTPUT_MEMORY);

        // Set library path (absolute path required)
        QString appDir = QCoreApplication::applicationDirPath();
        QString libPath = appDir + "/tcc_libs";
        tcc_set_lib_path(state, libPath.toUtf8().constData());

        // Register safe API functions
        tcc_add_symbol(state, "host_create_cylinder", (void*)host_create_cylinder);
        tcc_add_symbol(state, "host_set_radius", (void*)host_set_radius);
        tcc_add_symbol(state, "log_msg", (void*)log_msg_wrapper);

        return true;
    }

    bool runScript(const QString& code, QString& errorMsg) {
        if (!state) { errorMsg = "Engine not initialized"; return false; }

        // Compile string
        if (tcc_compile_string(state, code.toUtf8().constData()) == -1) {
            errorMsg = "Compilation Failed";
            tcc_delete(state);
            state = tcc_new();
            reRegisterSymbols();
            return false;
        }

        // Relocate (critical: pass NULL)
        if (tcc_relocate(state, NULL) == -1) {
            errorMsg = "Relocation Failed";
            return false;
        }

        // Get entry point
        typedef int (*MainFunc)();
        MainFunc main_func = (MainFunc)tcc_get_symbol(state, "main");

        if (!main_func) {
            errorMsg = "Entry point 'main' not found";
            return false;
        }

        // Execute script with exception handling
        try {
            int ret = main_func();
            return (ret == 0);
        } catch (...) {
            errorMsg = "Runtime Exception in Script";
            return false;
        }
    }

    void cleanup() {
        if (state) {
            tcc_delete(state);
            state = nullptr;
        }
    }

private:
    void reRegisterSymbols() {
        // Re-register symbols...
    }
};
```

### Host API Implementation

```cpp
// Global handle manager
static std::map<int, std::shared_ptr<GeoFeature>> g_GeoRegistry;
static int g_NextHandle = 1000;

extern "C" {

    // Safe cylinder creation function
    __declspec(dllexport) int host_create_cylinder(float r, float h) {
        try {
            // Business logic
            auto feat = std::make_shared<CylinderFeature>(r, h, 32);
            
            // Allocate handle
            int handle = g_NextHandle++;
            g_GeoRegistry[handle] = feat;
            
            // Trigger host rebuild
            // g_ActiveModel->addFeature(feat); 
            
            return handle; // Return handle to script
        } catch (const std::exception& e) {
            fprintf(stderr, "Host Error: %s\n", e.what());
            return -1; // Return error code
        } catch (...) {
            fprintf(stderr, "Host Error: Unknown exception\n");
            return -1;
        }
    }

    // Safe parameter modification function
    __declspec(dllexport) int host_set_radius(int handle, float new_r) {
        try {
            // Validate handle
            auto it = g_GeoRegistry.find(handle);
            if (it == g_GeoRegistry.end()) {
                return -1; // Invalid handle
            }

            // Execute operation
            it->second->setParam("radius", new_r);
            
            // Trigger rebuild
            // g_ActiveModel->rebuild();
            
            return 0; // Success
        } catch (...) {
            return -1;
        }
    }
    
    // Simple log wrapper
    __declspec(dllexport) void log_msg(const char* msg) {
        printf("[Script] %s\n", msg);
    }
}
```

### Script Template

```c
// user_script.c

// Entry function
int main() {
    log_msg("Start generating geometry...");

    // Create cylinder, get handle
    int cyl_id = host_create_cylinder(5.0, 10.0);
    if (cyl_id < 0) {
        log_msg("Failed to create cylinder");
        return -1;
    }

    // Modify parameter
    host_set_radius(cyl_id, 8.0);

    log_msg("Geometry generated successfully.");
    return 0;
}
```

## Common Issues and Solutions

### Relocation Failure
- **Issue**: `tcc_relocate` crashes
- **Solution**: Always pass `NULL` to let TCC allocate executable memory automatically

### Missing Files
- **Issue**: "file not found: stdio.h" or "libtcc1.c not found"
- **Solution**: 
  - Verify `tcc_set_lib_path` uses absolute path
  - Check directory structure includes `include` and `lib` folders
  - Ensure proper path separator handling

### Security Concerns
- **Issue**: Scripts accessing system resources
- **Solution**: Never link system libraries or expose dangerous functions

### Debugging
- **Issue**: Script errors are hard to diagnose
- **Solution**: 
  - Use `tcc_set_error_func` to capture compilation errors
  - Add detailed logging with `log_msg`
  - Wrap script execution in try-catch

## Best Practices

1. **Path Configuration**: Always use absolute paths for TCC library files
2. **Relocation**: Always pass `NULL` to `tcc_relocate`
3. **API Exposure**: Minimize exposed functions and ensure exception safety
4. **Security**: Rely on architecture design rather than TCC's built-in bounds checking
5. **Error Handling**: Implement comprehensive error reporting for script execution

## Conclusion

By following this guide, you can successfully integrate LibTCC into your application, enabling powerful scriptable features while maintaining a secure execution environment. This approach is particularly well-suited for geometric modeling applications where users need to define custom geometry through C scripts.

The key to success is careful path configuration, proper memory management, and a robust handle-based communication system between the script and host application.