#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
#pragma once

#include <sstream>
#include "./Parser.cpp"

class Generator {
    public:
        inline explicit Generator(NodeProgram pRoot, const std::string& os): root(std::move(pRoot)) {
            if (os == "MacOS") {
                main_name.assign("_main");
                std::stringstream name;
                name << "0x" << (2000000 + getBsdCall("exit"));
                assign(exit_name, name);
            } else if (os == "Linux") {
                main_name.assign("_start");
            } else if (os == "BSD") {
                main_name.assign("_start");
                exit_name.assign(&"" [ getBsdCall("exit")]);
            }
        }

        static void assign(std::string& var, std::stringstream& stream) {
            var.assign(stream.str());
            stream.clear();
            std::cerr << var;
        }

        [[nodiscard]] int getBsdCall(const std::string& name) const{
            return bsdCalls.at(name);
        }

        [[nodiscard]] int getLinuxCall(const std::string& name) const{
            return linuxCalls.at(name);
        }

        void generateTerm(const NodeTerm* term) {
            struct TermVisitor {
                Generator* generator;

                void operator()(const NodeTermIntLit* intLitTerm) const {
                    generator->output << "    mov rax, " << intLitTerm->int_lit.value.value() << "\n";
                    generator->push("rax");
                }

                void operator()(const NodeTermIdent* identTerm) const {
                    if (!generator->vars.contains(identTerm->ident.value.value())) {
                        std::cerr << "Undeclared identifier: " << identTerm->ident.value.value() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    const auto& var = generator->vars.at(identTerm->ident.value.value());
                    std::stringstream offset;
                    offset << "QWORD [rsp + " << (generator->stackSize -var.stackLocation - 1) * 8 << "]";
                    generator->push(offset.str());
                }
            };

            TermVisitor visitor{.generator = this};
            std::visit(visitor, term->var);
        }

        void generateBinaryExpr(const NodeBinExpr* binExpr) {
            struct BinExprVisitor{
                Generator* generator;
                void operator()(const NodeBinExprAdd* add) const {
                    generator->generateExpr(add->lhs);
                    generator->generateExpr(add->rhs);
                    generator->pop("rax");
                    generator->pop("rbx");
                    generator->output << "    add rax, rbx\n";
                    generator->push("rax");
                }
                void operator()(const NodeBinExprMulti* multi) const {
                    generator->generateExpr(multi->lhs);
                    generator->generateExpr(multi->rhs);
                    generator->pop("rax");
                    generator->pop("rbx");
                    generator->output << "    mul rbx\n";
                    generator->push("rax");
                }
            };
            BinExprVisitor visitor({.generator = this});
            std::visit(visitor, binExpr->var);
        }

        void generateExpr(const NodeExpr* expr) {
            struct ExprVisitor {
                Generator* generator;

                void operator()(const NodeTerm* term) const {
                    generator->generateTerm(term);
                }

                void operator()(const NodeBinExpr* binExpr) const {
                    generator->generateBinaryExpr(binExpr);
                }
            };

            ExprVisitor visitor{.generator = this};
            std::visit(visitor, expr->var);
        }

        void generateStmt(const NodeStmt* stmt) {
            struct StmtVisitor {
                Generator* generator;

                void operator ()(const NodeStmtExit* exitStmt) const {
                    generator->generateExpr(exitStmt->expr);
                    generator->output << "    mov rax, " << generator->exit_name << "\n";
                    generator->pop("rdi");
                    generator->output << "    syscall\n";
                }

                void operator ()(const NodeStmtVar* varStmt) const {
                    if (generator->vars.contains(varStmt->ident.value.value())) {
                        std::cerr << "Identifier already used!" << varStmt->ident.value.value() << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    generator->vars.insert({varStmt->ident.value.value(), Var{.stackLocation = generator->stackSize}});
                    generator->generateExpr(varStmt->expr);
                }
            };

            StmtVisitor visitor{.generator = this};
            std::visit(visitor, stmt->var);
        }


        [[nodiscard]] std::string generateProgram() {
            output << "global " << main_name << "\n" << main_name << ":\n";

            for (const NodeStmt* stmt: root.stmts) {
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
        std::string main_name;
        std::stringstream output;
        std::string exit_name;
        size_t stackSize = 0;
        std::unordered_map<std::string, Var> vars {};
        std::unordered_map<std::string, int> bsdCalls {{"exit", 1}};
        std::unordered_map<std::string, int> linuxCalls {{"exit", 60}};
};