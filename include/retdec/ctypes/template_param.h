#ifndef RETDEC_TEMPLATES_PARAM_H
#define RETDEC_TEMPLATES_PARAM_H

#include "retdec/ctypes/type.h"

namespace retdec {
namespace ctypes {

class TemplateParam
{
	public:
		enum class Kind {
			KValue,
			KBuiltIn
		};

	public:
		explicit TemplateParam(std::string value);
		explicit TemplateParam(std::shared_ptr<Type> type);

		Kind getKind();
		std::shared_ptr<Type> getType();
		std::string getValue();

	private:
		Kind K;

		std::string value;
		std::shared_ptr<Type> type;

};

}
}

#endif //RETDEC_TEMPLATES_PARAM_H
