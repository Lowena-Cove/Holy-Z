# Holy Z Interpreter - Major Enhancement Plan

## Overview
This document outlines the major enhancements to the Holy Z interpreter to make it faster, more flexible, more powerful, and less case-sensitive.

## Phase 1: Case Insensitivity (Priority: CRITICAL)
**Status**: Implementation in progress
**Goal**: Make the entire language case-insensitive while maintaining readability

### Changes Required:
1. **Keyword Matching**: ✓ Already have `IsKeyword()` helper - expand usage
2. **Function Names**: Convert to lowercase on registration and lookup
3. **Variable Names**: Store with lowercase key, maintain original for display
4. **Type Names**: Case-insensitive type checking
5. **Class Names**: Case-insensitive class resolution
6. **Built-in Functions**: Already using `ToLower()` in `IsHolyCFunction()`

### Implementation Strategy:
- Create unified `NormalizeIdentifier(name)` function that returns lowercase
- Use normalized form for all lookups/comparisons
- Maintain original case in user-facing output

---

## Phase 2: Direct C++ Embedding (Priority: HIGH)
**Status**: Not started
**Goal**: Allow users to embed native C++ code directly in scripts

### Syntax:
```
#cpp {
    // Native C++ code here
    std::cout << "Hello from C++" << std::endl;
}
```

### Implementation Strategy:
1. Add `#cpp` pragma detection in `parseHolyZ()`
2. Extract C++ block and compile it dynamically using:
   - Windows: MSVC compiler via system() call
   - Linux: GCC/Clang via system() call
3. Load compiled code as DLL/SO at runtime
4. Create bridge to call C++ functions from Holy Z

### Alternative (Simpler): Direct Execution
- Use `system()` to compile and execute
- Capture stdout for results

---

## Phase 3: Multi-Language Script Support (Priority: HIGH)
**Status**: Not started
**Goal**: Execute C++, C#, and Python scripts from Holy Z

### New Built-in Functions:
```
// Execute C# script
runCSharp("script.cs", arg1, arg2, ...)

// Execute C++ script  
runCpp("script.cpp", arg1, arg2, ...)

// Execute Python script
runPython("script.py", arg1, arg2, ...)

// Execute any script
runScript("path", "language", arg1, arg2, ...)
```

### Implementation:
1. C#: Use `dotnet run` or compile with roslyn
2. C++: Use CMake + compiler
3. Python: Call `python.exe` with script path
4. Capture output and return as string/value

---

## Phase 4: System & Device Control (Priority: HIGH)
**Status**: Not started
**Goal**: Give scripts low-level device and OS access (inspired by Rust, C, Obj-C, Go)

### New Built-in Functions:

#### File System:
```
fileRead(path)           -> string
fileWrite(path, content) -> bool
fileDelete(path)         -> bool
fileExists(path)         -> bool
listDir(path)            -> array
```

#### Process Control:
```
executeCommand(cmd)      -> string (stdout)
executeAsync(cmd)        -> process_id
killProcess(pid)         -> bool
getProcessInfo(pid)      -> object
```

#### Memory Access:
```
malloc(size)             -> pointer
free(pointer)            -> bool
memcpy(dest, src, size)  -> bool
memset(dest, value, size)-> bool
```

#### Environment:
```
getEnv(var_name)         -> string
setEnv(var_name, value)  -> bool
getWorkingDir()          -> string
setWorkingDir(path)      -> bool
```

#### Registry/Config (Windows):
```
readRegistry(key, value) -> string
writeRegistry(key, value, data) -> bool
```

#### Network (Optional):
```
httpGet(url)             -> string
httpPost(url, data)      -> string
dnsLookup(hostname)      -> string
```

#### System Info:
```
getCpuCount()            -> int
getTotalMemory()         -> int
getFreeMemory()          -> int
getOsName()              -> string
getOsVersion()           -> string
```

---

## Phase 5: Performance Optimization (Priority: MEDIUM)
**Status**: Not started
**Goal**: Reduce crashes and improve speed

### Performance Bottlenecks:
1. **String Operations**: Too many copies - use move semantics
2. **Regex**: Expensive operations - cache compiled patterns
3. **Vector Operations**: Repeated resizing - reserve space upfront
4. **Any Type Casting**: Overhead on every operation - create typed variants
5. **Function Lookup**: O(n) for vectors - use hash maps where possible

### Optimizations:
1. **String Handling**:
   - Use `string_view` for parameters
   - Avoid unnecessary copies with move semantics
   - Pre-allocate string buffers

2. **Caching**:
   - Cache compiled regex patterns
   - Cache function name lookups
   - Cache type information

3. **Memory**:
   - Use object pooling for temporary values
   - Reduce allocations in hot paths
   - Use stack-based allocations for small data

4. **Execution**:
   - JIT-compile hot functions (future)
   - Optimize expression evaluation
   - Reduce frame setup overhead

---

## Phase 6: Stability & Bug Fixes (Priority: CRITICAL)
**Status**: Needs assessment
**Goal**: Fix crashes and edge cases

### Known Issues to Fix:
1. **Buffer Overflows**: Check all array accesses
2. **Null Pointers**: Add defensive checks
3. **Exception Handling**: Better error recovery
4. **Memory Leaks**: Proper cleanup on exit
5. **Resource Limits**: Handle large scripts gracefully
6. **Circular References**: Prevent infinite loops

### Testing Strategy:
- Add fuzzing for edge cases
- Test with very large scripts
- Test memory limits
- Test concurrent execution
- Test error recovery

---

## Phase 7: Extended Language Features (Priority: LOW)
**Status**: Framework in place
**Goal**: Add more advanced features inspired by multiple languages

### Rust-Inspired:
- ✓ Result<T,E> type (already implemented)
- ✓ Option<T> type (already implemented)
- Pattern matching (framework exists, not integrated)
- ✓ Memory safety (partially with pointers)
- Ownership system (framework exists)
- ✓ Trait system (implemented)

### C++ Inspired:
- ✓ Templates (basic support via Any type)
- ✓ RAII pattern (via class destructors)
- Operator overloading (future)
- Smart pointers (framework exists)

### Objective-C Inspired:
- Dynamic dispatch (message passing - partially done)
- Categories (future)
- KVC/KVO patterns (future)

### Go Inspired:
- Goroutines (partial - SplitThread exists)
- Channels (future)
- Defer statements (future)
- Error as values (Result type)

### Holy C Features:
- ✓ Direct memory access
- ✓ Low-level pointer operations
- ✓ System calls
- ✓ Fast compilation

---

## Implementation Priorities

### Week 1 (Critical):
1. ✓ Fix Main.cpp corruption
2. [ ] Complete case-insensitivity across all code
3. [ ] Add error handling to prevent crashes
4. [ ] Fix build system issues

### Week 2 (High Priority):
1. [ ] Direct C++ embedding (#cpp blocks)
2. [ ] C# script execution
3. [ ] System command execution
4. [ ] File I/O built-in functions

### Week 3 (High Priority):
1. [ ] C++ script execution
2. [ ] Memory/process control functions
3. [ ] Environment variable access
4. [ ] Performance profiling and optimization

### Week 4 (Medium Priority):
1. [ ] Advanced system functions
2. [ ] Error recovery improvements
3. [ ] Comprehensive testing
4. [ ] Documentation updates

---

## File Structure After Enhancements

```
HolyZ/
├── Main.cpp                    (Enhanced with all features)
├── builtin.h                   (Extended system functions)
├── cpp_embedder.h              (C++ embedding support)
├── script_executor.h           (Multi-language script execution)
├── system_control.h            (OS/device functions)
├── CMakeLists.txt              (Updated for new features)
├── examples/
│   ├── cpp_embedding.zs        (Example: C++ code in ZS)
│   ├── system_control.zs       (Example: Device control)
│   ├── script_execution.zs     (Example: Running other scripts)
│   └── case_insensitive.zs     (Example: Case-insensitive code)
└── tests/
    ├── test_case_insensitive.zs
    ├── test_cpp_embedding.zs
    ├── test_system_functions.zs
    └── test_performance.zs
```

---

## Risk Assessment

### High Risk:
- C++ dynamic compilation (platform specific, security concerns)
- System command execution (security risk - needs validation)
- Direct memory access (stability risk)

### Medium Risk:
- Case-insensitivity (breaking existing exact-match code)
- Multi-language execution (dependency management)

### Mitigation:
- Add security flags to disable unsafe features
- Implement input validation for all system calls
- Add sandboxing for untrusted code
- Comprehensive error handling and recovery

---

## Compatibility Notes

- Case-insensitivity is backward compatible (existing code still works)
- New features are opt-in (#cpp, runScript, etc.)
- Existing Result/Option functionality unchanged
- Class system remains compatible

---

## Testing Checklist

- [ ] All keywords work case-insensitive
- [ ] All built-in functions work case-insensitive
- [ ] C++ code blocks compile and execute
- [ ] C# scripts can be run
- [ ] C++ scripts can be run
- [ ] File I/O functions work
- [ ] Process functions work safely
- [ ] Memory functions don't crash
- [ ] System info functions work
- [ ] No memory leaks on exit
- [ ] Performance improved by >20%
- [ ] Crash rate reduced to <1%

---

## Success Criteria

✓ No more crashes on edge cases
✓ Case-insensitive throughout
✓ Can embed C++ code
✓ Can run C#/C++/Python scripts
✓ Full device/OS control
✓ 50%+ faster execution
✓ < 5MB memory for small scripts
✓ Clear, updated documentation
