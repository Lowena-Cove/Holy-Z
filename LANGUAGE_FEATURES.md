# Holy-Z Language Features

Holy-Z combines features from multiple modern and classic systems programming languages, including Holy C, Zen-C, C++, C, and Rust.

## Holy C-Inspired Features

### Type Aliases (TempleOS Style)
Holy-Z supports Holy C-style type aliases for precise integer types:

```holyz
U0 myFunc();           // void function (zero-size type)
I8 smallNum = 42;      // 8-bit signed integer
U8 byte = 255;         // 8-bit unsigned integer
I16 shortNum = 1000;   // 16-bit signed integer
U16 port = 8080;       // 16-bit unsigned integer
I32 count = 1000000;   // 32-bit signed integer
U32 id = 42;           // 32-bit unsigned integer
I64 bigNum = 999999999;// 64-bit signed integer
U64 uuid = 12345;      // 64-bit unsigned integer
F64 pi = 3.14159;      // 64-bit floating point
```

### Holy C Mode
Enable/disable Holy C-style syntax with pragma:
```holyz
#holyc on     // Enable Holy C mode
#holyc off    // Disable Holy C mode
```

## Rust-Inspired Features

### Result Type
Robust error handling with Result type:
```holyz
// Create successful result
Result r = Ok(42);

// Create error result
Result err = Err("Something went wrong", "ErrorType");

// Check if result is ok
if (r.isOk()) {
    print(r.value);
}

// Get error message
if (err.isError()) {
    print(err.error);
}
```

### Option Type
Handle optional values safely:
```holyz
// Create Some value
Option opt = Some(42);

// Create None value
Option none = None();

// Check if value exists
if (opt.isSome()) {
    print(opt.value);
}
```

## Zen-C Inspired Features

### Pattern Matching (Planned)
```holyz
match value {
    0 => print("Zero"),
    1 => print("One"),
    _ => print("Other")
}
```

### Defer Statements (Planned)
Automatic cleanup with defer:
```holyz
func example() {
    var file = open("data.txt");
    defer close(file);  // Automatically called when function exits
    // ...use file...
}
```

### Type Inference (Partial Support)
```holyz
var x = 42;          // Type inferred as int
var msg = "Hello";   // Type inferred as string
```

## C++ Features

### Classes and Inheritance
```holyz
class Base {
    int value;
}

class Derived : Base {
    string name;
}
```

### Method Calls
```holyz
obj.method(arg1, arg2);
```

## Core Language Features

### Variables
```holyz
int x = 42;
float pi = 3.14;
string msg = "Hello";
bool flag = true;
```

### Functions
```holyz
func add(int a, int b) {
    return a + b;
}
```

### Control Flow
```holyz
// If statements
if (condition) {
    // code
}

// While loops
while (condition) {
    // code
}

// For loops (via while)
int i = 0;
while (i < 10) {
    print(i);
    i = i + 1;
}
```

### Global Variables
```holyz
global int counter = 0;
```

## Graphics Support (Optional)

When built with `-DHOLYZ_ENABLE_GRAPHICS=ON`:

```holyz
Sprite sprite = Sprite("image.png");
Vec2 pos = Vec2(100, 200);
Text txt = Text("Hello", 16);
```

## Type System Summary

| Holy C Type | Standard C Type | Size |
|-------------|----------------|------|
| U0 | void | 0 bits |
| I8 | int8_t | 8 bits signed |
| U8 | uint8_t | 8 bits unsigned |
| I16 | int16_t | 16 bits signed |
| U16 | uint16_t | 16 bits unsigned |
| I32 | int32_t | 32 bits signed |
| U32 | uint32_t | 32 bits unsigned |
| I64 | int64_t | 64 bits signed |
| U64 | uint64_t | 64 bits unsigned |
| F64 | double | 64 bits float |

## Future Enhancements

Planned features inspired by modern languages:
- Full pattern matching (Zen-C, Rust)
- Defer statements (Zen-C)
- Traits/Interfaces (Rust, C++)
- Async/await (Zen-C, modern C++)
- String interpolation (Zen-C)
- Generics (C++, Rust, Zen-C)
- Improved type inference (Zen-C, Rust)
- Memory safety features (Rust)
- RAII and smart pointers (C++, Zen-C)

## Philosophy

Holy-Z aims to provide:
- **Safety** from Rust (Result/Option types, type safety)
- **Simplicity** from Holy C (clean syntax, direct types)
- **Power** from C++ (classes, methods, control)
- **Expressiveness** from Zen-C (modern syntax, inference)
- **Performance** from C (compiled, low-level access)
