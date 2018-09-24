/**
* @file src/ctypes/template_param.cpp
* @brief Implementation of Template parameters.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#include "retdec/ctypes/template_param.h"

namespace retdec {
namespace ctypes {

/**
 * @brief Constructs Tempalte paramter based on string. Parameter kind is initialized on KValue.
 */
TemplateParam::TemplateParam(const std::string &value) :
	K(Kind::KValue), value{value} {}

/**
 * @brief Constructs Tempalte paramter based on Type. Parameter kind is initialized on KBuiltIn.
 */
TemplateParam::TemplateParam(const std::shared_ptr<retdec::ctypes::Type> &type) :
	K(Kind::KBuiltIn), type(type) {}

/**
 * Returns Kind of parameter.
 */
TemplateParam::Kind TemplateParam::getKind() { return K; }

/**
 * Returns built-in type or nullptr if parameter kind is not KBuiltIn.
 */
std::shared_ptr<Type> TemplateParam::getType() { return K == Kind::KBuiltIn ? type : nullptr; }


/**
 * Returns string representation of paramter or "" if parameter kind is not KValue.
 */
std::string TemplateParam::getValue() { return K == Kind::KValue ? value : ""; }

} // namespace ctypes
} // namespace retdec
