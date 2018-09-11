/**
* @file tests/ctypes/template_tests.cpp
* @brief Tests for the template module.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include <gtest/gtest.h>

#include "retdec/ctypes/call_convention.h"
#include "retdec/ctypes/context.h"
#include "retdec/ctypes/function.h"
#include "retdec/ctypes/function_type.h"
#include "retdec/ctypes/integral_type.h"
#include "retdec/ctypes/parameter.h"
#include "retdec/ctypes/template.h"
#include "retdec/ctypes/template_param.h"

using namespace ::testing;

namespace retdec {
namespace ctypes {
namespace tests {

class TemplateTests : public Test
{
	public:
		TemplateTests() :
			context(std::make_shared<Context>()),
			intType(IntegralType::create(context, "int", 32)),
			paramsOneInt{Parameter("firstParamName", intType)},
			function(Function::create(context, "foo", intType, emptyParams)){}

	protected:
		std::shared_ptr <Context> context;
		std::shared_ptr <Type> intType;
		Function::Parameters emptyParams;
		Function::Parameters paramsOneInt;
		std::shared_ptr <Function> function;

};

TEST_F(TemplateTests,
BasicUsage)
{
	std::vector <TemplateParam> params{};

	TemplateParam par1{"foo"};
	params.emplace_back(par1);

	TemplateParam par2{"3"};
	params.emplace_back(par2);

	TemplateParam par3{intType};
	params.emplace_back(par3);

	Template templ{params, function};

	EXPECT_EQ(templ.getFunc(),function);

	EXPECT_EQ(templ.getParams().size(), 3);
	EXPECT_EQ(templ.getParams()[0].getValue(), "foo");
}

TEST_F(TemplateTests,
NoFunction)
{
	std::vector <TemplateParam> params{};

	TemplateParam par1{"foo"};
	params.emplace_back(par1);

	Template templ{params, nullptr};

	EXPECT_EQ(templ.getFunc(),nullptr);

	EXPECT_EQ(templ.getParams().size(), 1);
}

}
}
}