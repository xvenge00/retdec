/**
* @file tests/ctypes/reference_type_tests.cpp
* @brief Tests for the @c reference_type module.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#include <memory>

#include <gtest/gtest.h>

#include "retdec/ctypes/context.h"
#include "retdec/ctypes/integral_type.h"
#include "retdec/ctypes/reference_type.h"

using namespace ::testing;

namespace retdec {
namespace ctypes {
namespace tests {

class ReferenceTypeTests : public Test
{
	public:
		ReferenceTypeTests():
			context(std::make_shared<Context>()),
			intType(IntegralType::create(context, "int", 32)),
			charType(IntegralType::create(context, "char", 8)) {}

	protected:
		std::shared_ptr<Context> context;
		std::shared_ptr<IntegralType> intType;
		std::shared_ptr<IntegralType> charType;
};


TEST_F(ReferenceTypeTests,
	   EveryUniqueReferencedTypeIsCreatedOnlyOnce)
{
	auto obj1 = ReferenceType::create(context, intType);
	auto obj2 = ReferenceType::create(context, intType);

	EXPECT_EQ(obj1, obj2);
}

TEST_F(ReferenceTypeTests,
	   TwoPointerTypesWithDifferentReferencedTypesDiffer)
{
	auto obj1 = ReferenceType::create(context, intType);
	auto obj2 = ReferenceType::create(context, charType);

	EXPECT_NE(obj1, obj2);
}

TEST_F(ReferenceTypeTests,
	   GetReferencedTypeReturnsCorrectType)
{
	auto ptr = ReferenceType::create(context, intType);

	EXPECT_EQ(intType, ptr->getReferencedType());
}

TEST_F(ReferenceTypeTests,
	   IsReferenceReturnsTrueOnPointerType)
{
	EXPECT_TRUE(ReferenceType::create(context, intType)->isReference());
}

TEST_F(ReferenceTypeTests,
	   IsReferenceReturnsFalseOnNonPointerType)
{
	EXPECT_FALSE(intType->isReference());
}

TEST_F(ReferenceTypeTests,
	   CreateSetsBitWidthCorrectly)
{
	auto ptr = ReferenceType::create(context, intType, 33);

	EXPECT_EQ(33, ptr->getBitWidth());
}

TEST_F(ReferenceTypeTests,
	   DefaultBitWidthIsZero)
{
	auto ptr = ReferenceType::create(context, intType);

	EXPECT_EQ(0, ptr->getBitWidth());
}

} // namespace tests
} // namespace ctypes
} // namespace retdec
