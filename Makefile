CXX=g++-8
RM=rm -f
CPPFLAGS=-Wall -std=c++17
ADDED_CPPFLAGS=
LDFLAGS=
LDLIBS=

MAIN=nbody.cpp
SRCS=
OBJS=$(subst .cpp,.o,$(SRCS))

release: ADDED_CPPFLAGS=-O3

all release: nbody

nbody: $(SRCS) $(MAIN)
	$(CXX) -o $@ $(MAIN) $(SRCS) $(LDLIBS) $(LDFLAGS) $(CPPFLAGS) $(ADDED_CPPFLAGS)

clean:
	$(RM) $(subst .cpp, ,$(SRCS))
	$(RM) $(subst .cpp, ,$(MAIN))
	$(RM) test
	$(RM) $(subst .cpp,.o,$(SRCS))
	$(RM) $(subst .cpp,.o,$(MAIN))