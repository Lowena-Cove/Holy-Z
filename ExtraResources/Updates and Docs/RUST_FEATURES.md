# Holy Z (Z+) - Rust-Inspired Features

Holy Z has been enhanced with several Rust-inspired features to provide stronger thread safety guarantees, cleaner error handling, and more flexible trait system.

## Result<T, E> Type - Error Handling Without Exceptions

Instead of exceptions, Holy Z now supports Rust-style `Result` types for explicit error handling:

```holyc
// Create a successful result
let success = Ok(42);

// Create an error result
let failure = Err("Something went wrong", "IOError");

// Check if Result is Ok
if isOk(success) {
    print("Success!");
}

// Unwrap with panic on error
let value = unwrap(success);

// Unwrap with default value
let value = unwrapOr(failure, 0);

// Unwrap with custom error message
let value = expect(result, "Expected a valid value");
```

### Result Operations
- `Ok(value)` - Create successful Result
- `Err(message, type)` - Create error Result  
- `isOk(result)` - Check if Result is success
- `isErr(result)` - Check if Result is error
- `unwrap(result)` - Get value or panic
- `unwrapOr(result, default)` - Get value or default
- `expect(result, message)` - Get value or panic with message

## Option<T> Type - Safe Null Handling

Holy Z now supports `Option` types for safe handling of optional values:

```holyc
// Create Option with value
let some_val = Some(42);

// Create empty Option
let none_val = None();

// Check if Option contains value
if isSome(some_val) {
    print("Has value");
}

// Use isNone to check for absence
if isNone(none_val) {
    print("No value");
}
```

### Option Operations
- `Some(value)` - Create Option with value
- `None()` - Create empty Option
- `isSome(option)` - Check if contains value
- `isNone(option)` - Check if empty

## Immutability by Default - let vs mut

Holy Z now supports Rust-style variable binding with immutability by default:

```holyc
// Immutable binding (default, like Rust's let)
let x = 42;
// x = 50;  // ERROR: Cannot modify immutable variable

// Mutable binding (explicit with mut keyword)
mut y = 42;
y = 50;  // OK: Mutable variables can be changed

// Traditional type declarations still work but create mutable variables
int z = 42;
z = 50;  // OK: Mutable
```

### Variable Binding Rules
1. `let varName = value;` - Creates **immutable** variable
2. `mut varName = value;` - Creates **mutable** variable  
3. `type varName = value;` - Traditional syntax, creates mutable variable
4. Attempting to modify a `let` variable causes a compile/runtime error

## Enhanced Trait System

Holy Z now supports more flexible traits similar to Rust traits:

```holyc
trait Iterator {
    next();
    hasNext();
    // Default implementations can be provided
}

// Implement trait for custom type
impl Iterator for MyCollection {
    next() {
        // Custom implementation
    }
    
    hasNext() {
        // Custom implementation
    }
}
```

### Trait Features
- **Default Implementations**: Provide default method bodies in traits
- **Multiple Implementations**: Implement same trait for multiple types
- **Associated Types**: Types associated with trait methods
- **Structural Typing**: Traits are more structural than nominal

## Borrowing and References (Simplified)

Holy Z supports simplified borrowing semantics:

```holyc
// Create a reference (immutable borrow)
let original = 42;
let ref = addressof(original);

// Dereference to access value
let value = deref(ref);

// Move semantics with ownership
let x = Some(42);
let y = x;  // x is moved, x is no longer valid
```

## Thread Safety Improvements

- **Immutable by default**: Reduces accidental mutations in concurrent code
- **Result types**: Forces explicit error handling in multi-threaded contexts
- **Message Passing**: `send()` function for safe inter-thread communication
- **Borrowing Rules**: Prevent data races through reference tracking

## Type System Enhancements

New types added to Holy Z's type system:
- `Result` - Result<T, E> type for error handling
- `Option` - Option<T> type for optional values

Enhanced runtime type checking:
```holyc
// Check type at runtime
if typecheck("Result", my_value) {
    print("Value is a Result");
}

// Get type as string
let t = typeof(my_value);
```

## Trait System (Advanced)

Global trait definitions and implementations:
- `globalTraits`: Registry of trait definitions
- `globalTraitImpls`: Trait implementations for types
- Default method implementations reduce boilerplate
- Full trait inheritance support planned

## Migration Guide

### From C++ Exception Handling
```holyc
// OLD: Exception handling
try {
    // code that might throw
} catch (exception e) {
    // handle error
}

// NEW: Result-based error handling
let result = someFunction();
if isErr(result) {
    let error = unwrapOr(result, "Unknown error");
    // handle error
}
```

### From Null Pointers
```holyc
// OLD: Null checks
if (ptr != null) {
    // use ptr
}

// NEW: Option types
let opt = Some(value);
if isSome(opt) {
    let val = unwrap(opt);
    // use val
}
```

### From Mutable-by-Default
```holyc
// OLD: All variables mutable
int x = 42;
x = 50;  // Always allowed

// NEW: Immutable by default
let x = 42;
// x = 50;  // Compile error!
mut y = 42;
y = 50;  // Explicitly mutable
```

## Benefits

1. **Memory Safety**: Immutable by default prevents unintended mutations
2. **Thread Safety**: Owned values and borrowing prevent data races
3. **Error Handling**: Result types force explicit error handling
4. **Null Safety**: Option types eliminate null pointer errors
5. **Code Clarity**: Intent is explicit (let vs mut, Ok vs Err)
6. **Expressiveness**: Traits provide flexible abstraction mechanisms

## Future Enhancements

Planned Rust-like features for future versions:
- Generic types with bounds
- Lifetime annotations
- Destructuring
- Pattern matching for Results/Options
- async/await for async operations
- Type inference improvements
- More complete standard library with iterators

## Compatibility Notes

- Existing code continues to work unchanged
- Traditional type declarations (`int x = 5;`) remain valid
- Mix Rust-style and traditional syntax in same code
- Gradual migration path available
