/**
* @file include/retdec/ctypes/template.h
* @brief Representation of C++ templates.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_CTYPES_TEMPLATE_H
#define RETDEC_CTYPES_TEMPLATE_H

#include "retdec/ctypes/template_param.h"
#include "retdec/ctypes/function.h"

namespace retdec {
namespace ctypes {

/**
 * @brief Representation of C++ templates.
 */
class Template
{
	public:
		Template(std::vector<TemplateParam> &params,
				 std::shared_ptr<Function> function);


		std::vector<TemplateParam> getParams();
		std::shared_ptr<Function> getFunc();


	private:
		std::vector<TemplateParam> params;
		std::shared_ptr<Function> function;

};

}
}

#endif //RETDEC_CTYPES_TEMPLATE_H
