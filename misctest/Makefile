SRCDIR = src
BUILDDIR = build
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
DEP = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.d,$(SRC))
RWDIR=$(HOME)/src/rwtools
LDLIBS=-lpthread -lX11 -lXrandr -lXi -lXxf86vm -lGL -lGLEW -lm -ljsoncpp
CFLAGS=-g -I$(RWDIR)/include -I/usr/include/jsoncpp -Wall -Wextra
OUT=$(shell basename `pwd`)

$(OUT): $(OBJ) ../engine/gtaengine.a $(RWDIR)/lib/librwtools.a /usr/local/lib/libglfw3.a 
	$(CXX) $^ $(LDLIBS) -o $(OUT)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: $(SRCDIR)/%.cpp
	$(CXX) -MM -MT '$(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$<)' $(CFLAGS) $< > $@
	
dep: $(DEP)

clean:
	rm build/*.o build/*.d

-include $(DEP)

