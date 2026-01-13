// Example 2: File I/O Operations
// File: examples/file_operations.zs

print "=== Holy Z File I/O Operations ==="
print ""

// Create and write a file
string testFile = "example.txt"
string content = "Hello, Holy Z!\nThis is a test file.\nWe support case-insensitive operations!"

if fileWrite(testFile, content) {
    print "✓ File written successfully"
} else {
    print "✗ Failed to write file"
}

print ""

// Check if file exists
IF fileExists(testFile) {
    print "✓ File exists"
    
    // Read the file
    string fileContent = fileRead(testFile)
    print "File contents:"
    print fileContent
} ELSE {
    print "✗ File does not exist"
}

print ""

// List directory
PRINT "Files in current directory:"
array files = listDir(".")
for i = 0; i < files.length; i++ {
    Print "  - " + files[i]
}

print ""

// Demonstrate case-insensitive keywords
string testVar = "case insensitive"
IF testVar == "case insensitive" {
    PRINT "✓ Case-insensitive comparison works"
}

// Clean up
fileDelete(testFile)
PRINT "✓ Test file deleted"
