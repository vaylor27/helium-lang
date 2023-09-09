#pragma once

#include <vector>
#include "./Tokenization.cpp"
#include "variant"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr {
    std::variant<NodeExprIntLit, NodeExprIdent> var;
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmtVar {
    Token ident;
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtVar> var;
};

struct NodeProgram {
    std::vector<NodeStmt> stmts;
};

class Parser {
    public:
        inline explicit Parser(std::vector<Token> pTokens): tokens(std::move(pTokens)) {
        }

        std::optional<NodeExpr> parseExpr() {
            if (peek().has_value() && peek().value().type == TokenType::int_lit) {
                return NodeExpr{.var = NodeExprIntLit{.int_lit = consume()}};
            } else if (peek().has_value() && peek().value().type == TokenType::ident) {
                return NodeExpr{.var = NodeExprIdent{.ident = consume()}};
            }
            else {
                return {};
            }
        }

        std::optional<NodeStmt> parseStmt() {
            if (peek().value().type == TokenType::exit && peek(1).has_value() && peek(1).value().type == TokenType::open_paren) {
                consume();
                consume();
                NodeStmtExit stmtExit;
                if (auto exprNode = parseExpr()) {
                    stmtExit = NodeStmtExit{.expr = exprNode.value()};
                } else {
                    std::cerr << "Exit Does Not Contain An Integer/Expression as exit code!" << std::endl;
                    exit(EXIT_FAILURE);
                };
                if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                    consume();
                } else {
                    std::cerr << "Expected ')'" << std::endl;
                }
                if (peek().has_value() && peek().value().type == TokenType::semi) {
                    consume();
                } else {
                    std::cerr << "Semicolon not found at end of line!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                return NodeStmt{.var = stmtExit};
            } else if (peek().has_value() && peek().value().type == TokenType::var && peek(1).has_value() && peek(1).value().type == TokenType::ident && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
                consume();
                auto varStmt = NodeStmtVar{.ident = consume()};
                consume();
                if (auto expr = parseExpr()) {
                    varStmt.expr = expr.value();
                } else {
                    std::cerr << "Invalid expression for identifier!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (peek().has_value() && peek().value().type == TokenType::semi) {
                    consume();
                } else {

                    std::cerr << "Expected Semicolon: ';'!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                return NodeStmt{.var = varStmt};
            } else {
                return {};
            }
        }

        std::optional<NodeProgram> parseProgram() {
            NodeProgram program;
            while (peek().has_value()) {
                if (auto stmt = parseStmt()) {
                    program.stmts.push_back(stmt.value());
                } else {
                    std::cerr << "Invalid Statement" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            return program;
        }

    private:
        const std::vector<Token> tokens;
        size_t index = 0;


        [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const {
            if (index + offset >= tokens.size()) {
                return {};
            } else {
                return tokens.at(index + offset);
            }
        }

        inline Token consume() {
            return tokens.at(index++);
        }
};