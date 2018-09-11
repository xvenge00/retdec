/**
* @file tests/ctypes/namespace_tests.cpp
* @brief Tests for the namespace module.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include <gtest/gtest.h>

#include "retdec/ctypes/namespace.h"

using namespace ::testing;

namespace retdec {
namespace ctypes {
namespace tests {

class NamespaceTests : public Test
{
	public:
		NamespaceTests(){}

};

TEST_F(NamespaceTests,
BasicUsage)
{
	Namespace ns1{};
	EXPECT_EQ(1,1);
	EXPECT_EQ(ns1.getNames().size(), 0);

	std::vector<std::string> names{};
	names.emplace_back("foo");
	names.emplace_back("bar");
	Namespace ns2{names};

	EXPECT_EQ(ns2.getNames().size(), 2);
	EXPECT_EQ(ns2.getNames()[0], "foo");
};


} // namespace tests
} // namespace ctypes
} // namespace retdec
