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
#include "llvm/Demangle/allocator.h"

#include "retdec/ctypesparser/ast_ctypes_parser.h"
#include "retdec/ctypes/context.h"
#include "retdec/ctypes/function.h"
#include "retdec/ctypes/reference_type.h"
#include "retdec/ctypes/qualifiers.h"

using namespace ::testing;

namespace retdec {
namespace ctypesparser {
namespace tests {

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
			parser(){
			setMap(parser);
		}

		std::shared_ptr<ctypes::Context> mangledToCtypes(const char *MangledName){
			if (MangledName == nullptr) {
				return nullptr;
			}

			llvm::itanium_demangle::Db<llvm::itanium_demangle::DefaultAllocator>
				demangler{MangledName, MangledName + std::strlen(MangledName)};

			auto ast = llvm::itaniumDemangleToAST(&status, &demangler);

			// TODO check status

			return parser.parse(ast);
		}

	protected:
		int status;
		ASTCTypesParser parser;
};

TEST_F(ASTCTypesParserTests, Basic)
{
	const char *mangled = "_Z3fooii";	//foo(int, int);

	auto context = mangledToCtypes(mangled);

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

TEST_F(ASTCTypesParserTests, ClassNameAsParameter)
{
	const char *mangled = "_Z3fooR3Bar";	// foo(Bar&);

	auto context = mangledToCtypes(mangled);

	EXPECT_TRUE(context->hasFunctionWithName("foo"));
	EXPECT_TRUE(context->hasNamedType("Bar"));
	auto Bar = context->getNamedType("Bar");
	EXPECT_TRUE(Bar->isClass());
}

TEST_F(ASTCTypesParserTests, functionWithNamespace)
{
	std::string name{"foo"};
	std::string nameSpace{"n1::n2::n3::n4"};
	const char *mangled = "_ZN2n12n22n32n43fooEi"; // n1::n2::n3::n4::foo(int)

	auto context = mangledToCtypes(mangled);

	EXPECT_FALSE(context->hasFunctionWithName(name));
	EXPECT_TRUE(context->hasFunctionWithName(name, nameSpace));

	auto func = context->getFunctionWithName(name, nameSpace);
	EXPECT_EQ(func->getName(), name);
	EXPECT_EQ(func->getNameSpace(), nameSpace);
}

TEST_F(ASTCTypesParserTests, AfterTwoRunsTwoFunctionsInContext)
{
 	const char *mangled1 = "_Z3food";
 	const char *mangled2 = "_Z3bard";

	auto context = std::make_shared<ctypes::Context>();

	llvm::itanium_demangle::Db<llvm::itanium_demangle::DefaultAllocator>
		demangler1{mangled1, mangled1 + std::strlen(mangled1)};
	llvm::itanium_demangle::Db<llvm::itanium_demangle::DefaultAllocator>
		demangler2{mangled2, mangled2 + std::strlen(mangled2)};

	auto ast1 = llvm::itaniumDemangleToAST(&status, &demangler1);
	auto ast2 = llvm::itaniumDemangleToAST(&status, &demangler2);

	parser.parseInto(ast1, context);
	parser.parseInto(ast2, context);

 	EXPECT_TRUE(context->hasFunctionWithName("foo"));
	EXPECT_TRUE(context->hasFunctionWithName("bar"));
}

TEST_F(ASTCTypesParserTests, TemplateParsing)
{
	const char *mangled = "_Z3fooIiET_S0_";	// int foo<int>(int)
	auto context = mangledToCtypes(mangled);

	EXPECT_TRUE(context->hasFunctionWithName("foo"));
	auto foo = context->getFunctionWithName("foo");
	EXPECT_TRUE(foo->getReturnType()->isIntegral());
	EXPECT_EQ(foo->getParameterCount(), 1);
	EXPECT_TRUE(foo->getParameter(1).getType()->isIntegral());
}

} // namespace tests
} // namespace ctypesparser
} // namespace retdec

