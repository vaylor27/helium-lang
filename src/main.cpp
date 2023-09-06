#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

#define String std::string
#define StringStream std::stringstream
#define Vector std::vector
#define Optional std::optional
#define newLine std::endl
#define isAlphabeticCharacter std::isalpha
#define isAlphabeticOrNumericCharacter std::isalpha
#define error std::cerr
#define print std::cout
#define in std::ios::in
#define out std::ios::out
#define notNull &
#define FileStream std::fstream

enum class TokenType {
    exit,
    int_lit,
    semi
};

struct Token {
    TokenType type;
    Optional<String> value {};
};

Vector<Token> tokenize(const String& str) {
    Vector<Token> tokens;
    String buf;
    for (int i = 0; i < str.length(); i++) {
        char c = str.at(i);
        if (isAlphabeticCharacter(c)) {
            buf.push_back(c);
            i++;
            while (isAlphabeticOrNumericCharacter(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;

            if (buf == "exit") {
                tokens.push_back({.type = TokenType::exit});
                buf.clear();
                continue;
            } else {
                error << "WTF";
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isdigit(c)) {
            while (std::isdigit(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;
            tokens.push_back({.type = TokenType::int_lit, .value = buf});
            buf.clear();
        }
        else if (c == ';') {
            tokens.push_back({.type = TokenType::semi});
        }
        else if (std::isspace(c)) {
            continue;
        } else {
            error << "WTF";
            exit(EXIT_FAILURE);
        }
    }
    return tokens;
}

String tokens_to_asm(const Vector<Token> notNull tokens) {
    // is global _start on Linux
    StringStream output;
    output << "global _main\n_main:\n";
    for (int i = 0; i < tokens.size(); i++) {
        const Token notNull token = tokens.at(i);
        if (token.type == TokenType::exit) {
            if (i+ 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit) {
                if (i+ 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semi) {
                    // 0x2000000 is a magic number that needs to be added to the bsd syscalls when on macOS. MacOS uses bsd syscalls
                    output << "    mov rax, 0x2000001\n";
                    // This is the same on Mac as on Linux
                    output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }
    return output.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        error << "Requires Helium File (.he Extension) As Argument" << std::endl;
        return EXIT_FAILURE;
    }

    String contents;
    StringStream contents_stream;
    {
        FileStream input(argv[1], in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Vector<Token> tokens = tokenize(contents);

    {
        FileStream file("out.asm", out);
        file << tokens_to_asm(tokens);
    }

    system("nasm -f macho64 out.asm");
    system("ld out.o -o out -macosx_version_min 10.13 -L/Library/Developer/CommandLineTools/SDKs/MacOSX13.3.sdk/usr/lib -lSystem");

    return EXIT_SUCCESS;
}