// Example 1: System Information and Device Control
// File: examples/system_info.zs

print "=== Holy Z System Information Script ==="
print ""

// Get OS information
print "Operating System:"
print "  Name: " + getOsName()
print "  Version: " + getOsVersion()
print ""

// Get CPU and Memory information
print "Hardware:"
print "  CPU Cores: " + getCpuCount()
print "  Total Memory: " + (getTotalMemory() / 1024 / 1024) + " MB"
print "  Free Memory: " + (getFreeMemory() / 1024 / 1024) + " MB"
print "  Process Memory: " + (getProcessMemory() / 1024) + " KB"
print ""

// Get environment information
print "Environment:"
print "  Username: " + getUsername()
print "  Hostname: " + getHostname()
print "  Working Directory: " + getWorkingDir()
print ""

// Environment variable access
string pathVar = getEnv("PATH")
print "PATH variable (first 100 chars): " + pathVar.substr(0, 100)
