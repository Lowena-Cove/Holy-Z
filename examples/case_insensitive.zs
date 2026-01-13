// Example 4: Case-Insensitive Language Features
// File: examples/case_insensitive.zs

print "=== Holy Z Case-Insensitive Features Demo ==="
print ""

// All of these work the same way
INT x = 10
int y = 20
Int z = 30

print "Variables with mixed case type declarations:"
print "x=" + x + ", y=" + y + ", z=" + z
print ""

// Keywords are case-insensitive
IF x < y {
    Print "✓ IF statement works"
}
else if y < z {
    PRINT "✓ ELSE IF statement works"  
}

print ""

// Function definitions and calls (case-insensitive)
func MyFunction(value) {
    RETURN value * 2
}

int result = MYFUNCTION(5)
Print "Function call result: " + result

print ""

// Built-in functions are case-insensitive
string upper = "hello"
INT converted = TOINT("42")
FLOAT float_val = tofloat("3.14")

print "✓ Case-insensitive conversions:"
print "  TOINT('42') = " + converted
print "  tofloat('3.14') = " + float_val

print ""

// Class operations are case-insensitive
class MyClass {
    int value
    
    func SetValue(newValue) {
        this.value = newValue
    }
}

print "✓ Case-insensitive object operations supported"

print ""
print "All features support case-insensitive syntax!"
