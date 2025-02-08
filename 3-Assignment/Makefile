LIBDIR = lib
TESTDIR = TEST
TESTOUTDIR = TEST_OUT
SRCFILEs := $(wildcard ${LIBDIR}/*.cpp)
OBJFILEs := $(subst .cpp,.o, ${SRCFILEs})
TESTSRCFILEs := $(wildcard ${TESTDIR}/*.c)
TESTBCFILEs := $(subst .c,.bc, ${TESTSRCFILEs})
TESTIRFILEs := $(subst .c,.ll, ${TESTSRCFILEs})

LICM := ${LIBDIR}/LICM.so

CXX := clang
CXXFLAGS := $(shell llvm-config --cxxflags) -fPIC -std=c++23
LLVMFLAGS := $(shell llvm-config --ldflags --libs --system-libs)

all: $(LICM) $(TESTIRFILEs)

$(LIBDIR)/%.o: $(LIBDIR)/%.cpp
	echo "Compiling $<"
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(LICM): $(OBJFILEs)
	$(CXX) ${LLVMFLAGS} -dylib -shared $^ -o $@

$(TESTDIR)/%.ll: $(TESTDIR)/%.c
	$(CXX) -S -c -O0 -Xclang -disable-O0-optnone -emit-llvm -o $@ $<

clean:
	$(RM) $(LICM) $(OBJFILEs) $(TESTDIR)/*.ll $(TESTDIR)/*.bc $(TESTOUTDIR)/*

licm:
	make $(file)
	make $(LICM)
	opt -passes=mem2reg $(file) -o $(subst .ll,.bc,$(file))
	llvm-dis $(subst .ll,.bc,$(file)) -o $(subst .ll,_mem2reg.ll,$(file))
	opt -load-pass-plugin=${LIBDIR}/LICM.so -passes=MyLICM $(subst .ll,.bc,$(file)) -o $(subst .ll,.bc,$(file))
	llvm-dis $(subst .ll,.bc,$(file)) -o $(TESTOUTDIR)/$(notdir $(subst .ll,_optimized.ll,$(file)))
	@echo "Original file results:"
	@lli $(subst .ll,.bc,$(file))
	@echo "Optimized file results:"
	@lli $(TESTOUTDIR)/$(notdir $(subst .ll,_optimized.ll,$(file)))

test: $(TESTIRFILEs)
	@echo ".ll files generated"

.PHONY: clean all licm