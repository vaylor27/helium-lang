#pragma once

#include <vector>
#include "./Tokenization.cpp"
#include "variant"
#include "Arena.cpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

//struct NodeBinExprMulti {
//    NodeExpr* lhs;
//    NodeExpr* rhs;
//};

struct NodeBinExpr {
    NodeBinExprAdd* var;
};

struct NodeTerm {
    std::variant<NodeTermIdent*, NodeTermIntLit*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};


struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtVar {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtVar*> var;
};

struct NodeProgram {
    std::vector<NodeStmt*> stmts;
};

class Parser {
    public:
        inline explicit Parser(std::vector<Token> pTokens): tokens(std::move(pTokens)), allocator(1024 * 1024 * 50) {
        }

        std::optional<NodeTerm*> parseTerm() {
            if (auto intLit = tryConsume(TokenType::int_lit)) {
                auto intLitTerm = allocator.alloc<NodeTermIntLit>();
                intLitTerm->int_lit = intLit.value();
                auto term = allocator.alloc<NodeTerm>();
                term->var = intLitTerm;
                return term;
            } else if (auto ident = tryConsume(TokenType::ident)) {
                auto identTerm = allocator.alloc<NodeTermIdent>();
                identTerm->ident = ident.value();
                auto expr = allocator.alloc<NodeTerm>();
                expr->var = identTerm;
                return expr;
            } else {
                return {};
            }
        }

        std::optional<NodeExpr*> parseExpr() {
            if (auto term = parseTerm()) {
                if (tryConsume(TokenType::plus).has_value()) {
                        auto binExpr = allocator.alloc<NodeBinExpr>();
                        auto binExprAdd = allocator.alloc<NodeBinExprAdd>();
                        auto lhsExpr = allocator.alloc<NodeExpr>();
                        lhsExpr->var = term.value();
                        binExprAdd->lhs = lhsExpr;
                        if (auto rhs = parseExpr()) {
                            binExprAdd->rhs = rhs.value();
                            binExpr->var = binExprAdd;
                            auto expr = allocator.alloc<NodeExpr>();
                            expr->var = binExpr;
                            return expr;
                        } else {
                            std::cerr << "Expected Expression" << std::endl;
                            exit(EXIT_FAILURE);
                        }
                } else {
                    auto expr = allocator.alloc<NodeExpr>();
                    expr->var = term.value();
                    return expr;
                }
            } else {
                return {};
            }
        }

        std::optional<NodeStmt*> parseStmt() {
            if (peek().value().type == TokenType::exit && peek(1).has_value() && peek(1).value().type == TokenType::open_paren) {
                consume();
                consume();
                NodeStmtExit* stmtExit;
                if (auto exprNode = parseExpr()) {
                    stmtExit = allocator.alloc<NodeStmtExit>();
                    stmtExit->expr = exprNode.value();
                } else {
                    std::cerr << "Exit Does Not Contain An Integer/Expression as exit code!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                tryConsume(TokenType::close_paren, ')');
                tryConsume(TokenType::semi, ';');
                auto stmtNode = allocator.alloc<NodeStmt>();
                stmtNode->var = stmtExit;
                return stmtNode;
            } else if (peek().has_value() && peek().value().type == TokenType::var && peek(1).has_value() && peek(1).value().type == TokenType::ident && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
                consume();
                auto varStmt = allocator.alloc<NodeStmtVar>();
                varStmt->ident = consume();
                consume();
                if (auto expr = parseExpr()) {
                    varStmt->expr = expr.value();
                } else {
                    std::cerr << "Invalid expression for identifier!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                tryConsume(TokenType::semi, ';');
                auto stmtNode = allocator.alloc<NodeStmt>();
                stmtNode->var = varStmt;
                return stmtNode;
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
        ArenaAllocator allocator;


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

        inline Token tryConsume(TokenType type, char c) {
            if (peek().has_value() && peek().value().type == type) {
                return consume();
            } else {
                std::cerr << "Expected '" << c << "'" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        inline std::optional<Token> tryConsume(TokenType type) {
            if (peek().has_value() && peek().value().type == type) {
                return consume();
            } else {
                return {};
            }
        }
};