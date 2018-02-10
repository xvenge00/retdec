/**
 * @file include/retdec/capstone2llvmir/capstone2llvmir.h
 * @brief Converts bytes to Capstone representation, and Capstone representation
 *        to LLVM IR.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#ifndef RETDEC_CAPSTONE2LLVMIR_RETDEC_CAPSTONE2LLVMIR_H
#define RETDEC_CAPSTONE2LLVMIR_RETDEC_CAPSTONE2LLVMIR_H

#include <cassert>
#include <memory>

#include <capstone/capstone.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

#include "retdec/utils/address.h"
#include "retdec/capstone2llvmir/exceptions.h"

namespace retdec {
namespace capstone2llvmir {

/**
 * This is an abstract Capstone 2 LLVM IR translator class.
 * It can be used to create instances of concrete classes.
 * It should also be possible to create concreate classes on their own (they
 * should have public constructors), so that it is not  neccessary to modify
 * this class when adding new translators.
 */
class Capstone2LlvmIrTranslator
{
	// Named constructors.
	//
	public:
		static std::unique_ptr<Capstone2LlvmIrTranslator> createArch(
				cs_arch a,
				llvm::Module* m,
				cs_mode basic = CS_MODE_LITTLE_ENDIAN,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createArm(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createThumb(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createArm64(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createMips32(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createMips64(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createMips3(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createMips32R6(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createX86_16(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createX86_32(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createX86_64(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createPpc32(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createPpc64(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createPpcQpx(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createSparc(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createSysz(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
		static std::unique_ptr<Capstone2LlvmIrTranslator> createXcore(
				llvm::Module* m,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);

		virtual ~Capstone2LlvmIrTranslator();

	// Capstone related getters.
	//
	public:
		virtual const csh& getCapstoneEngine() const = 0;
		virtual cs_arch getArchitecture() const = 0;
		virtual cs_mode getBasicMode() const = 0;
		virtual cs_mode getExtraMode() const = 0;

		virtual bool hasDelaySlot(uint32_t id) const = 0;
		virtual bool hasDelaySlotTypical(uint32_t id) const = 0;
		virtual bool hasDelaySlotLikely(uint32_t id) const = 0;
		virtual std::size_t getDelaySlot(uint32_t id) const = 0;

	// LLVM related getters and query methods.
	//
	public:
		virtual llvm::Module* getModule() const = 0;

		virtual bool isSpecialAsm2LlvmMapGlobal(llvm::Value* v) const = 0;
		virtual llvm::StoreInst* isSpecialAsm2LlvmInstr(llvm::Value* v) const = 0;
		virtual llvm::GlobalVariable* getAsm2LlvmMapGlobalVariable() const = 0;

		virtual bool isCallFunction(llvm::Function* f) const = 0;
		virtual bool isCallFunctionCall(llvm::CallInst* c) const = 0;
		virtual llvm::Function* getCallFunction() const = 0;

		virtual bool isReturnFunction(llvm::Function* f) const = 0;
		virtual bool isReturnFunctionCall(llvm::CallInst* c) const = 0;
		virtual llvm::Function* getReturnFunction() const = 0;

		virtual bool isBranchFunction(llvm::Function* f) const = 0;
		virtual bool isBranchFunctionCall(llvm::CallInst* c) const = 0;
		virtual llvm::Function* getBranchFunction() const = 0;

		virtual bool isCondBranchFunction(llvm::Function* f) const = 0;
		virtual bool isCondBranchFunctionCall(llvm::CallInst* c) const = 0;
		virtual llvm::Function* getCondBranchFunction() const = 0;

		virtual llvm::Function* getAsmFunction(const std::string& name) const = 0;

		virtual llvm::GlobalVariable* isRegister(llvm::Value* v) const = 0;
		virtual uint32_t getCapstoneRegister(llvm::GlobalVariable* gv) const = 0;
		virtual llvm::GlobalVariable* getRegister(uint32_t r) = 0;
		virtual std::string getRegisterName(uint32_t r) const = 0;
		virtual uint32_t getRegisterBitSize(uint32_t r) const = 0;
		virtual uint32_t getRegisterByteSize(uint32_t r) const = 0;
		virtual llvm::Type* getRegisterType(uint32_t r) const = 0;

	// Translation methods.
	//
	public:
		struct TranslationResult
		{
			llvm::StoreInst* first = nullptr;
			llvm::StoreInst* last = nullptr;
			std::size_t size = 0;
			/// This is any branch type. i.e. call, return, branch, cond branch.
			llvm::CallInst* branchCall = nullptr;
			bool inCondition = false;
			bool failed() const { return size == 0; }
		};

		virtual TranslationResult translate(
				const std::vector<uint8_t>& bytes,
				retdec::utils::Address a,
				llvm::IRBuilder<>& irb,
				bool stopOnBranch = false) = 0;

	// Public pure virtual methods that must be implemented in concrete classes.
	//
	public:
		/**
		 * Check if mode @c m is an allowed basic mode for the translator.
		 * This must be implemented in concrete classes, since it is
		 * architecture and translator specific.
		 * @return @c True if mode is allowed, @c false otherwise.
		 */
		virtual bool isAllowedBasicMode(cs_mode m) = 0;
		/**
		 * Check if mode @c m is an allowed extra mode for the translator.
		 * This must be implemented in concrete classes, since it is
		 * architecture and translator specific.
		 * @return @c True if mode is allowed, @c false otherwise.
		 */
		virtual bool isAllowedExtraMode(cs_mode m) = 0;
		/**
		 * Modify basic mode (e.g. CS_MODE_ARM to CS_MODE_THUMB). This must be
		 * implemented in concrete classes, so they can check if the requested
		 * mode is applicable. Not every basic mode can be used with every
		 * architecture. Translators for some architectures (e.g. CS_ARCH_X86)
		 * may not even allow switching between modes that is otherwise allowed
		 * by Capstone due to internal problems (e.g. different register
		 * environments between 16/32/64 x86 architectures).
		 */
		virtual void modifyBasicMode(cs_mode m) = 0;
		/**
		 * Modify extra mode (e.g. CS_MODE_LITTLE_ENDIAN to CS_MODE_BIG_ENDIAN).
		 * This must be implemented in concrete classes, so they can check if
		 * the requested mode is applicable. Not every special mode can be used
		 * with every architecture.
		 */
		virtual void modifyExtraMode(cs_mode m) = 0;

		virtual uint32_t getArchByteSize() = 0;
		virtual uint32_t getArchBitSize() = 0;
};

} // namespace capstone2llvmir
} // namespace retdec

#endif
