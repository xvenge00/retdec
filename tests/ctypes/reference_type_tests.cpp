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
	   TwoReferenceTypesWithDifferentReferencedTypesDiffer)
{
	auto obj1 = ReferenceType::create(context, intType);
	auto obj2 = ReferenceType::create(context, charType);

	EXPECT_NE(obj1, obj2);
}

TEST_F(ReferenceTypeTests,
	   GetReferencedTypeReturnsCorrectType)
{
	auto ref = ReferenceType::create(context, intType);

	EXPECT_EQ(intType, ref->getReferencedType());
}

TEST_F(ReferenceTypeTests,
	   IsReferenceReturnsTrueOnReferenceType)
{
	EXPECT_TRUE(ReferenceType::create(context, intType)->isReference());
}

TEST_F(ReferenceTypeTests,
	   IsReferenceReturnsFalseOnNonReferenceType)
{
	EXPECT_FALSE(intType->isReference());
}

TEST_F(ReferenceTypeTests,
	   CreateSetsBitWidthCorrectly)
{
	auto ref = ReferenceType::create(context, intType, ReferenceType::Constantness::Nonconstant, 33);

	EXPECT_EQ(33, ref->getBitWidth());
}

TEST_F(ReferenceTypeTests,
	   DefaultBitWidthIsZero)
{
	auto ref = ReferenceType::create(context, intType);

	EXPECT_EQ(0, ref->getBitWidth());
}

TEST_F(ReferenceTypeTests,
	   CreatedConstantReference)
{
	auto ref = ReferenceType::create(context, intType, ReferenceType::Constantness::Constant);

	EXPECT_EQ(ReferenceType::Constantness::Constant, ref->getConstantness());
}

TEST_F(ReferenceTypeTests,
	   ConstantnessDeafultValueIsNonconstant)
{
	auto ref = ReferenceType::create(context, intType);

	EXPECT_EQ(ReferenceType::Constantness::Nonconstant, ref->getConstantness());
}

} // namespace tests
} // namespace ctypes
} // namespace retdec
