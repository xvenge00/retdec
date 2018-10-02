/**
* @file src/ctypesparser/ast_ctypes_parser.cpp
* @brief Parser for C-types from AST created by itanium demangler.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#include <regex>
#include <glob.h>

#include "llvm/Demangle/StringView.h"

#include "retdec/ctypesparser/ast_ctypes_parser.h"
#include "retdec/ctypes/context.h"
#include "retdec/ctypes/class_type.h"
#include "retdec/ctypes/floating_point_type.h"
#include "retdec/ctypes/function.h"
#include "retdec/ctypes/integral_type.h"
#include "retdec/ctypes/parameter.h"
#include "retdec/ctypes/pointer_type.h"
#include "retdec/ctypes/qualifiers.h"
#include "retdec/ctypes/reference_type.h"
#include "retdec/ctypes/type.h"
#include "retdec/ctypes/void_type.h"
#include "retdec/ctypes/unknown_type.h"
#include "retdec/utils/container.h"

namespace retdec {
namespace ctypesparser {

namespace {
const std::string stringViewToString(StringView &sView)
{
	const char *begin{sView.begin()};
	size_t length = sView.end() - sView.begin();

	return (length > 0) ? std::string{begin, length} : std::string{};
}

const std::string genName(const std::string &baseName = std::string(""))
{    //TODO better gen
	static unsigned long long generator;
	return baseName + std::to_string(generator);
}
} // anonymous namespace

using Kind = ::llvm::itanium_demangle::Node::Kind;

ASTCTypesParser::ASTCTypesParser() :
	CTypesParser(), context(std::make_shared<ctypes::Context>()),
	defaultCallConv(ctypes::CallConvention()) {}

ASTCTypesParser::ASTCTypesParser(unsigned defaultBitWidth) :
	CTypesParser(defaultBitWidth), context(std::make_shared<ctypes::Context>()),
	defaultCallConv(ctypes::CallConvention()) {}

void ASTCTypesParser::setCallConv(const retdec::ctypes::CallConvention &callConv)
{
	defaultCallConv = callConv;
}

void ASTCTypesParser::addTypesToMap(const TypeWidths &widthmap)
{
	//TODO
	typeWidths = widthmap;
}

unsigned ASTCTypesParser::getBitWidthOrDefault(const std::string &typeName) const
{
	return retdec::utils::mapGetValueOrDefault(typeWidths, typeName, defaultBitWidth);
}

ctypes::IntegralType::Signess ASTCTypesParser::getSigness(const std::string &type)
{
	static const std::regex reUnsigned("^u.{6,8}|\\bunsigned\\b");

	bool search = std::regex_search(type, reUnsigned);
	return search ? ctypes::IntegralType::Signess::Unsigned : ctypes::IntegralType::Signess::Signed;
}

std::pair<ASTCTypesParser::Types,
		  unsigned> ASTCTypesParser::getTypeAndWidth(const std::string &typeName)
{
	//TODO uint16_t...
	std::string toSearch;
	ASTCTypesParser::Types type;

	static const std::regex reChar("\\b(unsigned )?char\\b");
	static const std::regex reShort("\\b(unsigned )?short\\b");
	static const std::regex reLongLong("\\b(unsigned )?long long\\b");
	static const std::regex reLong("\\b(unsigned )?long\\b");
	static const std::regex reInt("\\b(unsigned )?int\\b");
	static const std::regex reUnSigned("^(un)?signed$");

	static const std::regex reFloat("\\bfloat\\b");
	static const std::regex reDouble("\\bdouble\\b");

	static const std::regex reBool("\\bbool\\b");

	if (std::regex_search(typeName, reChar)) {
		toSearch = "char";
		type = Types::TIntegral;
	} else if (std::regex_search(typeName, reShort)) {
		toSearch = "short";
		type = Types::TIntegral;
	} else if (std::regex_search(typeName, reLongLong)) {
		toSearch = "long long";
		type = Types::TIntegral;
	} else if (std::regex_search(typeName, reLong)) {
		toSearch = "long";
		type = Types::TIntegral;
	} else if (std::regex_search(typeName, reInt)) {
		toSearch = "int";
		type = Types::TIntegral;
	} else if (std::regex_search(typeName, reUnSigned)) {
		toSearch = "int";
		type = Types::TIntegral;
	} else if (std::regex_search(typeName, reDouble)) {
		toSearch = "double";
		type = Types::TFloat;
	} else if (std::regex_search(typeName, reFloat)) {
		toSearch = "float";
		type = Types::TFloat;
	} else if (std::regex_search(typeName, reBool)) {
		toSearch = "bool";
		type = Types::TBool;
	} else {
		//probably class name
		return {Types::TClass, 0};
	}

	return {type, getBitWidthOrDefault(toSearch)};
}

std::shared_ptr<ctypes::Type> ASTCTypesParser::parseType(
	const std::string &typeName)
{
	unsigned bitWidth;
	ASTCTypesParser::Types types;
	std::tie(types, bitWidth) = getTypeAndWidth(typeName);

	switch (types) {
	case Types::TIntegral: {
		ctypes::IntegralType::Signess signess = getSigness(typeName);
		auto generatedName = genName(typeName);
		return ctypes::IntegralType::create(context, generatedName, bitWidth, signess);
	}
	case Types::TFloat: {
		auto generatedName = genName(typeName);
		return ctypes::FloatingPointType::create(context, generatedName, bitWidth);
	}
	case Types::TClass: {
		return ctypes::ClassType::create(context, typeName);
	}
	default: return ctypes::UnknownType::create();
	}
}

/**
 * @brief Parses Names from AST to ctypes::Type.
 * @param nameNode Node of Kind::KNameType
 */
std::shared_ptr<ctypes::Type> ASTCTypesParser::parseName(
	const llvm::itanium_demangle::NameType *nameNode)
{
	assert(nameNode && "violated precondition - nameNode cannot be null");

	auto nameV = nameNode->getBaseName();
	return parseType(stringViewToString(nameV));
}

/**
 * @brief Parses Qualified Name from AST to ctypes::Type.
 * @param qualNode Node of Kind::KQualType
 */
std::shared_ptr<ctypes::Type> ASTCTypesParser::parseQualifiedName(
	const llvm::itanium_demangle::QualType *qualNode)
{
	assert(qualNode && "violated precondition - qualNode cannot be null");

	auto quals = qualNode->getQuals();
	auto childNode =
		dynamic_cast<const llvm::itanium_demangle::NameType *>(qualNode->getChild());
	auto referencedType = parseName(childNode);

	parseQuals(quals, referencedType);

	return referencedType;
}

/**
 * Parses qualifiers and aplies them to type.
 */
void ASTCTypesParser::parseQuals(
	llvm::itanium_demangle::Qualifiers quals,
	std::shared_ptr<retdec::ctypes::Type> &type)
{
	assert(type && "violated precondition - type cannot be null");

	using qualifiers = llvm::itanium_demangle::Qualifiers;

	if (quals & qualifiers::QualConst) {
		type->setAsConstant();
	}
}

/**
 * @brief Parses pointer from AST to ctypes::PointerType.
 * @param pointerNode Node of KPointerType
 */
std::shared_ptr<ctypes::PointerType> ASTCTypesParser::parsePointer(
	const llvm::itanium_demangle::PointerType *pointerNode)
{
	assert(pointerNode && "violated precondition - pointerNode cannot be null");

	auto pointeeNode = pointerNode->getPointee();

	switch (pointeeNode->getKind()) {
	case Kind::KNameType: {
		auto childNode = dynamic_cast<const llvm::itanium_demangle::NameType *>(pointeeNode);
		auto pointeeType = parseName(childNode);
		return ctypes::PointerType::create(context, pointeeType);
	}
	case Kind::KQualType: {
		auto childNode = dynamic_cast<const llvm::itanium_demangle::QualType *>(pointeeNode);
		auto pointeeType = parseQualifiedName(childNode);
		return ctypes::PointerType::create(context, pointeeType);
	}
	default:return nullptr;
	}
	//TODO pointer width
}

/**
 * @brief Parses references from AST to ctypes::ReferenceType
 * @param refNode Node of Kind::KReferencetype
 */
std::shared_ptr<ctypes::ReferenceType> ASTCTypesParser::parseRef(
	const llvm::itanium_demangle::ReferenceType *refNode)
{
	assert(refNode && "violated precondition - refNode cannot be null");

	auto referencedNode = refNode->getPointee();

	switch (referencedNode->getKind()) {
	case Kind::KNameType: {
		auto childNode = dynamic_cast<const llvm::itanium_demangle::NameType *>(referencedNode);
		auto referencedType = parseName(childNode);
		return ctypes::ReferenceType::create(context, referencedType);
	}
	case Kind::KQualType: {
		auto qualNode = dynamic_cast<const llvm::itanium_demangle::QualType *>(referencedNode);
		auto referencedType = parseQualifiedName(qualNode);
		return ctypes::ReferenceType::create(context, referencedType);
	}
	default: return nullptr;
	}
	//TODO ref width
	//TODO referenceKind
}

/**
 * @brief Parses function paramaters from AST to ctypes.
 */
ctypes::Function::Parameters ASTCTypesParser::parseParameters(
	const llvm::itanium_demangle::NodeArray &paramsArray)
{
	//TODO parsed params test for nullptr
	size_t size{paramsArray.size()};
	auto params = paramsArray.begin();
	ctypes::Function::Parameters retParams{};

	for (size_t i = 0; i < size; ++i) {
		switch (params[i]->getKind()) {
		case Kind::KNameType: {
			auto nameNode = dynamic_cast<const llvm::itanium_demangle::NameType *>(params[i]);
			auto type = parseName(nameNode);
			retParams.emplace_back(ctypes::Parameter(type->getName(), type));
			break;
		}
		case Kind::KPointerType: {
			auto pointerNode = dynamic_cast<const llvm::itanium_demangle::PointerType *>(params[i]);
			auto pointerType = parsePointer(pointerNode);
			retParams.emplace_back(ctypes::Parameter(pointerType->getName(), pointerType));
			break;
		}
		case Kind::KReferenceType: {
			auto referenceNode =
				dynamic_cast<const llvm::itanium_demangle::ReferenceType *>(params[i]);
			auto referenceType = parseRef(referenceNode);
			retParams.emplace_back(ctypes::Parameter(referenceType->getName(), referenceType));
			break;
		}
		default: break;
		}
	}

	return retParams;
}

/**
 * @brief Parses return types of functions.
 * @param retTypeNode Node can be null.
 * @return Parsed Type
 */
std::shared_ptr<ctypes::Type> ASTCTypesParser::parseRetType(
	const retdec::ctypesparser::ASTCTypesParser::Node *retTypeNode)
{
	if (retTypeNode == nullptr) {
		return ctypes::UnknownType::create();
	}

	switch (retTypeNode->getKind()) {
	case Kind::KNameType: {
		return parseName(dynamic_cast<const llvm::itanium_demangle::NameType *>(retTypeNode));
	}
//	case Kind::KQualifiedName:{
//		return parseQualifiedName(dynamic_cast<const llvm::itanium_demangle::QualType *>(retTypeNode));
//	} //TODO
	default: return ctypes::UnknownType::create();
	}
}

/**
 * @brief Gets string representation od Node name
 * @param nameNode Node of KNameNode or KNestedName kind.
 * @return string representation of nested name.
 */
std::string ASTCTypesParser::getName(const llvm::itanium_demangle::Node *nameNode)
{
	auto nameView = nameNode->getBaseName();
	return stringViewToString(nameView);
}

/**
 * @brief Gets Nested names from Node in form "n1::n2::n3"
 * @param nameNode Node of KNameNode or KNestedName kind.
 * @return string representation of nested name.
 */
std::string ASTCTypesParser::getNestedName(
	const llvm::itanium_demangle::Node *nameNode)
{
	switch (nameNode->getKind()) {
	case Kind::KNameType: return getName(nameNode);
	case Kind::KNestedName: {
		std::string nestedName = getNestedName(
			dynamic_cast<const llvm::itanium_demangle::NestedName *>(nameNode)->getQual());
		std::string name = getName(nameNode);
		return nestedName + "::" + name;
	}
	default: return "";
	}
}

/**
 * @brief Parses name from Node into namespace and name pair
 * @param nameNode pointer to Node of KNameNode or KNestedName type
 * @return pair of <namespaceName, name>
 */
std::pair<std::string, std::string> ASTCTypesParser::parseFuncName(
	const llvm::itanium_demangle::Node *nameNode)
{
	std::string name{};
	std::string namespaceName{};

	auto nameView = nameNode->getBaseName();
	name = stringViewToString(nameView);

	if (nameNode->getKind() == Kind::KNestedName) {

		namespaceName = getNestedName(
			dynamic_cast<const llvm::itanium_demangle::NestedName *>(nameNode)->getQual());
	}

	return {namespaceName, name};
}

/**
 * @brief Parses AST function into ctypes::Function
 * @param funcN Node of Kind::KFunctionEncoding
 */
std::shared_ptr<retdec::ctypes::Function> ASTCTypesParser::parseFunction(
	const llvm::itanium_demangle::FunctionEncoding *funcN)
{
	assert(funcN && "violated precondition - funcN cannot be null");

	auto nameNode = funcN->getName();

//	bool isTemplate = nameNode->getKind() == Kind::KNameWithTemplateArgs;
	std::string funcName{};
	std::string funcNamespace{};
	std::tie(funcNamespace, funcName) = parseFuncName(nameNode);

	auto retT = parseRetType(funcN->getReturnType());
	auto paramsT = parseParameters(funcN->getParams());
	auto varargness = ctypes::FunctionType::VarArgness::IsNotVarArg;
	auto function = ctypes::Function::create(
		context, funcName, retT, paramsT, defaultCallConv, varargness, funcNamespace);

	//TODO rewrite context to accept template
	return function;
}

/**
 * @brief Parses CTypes from AST created by itanium demangler.
 */
std::shared_ptr<ctypes::Context> ASTCTypesParser::parse(
	const llvm::itanium_demangle::Node *ast)
{
	switch (ast->getKind()) {
	case llvm::itanium_demangle::Node::Kind::KFunctionEncoding : {
		auto funcN = dynamic_cast<const llvm::itanium_demangle::FunctionEncoding *>(ast);
		auto funcT = parseFunction(funcN);
		break;
	}
	default: break;
	}

	return context;
}

} // namespace ctypesparser
} // namespace retdec

