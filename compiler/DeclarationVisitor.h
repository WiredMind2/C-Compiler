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

	std::any visitBitwiseOR(ifccParser::BitwiseORContext *ctx) override {
		return 0;
	}

	std::any visitBitwiseXOR(ifccParser::BitwiseXORContext *ctx) override {
		return 0;
	}

	std::any visitBitwiseAND(ifccParser::BitwiseANDContext *ctx) override {
		return 0;
	}

	std::any visitEquality(ifccParser::EqualityContext *ctx) override {
		return 0;
	}

	std::any visitAdditive(ifccParser::AdditiveContext *ctx) override {
		return 0;
	}

	std::any visitMultiplicative(ifccParser::MultiplicativeContext *ctx) override {
		return 0;
	}

	std::any visitUnary(ifccParser::UnaryContext *ctx) override {
		return 0;
	}

	std::any visitPrimitive(ifccParser::PrimitiveContext *ctx) override {
		return 0;
	}
};
