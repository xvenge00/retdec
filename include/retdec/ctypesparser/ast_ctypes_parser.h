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
#include "retdec/ctypes/pointer_type.h"
#include "retdec/ctypes/reference_type.h"
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

		void setCallConv(const ctypes::CallConvention &callConv);

		std::shared_ptr<ctypes::Context> parse(
			const Node *ast);

		void addTypesToMap(const TypeWidths &widthMap);

	private:
		std::shared_ptr<ctypes::Context> context;
		ctypes::CallConvention defaultCallConv;

		enum class Types
		{
			TIntegral,
			TFloat,
			TBool,
			TClass,
			TUnknown
		};

		/// @name Parsing methods.
		/// @{
		std::shared_ptr<ctypes::Function> parseFunction(
			const llvm::itanium_demangle::FunctionEncoding *funcN);

		std::pair<std::string, std::string> parseFuncName(
			const llvm::itanium_demangle::Node *nameNode);

		ctypes::Function::Parameters parseParameters(
			const llvm::itanium_demangle::NodeArray &params);

		std::shared_ptr<ctypes::Type> parseRetType(
			const Node *retTypeNode);

		std::shared_ptr<ctypes::Type> parseName(
			const llvm::itanium_demangle::NameType *nameNode);

		std::string getName(
			const llvm::itanium_demangle::Node *nameNode);

		std::string getNestedName(
			const llvm::itanium_demangle::Node *nameNode);

		std::shared_ptr<ctypes::Type> parseQualifiedName(
			const llvm::itanium_demangle::QualType *qualNode);

		void parseQuals(llvm::itanium_demangle::Qualifiers quals,
			std::shared_ptr<ctypes::Type> &type);

		std::shared_ptr<ctypes::PointerType> parsePointer(
			const llvm::itanium_demangle::PointerType *pointerNode);

		std::shared_ptr<ctypes::ReferenceType> parseRef(
			const llvm::itanium_demangle::ReferenceType *refNode);

		std::shared_ptr<ctypes::Type> parseType(const std::string &typeName);

		unsigned getBitWidthOrDefault(const std::string &typeName) const;
		ctypes::IntegralType::Signess getSigness(const std::string &type);
		std::pair<Types, unsigned> getTypeAndWidth(const std::string &typeName);
		/// @}
};

} // namespace ctypesparser
} // namespace retdec

#endif //RETDEC_CTYPESPARSER_AST_CTYPES_PARSER_H
