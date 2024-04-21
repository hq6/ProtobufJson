# Protobuf JSON Converter

This CLI tool is a thin wrapper over the Protobuf C++ API for converting back
and forth between JSON and Protobuf.

It is intended to be used as part of scripts that talk to services that speak
proto-over-http, but it can also be used in isolation for decoding protobuf
messages, if you prefer JSON over the protobuf text format.

## Installation

This tool can be installed by downloading the latest
[release](https://github.com/hq6/ProtobufJson/releases) and unzipping it into your PATH,
or building from source. It runs on MacOS and Linux.

```
git clone https://github.com/hq6/ProtobufJson.git
cd ProtobufJson
make
```

## Example usage

Here's an invocation to convert JSON to protobuf.

```bash
./JsonToProto -I exampleProtoSearchPath Example '{"x": 5, "text": "Hello World"}' > Output.bin
```

Here's an invocation to convert protobuf to JSON.

```bash
./ProtoToJson -I exampleProtoSearchPath Example < Output.bin
```

A full "round trip" looks like:

```bash
./JsonToProto \
  --proto_path=exampleProtoSearchPath \
  Example \
  '{"x": 5, "text": "Hello World"}' | \
  ./ProtoToJson \
  --proto_path=exampleProtoSearchPath \
  Example
```

producing output:

```json
{
 "x": 5,
 "text": "Hello World"
}

```

## Proto imports are resolved relative to proto_path

In the following example, Example2.proto imports
`anotherSubdirectory/Example3.proto`, and both protos are resolved resolved relative to
the import path `exampleProtoSearchPath`, rather than relative to the path of Example2.proto.

```bash
./JsonToProto \
  --proto_path=exampleProtoSearchPath \
  -P anotherSubdirectory/Example2.proto \
  Example2 \
  '{"message":{"y": 5, "name": "Hello ProtobufJson"}}' > Output.bin


## Optimized usage

As an optimization for larger collections of protos, one can reduce search time
by explicitly specifying the proto file that defines the relevant proto.

```bash
./JsonToProto -I exampleProtoSearchPath -P example.proto Example '{"x": 5, "text": "Hello World"}' > Output.bin
./ProtoToJson -I exampleProtoSearchPath -P example.proto Example < Output.bin
```

Note that the reference to the `.proto` file must be provided relative to the
proto search paths specified with `--proto_path`. Attempts to reference a
`.proto` file by a full path (such as `${HOME}/path/to/example.proto`) will
fail.

If you specify -I without -P, the tool will print a hint to stderr with the
correct relative path.

## Dependencies

- [Protocol Buffers](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)
