/**
* @file include/retdec/ctypes/class_type.h
* @brief A representation of custom classes.
* @copyright (c) 2018 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_CTYPES_CLASS_TYPE_H
#define RETDEC_CTYPES_CLASS_TYPE_H

#include "retdec/ctypes/type.h"
#include "retdec/ctypes/context.h"

namespace retdec {
namespace ctypes {

/**
 * @brief A representation of custom classes.
 */
class ClassType: public Type {
	public:
		static std::shared_ptr<ClassType> create(
			const std::shared_ptr<Context> &context,
			const std::string &name
			);

		/// @name Visitor interface.
		/// @{
		void accept(Visitor *v) override;
		/// @}

		bool isClass() const override;

	private:
		explicit ClassType(const std::string &name);
};

} // namespace ctypes
} // namespace retdec

#endif //RETDEC_CTYPES_CLASS_TYPE_H
