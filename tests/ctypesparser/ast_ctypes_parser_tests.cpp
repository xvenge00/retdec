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
#include "retdec/ctypes/context.h"

using namespace ::testing;

namespace retdec {
namespace ctypesparser {
namespace tests {

//TODO build AST myself
//TODO rewrite test
//TODO check return codes
//TODO test regexps

void setMap(ASTCTypesParser &parser) {
	const CTypesParser::TypeWidths widths{
		{"int", 32},
		{"double", 64}
	};

	parser.addTypesToMap(widths);
}

class ASTCTypesParserTests : public Test
{
	public:
		ASTCTypesParserTests():
			status(),
			parser(),
			demangler(){
			setMap(parser);
		}
		~ASTCTypesParserTests() override {
			free(demangler);	//TODO valgrind - missmached free
		}

		std::shared_ptr<ctypes::Context> mangledToCtypes(const char *mangled){
			auto ast = llvm::itaniumDemangleToAST(mangled, &status, &demangler);
			return parser.parse(ast);
		}

	protected:
		int status;
		ASTCTypesParser parser;
		llvm::itanium_demangle::Db<llvm::itanium_demangle::DefaultAllocator> *demangler;
};

TEST_F(ASTCTypesParserTests, Basic)
{
	const char *mangled = "_Z3fooii";	//foo(int, int);
	auto ast = llvm::itaniumDemangleToAST(mangled, &status, &demangler);

	auto context = parser.parse(ast);
	EXPECT_TRUE(context->hasFunctionWithName("foo"));

	auto function = context->getFunctionWithName("foo");
	EXPECT_TRUE(function->getReturnType()->isUnknown());
	EXPECT_EQ(function->getParameterCount(), 2);
}

TEST_F(ASTCTypesParserTests, pointerTest)
{
	const char *mangled = "_Z3fooPi";	//foo(int *)

	auto context = mangledToCtypes(mangled);

	EXPECT_TRUE(context -> hasFunctionWithName("foo"));

	auto function = context->getFunctionWithName("foo");
	EXPECT_EQ(function->getParameterCount(), 1);
	EXPECT_TRUE(function->getParameter(1).getType()->isPointer());
}

TEST_F(ASTCTypesParserTests, floatingPointTest)
{
	const char *mangled = "_Z3food";	//foo(double);

	auto context = mangledToCtypes(mangled);

	EXPECT_TRUE(context->hasFunctionWithName("foo"));

	auto function = context->getFunctionWithName("foo");
	EXPECT_EQ(function->getParameterCount(), 1);
	EXPECT_TRUE(function->getParameter(1).getType()->isFloatingPoint());
}

//TEST_F(ASTCTypesParserTests, QualifiersTest)
//{
//	const char *mangled = "_Z3fooIiEVKdPid";
//}

} // namespace tests
} // namespace ctypesparser
} // namespace retdec

