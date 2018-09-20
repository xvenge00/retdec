#ifndef RETDEC_CTYPES_REFERENCE_TYPE_H
#define RETDEC_CTYPES_REFERENCE_TYPE_H

#include <memory>

#include "retdec/ctypes/context.h"
#include "retdec/ctypes/type.h"

namespace retdec{
namespace ctypes{

class ReferenceType: public Type
{
	public:
		enum class Constantness {
				Constant,
				Nonconstant
		};
	public:
		static std::shared_ptr<ReferenceType> create(
			const std::shared_ptr<Context> &context,
			const std::shared_ptr<Type> &referencedType,
			Constantness constantness = Constantness::Nonconstant,
			unsigned bitWidth = 0
			);

		std::shared_ptr<Type> getReferencedType() const;
		Constantness getConstantness();

		bool isReference() const override;

		/// @name Visitor interface.
		/// @{
		virtual void accept(Visitor *v) override;
		/// @}

	private:
		explicit ReferenceType(const std::shared_ptr<Type> &referencedType,
			Constantness constantness = Constantness::Nonconstant, unsigned bitWidth=0);

	private:
		std::shared_ptr<Type> referencedType;
		Constantness constantness;
};

}
}

#endif //RETDEC_CTYPES_REFERENCE_TYPE_H
