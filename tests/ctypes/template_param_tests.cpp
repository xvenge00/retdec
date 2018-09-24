/**
* @file tests/ctypes/template_param_tests.cpp
* @brief Tests for the template parameters module.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include <gtest/gtest.h>

#include "retdec/ctypes/call_convention.h"
#include "retdec/ctypes/context.h"
#include "retdec/ctypes/function.h"
#include "retdec/ctypes/function_type.h"
#include "retdec/ctypes/integral_type.h"
#include "retdec/ctypes/parameter.h"
#include "retdec/ctypes/template_param.h"

using namespace ::testing;

namespace retdec {
namespace ctypes {
namespace tests {

class TemplateParamTests : public Test
{
	public:
		TemplateParamTests() :
			context(std::make_shared<Context>()),
			intType(IntegralType::create(context, "int", 32)){}

	protected:
		std::shared_ptr <Context> context;
		std::shared_ptr <Type> intType;

};

TEST_F(TemplateParamTests,
	BasicUsage)
{
	TemplateParam par1{"foo"};
	EXPECT_EQ(par1.getValue(), "foo");
	EXPECT_EQ(par1.getType(), nullptr);

	TemplateParam par2{intType};
	EXPECT_EQ(par2.getValue(), "");
	EXPECT_EQ(par2.getType(), intType);
}

} // namespace tests
} // namespace ctypes
} // namespace retdec