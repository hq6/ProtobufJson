# Protobuf JSON Converter

This CLI tool is a thin wrapper over the Protobuf C++ API for converting back
and forth between JSON and Protobuf.

It is intended to be used as part of scripts that talk to services that speak
proto-over-http.

Once the dependency is installed this project can be built with an invocation
of `make` on either OSX or Linux.

## Example usage

Here's an invocation to convert JSON to protobuf.

```bash
./ProtobufJson ToProto Example.proto Example '{"x": 5, "text": "Hello World"}' > Output.bin
```

Here's an invocation to convert protobuf to JSON.

```bash
./ProtobufJson  ToJson Example.proto Example < Output.bin
```

Note that the reference to the `.proto` file must be provided relative to the proto search paths specified with `--proto_path` (by default, the proto search path is the current directory). Attempts to reference a `.proto` file by a full path (such as `${HOME}/path/to/example.proto`) will fail. Instead, specify `--proto_path=${HOME}/path/to` and then specify `example.proto`, or specify `--proto_path=${HOME}/path` and then specify `to/example.proto`.

For example, from within this project directory, run:

```bash
./ProtobufJson ToProto \
  --proto_path=exampleProtoSearchPath \
  anotherSubdirectory/Example2.proto \
  Example2 \
  '{"message":{"y": 5, "name": "Hello ProtobufJson"}}' \
  > Output.bin
```

The directory `exampleProtoSearchPath` will be used for locating `.proto` files, including `Example2.proto`. Then, when `Example2.proto` imports `anotherSubdirectory/Example3.proto` `ProtobufJson` will search for that path within all specified `--proto_path` directories.

A full "round trip" looks like:

```bash
./ProtobufJson ToProto \
  --proto_path=exampleProtoSearchPath \
  anotherSubdirectory/Example2.proto \
  Example2 \
  '{"message":{"y": 5, "name": "Hello ProtobufJson"}}' | \
  ./ProtobufJson ToJson \
    --proto_path=exampleProtoSearchPath \
    anotherSubdirectory/Example2.proto \
    Example2
```

producing output:

```json
{
  "message": {
    "y": 5,
    "name": "Hello ProtobufJson"
  }
}
```

## Dependencies

- [Protocol Buffers](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)
