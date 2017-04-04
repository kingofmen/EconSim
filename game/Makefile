# Makefile for game executable
TARGET_NAME = game

include libs.mk

LIBS += $(POPULATION_LIB)
LIBS += $(MARKET_LIB)
LIBS += $(PROTOBUF_LIB)
LIBS += $(INDUSTRY_LIB)
LIBS += $(STATUS_LIB)

include common.mk

default: sinews

sinews:	$(patsubst %.cc,%.o,$(wildcard *.cc))
	$(LINKER) $^ -o $@ $(LIBS)

%.o:	%.cc
	$(CXX) $(CXXFLAGS) $(INCPATH) -c -o $@ $<

clean:
	rm -f sinews.exe
	rm *.o


