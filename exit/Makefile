LOADLIBES += -lclang-cpp
CXXFLAGS += -g
TOOL := exrewrite
# /usr/bin/../lib/clang/17 with /usr stripped off:
CLANG_PREFIX_WITHOUT_USR := lib/clang/17
INSTALLDIR := tool

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
	(cd $@/$(CLANG_PREFIX_WITHOUT_USR) && ln -s /usr/$(CLANG_PREFIX_WITHOUT_USR)/include)

# vim: noet ts=8 sw=8
