/**
* @file src/ctypes/class_type.cpp
* @brief Implementation of custom classes.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#include <cassert>

#include "retdec/ctypes/class_type.h"
#include "retdec/ctypes/visitor.h"

namespace retdec {
namespace ctypes {

/**
 * @brief Constructs new ClassType
 */
ClassType::ClassType(const std::string &name) : Type(name, 0) {}

/**
* @brief Creates class type.
*
* @param context Storage for already created functions, types.
* @param name Name of new class type.
*
* @par Preconditions
*  - @a context is not null
*
* Does not create new class type, if one
* has already been created and stored in @c context.
*/
std::shared_ptr<ClassType> ClassType::create(
	const std::shared_ptr<retdec::ctypes::Context> &context,
	const std::string &name)
{
	assert(context && "violated precondition - context cannot be null");

	auto type = context->getNamedType(name);
	if (type && type->isClass()) {
		return std::static_pointer_cast<ClassType>(type);
	}

	std::shared_ptr<ClassType> newType(new ClassType(name));
	context->addNamedType(newType);
	return newType;
}

/**
* Returns @c true when Type is class, @c false otherwise.
*/
bool ClassType::isClass() const
{
	return true;
}

void ClassType::accept(retdec::ctypes::Visitor *v)
{
	v->visit(std::static_pointer_cast<ClassType>(shared_from_this()));
}

} // namespace ctypes
} // namespace retdec
