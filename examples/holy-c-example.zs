// Holy Z Example Script - Demonstrates Holy C features
#holyc on

func Main()
{
    "Hello from Holy Z!\n";  // Auto-prints string literals
    
    int i = 5;
    
    // Range operators (Holy C style)
    if (0 < i < 10) 
    {
        Printl("i is in range 0-10");
    }
    
    // Exponentiation operator
    int result = 2 ^ 3;
    Printl("2^3 = " + ToStr(result));
    
    // Holy C type conversion functions
    float f = ToFloat(42);
    string s = ToStr(f);
    
    Printl("Converted: " + s);
}
