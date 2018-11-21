/**
 * @file include/llvm/Demangle/demangler_base.h
 * @brief Demangler library.
 * @copyright (c) 2018 Avast Software, licensed under the MIT license
 */

#ifndef RETDEC_LLVM_DEMANGLE_RETDEC_H
#define RETDEC_LLVM_DEMANGLE_RETDEC_H

#include <string>

namespace retdec {
namespace demangler {

/**
 * Abstract base class for all demanglers
 */
class Demangler
{
	public:
		enum Status: u_char
		{
			success = 0,
			init_fail = 1,
			memory_alloc_failure = 2,
			invalid_mangled_name = 3,
			unknown = 4,
		};

	public:
		explicit Demangler(const std::string &compiler);

		virtual ~Demangler() = default;

		virtual std::string demangleToString(const std::string &mangled) = 0;

		Status status();

	protected:
		std::string _compiler;
		Status _status;
};

}
}

#endif //RETDEC_LLVM_DEMANGLE_RETDEC_H