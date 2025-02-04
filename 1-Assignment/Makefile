LIBDIR = lib
TESTDIR = TEST
TESTOUTDIR = TEST_OUT
SRCFILEs := $(wildcard ${LIBDIR}/*.cpp)
OBJFILEs := $(subst .cpp,.o, ${SRCFILEs})
TESTSRCFILEs := $(wildcard ${TESTDIR}/*.cpp)
TESTBCFILEs := $(subst .cpp,.bc, ${TESTSRCFILEs})
TESTIRFILEs := $(subst .cpp,.ll, ${TESTSRCFILEs})

STRENGTH := ${LIBDIR}/Strength.so
IDENTITY := ${LIBDIR}/AlgebraicIdentity.so
MULTIINSTRUCTIONS := ${LIBDIR}/MultiInstructions.so

CXX := clang
CXXFLAGS := $(shell llvm-config --cxxflags) -fPIC -std=c++23
LLVMFLAGS := $(shell llvm-config --ldflags --libs --system-libs)

all: $(STRENGTH) $(IDENTITY) $(MULTIINSTRUCTIONS) $(TESTIRFILEs)

$(LIBDIR)/%.o: $(LIBDIR)/%.cpp
	echo "Compiling $<"
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(STRENGTH): $(OBJFILEs)
	$(CXX) ${LLVMFLAGS} -dylib -shared $^ -o $@

$(IDENTITY): $(OBJFILEs)
	$(CXX) ${LLVMFLAGS} -dylib -shared $^ -o $@

$(MULTIINSTRUCTIONS): $(OBJFILEs)
	$(CXX) ${LLVMFLAGS} -dylib -shared $^ -o $@

$(TESTDIR)/%.ll: $(TESTDIR)/%.cpp
	$(CXX) -S -c -O0 -Xclang -disable-O0-optnone -emit-llvm -o $@ $< $(CXXFLAGS)

clean:
	$(RM) $(STRENGTH) $(IDENTITY) $(MULTIINSTRUCTIONS) $(OBJFILEs) $(TESTBCFILEs) $(TESTIRFILEs) $(TESTOUTDIR)/*

st:
	make $(file)
	make $(STRENGTH)
	opt -passes=mem2reg $(file) -o $(subst .ll,.bc,$(file))
	opt -load-pass-plugin=${LIBDIR}/Strength.so -passes=strength $(file) -o $(subst .ll,.bc,$(file))
	llvm-dis $(subst .ll,.bc,$(file)) -o $(TESTOUTDIR)/$(notdir $(subst .ll,_optimized.ll,$(file)))

ai:
	make $(file)
	make $(IDENTITY)
	opt -passes=mem2reg $(file) -o $(subst .ll,.bc,$(file))
	opt -load-pass-plugin=${LIBDIR}/AlgebraicIdentity.so -passes=ai $(file) -o $(subst .ll,.bc,$(file))
	llvm-dis $(subst .ll,.bc,$(file)) -o $(TESTOUTDIR)/$(notdir $(subst .ll,_optimized.ll,$(file)))

mi:
	make $(file)
	make $(MULTIINSTRUCTIONS)
	opt -passes=mem2reg $(file) -o $(subst .ll,.bc,$(file))
	opt -load-pass-plugin=${LIBDIR}/MultiInstructions.so -passes=multiinst $(subst .ll,.bc,$(file)) -o $(subst .ll,.bc,$(file))
	llvm-dis $(subst .ll,.bc,$(file)) -o $(TESTOUTDIR)/$(notdir $(subst .ll,_optimized.ll,$(file)))

test: $(TESTIRFILEs)
	@echo ".ll files generated"

.PHONY: clean all st ai mi test