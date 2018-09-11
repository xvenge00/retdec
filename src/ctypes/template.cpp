/**
* @file src/ctypes/template.cpp
* @brief Implementation of Templates.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include "retdec/ctypes/template.h"
#include "retdec/ctypes/template_param.h"

namespace retdec {
namespace ctypes {

Template::Template(std::vector<retdec::ctypes::TemplateParam> &params,
				   std::shared_ptr<retdec::ctypes::Function> function) : params(params), function(function) {}

std::shared_ptr<Function> Template::getFunc()
{
	return function;
}

std::vector<TemplateParam> Template::getParams()
{
	return params;
}

}
}
