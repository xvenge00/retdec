/**
* @file tests/ctypesparser/ast_ctypes_parser_tests.cpp
* @brief Tests for the AST Ctypes Parser module.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "llvm/Demangle/ItaniumDemangle.h"
#include "llvm/Demangle/Demangle.h"
#include "retdec/ctypesparser/ast_ctypes_parser.h"
#include "retdec/ctypes/function.h"

using namespace ::testing;

namespace retdec {
namespace ctypesparser {
namespace tests {

//TODO build AST myself
//TODO rewrite test

void setMap(ASTCTypesParser &parser) {
	const CTypesParser::TypeWidths widths{
		{"int", 32}
	};

	parser.addTypesToMap(widths);
}

class ASTCTypesParserTests : public Test
{
	public:
		ASTCTypesParserTests():
			status(),
			parser(){
			setMap(parser);
		}

	protected:
		int status;
		ASTCTypesParser parser;
};

TEST_F(ASTCTypesParserTests, Basic)
{
	setMap(parser);
	const char *mangled = "_Z3fooii";
	llvm::itanium_demangle::Db<llvm::itanium_demangle::DefaultAllocator> *demangler;
	auto ast = llvm::itaniumDemangleToAST(mangled, &status, &demangler);


	auto module = parser.parse(ast);

	EXPECT_TRUE(module->hasFunctionWithName("foo"));

	auto function = module->getFunctionWithName("foo");
	EXPECT_TRUE(function->getReturnType()->isUnknown());
	EXPECT_EQ(function->getParameterCount(), 2);
	EXPECT_TRUE(true);

	free(demangler); 	//TODO valgrind - missmached free
}



} // namespace tests
} // namespace ctypesparser
} // namespace retdec

