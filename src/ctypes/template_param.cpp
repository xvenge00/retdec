#include "retdec/ctypes/template_param.h"

namespace retdec {
namespace ctypes {

TemplateParam::TemplateParam(std::string value) :
	K(Kind::KValue), value{value} {}

TemplateParam::TemplateParam(std::shared_ptr<retdec::ctypes::Type> type) :
	K(Kind::KBuiltIn), type(type) {}

TemplateParam::Kind TemplateParam::getKind() { return K; }

std::shared_ptr<Type> TemplateParam::getType() { return K == Kind::KBuiltIn ? type : nullptr; }

std::string TemplateParam::getValue() { return K == Kind::KValue ? value : ""; }

}
}

