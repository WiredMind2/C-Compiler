#pragma once

#include <vector>
#include <string>
#include "antlr4-runtime.h"
#include "SymbolTable.h"
#include "generated/ifccBaseVisitor.h"


class DeclarationVisitor : public ifccBaseVisitor {
	SymbolTable *symbolTable = new SymbolTable();

	std::vector<std::string> scopeStack;

	const std::string &currentScope() const { return scopeStack.back(); }

public:
	~DeclarationVisitor() override {
		delete symbolTable;
	}

	[[nodiscard]] SymbolTable *getSymbolTable() const {
		return symbolTable;
	}

	std::any visitProg(ifccParser::ProgContext *ctx) override;

	std::any visitStatement(ifccParser::StatementContext *ctx) override;

	std::any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override {
		return 0;
	}

	std::any visitDeclaration(ifccParser::DeclarationContext *ctx) override;

	std::any visitDeclaration_assignement(ifccParser::Declaration_assignementContext *ctx) override;

	std::any visitAssignment(ifccParser::AssignmentContext *ctx) override {
		return 0;
	}

	std::any visitExpr(ifccParser::ExprContext *ctx) override {
		return 0;
	}

};
