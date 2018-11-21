/**
 * @file include/llvm/Demangle/borland_ast.h
 * @brief Representation of syntactic tree for borland demangler.
 * @copyright (c) 2018 Avast Software, licensed under the MIT license
 */

#ifndef RETDEC_BORLAND_AST_H
#define RETDEC_BORLAND_AST_H

#include <memory>
#include <string>
#include <vector>

#include "llvm/Demangle/StringView.h"

namespace retdec {
namespace demangler {
namespace borland {

/**
 * @brief Base class for all nodes in AST.
 */
class Node
{
	public:
		enum class Kind
		{
				KBuiltIn,
				KCallConv,
				KFunction,
				KName,
				KNestedName,
				KNodeArray,
		};

	public:
		explicit Node(Kind kind, bool has_right_side = false);

		virtual ~Node() = default;

		void print(std::ostream &s);

		std::string str();

		Kind kind();

	protected:
		virtual void printLeft(std::ostream &s) = 0;

		virtual void printRight(std::ostream &s);

	protected:
		Kind _kind;
		bool _has_right;
};

/**
 * @brief Node for representation of built-in types.
 */
class BuiltInType: public Node
{
	public:
		static std::unique_ptr<BuiltInType> create(const StringView &typeName);

	private:
		explicit BuiltInType(const StringView &typeName);

		void printLeft(std::ostream &s) override;

	private:
		StringView _typeName;
};

/**
 * @brief Node for representation of calling conventions.
 */
class CallConv: public Node
{
	public:
		enum class Conventions
		{
				fastcall,
				cdecl,
				pascal,
				stdcall,
				unknown,
		};

	public:
		static std::unique_ptr<CallConv> create(Conventions &conv);

		Conventions conv();

	private:
		explicit CallConv(CallConv::Conventions &conv, bool has_right);

		void printLeft(std::ostream &s) override;

		void printRight(std::ostream &s) override;

	private:
		Conventions _conv;
};

/**
 * Node for representation of functions.
 */
class FunctionNode: public Node
{
	public:
		static std::unique_ptr<FunctionNode> create(
			std::unique_ptr<CallConv> call_conv,
			std::unique_ptr<Node> name,
			std::unique_ptr<Node> params);

	private:
		FunctionNode(
			std::unique_ptr<CallConv> call_conv,
			std::unique_ptr<Node> name,
			std::unique_ptr<Node> params);

		void printLeft(std::ostream &s) override;

	private:
		std::unique_ptr<CallConv> _call_conv;
		std::unique_ptr<Node> _name;
		std::unique_ptr<Node> _params;

};

/**
 * @brief Node for representation of names.
 */
class NameNode: public Node
{
	public:
		static std::unique_ptr<NameNode> create(const StringView &name);

	private:
		explicit NameNode(const StringView &name);

		void printLeft(std::ostream &s) override;

	private:
		StringView _name;
};

/**
 * @brief Node for representation of nested names.
 */
class NestedNameNode: public Node
{
	public:
		static std::unique_ptr<NestedNameNode> create(
			std::unique_ptr<Node> super, std::unique_ptr<Node> name);

	private:
		NestedNameNode(std::unique_ptr<Node> super, std::unique_ptr<Node> name);

		void printLeft(std::ostream &s) override;

	private:
		std::unique_ptr<Node> _super;
		std::unique_ptr<Node> _name;
};

/**
 * @brief Node for representation of arrays of nodes.
 */
class NodeArray: public Node
{
	public:
		static std::unique_ptr<NodeArray> create();

		void addNode(std::unique_ptr<Node> node);

	private:
		NodeArray();

		void printLeft(std::ostream &s) override;

	private:
		std::vector<std::unique_ptr<Node>> _nodes;
};

}    // borland
}    // demangler
}    // retdec

#endif //RETDEC_BORLAND_AST_H