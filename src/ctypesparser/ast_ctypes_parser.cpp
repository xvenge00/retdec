
#include <regex>
#include <glob.h>

#include <retdec/ctypes/floating_point_type.h>
#include <retdec/ctypes/reference_type.h>
#include "retdec/ctypesparser/ast_ctypes_parser.h"
#include "retdec/ctypes/function.h"
#include "llvm/Demangle/StringView.h"
#include "retdec/ctypes/type.h"
#include "retdec/ctypes/integral_type.h"
#include "retdec/ctypes/void_type.h"
#include "retdec/ctypes/unknown_type.h"
#include "retdec/ctypes/context.h"
#include "retdec/ctypes/parameter.h"
#include "retdec/ctypes/pointer_type.h"
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

ASTCTypesParser::ASTCTypesParser() : CTypesParser() {}
ASTCTypesParser::ASTCTypesParser(unsigned defaultBitWidth) : CTypesParser(defaultBitWidth) {}

using Kind = ::llvm::itanium_demangle::Node::Kind;

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

std::pair<ASTCTypesParser::Types, unsigned> ASTCTypesParser::getTypeAndWidth(const std::string &typeName)
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
	} else if (std::regex_search(typeName, reDouble)){
		toSearch = "double";
		type = Types::TFloat;
	} else if(std::regex_search(typeName, reFloat)) {
		toSearch = "float";
		type = Types::TFloat;
	} else if(std::regex_search(typeName, reBool)) {
		toSearch = "bool";
		type = Types::TBool;
	} else {
		toSearch = typeName;
		type = Types::TUnknown;
	}

	return {type, getBitWidthOrDefault(toSearch)};
}

std::shared_ptr<ctypes::Type> ASTCTypesParser::parseType(const std::string &typeName)
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
	default: return ctypes::UnknownType::create();
	}
}

ctypes::Function::Parameters ASTCTypesParser::parseParameters(const llvm::itanium_demangle::NodeArray &paramsArray,
															  const std::shared_ptr<ctypes::Context> &context)
{
	size_t size{paramsArray.size()};
	auto params = paramsArray.begin();
	ctypes::Function::Parameters retParams{};

	for (size_t i = 0; i < size; ++i) {
		switch (params[i]->getKind()) {
		case Kind::KNameType: {
			auto nameV = params[i]->getBaseName();
			const std::string name = stringViewToString(nameV);

			auto type = parseType(name);
			retParams.emplace_back(ctypes::Parameter(type->getName(), type));

			break;
		}
		case Kind::KPointerType: {
			auto pointerNode = dynamic_cast<llvm::itanium_demangle::PointerType *>(params[i]);

			auto pointeeNode = pointerNode->getPointee();

			auto pointeeNameView = pointeeNode->getBaseName();
			auto pointeeNameString = stringViewToString(pointeeNameView);

			auto pointeeType = parseType(pointeeNameString);

			auto pointerType = ctypes::PointerType::create(context, pointeeType);    //TODO pointer width

			retParams.emplace_back(ctypes::Parameter(pointerType->getName(), pointerType));

			break;
		}
		case Kind::KReferenceType: {
			auto referenceNode = dynamic_cast<llvm::itanium_demangle::ReferenceType *>(params[i]);

			auto referencedNode = referenceNode->getPointee();

			auto referencedNameView = referencedNode->getBaseName();
			auto referencedNameString = stringViewToString(referencedNameView);

			auto referencedType = parseType(referencedNameString);

			auto referenceType = ctypes::ReferenceType::create(context, referencedType);

			retParams.emplace_back(ctypes::Parameter(referenceType->getName(), referenceType));
		}
		default: break;
		}
	}

	return retParams;
}

std::shared_ptr<ctypes::Type> ASTCTypesParser::parseRetType(const retdec::ctypesparser::ASTCTypesParser::Node *retTypeNode,
															const std::shared_ptr<retdec::ctypes::Context> &context)
{
	if (retTypeNode == nullptr) {
		return ctypes::UnknownType::create();
	}

	if (retTypeNode->getKind() == llvm::itanium_demangle::Node::Kind::KNameType) {
		auto typeNameView = retTypeNode->getBaseName();
		return parseType(stringViewToString(typeNameView));
	}

	return ctypes::UnknownType::create();

}

std::shared_ptr<retdec::ctypes::Function> ASTCTypesParser::parseFunction(const llvm::itanium_demangle::FunctionEncoding *funcN,
																		 const std::shared_ptr<ctypes::Context> &context,
																		 const ctypes::CallConvention &callConvention)
{
	auto nameNode = funcN->getName();

//	bool isTemplate = nameNode->getKind() == Kind::KNameWithTemplateArgs;

	StringView nameV{nameNode->getBaseName()};
	std::string name{stringViewToString(nameV)};

	auto retNode = funcN->getReturnType();
	auto retT = parseRetType(retNode, context);

	auto paramsNode = funcN->getParams();
	auto paramsT = parseParameters(paramsNode, context);

	auto function = ctypes::Function::create(context, name, retT, paramsT, callConvention);


	//TODO rewrite context to accept template
	return function;
}

std::shared_ptr<ctypes::Context> ASTCTypesParser::parse(const llvm::itanium_demangle::Node *ast,
														const retdec::ctypes::CallConvention &callConvention)
{
	auto context(std::make_shared<ctypes::Context>());

	switch (ast->getKind()) {
	case llvm::itanium_demangle::Node::Kind::KFunctionEncoding : {
		auto funcN = dynamic_cast<const llvm::itanium_demangle::FunctionEncoding *>(ast);
		auto funcT = parseFunction(funcN, context);

		break;
	}
	default: break;
	}

	return context;
}

}
}

