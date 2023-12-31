
#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include "Tokenization.cpp"
#include "Parser.cpp"
#include "Generation.cpp"
#include "AstPrinter.cpp"

#define String std::string
#define StringStream std::stringstream
#define Vector std::vector
#define error std::cerr
#define in std::ios::in
#define out std::ios::out
#define FileStream std::fstream

int main(int argc, char* argv[]) {
    if (argc < 2) {
        error << "Requires Helium File (.he Extension) As Argument" << std::endl;
        return EXIT_FAILURE;
    }
    if (argc < 3) {
        error << "Requires OS As Argument [Linux, BSD, or MacOS]" << std::endl;
        return EXIT_FAILURE;
    }

    String contents;
    StringStream contents_stream;
    {
        FileStream input(argv[1], in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    Vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProgram> root = parser.parseProgram();

    if (!root.has_value()) {
        std::cerr << "No exit node found!" << std::endl;
        exit(EXIT_FAILURE);
    }
    ASTPrinter printer(root.value());
    std::cout << printer.generateProgram();

    Generator generator(root.value(), argv[2]);

    {
        FileStream file("out.asm", out);
        file << generator.generateProgram();
    }

    system("nasm -f macho64 out.asm");
    system("ld out.o -o out -macosx_version_min 10.13 -L/Library/Developer/CommandLineTools/SDKs/MacOSX13.3.sdk/usr/lib -lSystem");

    return EXIT_SUCCESS;
}