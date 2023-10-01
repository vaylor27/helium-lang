#pragma once


#include <iostream>
#include <vector>

enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    var,
    eq,
    plus,
    star
};

bool isBinaryOperator(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::star:
            return true;
        default:
            return false;
    }
}

std::optional<int> binaryPrecedence(TokenType type) {
    switch (type) {
        case TokenType::plus:
            return 0;
        case TokenType::star:
            return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};

class Tokenizer {
    public:
        inline explicit Tokenizer(std::string src) : source(std::move(src)) {
        }

        inline std::vector<Token> tokenize() {
            std::vector<Token> tokens;
            std::string buf;
            while (peek().has_value()) {
                if (std::isalpha(peek().value())) {
                    buf.push_back(consume());
                    while (peek() && std::isalnum(peek().value())) {
                        buf.push_back(consume());
                    }
                    if (buf == "exit") {
                        tokens.push_back({.type = TokenType::exit});
                        buf.clear();
                        continue;
                    } else if (buf == "var") {
                        tokens.push_back({.type = TokenType::var});
                        buf.clear();
                        continue;
                    }
                    else {
                        tokens.push_back({.type = TokenType::ident, .value = buf});
                        buf.clear();
                        continue;
                    }
                } else if (std::isdigit(peek().value())) {
                    buf.push_back(consume());
                    while (peek().has_value() && std::isdigit(peek().value())) {
                        buf.push_back(consume());
                    }
                    tokens.push_back({.type = TokenType::int_lit, .value = buf});
                    buf.clear();
                    continue;
                //TODO: make this whole section here into a token string which is checked for
                } else if (peek().value() == '(') {
                    consume();
                    tokens.push_back({.type = TokenType::open_paren});
                    continue;
                } else if (peek().value() == ')') {
                    consume();
                    tokens.push_back({.type = TokenType::close_paren});
                    continue;
                }
                else if (peek().value() == ';') {
                    consume();
                    tokens.push_back({.type = TokenType::semi});
                    continue;
                }
                else if (peek().value() == '+') {
                    consume();
                    tokens.push_back({.type = TokenType::plus});
                    continue;
                } else if (peek().value() == '*') {
                    consume();
                    tokens.push_back({.type = TokenType::star});
                    continue;
                }
                else if (peek().value() == '=') {
                    consume();
                    tokens.push_back({.type = TokenType::eq});
                    continue;
                } else if (isspace(peek().value())) {
                    consume();
                    continue;
                } else {
                    std::cerr << "WTF 1" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            index = 0;
            return tokens;
        }

    private:
        [[nodiscard]] inline std::optional<char> peek(int offset = 0) const {
            if (index + offset >= source.length()) {
                return {};
            } else {
                return source.at(index + offset);
            }
        }

        char consume() {
            return source.at(index++);

        }

        const std::string source;
        size_t index = 0;
};