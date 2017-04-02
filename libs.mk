LIBS                 = -static -static-libgcc

GEOGRAPHY_PROTO_LIB  = -L$(BASE)/geography/proto/ -lgeography_proto
GEOGRAPHY_LIB        = -L$(BASE)/geography/ -lgeography $(GEOGRAPHY_PROTO_LIB)

INDUSTRY_PROTO_LIB   = -L$(BASE)/industry/proto/ -lindustry_proto
INDUSTRY_LIB         = -L$(BASE)/industry/ -lindustry $(INDUSTRY_PROTO_LIB)

MARKET_PROTO_LIB     = -L$(BASE)/market/proto/ -lmarket_proto
MARKET_LIB           = -L$(BASE)/market/ -lmarket $(MARKET_PROTO_LIB)

POPULATION_PROTO_LIB = -L$(BASE)/population/proto/ -lpopulation_proto
POPULATION_LIB       = -L$(BASE)/population/ -lpopulation $(POPULATION_PROTO_LIB)

PROTOBUF_LIB         = -L"C:/Users/Rolf/protobuf/src-3.2.0/cmake/build/release/" -lprotobuf

STATUS_LIB           = -L$(BASE)/util/status/ -lstatus