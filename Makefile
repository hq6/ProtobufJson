CXXFLAGS ?= -I/usr/local/include
LDFLAGS ?= -L/usr/local/lib
LDLIBS ?= -lprotobuf

ProtobufJson: ProtobufJson.cc
	g++ -std=c++11 -g -o ProtobufJson ProtobufJson.cc $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f ProtobufJson
