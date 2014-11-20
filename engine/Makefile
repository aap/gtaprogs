SRCDIR = src
BUILDDIR = build
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
DEP = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.d,$(SRC))
RWDIR=$(HOME)/src/rwtools
OUT=gtaengine.a
CFLAGS=-g -I$(RWDIR)/include -Wall -Wextra

$(OUT): $(OBJ)
	ar scr $(OUT) $(OBJ)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: $(SRCDIR)/%.cpp
	$(CXX) -MM -MT '$(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$<)' $(CFLAGS) $< > $@
	
clean:
	rm -f $(OUT) build/*.o build/*.d

-include $(DEP)
