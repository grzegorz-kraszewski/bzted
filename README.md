# Bzted
Experimental concatenative programming language for Amiga
## Design goals
* Concatenative language using stack concept and reverse Polish notation. It may use glyphs for basic operators.
* Using Amiga OS API should be easy, so the API becomes a standard library. Example "HelloWorld"? `"Hello world!\n" PutStr`
* It should be possible to compile (from C++ sources), use the compiler and use compiled programs written in Bzted on a real, slightly expanded Amiga 1200. My reference system has M68020 processor at 28 MHz, 64 MB fast RAM. While I expect that compiling Bzted compiler may be slow, the Bzted compiler itself should be fast (comparable to Amiga E compiler).
* Compiler will produce M68k assembler source, compatible with [vasm](http://www.compilers.de/vasm.html) assembler. Producing an executable file without intermediate stage would be cool (and still not excluded in the future), but debugging Bzted compiler will be easier, I can also call `vasm` from the compiler, so it looks like a single stage build.
* I want Bzted to be a typed language. Not yet sure if I manage to do it by type inference. Basic types are int32 and string. Floating point will be added later. Strings will be nullterminated, as most of AmigaOS API expects it. For now strings are in local 8-bit codepage, UTF-8 is the plan.

## Cheat sheet
### Code formatting
Free formatting. Whitespaces: space, TAB, CR, LF. At least one whitespace (or any number of them in any sequence) is a word separator.
```
[ 1 2 3 ] [ 4 5 6 ]
[1 2 3][4 5 6]        # brackets have "implied separators" on both sides
(a b(c{d}))           # of course real separators can be put around brackets
foo  {  inside  }p    # at will
```
### Comments
```
# comment starts with hash and goes to the end of line
##### the first hash is important, the rest is, well, just a comment
4 637 + 12 -    # needs not to be full-line
```
### Integers
Signed 32-bit range, numbers outside it trigger compilation error.
```
3 -9 18277 +8372 -0 +0                             # both signed zeros are just zero
$4A $e8B4a2                                        # hexadecimal
%01110 %1110001001 %0                              # binary
0000028 $00000000000000000376C %00000000100010     # any number of leading zeros is ignored
```
### Strings
```
'some string' "a string too" '{"x":9, "y":17}' "ok, some apostrophes '''"
'multiline
string'          # becomes 'multiline\nstring'
"escaping: \n \t \r \\ \$1E \$A \$1928"
```
### Identifiers
The rest of words are identifiers and are further categorized by table search:
* builtin operators
* AmigaOS API functions (treated as operators too)
* defined named functions and variables
