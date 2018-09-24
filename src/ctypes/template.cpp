/**
* @file src/ctypes/template.cpp
* @brief Implementation of Templates.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#include "retdec/ctypes/template.h"
#include "retdec/ctypes/template_param.h"

namespace retdec {
namespace ctypes {

/**
 * @brief Constructs template representation
 * @param params Vector of template parameters.
 * @param function Function instantiated from template.
 */
Template::Template(
	std::vector<retdec::ctypes::TemplateParam> &params,
	std::shared_ptr<retdec::ctypes::Function> &function)
	: params(params), function(function) {}

/**
 * @brief Returns instantiated function.
 */
std::shared_ptr<Function> Template::getFunc()
{
	return function;
}

/**
 * @brief Returns template parameters.
 */
std::vector<TemplateParam> Template::getParams()
{
	return params;
}

} // namespace ctypes
} // namespace retdec
