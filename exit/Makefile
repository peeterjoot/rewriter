LOADLIBES += -lclang-cpp
CXXFLAGS += -g
TOOL := exrewrite
INSTALLDIR := tool

# customize these three vars to match your clang installation:
LLVMPREFIX := /usr/local/llvm-20.1.7
# /usr/bin/../lib64/clang/20 with /usr stripped off:
# /usr/local/llvm-20.1.7/bin/../lib64/clang/20 with the prefix stripped off:
CLANG_PREFIX_WITHOUT_USR := lib64/clang/20
CLANG_VV_INCLUDE := $(LLVMPREFIX)/$(CLANG_PREFIX_WITHOUT_USR)/include

# comment this out if default llvm/clang has a usable set of the AST includes (AST.h, ...)
LLVMBIN := $(LLVMPREFIX)/bin/

all : $(INSTALLDIR)

$(TOOL) : $(TOOL).o
	clang++ $^ -o $@ `$(LLVMBIN)llvm-config --ldflags --system-libs --libs all` $(LOADLIBES)

%.o : %.cpp
	clang++ $(CXXFLAGS) -c $^ `$(LLVMBIN)llvm-config --cxxflags`

clean :
	rm -f *.o $(TOOL)
	rm -rf $(INSTALLDIR)

# https://stackoverflow.com/questions/79048925/how-do-i-tell-a-clang-ast-tool-where-to-find-stddef-h
$(INSTALLDIR): $(TOOL)
	rm -rf $@
	mkdir -p $@/bin $@/$(CLANG_PREFIX_WITHOUT_USR)
	install $(TOOL) $@/bin/
	(cd $@/$(CLANG_PREFIX_WITHOUT_USR) && ln -s $(CLANG_VV_INCLUDE))

# vim: noet ts=8 sw=8
