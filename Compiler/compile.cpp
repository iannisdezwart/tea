#include "Compiler/compiler.hpp"
#include "Compiler/util.hpp"

int
main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: ./compile input.tea output.teax [ --debug ]\n");
		exit(1);
	}

	char *input_file_name  = argv[1];
	char *output_file_name = argv[2];
	std::string debug_flag = argc > 3 ? argv[3] : "";
	bool debug = _debug = debug_flag == "--debug" || debug_flag == "-d";

	Compiler compiler(input_file_name, output_file_name, debug);
	compiler.compile();
}