UNAME_S := $(shell uname -s)

CXXFLAGS ?= -Ilib/protobuf-3.21.2/dist/include
LDLIBS ?= lib/protobuf-3.21.2/dist/lib/libprotobuf.a

ifeq ($(UNAME_S),Linux)
	LDLIBS += -lstdc++fs -pthread
endif

all: ProtoToJson JsonToProto

JsonToProto: ProtoToJson
	ln -f "$<" "$@"

ProtoToJson: ProtobufJson.cc  lib/.compile
	g++ -std=c++17 -g -o "$@" ProtobufJson.cc $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

lib/.compile:
	rm -rf lib
	./build_dependencies.sh
	touch $@

clean:
	rm -f JsonToProto ProtoToJson

distclean: clean
	rm -rf lib
