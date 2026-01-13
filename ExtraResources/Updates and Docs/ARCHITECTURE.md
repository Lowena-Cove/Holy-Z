# Holy Z Type System Architecture - Rust-Inspired Design

## Core Type Hierarchy

```
BaseType (via boost::any)
├── Primitive Types
│   ├── int
│   ├── float  
│   ├── bool
│   └── string
├── Graphics Types
│   ├── Sprite
│   ├── Vec2
│   └── Text
├── Error Handling (NEW)
│   ├── Result<T, E>
│   │   ├── Ok(T)
│   │   └── Err(String, String)
│   └── Option<T>
│       ├── Some(T)
│       └── None
├── Object Types
│   ├── ClassInstance
│   │   └── instanceAttributes: Map<String, Any>
│   ├── Pointer
│   │   ├── address: void*
│   │   └── pointedType: String
│   └── Custom Classes
└── Type System (NEW)
    ├── TraitDefinition
    │   ├── methods: Vec<(String, Vec<String>)>
    │   └── defaultImplementations: Map<String, Vec<Vec<String>>>
    └── TraitImplementation
        ├── typeName: String
        ├── traitName: String
        └── implementations: Map<String, Vec<Vec<String>>>
```

## Variable Binding Model

```
Variable Declaration
│
├─ Traditional (Type-based)
│  │  int x = 5;
│  │  string s = "hello";
│  └─ Status: Mutable (for compatibility)
│
├─ Immutable Binding (let)
│  │  let x = 5;
│  │  let s = "hello";
│  └─ Status: Immutable (tracked in immutableVariables)
│
└─ Mutable Binding (mut)
   │  mut x = 5;
   │  mut s = "hello";
   └─ Status: Mutable (explicitly marked)

Variable Tracking:
- globalVariableValues: Map<String, Any>      // All values
- immutableVariables: Map<String, bool>       // Mutability status
- borrowedVariables: Map<String, bool>        // Borrow status (future)
```

## Result Type Architecture

```
ResultValue (New)
├── isOk: bool
│   ├── true  → Success path
│   ├── false → Error path
├── value: Any         // Success value
├── error: String      // Error message
└── errorType: String  // Error type category

Operations:
├── Creation:  Ok(val), Err(msg, type)
├── Checking:  isOk(), isErr()
├── Unwrap:    unwrap(), expect(), unwrapOr()
└── Info:      toString()

Usage Pattern:
let result = someFunction();
if isOk(result) {
    let value = unwrap(result);
    // use value
} else {
    let error = expect(result, "fallback");
    // handle error
}
```

## Option Type Architecture

```
OptionValue (New)
├── isSome: bool
│   ├── true  → Contains value
│   └── false → None (empty)
└── value: Any    // Contained value (if isSome)

Static Factory Methods:
├── Some(val) → OptionValue with value
└── None()    → Empty OptionValue

Operations:
├── Creation:  Some(val), None()
├── Checking:  isSome(), isNone()
└── Info:      toString()

Usage Pattern:
let option = findValue();
if isSome(option) {
    let val = unwrap(option);
    // use val
} else {
    // handle None case
}
```

## Trait System Architecture

```
TraitDefinition (New)
├── traitName: String
├── methods: Vec<(String, Vec<String>)>
│   └── Pairs of method name and parameter list
├── defaultImplementations: Map<String, Vec<Vec<String>>>
│   └── Default method bodies (lines of code)
└── Operations:
    ├── addMethod(name, params)
    ├── setDefaultImplementation(name, body)
    └── hasDefaultImpl(name): bool

TraitImplementation (New)
├── typeName: String          // Type implementing trait
├── traitName: String         // Trait being implemented
└── implementations: Map<String, Vec<Vec<String>>>
    └── Method implementations (code lines)

Global Registries:
├── globalTraits: Map<String, TraitDefinition>
│   └── All defined traits
└── globalTraitImpls: Vec<TraitImplementation>
    └── All trait-type pairs

Usage Pattern:
trait Comparable {
    lessThan(other);
    equals(other);
    
    // Default implementation (future)
    greaterThan(other) {
        return !equals(other) && !lessThan(other);
    }
}

impl Comparable for int {
    lessThan(other) { ... }
    equals(other) { ... }
}
```

## Type Checking Pipeline

```
Value (boost::any)
│
├─ any_type() → int (0-9 type code)
│
├─ any_type_name() → String
│   ├── 0 → "int"
│   ├── 1 → "float"
│   ├── 2 → "bool"
│   ├── 3 → "string"
│   ├── 4 → "Sprite"
│   ├── 5 → "Vec2"
│   ├── 6 → "Text"
│   ├── 7 → "object"
│   ├── 8 → "Result"
│   └── 9 → "Option"
│
└─ Type Operations:
    ├── typeof(val) → String name
    ├── typecheck(expected, val) → bool
    └── istype(expected, val) → bool

Example:
let result = Ok(42);
let type_name = typeof(result);      // Returns "Result"
if typecheck("Result", result) {
    // Confirmed to be Result
}
```

## Immutability Tracking System

```
Variable Creation/Declaration:

Traditional Syntax:
int x = 5;
├─ globalVariableValues["x"] = 5
├─ immutableVariables["x"] = false  (default: mutable)
└─ Behavior: Can be modified

Rust-style let:
let x = 5;
├─ globalVariableValues["x"] = 5
├─ immutableVariables["x"] = true   (explicitly immutable)
└─ Behavior: Modification attempted → Runtime error

Rust-style mut:
mut x = 5;
├─ globalVariableValues["x"] = 5
├─ immutableVariables["x"] = false  (explicitly mutable)
└─ Behavior: Can be modified

Modification Check:
Before assignment to x:
├─ Check immutableVariables["x"]
├─ If true (immutable) → LogWarning, prevent modification
└─ If false (mutable) → Allow modification
```

## Error Handling Flow

### Old Pattern (Exceptions)
```
try {
    risky_operation();
} catch (std::exception& e) {
    handle_error();
}
```

### New Pattern (Results)
```
let result = risky_operation();
if isErr(result) {
    handle_error();
} else {
    let value = unwrap(result);
    use_value();
}
```

### Combined with Options
```
let maybe_value = find_something();
if isNone(maybe_value) {
    return Err("Not found", "NotFound");
}
let value = unwrap(maybe_value);

let result = process(value);
if isErr(result) {
    return Err("Processing failed", "ProcessError");
}
return Ok(unwrap(result));
```

## Memory Safety Model

```
Ownership (Simplified)
├── Local variables
│   └── Owned value dropped at scope end
├── Pointers (Explicit)
│   ├── Created with: addressof(value), malloc(type)
│   └── Freed with: free(ptr)
└── References (Future)
    ├── Immutable borrows: &T
    └── Mutable borrows: &mut T

Type Safety:
├── Strong types at compile time
├── Dynamic casting with boost::any
├── Runtime type validation
└── Safe conversions (AnyAsInt, etc.)

Thread Safety (Rust-inspired):
├── Immutable by default → No accidental mutations
├── Explicit mut → Intent clarity
├── Results → Error handling visibility
└── Message passing → Safe inter-thread communication
```

## Function Signatures with Results

```cpp
// Old style
int divide(int a, int b);  // Returns value or throws

// New style  
func divide(int a, int b) {
    if b == 0 {
        return Err("Division by zero", "Math");
    }
    return Ok(a / b);
}

// Usage
let result = divide(10, 2);
let quotient = unwrap(result);
```

## Integration Points

### With Class System
```
ClassInstance
├─ className: String
├─ instanceAttributes: Map<String, Any>
└─ Methods can return Results/Options

Example:
class FileHandle {
    read() { return Result }
    isOpen() { return Option<bool> }
}
```

### With Function System
```
Functions can:
├─ Accept Result/Option parameters
├─ Return Result/Option values
├─ Use let/mut bindings
└─ Throw informative Results instead of exceptions
```

### With Trait System
```
Traits can:
├─ Define methods returning Results
├─ Provide default implementations
├─ Enforce error handling patterns
└─ Enable composition and reuse
```

## Performance Implications

| Feature | Memory | CPU | Notes |
|---------|--------|-----|-------|
| Result | +56 bytes | None | Wrapped in boost::any |
| Option | +8 bytes | None | Wrapped in boost::any |
| Type checking | O(1) | Minimal | Hash map lookup |
| Immutability | +4 bytes | O(1) | Tracked in map |
| Traits | Varies | O(n) | Method lookup |

## Compilation & Linking

```
Type System Components:
├─ builtin.h: Type definitions (ResultValue, OptionValue, Traits)
├─ anyops.h: Type conversions and checking (any_type_name)
└─ Main.cpp: Function implementations (Ok, Err, etc.)

No external dependencies:
├─ All types use boost::any for storage
├─ No new libraries required
├─ Standard C++17 features
└─ Fully compatible with existing systems
```

## Future Architecture Roadmap

```
Phase 2: Enhanced Generics
├─ Generic types with type parameters
├─ Type bounds and constraints
└─ Variance annotations

Phase 3: Advanced Trait System
├─ Associated types
├─ Higher-ranked trait bounds
└─ Trait objects

Phase 4: Async/Await
├─ Future/Promise types
├─ Async functions
└─ Tokio-like runtime

Phase 5: Advanced Type System
├─ Lifetime annotations
├─ Mutable borrowing references
└─ Move semantics enforcement
```

This architecture provides a solid foundation for Rust-inspired programming in Holy Z while maintaining compatibility with the existing C++-based system.
