// Holy Z - Rust-Inspired Features Example
// Demonstrates Result, Option, and Rust-like patterns

// Example 1: Result Type for Error Handling
func divideWithResult(int a, int b) {
    if b == 0 {
        // Return error Result
        return Err("Division by zero", "MathError");
    }
    // Return success Result
    return Ok(a / b);
}

// Example 2: Option Type for Optional Values
func findElement(string value) {
    // In a real scenario, this would search a collection
    if value == "target" {
        return Some(42);  // Found it!
    }
    return None();  // Not found
}

// Example 3: Using Results
print("=== Result Examples ===";

let result1 = divideWithResult(10, 2);
if isOk(result1) {
    let quotient = unwrap(result1);
    print("10 / 2 = " + quotient);
}

let result2 = divideWithResult(10, 0);
if isErr(result2) {
    print("Error occurred in division");
}

// Unwrap with default value
let safe_result = unwrapOr(result2, -1);
print("Safe result: " + safe_result);

// Example 4: Using Options
print("=== Option Examples ===";

let found = findElement("target");
if isSome(found) {
    let value = unwrap(found);
    print("Found value: " + value);
}

let notFound = findElement("missing");
if isNone(notFound) {
    print("Element not found in collection");
}

// Example 5: Immutable and Mutable Variables
print("=== Variable Binding Examples ===";

// Immutable by default (Rust-style)
let immutable_x = 42;
print("Immutable x: " + immutable_x);
// immutable_x = 50;  // ERROR: Cannot modify immutable variable

// Explicit mutable binding
mut mutable_y = 42;
mutable_y = 50;  // OK: This is allowed
print("Mutable y: " + mutable_y);

// Traditional type declaration (still mutable for compatibility)
int z = 42;
z = 50;  // OK: Still mutable
print("Traditional z: " + z);

// Example 6: Runtime Type Checking
print("=== Type Checking Examples ===";

let my_result = Ok(100);
if typecheck("Result", my_result) {
    print("my_result is a Result type");
}

let my_option = Some(200);
if typecheck("Option", my_option) {
    print("my_option is an Option type");
}

// Get type as string
let result_type = typeof(my_result);
print("Type of my_result: " + result_type);

// Example 7: Pattern Matching (future feature simulation)
print("=== Handling Results ===";

func processData(int input) {
    if input < 0 {
        return Err("Negative value not allowed", "ValueError");
    }
    if input == 0 {
        return Err("Zero is not allowed", "ValueError");
    }
    return Ok(input * 2);
}

let data1 = processData(5);
let value1 = unwrap(data1);
print("Processed: " + value1);

let data2 = processData(-5);
let fallback = unwrapOr(data2, 0);
print("Processed (with fallback): " + fallback);

// Example 8: Threading with Results
print("=== Safe Threading Example ===";

mut counter = 0;  // Mutable counter for thread safety

// In multi-threaded code, explicit mutability is important
// for understanding which values can be modified across threads

func incrementCounter() {
    mut temp = counter;
    temp = temp + 1;
    counter = temp;
    return Ok(counter);
}

let thread_result = incrementCounter();
if isOk(thread_result) {
    print("Counter incremented safely");
}

// Example 9: Combining Features
print("=== Combining Results and Options ===";

func safeGetAndProcess(string key) {
    // Check if key exists (Option)
    let maybe_value = findElement(key);
    
    if isNone(maybe_value) {
        // Return error Result when Option is None
        return Err("Key not found: " + key, "NotFoundError");
    }
    
    let value = unwrap(maybe_value);
    
    // Process and wrap in Result
    if value < 0 {
        return Err("Value must be positive", "ValueError");
    }
    
    return Ok(value * 10);
}

let combined = safeGetAndProcess("target");
let final_value = expect(combined, "Expected valid processed value");
print("Final value: " + final_value);

print("=== All Examples Completed ===";
