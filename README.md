# rewriter

This is a collection of code rewriting scripts, or programs, most of which are hacks.

## bin/

- bin/addi Add '#include' or forward reference to a set of files.
- bin/codecleaner perl filter that changes "\n"s to '\n' and adds a space for trailing ';' characters (-> ' ;'), a convention used at a previous workplace.
- `bin/comment_includes` Comment out stuff matching output derived from 'grep -n'.
- bin/indent.args reindent function call text.
- bin/indentFunc re-indent a function proto or prologue, possibly spanning multiple lines
- bin/myindent Run really badly formatted code through gnu-indent with some options to make it less horrible, or at least consistent.
- bin/stripFunc Replaces function calls like `FOO( goo->moo )` with just `goo->moo`.
- bin/stripFunc2p Changes calls of the form FOO( bar, har ), to just bar = har.
- bin/searchAndReplace apply hardcoded search and replace expressions to specific lines in a file.

## exit/

This is a clang AST rewriter sample that changes all instances of a named function (say exit()) to another name, say, `foo_exit`.

The clang AST is quite powerful, but programs that use it can be finicky to build and maintain.  It used to be that any clang-AST based program would be broken by every new clang release, but the AST as an ABI may have stabilized, as this little program happens to work with clang-17, clang-20, and clang-21 (and probably 18,19 too).

If attempting to compile this program yourself, you'll probably have to hack exit/Makefile.  I used my own clang installation, built with:

```
cd ${HOME}/llvm-project
git checkout llvmorg-21.1.7
patch -p1 < ~/toycalculator/llvm-patches/llvm21.flang.patch # git@github.com:peeterjoot/toycalculator.git
(rm -rf ${HOME}/build-llvm && mkdir ${HOME}/build-llvm)
cd ${HOME}/build-llvm
cmake -G Ninja ../llvm-project/llvm -DBUILD_SHARED_LIBS=true -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_ASSERTIONS=TRUE -DLLVM_OPTIMIZED_TABLEGEN=ON -DLLVM_LIBDIR_SUFFIX=64 -DCMAKE_INSTALL_RPATH=/usr/local/llvm-21.1.7/lib64 -DLLVM_TARGETS_TO_BUILD="host" -DCMAKE_INSTALL_PREFIX=/usr/local/llvm-21.1.7 -DLLVM_ENABLE_RTTI=ON -DLLVM_ENABLE_PROJECTS='clang;flang;mlir'
ninja
sudo rm -rf /usr/local/llvm-21.1.7
sudo ninja install
```
