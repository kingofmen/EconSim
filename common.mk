CXX           = g++
LINKER        = g++
PROTOC        = "C:/Program Files (x86)/protoc/bin/protoc.exe"
# WINVER for OpenThread.
DEFINES       = -DUNICODE -DWINVER=0x0500
BASE          = "C:/Users/Rolf/base"
PROTOBUF_INCLUDES = "c:/Users/Rolf/protobuf/src-3.2.0/src/"
#CXXFLAGS      = -pipe -fno-keep-inline-dllexport -std=c++11 -frtti -Wall -Wextra -fexceptions -mthreads $(DEFINES) -isystem $(PROTOBUF_INCLUDES)
CXXFLAGS      = -pipe -fno-keep-inline-dllexport -std=gnu++11 -frtti -Wall -Wextra -fexceptions -mthreads $(DEFINES) -isystem $(PROTOBUF_INCLUDES)
INCPATH       = -I. -I$(BASE) -I$(PROTOBUF_INCLUDES)
PROTO_PATH    = --proto_path $(BASE)

