#define EMPTY(...)
// Defers expansion of the argument by 1, 2 or 3 scans
#define DEFER(...) __VA_ARGS__ EMPTY()
#define DEFER2(...) __VA_ARGS__ DEFER(EMPTY)()
#define DEFER3(...) __VA_ARGS__ DEFER2(EMPTY)()

// Concatenate the arguments to one token
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

// Apply multiple scans to the argument expression (>64 to allow uint64_t masks)
#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) __VA_ARGS__

// Always expand to the second token after expansion of arguments.
// One level of indirection to expand arguments before selecting.
#define SELECT_2ND(...) SELECT_2ND_INDIRECT(__VA_ARGS__, , )
#define SELECT_2ND_INDIRECT(x1, x2, ...) x2

// Expands to a comma (which means two empty tokens in a parameter list).
// Thus, SELECT_2ND will expand to an empty token if this is the first argument.
#define BITS_RECURSIVE__END_RECURSION ,

// Adds the END_RECURSION parameter, which marks the end of the arguments
#define BITS(...) \
    (0 EVAL(BITS_RECURSIVE(__VA_ARGS__, END_RECURSION,)))

// When hitting END_RECURSION, the CAT will expand to "," and SELECT_2ND
// will select the empty argument instead of the recursive call.
#define BITS_RECURSIVE(bit, ...) \
    SELECT_2ND(PRIMITIVE_CAT(BITS_RECURSIVE__, bit), \
             | (1ull<<bit) DEFER3(BITS_INDIRECT)()(__VA_ARGS__))
// Needed to circumvent disabling contexts for recursive expansion
#define BITS_INDIRECT() BITS_RECURSIVE

