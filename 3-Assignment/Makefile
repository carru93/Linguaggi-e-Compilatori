LIBDIR = lib
TESTDIR = TEST
TESTOUTDIR = TEST_OUT
SRCFILEs := $(wildcard ${LIBDIR}/*.cpp)
OBJFILEs := $(subst .cpp,.o, ${SRCFILEs})
TESTSRCFILEs := $(wildcard ${TESTDIR}/*.cpp)
TESTBCFILEs := $(subst .cpp,.bc, ${TESTSRCFILEs})
TESTIRFILEs := $(subst .cpp,.ll, ${TESTSRCFILEs})

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

$(TESTDIR)/%.ll: $(TESTDIR)/%.cpp
	$(CXX) -S -c -O0 -Xclang -disable-O0-optnone -emit-llvm -o $@ $< $(CXXFLAGS)

clean:
	$(RM) $(LICM) $(OBJFILEs) $(TESTBCFILEs) $(TESTIRFILEs) $(TESTOUTDIR)/*

licm:
	make $(file)
	make $(MULTIINSTRUCTIONS)
	opt -passes=mem2reg $(file) -o $(subst .ll,.bc,$(file))
	opt -load-pass-plugin=${LIBDIR}/LICM.so -passes=MyLICM $(subst .ll,.bc,$(file)) -o $(subst .ll,.bc,$(file))
	llvm-dis $(subst .ll,.bc,$(file)) -o $(TESTOUTDIR)/$(notdir $(subst .ll,_optimized.ll,$(file)))

test: $(TESTIRFILEs)
	@echo ".ll files generated"

.PHONY: clean all licm