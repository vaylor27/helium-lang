#pragma once

#include "./Parser.cpp"

class Generator {
    public:
        inline explicit Generator(NodeProgram pRoot): root(std::move(pRoot)) {
        }

        void generateExpr(const NodeExpr& expr) {
            struct ExprVisitor {
                Generator* generator;

                void operator()(const NodeExprIntLit& intLitExpr) const {
                    generator->output << "    mov rax, " << intLitExpr.int_lit.value.value() << "\n";
                    generator->push("rax");
                }

                void operator()(const NodeExprIdent& identExpr) const {
                    if (!generator->vars.contains(identExpr.ident.value.value())) {
                        std::cerr << "Undeclared identifier: " << identExpr.ident.value.value() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    const auto& var = generator->vars.at(identExpr.ident.value.value());
                    std::stringstream offset;
                    offset << "QWORD [rsp + " << (generator->stackSize -var.stackLocation - 1) * 8 << "]";
                    generator->push(offset.str());
                }
            };

            ExprVisitor visitor{.generator = this};
            std::visit(visitor, expr.var);
        }

        void generateStmt(const NodeStmt& stmt) {
            struct StmtVisitor {
                Generator* generator;

                void operator ()(const NodeStmtExit& exitStmt) const {
                    generator->generateExpr(exitStmt.expr);
                    generator->output << "    mov rax, " << generator->exit_name << "\n";
                    generator->pop("rdi");
                    generator->output << "    syscall\n";
                }

                void operator ()(const NodeStmtVar& varStmt) {
                    if (generator->vars.contains(varStmt.ident.value.value())) {
                        std::cerr << "Identifier already used!" << varStmt.ident.value.value() << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    generator->vars.insert({varStmt.ident.value.value(), Var{.stackLocation = generator->stackSize}});
                    generator->generateExpr(varStmt.expr);
                }
            };

            StmtVisitor visitor{.generator = this};
            std::visit(visitor, stmt.var);
        }

        [[nodiscard]] std::string generateProgram() {
            output << "global " << main_name << "\n" << main_name << ":\n";

            for (const NodeStmt& stmt: root.stmts) {
                generateStmt(stmt);
            }

            output << "    mov rax, " << exit_name << "\n";
            output << "    mov rdi, 0\n";
            output << "    syscall\n";
            return output.str();
        }

    private:

        void push(const std::string& reg) {
            output << "    push " << reg << "\n";
            stackSize++;
        }

        void pop(const std::string& reg) {
            output << "    pop " << reg << "\n";
            stackSize--;
        }

        struct Var {
            size_t stackLocation;
        };

        const NodeProgram root;
        //TODO: make these not hardcoded for other OSs
        const std::string main_name = "_main";
        std::stringstream output;
        const std::string exit_name = "0x2000001";
        size_t stackSize = 0;
        std::unordered_map<std::string, Var> vars {};
};