# Protobuf JSON Converter

This CLI tool is a thin wrapper over the Protobuf C++ API for converting back
and forth between JSON and Protobuf.

It is intended to be used as part of scripts that talk to services that speak
proto-over-http.

Once the dependency is installed this project can be built with an invocation
of `make` on either OSX or Linux.

## Example usage
Here's an invocation to convert JSON to protobuf.

    ./ProtobufJson ToProto Example.proto Example '{"x": 5, "text": "Hello World"}' > Output.bin

Here's an invocation to convert protobuf to JSON.

    ./ProtobufJson  ToJson Example.proto Example < Output.bin

## Dependencies
 * [Protocol Buffers](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)
