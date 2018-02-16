/**
* @file include/retdec/bin2llvmir/optimizations/decoder/decoder.h
* @brief Decode input binary into LLVM IR.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_BIN2LLVMIR_OPTIMIZATIONS_DECODER_DECODER_H
#define RETDEC_BIN2LLVMIR_OPTIMIZATIONS_DECODER_DECODER_H

#include <queue>
#include <sstream>

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

#include "retdec/utils/address.h"
#include "retdec/bin2llvmir/providers/asm_instruction.h"
#include "retdec/bin2llvmir/providers/config.h"
#include "retdec/bin2llvmir/providers/debugformat.h"
#include "retdec/bin2llvmir/providers/fileimage.h"
#include "retdec/bin2llvmir/optimizations/decoder/jump_targets.h"
#include "retdec/bin2llvmir/utils/ir_modifier.h"
#include "retdec/capstone2llvmir/capstone2llvmir.h"

namespace retdec {
namespace bin2llvmir {

class Decoder : public llvm::ModulePass
{
	public:
		static char ID;
		Decoder();
		virtual bool runOnModule(llvm::Module& m) override;
		bool runOnModuleCustom(
				llvm::Module& m,
				Config* c,
				FileImage* o,
				DebugFormat* d);

	private:
		bool runCatcher();
		bool run();

		bool initTranslator();
		void initEnvironment();
		void initEnvironmentAsm2LlvmMapping();
		void initEnvironmentPseudoFunctions();
		void initEnvironmentRegisters();

		void initRanges();
		void initAllowedRangesWithSegments();

		void initJumpTargets();

		void doDecoding();

		bool isArmOrThumb() const;

	private:
		llvm::Module* _module = nullptr;
		Config* _config = nullptr;
		FileImage* _image = nullptr;
		DebugFormat* _debug = nullptr;

		std::unique_ptr<capstone2llvmir::Capstone2LlvmIrTranslator> _c2l;

		retdec::utils::AddressRangeContainer _allowedRanges;
		retdec::utils::AddressRangeContainer _alternativeRanges;
		retdec::utils::AddressRangeContainer _processedRanges;
		JumpTargets _jumpTargets;

		const std::string _asm2llvmGv = "_asm_program_counter";
		const std::string _asm2llvmMd = "llvmToAsmGlobalVariableName";
		const std::string _callFunction = "__pseudo_call";
		const std::string _returnFunction = "__pseudo_return";
		const std::string _branchFunction = "__pseudo_branch";
		const std::string _condBranchFunction = "__pseudo_cond_branch";
		const std::string _x87dataLoadFunction = "__frontend_reg_load.fpr";
		const std::string _x87tagLoadFunction = "__frontend_reg_load.fpu_tag";
		const std::string _x87dataStoreFunction = "__frontend_reg_store.fpr";
		const std::string _x87tagStoreFunction = "__frontend_reg_store.fpu_tag";
		const std::string _entryPointFunction = "entry_point";
};

} // namespace bin2llvmir
} // namespace retdec

#endif
