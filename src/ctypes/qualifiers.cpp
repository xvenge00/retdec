/**
* @file src/ctypes/qualifiers.cpp
* @brief A representation of certain function and type qualifiers
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#include "retdec/ctypes/qualifiers.h"

namespace retdec {
namespace ctypes {

/**
 * @brief Constructs representation of constant qualifier. Doesn't make sense by itself.
 *
 * Default value is non constant.
 */
ConstantQualifier::ConstantQualifier(Constantness constantness) : constantness(constantness) {}

/**
 * @brief Sets value of qualified type as Constant.
 */
void ConstantQualifier::setAsConstant()
{
	constantness = Constantness::Constant;
}

/**
 * Returns @c true when modified type is constant.
 */
bool ConstantQualifier::isConstant() const
{
	return constantness == Constantness::Constant;
}

} // namespace ctypes
} // namespace retdec
