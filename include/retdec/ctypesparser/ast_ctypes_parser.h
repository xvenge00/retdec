/**
* @file include/retdec/ctypesparser/ast_ctypes_parser.h
* @brief Implementation of CTypes parser from AST created by itanium demangler.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_CTYPESPARSER_AST_CTYPES_PARSER_H
#define RETDEC_CTYPESPARSER_AST_CTYPES_PARSER_H

#include "llvm/Demangle/ItaniumDemangle.h"

#include "retdec/ctypes/integral_type.h"
#include "retdec/ctypes/module.h"
#include "retdec/ctypesparser/ctypes_parser.h"

namespace retdec {
namespace ctypesparser {

/**
 * @brief Parser of CTypes represented in AST from itanium demangle
 */
class ASTCTypesParser: public CTypesParser
{
	public:
		using Node = ::llvm::itanium_demangle::Node;

	public:
		ASTCTypesParser();
		explicit ASTCTypesParser(unsigned defaultBitWidth);

		std::shared_ptr<ctypes::Context> parse(
			const Node *ast,
			const ctypes::CallConvention &callConvention = ctypes::CallConvention());

		void addTypesToMap(const TypeWidths &widthMap);

	private:
		enum class Types
		{
				TIntegral,
				TFloat,
				TBool,
				TUnknown
		};

		/// @name Parsing methods.
		/// @{
		std::shared_ptr<ctypes::Function> parseFunction(
			const llvm::itanium_demangle::FunctionEncoding *funcN,
			const std::shared_ptr<ctypes::Context> &context,
			const ctypes::CallConvention &callConvention = ctypes::CallConvention());

		ctypes::Function::Parameters parseParameters(
			const llvm::itanium_demangle::NodeArray &params,
			const std::shared_ptr<ctypes::Context> &context);

		std::shared_ptr<ctypes::Type> parseRetType(
			const Node *retTypeNode,
			const std::shared_ptr<ctypes::Context> &context);

		std::shared_ptr<ctypes::Type> parseType(const std::string &typeName);

		unsigned getBitWidthOrDefault(const std::string &typeName) const;
		ctypes::IntegralType::Signess getSigness(const std::string &type);
		std::pair<Types, unsigned> getTypeAndWidth(const std::string &typeName);
		/// @}
};

} // namespace ctypesparser
} // namespace retdec

#endif //RETDEC_CTYPESPARSER_AST_CTYPES_PARSER_H
