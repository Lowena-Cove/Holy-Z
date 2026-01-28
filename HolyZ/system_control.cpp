// system_control.cpp - Implementation of system and device control functions

// Prevent std::byte conflict with Windows headers in C++17
// Must be defined BEFORE any standard library headers are included
#if defined(_WIN32) || defined(_WIN64)
    #define _HAS_STD_BYTE 0
#endif

#include "system_control.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <psapi.h>
    #include <winreg.h>
    #pragma comment(lib, "psapi.lib")
    #pragma comment(lib, "advapi32.lib")
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <signal.h>
    #include <sys/utsname.h>
#endif

// Performance stats
namespace Performance {
    ExecutionStats currentStats = {0, 0, 0, 0, 0, 0};
    
    void recordFunctionCall() {
        currentStats.functionCallCount++;
    }
    
    void recordMemoryAllocation(long long size) {
        currentStats.memoryAllocations++;
        if (size > currentStats.peakMemory) {
            currentStats.peakMemory = size;
        }
    }
}

// File System Implementation
namespace FileSystem {
    string fileRead(const string& path) {
        try {
            ifstream file(path);
            if (!file.is_open()) {
                throw HolyZException("Cannot open file: " + path);
            }
            stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        } catch (const exception& e) {
            throw HolyZException("File read error: " + string(e.what()));
        }
    }
    
    bool fileWrite(const string& path, const string& content) {
        try {
            ofstream file(path);
            if (!file.is_open()) {
                cerr << "Cannot open file for writing: " << path << endl;
                return false;
            }
            file << content;
            file.close();
            return true;
        } catch (const exception& e) {
            cerr << "File write error: " << e.what() << endl;
            return false;
        }
    }
    
    bool fileDelete(const string& path) {
        return remove(path.c_str()) == 0;
    }
    
    bool fileExists(const string& path) {
        ifstream f(path);
        return f.good();
    }
    
    vector<string> listDir(const string& path) {
        vector<string> files;
#if defined(_WIN32) || defined(_WIN64)
        WIN32_FIND_DATAA findFileData;
        HANDLE findHandle = FindFirstFileA((path + "\\*").c_str(), &findFileData);
        if (findHandle == INVALID_HANDLE_VALUE) return files;
        
        do {
            if (string(findFileData.cFileName) != "." && string(findFileData.cFileName) != "..") {
                files.push_back(findFileData.cFileName);
            }
        } while (FindNextFileA(findHandle, &findFileData));
        
        FindClose(findHandle);
#else
        DIR* dir = opendir(path.c_str());
        if (!dir) return files;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (string(entry->d_name) != "." && string(entry->d_name) != "..") {
                files.push_back(entry->d_name);
            }
        }
        closedir(dir);
#endif
        return files;
    }
    
    string getAbsolutePath(const string& path) {
#if defined(_WIN32) || defined(_WIN64)
        char buffer[MAX_PATH];
        if (GetFullPathNameA(path.c_str(), MAX_PATH, buffer, nullptr)) {
            return string(buffer);
        }
#else
        char buffer[PATH_MAX];
        if (realpath(path.c_str(), buffer)) {
            return string(buffer);
        }
#endif
        return path;
    }
}

// Process Control Implementation
namespace Process {
    string executeCommand(const string& cmd) {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        FILE* pipe = _popen((cmd + " 2>&1").c_str(), "r");
#else
        FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
#endif
        
        if (!pipe) {
            cerr << "Failed to execute command: " << cmd << endl;
            return "";
        }
        
        string result;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        
#if defined(_WIN32) || defined(_WIN64)
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        
        return result;
    }
    
    int executeAsync(const string& cmd) {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        STARTUPINFOA si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        
        if (CreateProcessA(nullptr, (char*)cmd.c_str(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            int processId = pi.dwProcessId;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return processId;
        }
        return -1;
#else
        pid_t pid = fork();
        if (pid == 0) {
            execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
            exit(127);
        }
        return (int)pid;
#endif
    }
    
    bool killProcess(int pid) {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hProcess) return false;
        bool result = TerminateProcess(hProcess, 1) != 0;
        CloseHandle(hProcess);
        return result;
#else
        return kill(pid, SIGTERM) == 0;
#endif
    }
    
    vector<string> getProcessInfo(int pid) {
        vector<string> info;
#if defined(_WIN32) || defined(_WIN64)
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            char processName[MAX_PATH];
            if (GetModuleBaseNameA(hProcess, nullptr, processName, sizeof(processName))) {
                info.push_back(string(processName));
            }
            CloseHandle(hProcess);
        }
#else
        info.push_back("PID: " + to_string(pid));
#endif
        return info;
    }
}

// Environment Implementation
namespace Environment {
    string getEnv(const string& varName) {
        Performance::recordFunctionCall();
        const char* value = getenv(varName.c_str());
        return value ? string(value) : "";
    }
    
    bool setEnv(const string& varName, const string& value) {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        return SetEnvironmentVariableA(varName.c_str(), value.c_str()) != 0;
#else
        return setenv(varName.c_str(), value.c_str(), 1) == 0;
#endif
    }
    
    string getWorkingDir() {
        Performance::recordFunctionCall();
        
        char buffer[256];
#if defined(_WIN32) || defined(_WIN64)
        if (GetCurrentDirectoryA(sizeof(buffer), buffer)) {
            return string(buffer);
        }
#else
        if (getcwd(buffer, sizeof(buffer))) {
            return string(buffer);
        }
#endif
        return "";
    }
    
    bool setWorkingDir(const string& path) {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        return SetCurrentDirectoryA(path.c_str()) != 0;
#else
        return chdir(path.c_str()) == 0;
#endif
    }
    
    string getUsername() {
        Performance::recordFunctionCall();
        return getEnv("USERNAME");
    }
    
    string getHostname() {
        Performance::recordFunctionCall();
        return getEnv("COMPUTERNAME");
    }
}

// System Information Implementation
namespace SystemInfo {
    int getCpuCount() {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwNumberOfProcessors;
#else
        return sysconf(_SC_NPROCESSORS_ONLN);
#endif
    }
    
    long long getTotalMemory() {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);
        return statex.ullTotalPhys;
#else
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
#endif
    }
    
    long long getFreeMemory() {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);
        return statex.ullAvailPhys;
#else
        long pages = sysconf(_SC_AVPHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
#endif
    }
    
    string getOsName() {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        return "Windows";
#elif defined(__APPLE__)
        return "macOS";
#elif defined(__linux__)
        return "Linux";
#else
        return "Unknown";
#endif
    }
    
    string getOsVersion() {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        DWORD dwVersion = GetVersion();
        return "Windows " + to_string(LOBYTE(LOWORD(dwVersion))) + "." + 
               to_string(HIBYTE(LOWORD(dwVersion)));
#else
        struct utsname uts;
        if (uname(&uts) == 0) {
            return string(uts.sysname) + " " + string(uts.release);
        }
        return "Unknown";
#endif
    }
    
    double getCpuUsage() {
        Performance::recordFunctionCall();
        // Simplified implementation - returns 0.0 for now
        return 0.0;
    }
    
    long long getProcessMemory() {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        HANDLE hProcess = GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize;
        }
#else
        // Unix implementation would require parsing /proc/self/status
#endif
        return 0;
    }
    
    long long getDiskSpace(const string& path) {
        Performance::recordFunctionCall();
        
#if defined(_WIN32) || defined(_WIN64)
        ULARGE_INTEGER freeBytesAvailable;
        if (GetDiskFreeSpaceExA(path.c_str(), &freeBytesAvailable, nullptr, nullptr)) {
            return freeBytesAvailable.QuadPart;
        }
#endif
        return -1;
    }
}

// Memory Control Implementation
namespace Memory {
    void* allocateMemory(size_t size) {
        Performance::recordFunctionCall();
        Performance::recordMemoryAllocation(size);
        return malloc(size);
    }
    
    bool freeMemory(void* ptr) {
        Performance::recordFunctionCall();
        if (ptr) {
            free(ptr);
            return true;
        }
        return false;
    }
    
    bool memcpy(void* dest, const void* src, size_t size) {
        Performance::recordFunctionCall();
        if (dest && src) {
            ::memcpy(dest, src, size);
            return true;
        }
        return false;
    }
    
    bool memset(void* dest, int value, size_t size) {
        Performance::recordFunctionCall();
        if (dest) {
            ::memset(dest, value, size);
            return true;
        }
        return false;
    }
    
    long long getAddressValue(void* ptr) {
        return reinterpret_cast<long long>(ptr);
    }
}

// Identifier Map Implementation
void IdentifierMap::insert(const string& identifier, const boost::any& value) {
    string normalized = identifier;
    // Convert to lowercase for internal key
    transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    caseMap[normalized] = identifier;
    valueMap[normalized] = value;
}

boost::any IdentifierMap::find(const string& identifier) {
    string normalized = identifier;
    transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    auto it = valueMap.find(normalized);
    return it != valueMap.end() ? it->second : boost::any();
}

bool IdentifierMap::exists(const string& identifier) {
    string normalized = identifier;
    transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    return valueMap.find(normalized) != valueMap.end();
}

void IdentifierMap::remove(const string& identifier) {
    string normalized = identifier;
    transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    caseMap.erase(normalized);
    valueMap.erase(normalized);
}

string IdentifierMap::getOriginalCase(const string& identifier) {
    string normalized = identifier;
    transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    auto it = caseMap.find(normalized);
    return it != caseMap.end() ? it->second : identifier;
}

void IdentifierMap::clear() {
    caseMap.clear();
    valueMap.clear();
}
