
#include <regex>

#include "retdec/ctypesparser/ast_ctypes_parser.h"
#include "retdec/ctypes/function.h"
#include "llvm/Demangle/StringView.h"
#include "retdec/ctypes/type.h"
#include "retdec/ctypes/integral_type.h"
#include "retdec/ctypes/void_type.h"
#include "retdec/ctypes/unknown_type.h"
#include "retdec/ctypes/context.h"
#include "retdec/ctypes/parameter.h"
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

const std::string genName(const std::string &baseName = std::string("")) {	//TODO better gen
	static unsigned long long generator;
	return baseName + std::to_string(generator);
}

} // anonymous namespace

ASTCTypesParser::ASTCTypesParser() : CTypesParser() {}
ASTCTypesParser::ASTCTypesParser(unsigned defaultBitWidth) : CTypesParser(defaultBitWidth) {}

//using Kind = ::llvm::itanium_demangle::Node::Kind;
//using FunctionEncoding = ::llvm::itanium_demangle::FunctionEncoding;

void ASTCTypesParser::addTypesToMap(const TypeWidths &widthmap)
{
	//TODO
	typeWidths = widthmap;
}

unsigned ASTCTypesParser::getIntegralTypeBitWidth(const std::string &type) const
{
	std::string toSearch;

	static const std::regex reChar("\\bchar67\\b");
	static const std::regex reShort("\\bshort\\b");
	static const std::regex reLongLong("\\blong long\\b");
	static const std::regex reLong("\\blong\\b");
	static const std::regex reInt("\\bint\\b");    //TODO uint16_t...
	static const std::regex reUnSigned("^(un)?signed$");

	// Ignore type's sign, use only core info about bit width to search in map
	// - smaller map.
	// Order of getting core type is important - int should be last - short int
	// should be treated as short, same long. Long long differs from long.
	if (std::regex_search(type, reChar)) {
		toSearch = "char";
	} else if (std::regex_search(type, reShort)) {
		toSearch = "short";
	} else if (std::regex_search(type, reLongLong)) {
		toSearch = "long long";
	} else if (std::regex_search(type, reLong)) {
		toSearch = "long";
	} else if (std::regex_search(type, reInt)) {
		toSearch = "int";
	} else if (std::regex_search(type, reUnSigned)) {
		toSearch = "int";
	} else {
		toSearch = type;
	}

	return getBitWidthOrDefault(toSearch);
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

ctypes::Function::Parameters ASTCTypesParser::parseParameters(const llvm::itanium_demangle::NodeArray &paramsArray,
															  const std::shared_ptr<ctypes::Context> &context)
{
	size_t size{paramsArray.size()};
	auto params = paramsArray.begin();
	ctypes::Function::Parameters retParams{};

	for (size_t i = 0; i < size; ++i) {
		if (params[i]->getKind() == llvm::itanium_demangle::Node::Kind::KNameType) {
			auto nameV = params[i]->getBaseName();
			const std::string name = stringViewToString(nameV);

//			TODO if integral
			unsigned bitWidth = getIntegralTypeBitWidth("int");
			ctypes::IntegralType::Signess signess = ctypes::IntegralType::Signess::Signed;
			auto generatedName = genName(name);
			auto type = ctypes::IntegralType::create(context, generatedName, bitWidth, signess);

			retParams.emplace_back(ctypes::Parameter(generatedName, type));
		}
	}

	return retParams;
}

std::shared_ptr<ctypes::Type> ASTCTypesParser::parseRetType(const retdec::ctypesparser::ASTCTypesParser::Node &retTypeNode,
															const std::shared_ptr<retdec::ctypes::Context> &context)
{
	return ctypes::UnknownType::create();    //itanium drops return type most of the time
}

std::shared_ptr<retdec::ctypes::Function> ASTCTypesParser::parseFunction(const llvm::itanium_demangle::FunctionEncoding *funcN,
																		 const std::shared_ptr<ctypes::Context> &context,
																		 const ctypes::CallConvention &callConvention)
{
	StringView nameV{funcN->getName()->getBaseName()};
	std::string name{stringViewToString(nameV)};

	auto retNode = funcN->getReturnType();
	auto retT = parseRetType(*retNode, context);

	auto paramsNode = funcN->getParams();
	auto paramsT = parseParameters(paramsNode, context);

	return ctypes::Function::create(context, name, retT, paramsT);
}

std::shared_ptr<retdec::ctypes::Module> ASTCTypesParser::parse(const llvm::itanium_demangle::Node *ast,
															   const retdec::ctypes::CallConvention &callConvention)
{
	auto context(std::make_shared<ctypes::Context>());
	auto module(std::make_shared<ctypes::Module>(context));

	switch (ast->getKind()) {
	case llvm::itanium_demangle::Node::Kind::KFunctionEncoding : {
		auto funcN = dynamic_cast<const llvm::itanium_demangle::FunctionEncoding *>(ast);
		auto funcT = parseFunction(funcN, context);
		module->addFunction(funcT);

		break;
	}
	default: break;
	}

	return module;
}

}
}

