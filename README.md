# rewriter

This is a collection of code rewriting scripts, or programs, most of which are hacks.

## bin/

- bin/addi Add '#include' or forward reference to a set of files.
- bin/codecleaner perl filter that changes "\n"s to '\n' and adds a space for trailing ';' characters (-> ' ;'), a convention used at a previous workplace.
- bin/`comment_includes` Comment out stuff matching output derived from 'grep -n'.
- bin/indent.args reindent function call text.
- bin/indentFunc re-indent a function proto or prologue, possibly spanning multiple lines
- bin/myindent Run really badly formatted code through gnu-indent with some options to make it less horrible, or at least consistent.
- bin/stripFunc Replaces function calls like `FOO( goo->moo )` with just `goo->moo`.
- bin/stripFunc2p Changes calls of the form FOO( bar, har ), to just bar = har.
- bin/searchAndReplace apply hardcoded search and replace expressions to specific lines in a file.

## exit/

This is a clang AST rewriter sample that changes all instances of a named function (say exit()) to another name, say, `foo_exit`.

The clang AST is quite powerful, but very finicky to build, and a clang-AST based program will often be broken by new clang release updates.
This particular program happens to work with clang-17 and clang-20 (and probably 18,19 too), but don't expect it to not break with other clang releases, or
work with older ones.

If attempting to compile this program yourself, you'll probably have to hack both exit/Makefile and exit/runit (`LD_LIBRARY_PATH`)
