#include <iostream>

#include "pe-parser.h"

int main(int argc, char ** argv) {
    std::string input_file;
    PARSER_OPERATION operation;
    if (argc < 2) {
        std::cerr << "Wrong number of arguments\n";
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "is-pe") == 0) {
            assert(i + 1 < argc);
            input_file = argv[++i];
            operation = PARSER_OPERATION::IS_PE;
        } else if (strcmp(argv[i], "import-functions") == 0) {
            assert(i + 1 < argc);
            input_file = argv[++i];
            operation = PARSER_OPERATION::IMPORT_FUNCTIONS;
        } else if (strcmp(argv[i], "export-functions") == 0) {
            input_file = argv[++i];
            operation = PARSER_OPERATION::EXPORT_FUNCTIONS;
        } else {
            std::cerr << "Wrong argument\n";
            return 1;
        }
    }

    PeParser parser;
    parser.open(input_file);
    std::vector<std::pair<std::string, std::vector<std::string>>> imports;
    std::vector<std::string> exports;
    switch (operation) {
        case PARSER_OPERATION::IMPORT_FUNCTIONS:
            imports = parser.import_dll();
            for (const auto &dll : imports) {
                std::cout << "" <<dll.first << "\n";
                for (const auto &func : dll.second) {
                    std::cout << "    " << func << "\n";
                }
            }
            parser.close();
            return 0;
        case PARSER_OPERATION::IS_PE:
            if (parser.is_pe()) {
                std::cout << "PE\n";
                parser.close();
                return 0;
            } else {
                std::cout << "Not PE\n";
                parser.close();
                return 1;
            }
        case PARSER_OPERATION::EXPORT_FUNCTIONS:
            exports = parser.export_function();
            for (const auto &e : exports) {
                std::cout << e << "\n";
            }
            parser.close();
            return 0;
        default:
            std::cerr << "Unsupported operation\n";
            parser.close();
            return 1;
    }
    return 0;
}
