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
#include "retdec/ctypes/context.h"
#include "retdec/ctypes/function.h"
#include "retdec/ctypes/reference_type.h"
#include "retdec/ctypes/qualifiers.h"

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
			delete demangler;
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

TEST_F(ASTCTypesParserTests, ConstantPointerAsParameter)
{
	const char *mangled = "_Z3fooPKi"; // foo(int const*)
	auto context = mangledToCtypes(mangled);

	EXPECT_TRUE(context -> hasFunctionWithName("foo"));
	auto function = context->getFunctionWithName("foo");

	EXPECT_EQ(function->getParameterCount(), 1);
	auto ptr = function->getParameter(1).getType();
	EXPECT_TRUE(ptr->isPointer());

	auto pointee = std::dynamic_pointer_cast<ctypes::PointerType>(ptr)->getPointedType();
	EXPECT_TRUE(pointee->isIntegral());
	EXPECT_TRUE(pointee->isConstant());
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

TEST_F(ASTCTypesParserTests, referenceTest)
{
	const char *mangled = "_Z3fooRi";	//foo(int&)

	auto context = mangledToCtypes(mangled);

	EXPECT_TRUE(context->hasFunctionWithName("foo"));

	auto function = context->getFunctionWithName("foo");
	EXPECT_EQ(function->getParameterCount(), 1);
	EXPECT_TRUE(function->getParameter(1).getType()->isReference());
}

TEST_F(ASTCTypesParserTests, ConstQualifierParameterTest) {
	const char *mangled = "_Z3fooRKi"; // foo(int const&)

	auto context = mangledToCtypes(mangled);

	EXPECT_TRUE(context->hasFunctionWithName("foo"));

	auto function = context->getFunctionWithName("foo");
	EXPECT_EQ(function->getParameterCount(), 1);

	auto ref = function->getParameter(1).getType();
	EXPECT_TRUE(ref->isReference());

	auto referenced = std::dynamic_pointer_cast<ctypes::ReferenceType>(ref)->getReferencedType();
	EXPECT_TRUE(referenced->isIntegral());
	EXPECT_TRUE(std::dynamic_pointer_cast<ctypes::IntegralType>(referenced)->isConstant());
}

TEST_F(ASTCTypesParserTests, ConstQualifierNotSetAsDefault)
{
	const char *mangled = "_Z3fooRi";	//foo(int&)
	auto context = mangledToCtypes(mangled);

	EXPECT_TRUE(context->hasFunctionWithName("foo"));

	auto function = context->getFunctionWithName("foo");
	auto ref = function->getParameter(1).getType();
	auto referenced = std::dynamic_pointer_cast<ctypes::ReferenceType>(ref)->getReferencedType();
	EXPECT_TRUE(referenced->isIntegral());
	EXPECT_FALSE(std::dynamic_pointer_cast<ctypes::IntegralType>(referenced)->isConstant());
}

//TEST_F(ASTCTypesParserTests, ClassNameAsParameter)
//{
//	const char *mangled = "_Z3fooR3Bar";
//
//	auto context = mangledToCtypes(mangled);
//}

//TEST_F(ASTCTypesParserTests, QualifiersTest)
//{
//	const char *mangled = "_Z3fooIiEVKdPid";	//is template
//	auto context = mangledToCtypes(mangled);
//}

} // namespace tests
} // namespace ctypesparser
} // namespace retdec

