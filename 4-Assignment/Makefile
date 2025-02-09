LIBDIR = lib
TESTDIR = TEST
TESTOUTDIR = TEST_OUT
SRCFILEs := $(wildcard ${LIBDIR}/*.cpp)
OBJFILEs := $(subst .cpp,.o, ${SRCFILEs})
TESTSRCFILEs := $(wildcard ${TESTDIR}/*.c)
TESTBCFILEs := $(subst .c,.bc, ${TESTSRCFILEs})
TESTIRFILEs := $(subst .c,.ll, ${TESTSRCFILEs})

LOOPFUSION := ${LIBDIR}/LoopFusion.so

CXX := clang
CXXFLAGS := $(shell llvm-config --cxxflags) -fPIC -std=c++23
LLVMFLAGS := $(shell llvm-config --ldflags --libs --system-libs)

all: $(LOOPFUSION) $(TESTIRFILEs)

$(LIBDIR)/%.o: $(LIBDIR)/%.cpp
	echo "Compiling $<"
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(LOOPFUSION): $(OBJFILEs)
	$(CXX) ${LLVMFLAGS} -dylib -shared $^ -o $@

$(TESTDIR)/%.ll: $(TESTDIR)/%.c
	$(CXX) -S -c -O0 -Xclang -disable-O0-optnone -emit-llvm -o $@ $<

clean:
	$(RM) $(LOOPFUSION) $(OBJFILEs) $(TESTDIR)/*.ll $(TESTDIR)/*.bc $(TESTOUTDIR)/*

lf:
	make $(file)
	make $(LOOPFUSION)
	opt -passes=mem2reg $(file) -o $(subst .ll,.bc,$(file))
	llvm-dis $(subst .ll,.bc,$(file)) -o $(subst .ll,_mem2reg.ll,$(file))
	opt -load-pass-plugin=${LIBDIR}/LoopFusion.so -passes=LoopFusion $(subst .ll,.bc,$(file)) -o $(subst .ll,.bc,$(file))
	llvm-dis $(subst .ll,.bc,$(file)) -o $(TESTOUTDIR)/$(notdir $(subst .ll,_optimized.ll,$(file)))
	@echo "Original file results:"
	@lli $(subst .ll,.bc,$(file))
	@echo "Optimized file results:"
	@lli $(TESTOUTDIR)/$(notdir $(subst .ll,_optimized.ll,$(file)))

test: $(TESTIRFILEs)
	@echo ".ll files generated"

.PHONY: clean all lf