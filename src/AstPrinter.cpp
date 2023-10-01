#include <iostream>
#include "sstream"
#include "Parser.cpp"

class ASTPrinter {;
    public:
        inline explicit ASTPrinter(NodeProgram pRoot): root(std::move(pRoot)) {
        }

        static std::string indentFromLevelsIndented(int levelsIndented) {
            std::stringstream stream = {};
            for (int i = 0; i < levelsIndented; ++i) {
                stream << "    ";
            }
            return stream.str();
        }

        static void assign(std::string& var, std::stringstream& stream) {
            var.assign(stream.str());
            stream.clear();
            std::cerr << var;
        }

        void generateTerm(const NodeTerm* term, int indentLevel) {
            struct TermVisitor {
                ASTPrinter* generator;
                int indentLevel;

                void operator()(const NodeTermIntLit* intLitTerm) const {
                    generator->output << indentFromLevelsIndented(indentLevel) << "Int Literal " << intLitTerm->int_lit.value.value() << std::endl;
                }

                void operator()(const NodeTermIdent* identTerm) const {
                    generator->output << indentFromLevelsIndented(indentLevel) << "Identifier " << identTerm->ident.value.value() << std::endl;
                }
            };

            TermVisitor visitor{.generator = this, .indentLevel = indentLevel};
            std::visit(visitor, term->var);
        }

        void generateBinaryExpr(const NodeBinExpr* binExpr, int indentLevel) {
            struct BinExprVisitor{
                ASTPrinter* generator;
                int indentLevel;

                void operator()(const NodeBinExprAdd* add) const {
                    generator->output << indentFromLevelsIndented(indentLevel) << "Addition Expression" << std::endl;
                    generator->output << indentFromLevelsIndented(indentLevel + 1) << "Left Hand Side" << std::endl;
                    generator->generateExpr(add->lhs, indentLevel + 2);
                    generator->output << indentFromLevelsIndented(indentLevel + 1) << "Right Hand Side" << std::endl;
                    generator->generateExpr(add->rhs, indentLevel + 2);
                }
                void operator()(const NodeBinExprMulti* multi) const {
                    generator->output << indentFromLevelsIndented(indentLevel) << "Multiplication Expression" << std::endl;
                    generator->output << indentFromLevelsIndented(indentLevel + 1) << "Left Hand Side" << std::endl;
                    generator->generateExpr(multi->lhs, indentLevel + 2);
                    generator->output << indentFromLevelsIndented(indentLevel + 1) << "Right Hand Side" << std::endl;
                    generator->generateExpr(multi->rhs, indentLevel + 2);
                }
            };
            BinExprVisitor visitor({.generator = this, .indentLevel = indentLevel});
            std::visit(visitor, binExpr->var);
        }

        void generateExpr(const NodeExpr* expr, int indentLevel) {
            struct ExprVisitor {
                ASTPrinter* generator;
                int indentLevel;

                void operator()(const NodeTerm* term) const {
                    generator->output << indentFromLevelsIndented(indentLevel) << "Term" << std::endl;
                    generator->generateTerm(term, indentLevel + 1);
                }

                void operator()(const NodeBinExpr* binExpr) const {
                    generator->output << indentFromLevelsIndented(indentLevel) << "Binary Expression" << std::endl;
                    generator->generateBinaryExpr(binExpr, indentLevel + 1);
                }
            };

            ExprVisitor visitor{.generator = this, .indentLevel = indentLevel};
            std::visit(visitor, expr->var);
        }

        void generateStmt(const NodeStmt* stmt, int indentLevel) {
            struct StmtVisitor {
                ASTPrinter* generator;
                int indentLevel;

                void operator ()(const NodeStmtExit* exitStmt) const {
                    generator->output << indentFromLevelsIndented(indentLevel) << "Exit" << std::endl;
                    generator->generateExpr(exitStmt->expr, indentLevel + 1);
                }

                void operator ()(const NodeStmtVar* varStmt) const {
                    generator->output << indentFromLevelsIndented(indentLevel) << "Variable Declaration " << varStmt->ident.value.value() << std::endl;
                    generator->generateExpr(varStmt->expr, indentLevel + 1);
                }
            };

            StmtVisitor visitor{.generator = this, .indentLevel = indentLevel};
            std::visit(visitor, stmt->var);
        }


        [[nodiscard]] std::string generateProgram() {
            output << "Program" << std::endl;

            for (const NodeStmt* stmt: root.stmts) {
                generateStmt(stmt, 1);
            }
            return output.str();
        }

    private:
        const NodeProgram root;
        std::stringstream output;
};