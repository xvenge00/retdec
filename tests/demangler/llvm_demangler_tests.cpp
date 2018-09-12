#include <gtest/gtest.h>

#include "llvm/Demangle/Demangle.h"
//#include "llvm/Demangle/ItaniumDemangle.h"

using namespace ::testing;

//#define DEM_EQ(mangled, demangled) EXPECT_EQ(demangled, borland.demangleToString(mangled))

namespace retdec {
namespace demangler {
namespace tests {

class LlvmDemanglerTests : public Test
{
	public:
		LlvmDemanglerTests(){};
};

TEST_F(LlvmDemanglerTests,
BasicTest)
{
std::string s_mangled("_ZN3fooILi1EEC5Ev");
std::string s_demangled("foo<1>::foo()");

const char *mangled = s_mangled.c_str();
const char *demangled = s_demangled.c_str();

int status;

char *result = llvm::itaniumDemangle(mangled, nullptr, nullptr, &status);

EXPECT_TRUE(strcmp(result, demangled) == 0);
}




} // namespace tests
} // namespace demangler
} // namespace retdec

