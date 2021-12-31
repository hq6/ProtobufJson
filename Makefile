ProtobufJson: ProtobufJson.cc
	g++ -std=c++11 -g -o ProtobufJson ProtobufJson.cc -I/usr/local/include -L/usr/local/lib -lprotobuf

clean:
	rm -f ProtobufJson
