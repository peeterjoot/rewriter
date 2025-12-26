LOADLIBES += -lclang-cpp
CXXFLAGS += -g
TOOL := exrewrite
INSTALLDIR := tool

#USE_MINE := 1
ifdef USE_MINE
# Customize these to match your clang installation:
LLVMMAJOR := 21
LLVMPREFIX := /usr/local/llvm-$(LLVMMAJOR).1.8
CLANG_PREFIX_WITHOUT_USR := lib64/clang/$(LLVMMAJOR)
CLANG_VV_INCLUDE := $(LLVMPREFIX)/$(CLANG_PREFIX_WITHOUT_USR)/include

# Comment this out if default llvm/clang has a usable set of the AST includes (AST.h, ...)
# (if that's the case the -rpath below is probably also not required)
LLVMBIN := $(LLVMPREFIX)/bin/
else
# clang-devel-20.1.8-4.fc42.x86_64
LLVMMAJOR := 20
CLANG_PREFIX_WITHOUT_USR := lib/clang/$(LLVMMAJOR)
CLANG_VV_INCLUDE := /usr/$(CLANG_PREFIX_WITHOUT_USR)/include
endif

LLVMCXXFLAGS := $(shell $(LLVMBIN)llvm-config --cxxflags)
LLVMLIBDIR := $(shell $(LLVMBIN)llvm-config --libdir)
LLVMLDFLAGS := $(shell $(LLVMBIN)llvm-config --ldflags --system-libs --libs all)

all : $(INSTALLDIR)

$(TOOL) : $(TOOL).o
	clang++ $^ -o $@ $(LLVMLDFLAGS) $(LOADLIBES) -Wl,-rpath,$(LLVMLIBDIR)

%.o : %.cpp
	clang++ $(CXXFLAGS) -c $^ $(LLVMCXXFLAGS)

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
