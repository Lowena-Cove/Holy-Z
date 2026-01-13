// Example 3: System Command Execution
// File: examples/process_execution.zs

print "=== Holy Z Process Execution Example ==="
print ""

// Execute a simple command
#holyc on

print "Executing system command..."

// On Windows
string result = executeCommand("dir /B")

// On Linux/Mac, use: string result = executeCommand("ls -la")

print "Command output:"
print result

#holyc off

print ""
print "✓ Command executed successfully"

// Example of async execution
// int processId = executeAsync("notepad.exe")
// print "Process started with ID: " + processId
