/**
* @file include/retdec/ctypes/qualifiers.h
* @brief A representation of certain function and type qualifiers
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_CTYPES_MODIFIERS_H
#define RETDEC_CTYPES_MODIFIERS_H

namespace retdec {
namespace ctypes {

/**
 * @brief A representation of constant qualifier.
 */
class ConstantQualifier
{
	public:
		enum class Constantness
		{
			Constant,
			Nonconstant
		};

	public:
		explicit ConstantQualifier(Constantness constantness = Constantness::Nonconstant);

		void setAsConstant();

		bool isConstant() const;

	protected:
		Constantness constantness;

};

} // namespace ctypes
} // namespace retdec

#endif //RETDEC_CTYPES_MODIFIERS_H
