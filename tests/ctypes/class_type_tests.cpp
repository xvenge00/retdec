/**
* @file tests/ctypes/class_type_tests.cpp
* @brief Tests for the class_type module.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#include <memory>

#include <gtest/gtest.h>

#include "retdec/ctypes/class_type.h"
#include "retdec/ctypes/context.h"

using namespace ::testing;

namespace retdec {
namespace ctypes {
namespace tests {

class ClassTypeTests: public Test
{
	public:
		ClassTypeTests():
			context(std::make_shared<Context>()) {}

	protected:
		std::shared_ptr<Context> context;
};

TEST_F(ClassTypeTests, CreateClass)
{
	const std::string className{"Foo"};
	auto cls = ClassType::create(context, className);

	EXPECT_TRUE(context->hasNamedType(className));
	EXPECT_EQ(cls, context->getNamedType(className));
	EXPECT_TRUE(cls->isClass());
}

TEST_F(ClassTypeTests, EveryUniqueClassTypeIsCreatedOnlyOnce)
{
	const std::string className{"Foo"};
	auto cls1 = ClassType::create(context, className);
	auto cls2 = ClassType::create(context, className);

	EXPECT_EQ(cls1, cls2);
}

TEST_F(ClassTypeTests, TwoClassTypesWithDifferentNamesDiffer)
{
	auto cls1 = ClassType::create(context, "Foo");
	auto cls2 = ClassType::create(context, "Bar");

	EXPECT_NE(cls1, cls2);
}

TEST_F(ClassTypeTests, ClassHasCorrectName)
{
	const std::string className{"Foo"};
	auto cls = ClassType::create(context, className);

	EXPECT_EQ(className, cls->getName());
}

} // namespace tests
} // namespace ctypes
} // namespace retdec
