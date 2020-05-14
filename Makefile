# Setup for Yiheng's local VM
CC=/usr/bin/gcc
CXX=/usr/bin/g++
 
OPT_FLAGS = -O2 -std=c++11
#OPT_FLAGS = -ggdb3 -O0

# Make from subdirectories
VPATH = src/game src/agent src/heuristic test

BUILDDIR = build/

BINDIR = bin/

PROGS = sokoban sokoban_test npuzzle npuzzle_test

all:: $(PROGS)

$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(OPT_FLAGS) $< -o $@ -c

npuzzle_test: $(BUILDDIR)/Game.o $(BUILDDIR)/NPuzzleHeuristic.o $(BUILDDIR)/Agent.o $(BUILDDIR)/NPuzzle.o $(BUILDDIR)/AstarSearchAgent.o $(BUILDDIR)/npuzzle_test.o
	$(CXX) $(OPT_FLAGS) -o $(BINDIR)/$@ $^

sokoban_test: $(BUILDDIR)/Game.o $(BUILDDIR)/SokobanHeuristic.o $(BUILDDIR)/Agent.o $(BUILDDIR)/Sokoban.o $(BUILDDIR)/AstarSearchAgent.o $(BUILDDIR)/sokoban_test.o
	$(CXX) $(OPT_FLAGS) -o $(BINDIR)/$@ $^

sokoban: $(BUILDDIR)/Game.o $(BUILDDIR)/PlayableGame.o $(BUILDDIR)/PlayerAgent.o $(BUILDDIR)/Agent.o $(BUILDDIR)/Sokoban.o $(BUILDDIR)/sokoban_play.o
	$(CXX) $(OPT_FLAGS) -o $(BINDIR)/$@ $^

npuzzle: $(BUILDDIR)/Game.o $(BUILDDIR)/PlayableGame.o $(BUILDDIR)/PlayerAgent.o $(BUILDDIR)/Agent.o $(BUILDDIR)/NPuzzle.o $(BUILDDIR)/npuzzle_play.o
	$(CXX) $(OPT_FLAGS) -o $(BINDIR)/$@ $^

clean::
	-rm -f *.o $(BUILDDIR)/*.o

rebuild::
	make clean;make
