#pragma once

#include <string>
#include <vector>
#include <boost/any.hpp>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <chrono>

using namespace std;

// ============================================================
// System & Device Control Functions
// ============================================================

// File System Operations
namespace FileSystem {
    string fileRead(const string& path);
    bool fileWrite(const string& path, const string& content);
    bool fileDelete(const string& path);
    bool fileExists(const string& path);
    vector<string> listDir(const string& path);
    string getAbsolutePath(const string& path);
}

// Process Control
namespace Process {
    string executeCommand(const string& cmd);
    int executeAsync(const string& cmd);
    bool killProcess(int pid);
    vector<string> getProcessInfo(int pid);
}

// Environment Variables
namespace Environment {
    string getEnv(const string& varName);
    bool setEnv(const string& varName, const string& value);
    string getWorkingDir();
    bool setWorkingDir(const string& path);
    string getUsername();
    string getHostname();
}

// System Information
namespace SystemInfo {
    int getCpuCount();
    long long getTotalMemory();
    long long getFreeMemory();
    string getOsName();
    string getOsVersion();
    double getCpuUsage();
    long long getProcessMemory();
    long long getDiskSpace(const string& path);
}

// Advanced Memory Control
namespace Memory {
    void* allocateMemory(size_t size);
    bool freeMemory(void* ptr);
    bool memcpy(void* dest, const void* src, size_t size);
    bool memset(void* dest, int value, size_t size);
    long long getAddressValue(void* ptr);
}

// ============================================================
// C++ Embedding Support
// ============================================================

namespace CppEmbedding {
    // Embed and execute C++ code
    bool compileCppBlock(const string& cppCode, const string& outputPath);
    boost::any executeCompiledCpp(const string& modulePath, const string& functionName, 
                                  const vector<boost::any>& args);
    
    // Dynamic library loading
    void* loadDynamicLibrary(const string& path);
    void unloadDynamicLibrary(void* handle);
    void* getFunctionAddress(void* handle, const string& functionName);
}

// ============================================================
// Multi-Language Script Execution
// ============================================================

namespace ScriptExecution {
    string runCSharp(const string& scriptPath, const vector<string>& args = {});
    string runCpp(const string& scriptPath, const vector<string>& args = {});
    string runPython(const string& scriptPath, const vector<string>& args = {});
    string runScript(const string& scriptPath, const string& language, 
                    const vector<string>& args = {});
    
    // Helper to detect script type by extension
    string detectLanguage(const string& scriptPath);
}

// ============================================================
// Case-Insensitive Identifier Handling
// ============================================================

class IdentifierMap {
    // Maps lowercase identifiers to their original case-preserved forms
    unordered_map<string, string> caseMap;
    unordered_map<string, boost::any> valueMap;
    
public:
    void insert(const string& identifier, const boost::any& value);
    boost::any find(const string& identifier);
    bool exists(const string& identifier);
    void remove(const string& identifier);
    string getOriginalCase(const string& identifier);
    void clear();
};

// ============================================================
// Enhanced Error Handling
// ============================================================

class HolyZException : public exception {
    string message;
    int lineNumber;
    string context;
    
public:
    HolyZException(const string& msg, int line = 0, const string& ctx = "");
    virtual const char* what() const noexcept;
    int getLineNumber() const { return lineNumber; }
    string getContext() const { return context; }
};

// Safe wrapper for risky operations
template<typename T>
class Result {
    bool success;
    T value;
    string error;
    
public:
    Result(const T& val) : success(true), value(val), error("") {}
    Result(const string& err) : success(false), error(err) {}
    
    bool isOk() const { return success; }
    T getValue() const { return value; }
    string getError() const { return error; }
};

// ============================================================
// Performance Monitoring
// ============================================================

namespace Performance {
    class Timer {
        chrono::high_resolution_clock::time_point start;
        
    public:
        Timer();
        long long elapsedMs() const;
        long long elapsedUs() const;
        void reset();
    };
    
    struct ExecutionStats {
        long long totalTime;
        long long parseTime;
        long long executeTime;
        int functionCallCount;
        int memoryAllocations;
        long long peakMemory;
    };
    
    extern ExecutionStats currentStats;
    void recordFunctionCall();
    void recordMemoryAllocation(long long size);
}

// ============================================================
// Inline Implementations
// ============================================================

inline Performance::Timer::Timer() {
    start = chrono::high_resolution_clock::now();
}

inline long long Performance::Timer::elapsedMs() const {
    auto now = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now - start).count();
}

inline long long Performance::Timer::elapsedUs() const {
    auto now = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::microseconds>(now - start).count();
}

inline void Performance::Timer::reset() {
    start = chrono::high_resolution_clock::now();
}

inline HolyZException::HolyZException(const string& msg, int line, const string& ctx)
    : message(msg), lineNumber(line), context(ctx) {}

inline const char* HolyZException::what() const noexcept {
    return message.c_str();
}
