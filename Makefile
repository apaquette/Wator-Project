# Joseph Kehoe
# This is very basic. It is just here to show you a simple sfml graphics display that you can use.
# I expect better from you!

CXX = g++
CPPFILES= wator.cpp
CPPFLAGS= -Wall
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -fopenmp
EXE= wator

SRCS=wator.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

#This rule says that each .o file depends on a .cpp file of the same name
#This is actually built into Make but anyways...
%.o: %.cpp
	$(CXX) -c -o $@ $< $(CPPFLAGS)  $(DEBUGFLAGS) $(LDFLAGS)


ALL: $(OBJS)
	$(CXX) $(OBJS) -o $(EXE) $(LDFLAGS)
