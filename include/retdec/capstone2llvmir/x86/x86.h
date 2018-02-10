/**
 * @file include/retdec/capstone2llvmir/x86/x86.h
 * @brief X86 implementation of @c Capstone2LlvmIrTranslator.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#ifndef RETDEC_CAPSTONE2LLVMIR_X86_X86_H
#define RETDEC_CAPSTONE2LLVMIR_X86_X86_H

#include <array>
#include <tuple>
#include <utility>

#include "retdec/capstone2llvmir/capstone2llvmir.h"
#include "retdec/capstone2llvmir/x86/x86_defs.h"

namespace retdec {
namespace capstone2llvmir {

class Capstone2LlvmIrTranslatorX86 : virtual public Capstone2LlvmIrTranslator
{
	public:
		virtual ~Capstone2LlvmIrTranslatorX86() {};

	public:
		virtual llvm::Function* getX87DataStoreFunction() = 0;
		virtual llvm::Function* getX87TagStoreFunction() = 0;
		virtual llvm::Function* getX87DataLoadFunction() = 0;
		virtual llvm::Function* getX87TagLoadFunction() = 0;
		virtual uint32_t getParentRegister(uint32_t r) = 0;
};

} // namespace capstone2llvmir
} // namespace retdec

#endif
