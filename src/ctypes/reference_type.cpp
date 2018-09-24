/**
* @file src/ctypes/reference_type.cpp
* @brief Implementation of ReferenceType.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#include <cassert>

#include "retdec/ctypes/context.h"
#include "retdec/ctypes/reference_type.h"
#include "retdec/ctypes/visitor.h"

namespace retdec {
namespace ctypes {

ReferenceType::ReferenceType(
	const std::shared_ptr<Type> &referencedType,
	unsigned int bitWidth) :
	Type("", bitWidth),referencedType(referencedType) {}

std::shared_ptr<ReferenceType> ReferenceType::create(
	const std::shared_ptr<Context> &context,
	const std::shared_ptr<Type> &referencedType,
	unsigned int bitWidth)
{
	assert(context && "violated precondition - context cannot be null");
	assert(referencedType && "violated precondition - referencedType cannot be null");

	auto type = context->getReferenceType(referencedType);
	if (type) {
		return type;
	}

	std::shared_ptr<ReferenceType> newType(new ReferenceType(referencedType, bitWidth));
	context->addReferenceType(newType);
	return newType;
}

std::shared_ptr<Type> ReferenceType::getReferencedType() const
{
	return referencedType;
}

bool ReferenceType::isReference() const
{
	return true;
}

void ReferenceType::accept(Visitor *v)
{
	v->visit(std::static_pointer_cast<ReferenceType>(shared_from_this()));
}

}
}


