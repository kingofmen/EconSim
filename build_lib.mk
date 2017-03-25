# Generic makefile for building a library named TARGET_NAME.

default: lib$(TARGET_NAME).a

lib$(TARGET_NAME).a:	$(patsubst %.cc,%.o,$(wildcard *.cc))
			ar rcs $@ $^

%.o:	%.cc %.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	rm *.o
	rm lib$(TARGET_NAME).a
