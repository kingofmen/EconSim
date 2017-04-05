# Makefile for game executable
include common.mk
include libs.mk

LIBS += -L$(BASE)/game/proto/ -lgame_world_proto
LIBS += $(POPULATION_LIB)
LIBS += $(STATUS_LIB)
LIBS += $(GEOGRAPHY_LIB)
LIBS += $(MARKET_LIB)
LIBS += $(INDUSTRY_LIB)
LIBS += $(PROTOBUF_LIB)

default: sinews

sinews:	$(patsubst %.cc,%.o,$(wildcard *.cc))
	$(LINKER) $^ -o $@ $(LIBS)

%.o:	%.cc
	$(CXX) $(CXXFLAGS) $(INCPATH) -c -o $@ $<

clean:
	rm -f sinews.exe
	rm *.o


