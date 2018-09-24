/**
* @file include/retdec/ctypes/template_param.h
* @brief Implementation of Template parameteres.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_TEMPLATES_PARAM_H
#define RETDEC_TEMPLATES_PARAM_H

#include "retdec/ctypes/type.h"

namespace retdec {
namespace ctypes {

/**
 * @brief Representation of Teplate parameteres.
 */
class TemplateParam
{
	public:
		enum class Kind
		{
			KValue,
			KBuiltIn
		};

	public:
		explicit TemplateParam(const std::string &value);
		explicit TemplateParam(const std::shared_ptr<Type> &type);

		Kind getKind();
		std::shared_ptr<Type> getType();
		std::string getValue();

	private:
		Kind K;

		std::string value;
		std::shared_ptr<Type> type;

};

} // namespace ctypes
} // namespace retdec

#endif //RETDEC_TEMPLATES_PARAM_H
