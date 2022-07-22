CXXFLAGS ?= -I/usr/local/include
LDFLAGS ?= -L/usr/local/lib
# To statically link, run `make` with `LDLIBS=/full/path/to/libprotobuf.a make`
LDLIBS ?= -lprotobuf

ProtobufJson: ProtobufJson.cc
	g++ -std=c++11 -g -o ProtobufJson ProtobufJson.cc $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f ProtobufJson
