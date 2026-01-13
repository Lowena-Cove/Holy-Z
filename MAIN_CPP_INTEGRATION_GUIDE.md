# Holy Z Main.cpp Integration Guide

## Overview
This document provides step-by-step instructions to integrate the new system control functions into Main.cpp.

## Changes Required

### 1. Add Include Directive
**Location**: After line 39 (after other #include statements)

**Add:**
```cpp
#include "system_control.h"
```

**Full context:**
```cpp
#include "eval.h"
#include "strops.h"
#include "builtin.h"
#include "main.h"
#include "anyops.h"
#include "system_control.h"  // <-- ADD THIS LINE

#include "ZS.h"
```

---

### 2. Extend IsHolyCFunction() 
**Location**: Around line 195

**Find:**
```cpp
bool IsHolyCFunction(const string& funcName)
{
	// List of built-in Holy C functions
	static const vector<string> holyCFunctions = {
		"ToInt", "ToFloat", "ToStr", "ToBool",
		"typeof", "TypeOf", "typecheck", "TypeCheck", "istype", "IsType",
		"malloc", "Malloc", "free", "Free", "addressof", "AddressOf", "ptr",
		"deref", "Deref", "dereference", "setvalue", "SetValue",
		"send", "Send", "hasmethod", "HasMethod", "getmethod", "GetMethod",
		// Result/Option functions (Rust-like)
		"Ok", "Err", "Some", "None", "isOk", "IsOk", "isErr", "IsErr", 
		"isSome", "IsSome", "isNone", "IsNone", "unwrap", "Unwrap",
		"expect", "Expect", "unwrapOr", "UnwrapOr"
	};
```

**Replace with:**
```cpp
bool IsHolyCFunction(const string& funcName)
{
	// List of built-in Holy C functions
	static const vector<string> holyCFunctions = {
		"ToInt", "ToFloat", "ToStr", "ToBool",
		"typeof", "TypeOf", "typecheck", "TypeCheck", "istype", "IsType",
		"malloc", "Malloc", "free", "Free", "addressof", "AddressOf", "ptr",
		"deref", "Deref", "dereference", "setvalue", "SetValue",
		"send", "Send", "hasmethod", "HasMethod", "getmethod", "GetMethod",
		// Result/Option functions (Rust-like)
		"Ok", "Err", "Some", "None", "isOk", "IsOk", "isErr", "IsErr", 
		"isSome", "IsSome", "isNone", "IsNone", "unwrap", "Unwrap",
		"expect", "Expect", "unwrapOr", "UnwrapOr",
		// File System Functions (NEW)
		"fileRead", "fileWrite", "fileDelete", "fileExists", "listDir",
		// Process Functions (NEW)
		"executeCommand", "executeAsync", "killProcess", "getProcessInfo",
		// Environment Functions (NEW)
		"getEnv", "setEnv", "getWorkingDir", "setWorkingDir", "getUsername", "getHostname",
		// System Information Functions (NEW)
		"getCpuCount", "getTotalMemory", "getFreeMemory", "getOsName", "getOsVersion",
		"getProcessMemory", "getDiskSpace",
		// Memory Functions (NEW)
		"allocateMemory", "freeMemory", "memcpy", "memset",
		// Script Execution Functions (NEW)
		"runCSharp", "runCpp", "runPython", "runScript"
	};
```

---

### 3. Extend ExecuteHolyCFunction()
**Location**: Around line 494 (find the function definition)

**Find the end of ExecuteHolyCFunction()** (before the final closing brace and before the next function):

**Add these implementations before the closing brace:**

```cpp
	// ============================================================
	// FILE SYSTEM FUNCTIONS (NEW)
	// ============================================================
	else if (functionName == "fileRead")
	{
		if (args.empty()) return nullType;
		string path = AnyAsString(args.at(0));
		try {
			return FileSystem::fileRead(path);
		} catch (const HolyZException& e) {
			LogWarning("fileRead() error: " + string(e.what()));
			return "";
		}
	}
	else if (functionName == "fileWrite")
	{
		if (args.size() < 2) return false;
		string path = AnyAsString(args.at(0));
		string content = AnyAsString(args.at(1));
		try {
			return FileSystem::fileWrite(path, content);
		} catch (const HolyZException& e) {
			LogWarning("fileWrite() error: " + string(e.what()));
			return false;
		}
	}
	else if (functionName == "fileDelete")
	{
		if (args.empty()) return false;
		string path = AnyAsString(args.at(0));
		return FileSystem::fileDelete(path);
	}
	else if (functionName == "fileExists")
	{
		if (args.empty()) return false;
		string path = AnyAsString(args.at(0));
		return FileSystem::fileExists(path);
	}
	else if (functionName == "listDir")
	{
		if (args.empty()) return vector<string>();
		string path = AnyAsString(args.at(0));
		vector<string> files = FileSystem::listDir(path);
		// Return as boost::any vector
		return files;
	}
	
	// ============================================================
	// PROCESS FUNCTIONS (NEW)
	// ============================================================
	else if (functionName == "executeCommand")
	{
		if (args.empty()) return "";
		string cmd = AnyAsString(args.at(0));
		try {
			return Process::executeCommand(cmd);
		} catch (const HolyZException& e) {
			LogWarning("executeCommand() error: " + string(e.what()));
			return "";
		}
	}
	else if (functionName == "executeAsync")
	{
		if (args.empty()) return -1;
		string cmd = AnyAsString(args.at(0));
		try {
			return Process::executeAsync(cmd);
		} catch (const HolyZException& e) {
			LogWarning("executeAsync() error: " + string(e.what()));
			return -1;
		}
	}
	else if (functionName == "killProcess")
	{
		if (args.empty()) return false;
		int pid = AnyAsInt(args.at(0));
		return Process::killProcess(pid);
	}
	else if (functionName == "getProcessInfo")
	{
		if (args.empty()) return vector<string>();
		int pid = AnyAsInt(args.at(0));
		vector<string> info = Process::getProcessInfo(pid);
		return info;
	}
	
	// ============================================================
	// ENVIRONMENT FUNCTIONS (NEW)
	// ============================================================
	else if (functionName == "getEnv")
	{
		if (args.empty()) return "";
		string varName = AnyAsString(args.at(0));
		return Environment::getEnv(varName);
	}
	else if (functionName == "setEnv")
	{
		if (args.size() < 2) return false;
		string varName = AnyAsString(args.at(0));
		string value = AnyAsString(args.at(1));
		return Environment::setEnv(varName, value);
	}
	else if (functionName == "getWorkingDir")
	{
		return Environment::getWorkingDir();
	}
	else if (functionName == "setWorkingDir")
	{
		if (args.empty()) return false;
		string path = AnyAsString(args.at(0));
		return Environment::setWorkingDir(path);
	}
	else if (functionName == "getUsername")
	{
		return Environment::getUsername();
	}
	else if (functionName == "getHostname")
	{
		return Environment::getHostname();
	}
	
	// ============================================================
	// SYSTEM INFORMATION FUNCTIONS (NEW)
	// ============================================================
	else if (functionName == "getCpuCount")
	{
		return SystemInfo::getCpuCount();
	}
	else if (functionName == "getTotalMemory")
	{
		return SystemInfo::getTotalMemory();
	}
	else if (functionName == "getFreeMemory")
	{
		return SystemInfo::getFreeMemory();
	}
	else if (functionName == "getOsName")
	{
		return SystemInfo::getOsName();
	}
	else if (functionName == "getOsVersion")
	{
		return SystemInfo::getOsVersion();
	}
	else if (functionName == "getProcessMemory")
	{
		return SystemInfo::getProcessMemory();
	}
	else if (functionName == "getDiskSpace")
	{
		if (args.empty()) return -1LL;
		string path = AnyAsString(args.at(0));
		return SystemInfo::getDiskSpace(path);
	}
	
	// ============================================================
	// MEMORY FUNCTIONS (NEW)
	// ============================================================
	else if (functionName == "allocateMemory")
	{
		if (args.empty()) return nullType;
		long long size = AnyAsInt(args.at(0));
		void* ptr = Memory::allocateMemory(size);
		Pointer p;
		p.address = reinterpret_cast<long long>(ptr);
		p.size = size;
		return p;
	}
	else if (functionName == "freeMemory")
	{
		if (args.empty()) return false;
		try {
			Pointer ptr = any_cast<Pointer>(args.at(0));
			return Memory::freeMemory(reinterpret_cast<void*>(ptr.address));
		} catch (boost::bad_any_cast) {
			LogWarning("freeMemory() requires a pointer");
			return false;
		}
	}
	else if (functionName == "memcpy")
	{
		if (args.size() < 3) return false;
		try {
			Pointer dest = any_cast<Pointer>(args.at(0));
			Pointer src = any_cast<Pointer>(args.at(1));
			long long size = AnyAsInt(args.at(2));
			return Memory::memcpy(reinterpret_cast<void*>(dest.address), 
									   reinterpret_cast<void*>(src.address), 
									   size);
		} catch (boost::bad_any_cast) {
			LogWarning("memcpy() requires pointer arguments");
			return false;
		}
	}
	else if (functionName == "memset")
	{
		if (args.size() < 3) return false;
		try {
			Pointer dest = any_cast<Pointer>(args.at(0));
			int value = AnyAsInt(args.at(1));
			long long size = AnyAsInt(args.at(2));
			return Memory::memset(reinterpret_cast<void*>(dest.address), value, size);
		} catch (boost::bad_any_cast) {
			LogWarning("memset() requires pointer arguments");
			return false;
		}
	}
	
	// ============================================================
	// SCRIPT EXECUTION FUNCTIONS (Framework - to be implemented)
	// ============================================================
	else if (functionName == "runCSharp" || functionName == "runCpp" || 
	         functionName == "runPython" || functionName == "runScript")
	{
		LogWarning("Script execution functions not yet implemented");
		return "";
	}
```

---

## Validation Checklist

After making these changes:

- [ ] File compiles without errors
- [ ] No duplicate function definitions
- [ ] All 50+ functions listed in IsHolyCFunction()
- [ ] All function implementations in ExecuteHolyCFunction()
- [ ] Case-insensitive comparison for function names works
- [ ] Example scripts can be run
- [ ] System information queries work
- [ ] File operations work
- [ ] Process execution works
- [ ] Environment access works

---

## Building After Integration

```bash
cd HolyZ
cmake -B ../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../build --config Release
```

---

## Testing After Integration

```bash
# Test system information
../build/HolyZ examples/system_info.zs

# Test file operations
../build/HolyZ examples/file_operations.zs

# Test process execution
../build/HolyZ examples/process_execution.zs

# Test case-insensitivity
../build/HolyZ examples/case_insensitive.zs
```

---

## Troubleshooting

### Compilation Errors

**Error: "system_control.h not found"**
- Make sure system_control.h is in the same directory as Main.cpp

**Error: "Undefined reference to FileSystem::fileRead"**
- Make sure system_control.cpp is included in CMakeLists.txt
- Rebuild clean: `rm -rf build && mkdir build && cd build && cmake ..`

**Error: "ToLower function not found"**
- ToLower should be defined in strops.h
- Check that strops.h is properly included

### Runtime Errors

**"File not found" when calling fileRead()**
- Use absolute paths or ensure working directory is correct
- Test with getWorkingDir() first

**"Command not found" when calling executeCommand()**
- Make sure command exists on your system
- Use full path to executable if needed

**Memory issues with allocateMemory()**
- Check that size parameter is reasonable
- Always call freeMemory() to avoid leaks

---

## Integration Complete!

Once these changes are applied and tested, the Holy Z interpreter will have:
- ✅ Full system and device control
- ✅ File I/O operations
- ✅ Process management
- ✅ Environment access
- ✅ System information queries
- ✅ Performance monitoring
- ✅ Case-insensitive syntax throughout

**Estimated Integration Time**: 30-60 minutes
**Estimated Testing Time**: 1-2 hours
**Total Project Enhancement**: ~5-10 hours from start to fully tested
