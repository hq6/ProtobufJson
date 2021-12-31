#include <getopt.h>
#include <strings.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/stubs/strutil.h>

using namespace google::protobuf::compiler;
using namespace google::protobuf::util;
using google::protobuf::FileDescriptor;
using google::protobuf::Descriptor;
using google::protobuf::DynamicMessageFactory;
using google::protobuf::Message;
using google::protobuf::Base64Unescape;

/**
 * Command line options for this program.
 */
struct Options {
  /**
   * If verbose > 0, print debugging information to stderr.
   */
  int verbose;

  /**
   * True means that we are converting protobuf to JSON. False means we are
   * converting JSON to Protobuf.
   */
  bool toJson;

  /**
   * A list of diretories to look for proto files in when resolving imports.
   * Imports must be rooted at one of these directories.
   * If omitted, the current working directory is assumed.
   */
  std::vector<const char*> protoPaths;

  /**
   * The name of the file defining the prototype message we wish to convert to
   * or from json.
   */
  const char* protoFilename;

  /**
   * The simple (excluding package path) name of the prototype message we wish to convert to or from json.
   */
  const char* messageName;

  /**
   * If given, this is either literal data provided on the command line or a
   * filename.
   * If not given, our converter reads from stdin.
   */
  const char* data;
};

/**
 * Print usage to stderr and exit.
 */
static void usage() {
  std::cerr <<
       "Usage: ProtobufJson <ToProto|ToJson> [--proto_path=PATH...] [--verbose] <proto_file> <message_name> [data] \n"
       "\n"
       "  There are two subcommands:\n"
       "    ToProto will assume the input is JSON and write binary protobuf to stdout.\n"
       "    ToJson will assume the input is Proto and write JSON to stdout.\n"
       "\n"
       "  Arguments:\n"
       "    -IPATH, --proto_path=PATH   Specify the directory in which to search for\n"
       "                                imports.  May be specified multiple times;\n"
       "                                directories will be searched in order.  If not\n"
       "                                given, the current working directory is used.\n"
       "    verbose                     When given, debug output will be printed to stderr.\n"
       "    proto_file                  The name of the file defining the prototype \n"
       "                                message we wish to convert to or from json.\n"
       "    message_name                The simple (excluding package path) name of the prototype\n"
       "                                message we wish to convert to or from json.\n"
       "    data                        If not provided as an argument, input is read from stdin.\n"
       "                                If provided, it is interpreted as a filename if it is prefixed\n"
       "                                with the `@` symbol, and otherwise as literal data.\n"
       "                                For ToProto, literal data is interpreted as JSON.\n"
       "                                For ToJson, literal data is interpreted as base64 encoded protobuf.\n";
  exit(1);
}

/**
 * Attempt to parse command line arguments. Print out an informative message to
 * stderr if an error is encountered.
 */
Options parseArguments(int argc, char** argv) {
  // Ensure that we have enough arguments for all required arguments.
  if (argc < 4 ) {
    fprintf(stderr, "Not enough arguments.");
    usage();
  }

  // Initialize default options
  Options options;
  options.verbose = 0;
  options.toJson = false;
  options.protoFilename = NULL;
  options.messageName = NULL;
  options.data = NULL;

  if (strcasecmp("ToProto", argv[1]) == 0) {
    options.toJson = false;
  } else if (strcasecmp("ToJson", argv[1]) == 0) {
    options.toJson = true;
  } else {
    fprintf(stderr, "Invalid command.");
    usage();
  }


  // Skip over command name and parse optional arguments
  argc--;
  argv++;
  static struct option long_options[] =
    {
      /* These options set a flag. */
      {"verbose", no_argument, &options.verbose, 1},
      {"proto_path", required_argument, 0, 'I'},
      {0, 0, 0, 0}
    };
  while (1) {
    int option_index = 0;
    int c;
    c = getopt_long(argc, argv, "I:", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
      // Do nothing if just set verbose flag
      case 0:
        break;
      case 'I':
        options.protoPaths.push_back(optarg);
        break;
      case '?':
        /* getopt_long already printed an error message. */
        break;
      default:
        usage();
    }
  }
  argc -= optind;
  argv += optind;

  // Make the protoPaths the current directory if empty.
  if (options.protoPaths.empty()) {
    options.protoPaths.push_back(".");
  }

  // Parse mandatory arguments and optional positional argument.
  if (argc < 2) {
    fprintf(stderr, "Missing mandatory positional arguments.\n");
    usage();
  }

  options.protoFilename = argv[0];
  options.messageName = argv[1];

  if (argc > 2) {
    options.data = argv[2];
  }

  // Print options output to debug option parsing.
  if (options.verbose) {
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\tToJson: %d\n", options.toJson);
    fprintf(stderr, "\tverbose: %d\n", options.verbose);
    fprintf(stderr, "\tproto_paths:\n");
    for (int i = 0; i < options.protoPaths.size(); i++) {
      fprintf(stderr, "\t\t%s\n", options.protoPaths[i]);
    }
    fprintf(stderr, "\tproto_file: %s\n", options.protoFilename);
    fprintf(stderr, "\tmessage_name: %s\n", options.messageName);
    fprintf(stderr, "\tdata: %s\n", options.data);
  }
  return options;
}

/**
 * Convert between JSON and protobuf.
 *
 * There are two subcommands:
 *   ToProto will assume the input is JSON and write binary protobuf to stdout.
 *   ToJson will assume the input is Proto and write JSON to stdout.
 *
 * If [data] is not provided as an argument, input is read from stdin.
 *
 * If [data] is provided, it is interpreted as a filename if it is prefixed
 * with the `@` symbol, and otherwise as literal data.
 *   For ToProto, literal data is interpreted as JSON.
 *   For ToJson, literal data is interpreted as base64 encoded protobuf.
 */
int main(int argc, char** argv){
  Options options = parseArguments(argc, argv);

  // Construct source tree and importer for source files.
  DiskSourceTree diskSourceTree;
  for (const char* importPath: options.protoPaths) {
    diskSourceTree.MapPath("", importPath);
  }

  class ErrorReporter: public MultiFileErrorCollector {
    public:
      virtual void AddError(const std::string & filename, int line, int column, const std::string & message) {
        std::cerr << "Error occured for " << filename << ":" << line << ":" <<
          column  << " " << message << std::endl;
      }
  } errorReporter;

  Importer importer(&diskSourceTree, &errorReporter);
  const FileDescriptor* fd = importer.Import(options.protoFilename);
  if (fd == NULL) {
    std::cerr << "Encoutered errors causing a full FD on import. Aborting..." << std::endl;
    exit(1);
  }
  if (options.verbose) {
    std::cerr << fd->DebugString() << std::endl;
  }

  // Construct a message from the given descriptor. This name must exclude the
  // package.
  const Descriptor* messageDescriptor =
      fd->FindMessageTypeByName(options.messageName);
  if (options.verbose) {
    std::cerr << messageDescriptor->DebugString() << std::endl;
  }
  DynamicMessageFactory dynamicMessageFactory;
  Message* message = dynamicMessageFactory.GetPrototype(messageDescriptor)->New();

  if (options.toJson) {
    // Converting protobuf to JSON
    if (options.data) {
      if (options.data[0] == '@') {
        // Read binary from file
        FILE* protoFile = fopen(options.data + 1, "rb");
        message->ParseFromFileDescriptor(fileno(protoFile));
        fclose(protoFile);
      } else {
        // Interpret as base64 encoded binary protobuf.
        std::string binaryProto;
        Base64Unescape(options.data, &binaryProto);
        message->ParseFromString(binaryProto);
      }
    } else {
      // Read binary from stdin.
      message->ParseFromIstream(&std::cin);
    }
    std::string jsonOutput;
    JsonPrintOptions printOptions;
    printOptions.preserve_proto_field_names = true;
    printOptions.always_print_primitive_fields = false;
    printOptions.add_whitespace = true;
    printOptions.always_print_enums_as_ints = false;
    MessageToJsonString(*message, &jsonOutput, printOptions);
    std::cout << jsonOutput << std::endl;
  } else {
    //  Converting JSON to protobuf
    if (options.data) {
      if (options.data[0] == '@') {
        // Read JSON from file
        std::ifstream jsonFile(options.data + 1);
        std::stringstream buffer;
        buffer << jsonFile.rdbuf();
        JsonStringToMessage(buffer.str(), message);
      } else {
        // Interpret as JSON Text
        JsonStringToMessage(options.data, message);
      }
    } else {
      // Read JSON from stdin.
      std::stringstream buffer;
      buffer << std::cin.rdbuf();
      JsonStringToMessage(buffer.str(), message);
    }
    message->SerializeToOstream(&std::cout);
  }
}
