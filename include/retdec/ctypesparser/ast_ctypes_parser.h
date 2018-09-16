#ifndef RETDEC_CTYPESPARSER_AST_CTYPES_PARSER_H
#define RETDEC_CTYPESPARSER_AST_CTYPES_PARSER_H

#include "retdec/ctypes/integral_type.h"
#include "retdec/ctypesparser/ctypes_parser.h"
#include "retdec/ctypes/module.h"
#include "llvm/Demangle/ItaniumDemangle.h"

namespace retdec {
namespace ctypesparser {

class ASTCTypesParser : public CTypesParser
{
	public:
		using Node = ::llvm::itanium_demangle::Node;

	public:
		ASTCTypesParser();
		ASTCTypesParser(unsigned defaultBitWidth);

		std::shared_ptr<ctypes::Module> parse(
			const Node *ast,
			const ctypes::CallConvention &callConvention = ctypes::CallConvention());

		void addTypesToMap(const TypeWidths &widthMap);

	private:
		std::shared_ptr<ctypes::Function> parseFunction(const llvm::itanium_demangle::FunctionEncoding *funcN,
														const std::shared_ptr<ctypes::Context> &context,
														const ctypes::CallConvention &callConvention = ctypes::CallConvention());

		ctypes::Function::Parameters parseParameters(const llvm::itanium_demangle::NodeArray &params,
													 const std::shared_ptr<ctypes::Context> &context);
		std::shared_ptr<ctypes::Type> parseRetType(const Node &retTypeNode,
												   const std::shared_ptr<ctypes::Context> &context);

		unsigned getIntegralTypeBitWidth(const std::string &type) const;
		ctypes::IntegralType::Signess getSigness(const std::string &type);

		unsigned getBitWidthOrDefault(const std::string &typeName) const;

};

}
}

#endif //RETDEC_CTYPESPARSER_AST_CTYPES_PARSER_H
