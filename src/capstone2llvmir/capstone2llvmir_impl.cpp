/**
 * @file src/capstone2llvmir/capstone2llvmir.cpp
 * @brief Converts bytes to Capstone representation, and Capstone representation
 *        to LLVM IR.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#include <iomanip>
#include <iostream>

#include "capstone2llvmir/capstone2llvmir_impl.h"

namespace retdec {
namespace capstone2llvmir {

Capstone2LlvmIrTranslator_impl::Capstone2LlvmIrTranslator_impl(
		cs_arch a,
		cs_mode basic,
		cs_mode extra,
		llvm::Module* m)
		:
		_arch(a),
		_basicMode(basic),
		_extraMode(extra),
		_module(m)
{
	// Do not call anything here, especially virtual methods.
}

Capstone2LlvmIrTranslator_impl::~Capstone2LlvmIrTranslator_impl()
{
	closeHandle();
}

//
//==============================================================================
// Mode query & modification methods - from Capstone2LlvmIrTranslator.
//==============================================================================
//

void Capstone2LlvmIrTranslator_impl::modifyBasicMode(cs_mode m)
{
	if (!isAllowedBasicMode(m))
	{
		throw Capstone2LlvmIrModeError(
				_arch,
				m,
				Capstone2LlvmIrModeError::eType::BASIC_MODE);
	}

	if (cs_option(_handle, CS_OPT_MODE, m + _extraMode) != CS_ERR_OK)
	{
		throw CapstoneError(cs_errno(_handle));
	}

	_basicMode = m;
}

void Capstone2LlvmIrTranslator_impl::modifyExtraMode(cs_mode m)
{
	if (!isAllowedExtraMode(m))
	{
		throw Capstone2LlvmIrModeError(
				_arch,
				m,
				Capstone2LlvmIrModeError::eType::EXTRA_MODE);
	}

	if (cs_option(_handle, CS_OPT_MODE, m + _basicMode) != CS_ERR_OK)
	{
		throw CapstoneError(cs_errno(_handle));
	}

	_extraMode = m;
}

uint32_t Capstone2LlvmIrTranslator_impl::getArchBitSize()
{
	return getArchByteSize() * 8;
}

//
//==============================================================================
// Translation methods - from Capstone2LlvmIrTranslator.
//==============================================================================
//

Capstone2LlvmIrTranslator_impl::TranslationResult Capstone2LlvmIrTranslator_impl::translate(
		const std::vector<uint8_t>& bytes,
		retdec::utils::Address a,
		llvm::IRBuilder<>& irb,
		bool stopOnBranch)
{
	TranslationResult res;

	cs_insn* insn = cs_malloc(_handle);

	const uint8_t* code = bytes.data();
	size_t size = bytes.size();
	uint64_t address = a;

	_branchGenerated = nullptr;
	_inCondition = false;

	// TODO: hack, solve better.
	bool disasmRes = cs_disasm_iter(_handle, &code, &size, &address, insn);
	if (!disasmRes && _arch == CS_ARCH_MIPS && _basicMode == CS_MODE_MIPS32)
	{
		modifyBasicMode(CS_MODE_MIPS64);
		disasmRes = cs_disasm_iter(_handle, &code, &size, &address, insn);
		modifyBasicMode(CS_MODE_MIPS32);
	}

	while (disasmRes)
	{
		auto* a2l = generateSpecialAsm2LlvmInstr(irb, insn);
		if (res.first == nullptr)
		{
			res.first = a2l;
		}
		res.last = a2l;
		res.size = (insn->address + insn->size) - a;

		translateInstruction(insn, irb);

		// TODO: Optimize -- to make generation easier and nicer, some things
		// can be generated suboptimally. We should inspect every generated
		// ASM insruction and optimize some known patterns:
		//
		// 1. Load propagation:
		// a = load r
		// ... use a, not change r, no fnc call, etc.
		// b = load r
		// ... use b -> replace by a, remove b
		//
		// 2. Conversions:
		// a = cast b
		// ... use a
		// c = cast b
		// ... use c -> replace by a, remove c
		//
		// 3. Unused values (e.g. from loadOpBinary() where only one op used):
		// a = load x
		// ... a unused
		//
		// 4. Values used only for their type (e.g. op0 load in translateMov()):
		// a = load x
		// b = load y
		// c = convert b to a.type
		// store c x
		//
		// etc.

		if (_branchGenerated && stopOnBranch)
		{
			res.branchCall = _branchGenerated;
			res.inCondition = _inCondition;
			return res;
		}

		insn = cs_malloc(_handle);

		// TODO: hack, solve better.
		disasmRes = cs_disasm_iter(_handle, &code, &size, &address, insn);
		if (!disasmRes && _arch == CS_ARCH_MIPS && _basicMode == CS_MODE_MIPS32)
		{
			modifyBasicMode(CS_MODE_MIPS64);
			disasmRes = cs_disasm_iter(_handle, &code, &size, &address, insn);
			modifyBasicMode(CS_MODE_MIPS32);
		}
	}

	// TODO: nothing translated, or translation ended far before end
	// -> throw || or signal this to user (decoder).

	cs_free(insn, 1);

	return res;
}

//
//==============================================================================
// Capstone related getters - from Capstone2LlvmIrTranslator.
//==============================================================================
//

const csh& Capstone2LlvmIrTranslator_impl::getCapstoneEngine() const
{
	return _handle;
}

cs_arch Capstone2LlvmIrTranslator_impl::getArchitecture() const
{
	return _arch;
}

cs_mode Capstone2LlvmIrTranslator_impl::getBasicMode() const
{
	return _basicMode;
}

cs_mode Capstone2LlvmIrTranslator_impl::getExtraMode() const
{
	return _extraMode;
}

bool Capstone2LlvmIrTranslator_impl::hasDelaySlot(uint32_t id) const
{
	return false;
}

bool Capstone2LlvmIrTranslator_impl::hasDelaySlotTypical(uint32_t id) const
{
	return false;
}

bool Capstone2LlvmIrTranslator_impl::hasDelaySlotLikely(uint32_t id) const
{
	return false;
}

std::size_t Capstone2LlvmIrTranslator_impl::getDelaySlot(uint32_t id) const
{
	return 0;
}

llvm::GlobalVariable* Capstone2LlvmIrTranslator_impl::getRegister(uint32_t r)
{
	// This could be optimized using cache.
	auto rn = getRegisterName(r);
	return _module->getNamedGlobal(rn);
}

std::string Capstone2LlvmIrTranslator_impl::getRegisterName(uint32_t r) const
{
	auto fIt = _reg2name.find(r);
	if (fIt == _reg2name.end())
	{
		if (auto* n = cs_reg_name(_handle, r))
		{
			return n;
		}
		else
		{
			throw Capstone2LlvmIrError(
					"Missing name for register number: " + std::to_string(r));
		}
	}
	else
	{
		return fIt->second;
	}
}

uint32_t Capstone2LlvmIrTranslator_impl::getRegisterBitSize(uint32_t r) const
{
	auto* rt = getRegisterType(r);
	if (auto* it = llvm::dyn_cast<llvm::IntegerType>(rt))
	{
		return it->getBitWidth();
	}
	else if (rt->isHalfTy())
	{
		return 16;
	}
	else if (rt->isFloatTy())
	{
		return 32;
	}
	else if (rt->isDoubleTy())
	{
		return 64;
	}
	else if (rt->isX86_FP80Ty())
	{
		return 80;
	}
	else if (rt->isFP128Ty())
	{
		return 128;
	}
	else
	{
		throw Capstone2LlvmIrError(
				"Unhandled type of register number: " + std::to_string(r));
	}
}

uint32_t Capstone2LlvmIrTranslator_impl::getRegisterByteSize(uint32_t r) const
{
	return getRegisterBitSize(r) / 8;
}

llvm::Type* Capstone2LlvmIrTranslator_impl::getRegisterType(uint32_t r) const
{
	auto fIt = _reg2type.find(r);
	if (fIt == _reg2type.end())
	{
		assert(false);
		throw Capstone2LlvmIrError(
				"Missing type for register number: " + std::to_string(r));
	}
	return fIt->second;
}

//
//==============================================================================
// LLVM related getters and query methods.
//==============================================================================
//

llvm::Module* Capstone2LlvmIrTranslator_impl::getModule() const
{
	return _module;
}

bool Capstone2LlvmIrTranslator_impl::isSpecialAsm2LlvmMapGlobal(
		llvm::Value* v) const
{
	return _asm2llvmGv == v;
}

llvm::StoreInst* Capstone2LlvmIrTranslator_impl::isSpecialAsm2LlvmInstr(
		llvm::Value* v) const
{
	if (auto* s = llvm::dyn_cast<llvm::StoreInst>(v))
	{
		if (isSpecialAsm2LlvmMapGlobal(s->getPointerOperand()))
		{
			return s;
		}
	}
	return nullptr;
}

llvm::GlobalVariable* Capstone2LlvmIrTranslator_impl::getAsm2LlvmMapGlobalVariable() const
{
	return _asm2llvmGv;
}

bool Capstone2LlvmIrTranslator_impl::isCallFunction(llvm::Function* f) const
{
	return f == _callFunction;
}

bool Capstone2LlvmIrTranslator_impl::isCallFunctionCall(llvm::CallInst* c) const
{
	return c ? isCallFunction(c->getCalledFunction()) : false;
}

llvm::Function* Capstone2LlvmIrTranslator_impl::getCallFunction() const
{
	return _callFunction;
}

bool Capstone2LlvmIrTranslator_impl::isReturnFunction(llvm::Function* f) const
{
	return f == _returnFunction;
}

bool Capstone2LlvmIrTranslator_impl::isReturnFunctionCall(
		llvm::CallInst* c) const
{
	return c ? isReturnFunction(c->getCalledFunction()) : false;
}

llvm::Function* Capstone2LlvmIrTranslator_impl::getReturnFunction() const
{
	return _returnFunction;
}

bool Capstone2LlvmIrTranslator_impl::isBranchFunction(llvm::Function* f) const
{
	return _branchFunction == f;
}

bool Capstone2LlvmIrTranslator_impl::isBranchFunctionCall(
		llvm::CallInst* c) const
{
	return c ? isBranchFunction(c->getCalledFunction()) : false;
}

llvm::Function* Capstone2LlvmIrTranslator_impl::getBranchFunction() const
{
	return _branchFunction;
}

bool Capstone2LlvmIrTranslator_impl::isCondBranchFunction(
		llvm::Function* f) const
{
	return _condBranchFunction == f;
}

bool Capstone2LlvmIrTranslator_impl::isCondBranchFunctionCall(
		llvm::CallInst* c) const
{
	return c ? isCondBranchFunction(c->getCalledFunction()) : false;
}

llvm::Function* Capstone2LlvmIrTranslator_impl::getCondBranchFunction() const
{
	return _condBranchFunction;
}

llvm::GlobalVariable* Capstone2LlvmIrTranslator_impl::isRegister(
		llvm::Value* v) const
{
	auto it = _allLlvmRegs.find(llvm::dyn_cast_or_null<llvm::GlobalVariable>(v));
	return it != _allLlvmRegs.end() ? it->first : nullptr;
}

uint32_t Capstone2LlvmIrTranslator_impl::getCapstoneRegister(
		llvm::GlobalVariable* gv) const
{
	auto it = _allLlvmRegs.find(gv);
	return it != _allLlvmRegs.end() ? it->second : 0;
}

//
//==============================================================================
//
//==============================================================================
//

void Capstone2LlvmIrTranslator_impl::initialize()
{
	if (!isAllowedBasicMode(_basicMode))
	{
		throw Capstone2LlvmIrModeError(
				_arch,
				_basicMode,
				Capstone2LlvmIrModeError::eType::BASIC_MODE);
	}
	if (!isAllowedExtraMode(_extraMode))
	{
		throw Capstone2LlvmIrModeError(
				_arch,
				_extraMode,
				Capstone2LlvmIrModeError::eType::EXTRA_MODE);
	}

	openHandle(); // Sets both _basicMode and _extraMode.
	configureHandle();

	initializeRegNameMap();
	initializeRegTypeMap();
	initializeArchSpecific();

	generateEnvironment();
}

void Capstone2LlvmIrTranslator_impl::openHandle()
{
	cs_mode finalMode = static_cast<cs_mode>(_basicMode + _extraMode);
	if (cs_open(_arch, finalMode, &_handle) != CS_ERR_OK)
	{
		throw CapstoneError(cs_errno(_handle));
	}
}

void Capstone2LlvmIrTranslator_impl::configureHandle()
{
	if (cs_option(_handle, CS_OPT_DETAIL, CS_OPT_ON) != CS_ERR_OK)
	{
		throw CapstoneError(cs_errno(_handle));
	}
}

void Capstone2LlvmIrTranslator_impl::closeHandle()
{
	if (_handle != 0)
	{
		if (cs_close(&_handle) != CS_ERR_OK)
		{
			throw CapstoneError(cs_errno(_handle));
		}
	}
}

void Capstone2LlvmIrTranslator_impl::generateEnvironment()
{
	generateSpecialAsm2LlvmMapGlobal();
	generateCallFunction();
	generateReturnFunction();
	generateBranchFunction();
	generateCondBranchFunction();

	generateEnvironmentArchSpecific();
	generateRegisters();
	generateDataLayout();
}

/**
 * The generated global variable is unnamed. capstone2llvmir library does not
 * allow to specify or set its name. Users can however get the variable with
 * @c getAsm2LlvmMapGlobalVariable() and do whatever they want with it
 * (e.g. rename).
 */
void Capstone2LlvmIrTranslator_impl::generateSpecialAsm2LlvmMapGlobal()
{
	llvm::GlobalValue::LinkageTypes lt = llvm::GlobalValue::InternalLinkage;
	llvm::Constant* initializer = nullptr;
	auto* t = llvm::IntegerType::getInt64Ty(_module->getContext());
	if (initializer == nullptr
			&& lt != llvm::GlobalValue::LinkageTypes::ExternalLinkage)
	{
		initializer = llvm::ConstantInt::get(t, 0);
	}

	_asm2llvmGv = new llvm::GlobalVariable(
			*_module,
			t,
			false, // isConstant
			lt,
			initializer);
}

llvm::StoreInst* Capstone2LlvmIrTranslator_impl::generateSpecialAsm2LlvmInstr(
		llvm::IRBuilder<>& irb,
		cs_insn* i)
{
	retdec::utils::Address a = i->address;
	auto* gv = getAsm2LlvmMapGlobalVariable();
	auto* ci = llvm::ConstantInt::get(gv->getValueType(), a, false);
	auto* s = irb.CreateStore(ci, gv, true);

	auto* cip = llvm::ConstantInt::get(irb.getInt64Ty(), reinterpret_cast<uintptr_t>(i));
	auto* mdc = llvm::ConstantAsMetadata::get(cip);
	auto* mdn = llvm::MDNode::get(_module->getContext(), {mdc});
	s->setMetadata("asm", mdn);
	return s;
}

void Capstone2LlvmIrTranslator_impl::generateCallFunction()
{
	auto* ft = llvm::FunctionType::get(
			llvm::Type::getVoidTy(_module->getContext()),
			{llvm::Type::getIntNTy(_module->getContext(), getArchBitSize())},
			false);
	_callFunction = llvm::Function::Create(
			ft,
			llvm::GlobalValue::LinkageTypes::ExternalLinkage,
			"",
			_module);
}

llvm::CallInst* Capstone2LlvmIrTranslator_impl::generateCallFunctionCall(
		llvm::IRBuilder<>& irb,
		llvm::Value* t)
{
	auto* a1t = _callFunction->getArgumentList().front().getType();
	t = irb.CreateSExtOrTrunc(t, a1t);
	_branchGenerated = irb.CreateCall(_callFunction, {t});
	return _branchGenerated;
}

void Capstone2LlvmIrTranslator_impl::generateReturnFunction()
{
	auto* ft = llvm::FunctionType::get(
			llvm::Type::getVoidTy(_module->getContext()),
			{llvm::Type::getIntNTy(_module->getContext(), getArchBitSize())},
			false);
	_returnFunction = llvm::Function::Create(
			ft,
			llvm::GlobalValue::LinkageTypes::ExternalLinkage,
			"",
			_module);
}

llvm::CallInst* Capstone2LlvmIrTranslator_impl::generateReturnFunctionCall(
		llvm::IRBuilder<>& irb,
		llvm::Value* t)
{
	auto* a1t = _returnFunction->getArgumentList().front().getType();
	t = irb.CreateSExtOrTrunc(t, a1t);
	_branchGenerated = irb.CreateCall(_returnFunction, {t});
	return _branchGenerated;
}

void Capstone2LlvmIrTranslator_impl::generateBranchFunction()
{
	auto* ft = llvm::FunctionType::get(
			llvm::Type::getVoidTy(_module->getContext()),
			{llvm::Type::getIntNTy(_module->getContext(), getArchBitSize())},
			false);
	_branchFunction = llvm::Function::Create(
			ft,
			llvm::GlobalValue::LinkageTypes::ExternalLinkage,
			"",
			_module);
}

llvm::CallInst* Capstone2LlvmIrTranslator_impl::generateBranchFunctionCall(
		llvm::IRBuilder<>& irb,
		llvm::Value* t)
{
	auto* a1t = _branchFunction->getArgumentList().front().getType();
	t = irb.CreateSExtOrTrunc(t, a1t);
	_branchGenerated = irb.CreateCall(_branchFunction, {t});
	return _branchGenerated;
}

void Capstone2LlvmIrTranslator_impl::generateCondBranchFunction()
{
	std::vector<llvm::Type*> params = {
			llvm::Type::getInt1Ty(_module->getContext()),
			llvm::Type::getIntNTy(_module->getContext(), getArchBitSize())};
	auto* ft = llvm::FunctionType::get(
			llvm::Type::getVoidTy(_module->getContext()),
			params,
			false);
	_condBranchFunction = llvm::Function::Create(
			ft,
			llvm::GlobalValue::LinkageTypes::ExternalLinkage,
			"",
			_module);
}

llvm::CallInst* Capstone2LlvmIrTranslator_impl::generateCondBranchFunctionCall(
		llvm::IRBuilder<>& irb,
		llvm::Value* cond,
		llvm::Value* t)
{
	auto* a1t = _condBranchFunction->getArgumentList().back().getType();
	t = irb.CreateSExtOrTrunc(t, a1t);
	_branchGenerated = irb.CreateCall(_condBranchFunction, {cond, t});
	return _branchGenerated;
}

llvm::GlobalVariable* Capstone2LlvmIrTranslator_impl::createRegister(
		uint32_t r,
		llvm::GlobalValue::LinkageTypes lt,
		llvm::Constant* initializer)
{
	auto* rt = getRegisterType(r);
	if (initializer == nullptr
			&& lt != llvm::GlobalValue::LinkageTypes::ExternalLinkage)
	{
		if (auto* it = llvm::dyn_cast<llvm::IntegerType>(rt))
		{
			initializer = llvm::ConstantInt::get(it, 0);
		}
		else if (rt->isFloatingPointTy())
		{
			initializer = llvm::ConstantFP::get(rt, 0);
		}
		else
		{
			throw Capstone2LlvmIrError("Unhandled register type.");
		}
	}

	auto* gv = new llvm::GlobalVariable(
			*_module,
			rt,
			false, // isConstant
			lt,
			initializer,
			getRegisterName(r));

	if (gv == nullptr)
	{
		throw Capstone2LlvmIrError("Memory allocation error.");
	}

	_allLlvmRegs[gv] = r;

	return gv;
}

//
//==============================================================================
// Carry/overflow/borrow add/sub generation routines.
//==============================================================================
//

/**
 * carry_add()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateCarryAdd(
		llvm::Value* add,
		llvm::Value* op0,
		llvm::IRBuilder<>& irb)
{
	return irb.CreateICmpULT(add, op0);
}

/**
 * carry_add_c()
 *
 * If @p cf is not passed, default cf register is used. Why pass it?
 * - Pass cf if you want to generate nicer code - prevent second cf load if
 *   it is already loaded by caller. This should however be taken care of by
 *   after generation optimizations.
 * - Use a different value as cf.
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateCarryAddC(
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb,
		llvm::Value* cf)
{
	auto* add1 = irb.CreateAdd(op0, op1);
	if (cf == nullptr)
	{
		cf = loadRegister(getCarryRegister(), irb);
	}
	auto* cfc = irb.CreateZExtOrTrunc(cf, add1->getType());
	auto* add2 = irb.CreateAdd(add1, cfc);
	auto* icmp1 = irb.CreateICmpULE(add2, op0);
	auto* icmp2 = irb.CreateICmpULT(add1, op0);
	auto* cff = irb.CreateZExtOrTrunc(cf, irb.getInt1Ty());
	return irb.CreateSelect(cff, icmp1, icmp2);
}

/**
 * carry_add_int4()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateCarryAddInt4(
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb)
{
	auto* ci15 = llvm::ConstantInt::get(op0->getType(), 15);
	auto* and0 = irb.CreateAnd(op0, ci15);
	auto* and1 = irb.CreateAnd(op1, ci15);
	auto* add = irb.CreateAdd(and0, and1);
	return irb.CreateICmpUGT(add, ci15);
}

/**
 * carry_add_c_int4()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateCarryAddCInt4(
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb,
		llvm::Value* cf)
{
	auto* ci15 = llvm::ConstantInt::get(op0->getType(), 15);
	auto* and0 = irb.CreateAnd(op0, ci15);
	auto* and1 = irb.CreateAnd(op1, ci15);
	auto* a = irb.CreateAdd(and0, and1);
	if (cf == nullptr)
	{
		cf = loadRegister(
				getCarryRegister(),
				irb,
				a->getType(),
				eOpConv::ZEXT_TRUNC);
	}
	auto* cfc = irb.CreateZExtOrTrunc(cf, a->getType());
	auto* add = irb.CreateAdd(a, cfc);
	return irb.CreateICmpUGT(add, ci15);
}

/**
 * overflow_add()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateOverflowAdd(
		llvm::Value* add,
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb)
{
	auto* xor0 = irb.CreateXor(op0, add);
	auto* xor1 = irb.CreateXor(op1, add);
	auto* ofAnd = irb.CreateAnd(xor0, xor1);
	return irb.CreateICmpSLT(
			ofAnd,
			llvm::ConstantInt::get(ofAnd->getType(), 0));
}

/**
 * overflow_add_c()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateOverflowAddC(
		llvm::Value* add,
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb,
		llvm::Value* cf)
{
	if (cf == nullptr)
	{
		cf = loadRegister(getCarryRegister(), irb);
	}
	auto* cfc = irb.CreateZExtOrTrunc(cf, add->getType());
	auto* ofAdd = irb.CreateAdd(add, cfc);
	auto* xor0 = irb.CreateXor(op0, ofAdd);
	auto* xor1 = irb.CreateXor(op1, ofAdd);
	auto* ofAnd = irb.CreateAnd(xor0, xor1);
	return irb.CreateICmpSLT(ofAnd, llvm::ConstantInt::get(ofAnd->getType(), 0));
}

/**
 * overflow_sub()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateOverflowSub(
		llvm::Value* sub,
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb)
{
	auto* xor0 = irb.CreateXor(op0, op1);
	auto* xor1 = irb.CreateXor(op0, sub);
	auto* ofAnd = irb.CreateAnd(xor0, xor1);
	return irb.CreateICmpSLT(
			ofAnd,
			llvm::ConstantInt::get(ofAnd->getType(), 0));
}

/**
 * overflow_sub_c()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateOverflowSubC(
		llvm::Value* sub,
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb,
		llvm::Value* cf)
{
	if (cf == nullptr)
	{
		cf = loadRegister(getCarryRegister(), irb);
	}
	auto* cfc = irb.CreateZExtOrTrunc(cf, sub->getType());
	auto* ofSub = irb.CreateSub(sub, cfc);
	auto* xor0 = irb.CreateXor(op0, op1);
	auto* xor1 = irb.CreateXor(op0, ofSub);
	auto* ofAnd = irb.CreateAnd(xor0, xor1);
	return irb.CreateICmpSLT(
			ofAnd,
			llvm::ConstantInt::get(ofAnd->getType(), 0));
}

/**
 * borrow_sub()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateBorrowSub(
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb)
{
	return irb.CreateICmpULT(op0, op1);
}

/**
 * borrow_sub_c()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateBorrowSubC(
		llvm::Value* sub,
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb,
		llvm::Value* cf)
{
	if (cf == nullptr)
	{
		cf = loadRegister(getCarryRegister(), irb);
	}
	auto* cfc = irb.CreateZExtOrTrunc(cf, sub->getType());
	auto* cfSub = irb.CreateSub(sub, cfc);
	auto* cfIcmp1 = irb.CreateICmpULT(op0, cfSub);
	auto* negOne = llvm::ConstantInt::getSigned(op1->getType(), -1);
	auto* cfIcmp2 = irb.CreateICmpULT(op1, negOne);
	auto* cfOr = irb.CreateOr(cfIcmp1, cfIcmp2);
	auto* cfIcmp3 = irb.CreateICmpULT(op0, op1);
	auto* cff = irb.CreateZExtOrTrunc(cf, irb.getInt1Ty());
	return irb.CreateSelect(cff, cfOr, cfIcmp3);
}

/**
 * borrow_sub_int4()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateBorrowSubInt4(
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb)
{
	auto* ci15 = llvm::ConstantInt::get(op0->getType(), 15);
	auto* and0 = irb.CreateAnd(op0, ci15);
	auto* and1 = irb.CreateAnd(op1, ci15);
	auto* afSub = irb.CreateSub(and0, and1);
	return irb.CreateICmpUGT(afSub, ci15);
}

/**
 * borrow_sub_c_int4()
 */
llvm::Value* Capstone2LlvmIrTranslator_impl::generateBorrowSubCInt4(
		llvm::Value* op0,
		llvm::Value* op1,
		llvm::IRBuilder<>& irb,
		llvm::Value* cf)
{
	auto* ci15 = llvm::ConstantInt::get(op0->getType(), 15);
	auto* and0 = irb.CreateAnd(op0, ci15);
	auto* and1 = irb.CreateAnd(op1, ci15);
	auto* sub = irb.CreateSub(and0, and1);
	if (cf == nullptr)
	{
		cf = loadRegister(getCarryRegister(), irb);
	}
	auto* cfc = irb.CreateZExtOrTrunc(cf, sub->getType());
	auto* add = irb.CreateAdd(sub, cfc);
	return irb.CreateICmpUGT(add, ci15);
}

//
//==============================================================================
// Non-virtual helper methods.
//==============================================================================
//

llvm::IntegerType* Capstone2LlvmIrTranslator_impl::getDefaultType()
{
	return getIntegerTypeFromByteSize(_module, getArchByteSize());
}

llvm::Value* Capstone2LlvmIrTranslator_impl::getThisInsnAddress(cs_insn* i)
{
	return llvm::ConstantInt::get(getDefaultType(), i->address);
}

llvm::Value* Capstone2LlvmIrTranslator_impl::getNextInsnAddress(cs_insn* i)
{
	return llvm::ConstantInt::get(getDefaultType(), i->address + i->size);
}

/**
 * @return Asm function associated with @p name, or @c nullptr if there is
 *         no such function.
 */
llvm::Function* Capstone2LlvmIrTranslator_impl::getAsmFunction(
		const std::string& name) const
{
	auto fIt = _asmFunctions.find(name);
	return fIt != _asmFunctions.end() ? fIt->second : nullptr;
}

/**
 * Get already existing asm functions associated with @p name, or if there
 * is no such function, create it using @p name and @p type, add it to asm
 * functions and return it.
 * @return Functions associated with @p insnId.
 */
llvm::Function* Capstone2LlvmIrTranslator_impl::getOrCreateAsmFunction(
		std::size_t insnId,
		const std::string& name,
		llvm::FunctionType* type)
{
	llvm::Function* fnc = getAsmFunction(name);
	if (fnc == nullptr)
	{
		fnc = llvm::Function::Create(
				type,
				llvm::GlobalValue::LinkageTypes::ExternalLinkage,
				name,
				_module);
		_asmFunctions[name] = fnc;
	}
	return fnc;
}

/**
 * The same as @c getOrCreateAsmFunction(std::size_t,std::string&, llvm::FunctionType*),
 * but function is created with zero parameters and @p retType return type.
 */
llvm::Function* Capstone2LlvmIrTranslator_impl::getOrCreateAsmFunction(
		std::size_t insnId,
		const std::string& name,
		llvm::Type* retType)
{
	return getOrCreateAsmFunction(
			insnId,
			name,
			llvm::FunctionType::get(retType, false));
}

/**
 * The same as @c getOrCreateAsmFunction(std::size_t,std::string&, llvm::FunctionType*),
 * but function is created with void return type and @p params parameters.
 *
 * TODO: This is not ideal, when used with only one argument (e.g. {i32}),
 * the llvm::Type* retType variant is used instead of this method.
 */
llvm::Function* Capstone2LlvmIrTranslator_impl::getOrCreateAsmFunction(
		std::size_t insnId,
		const std::string& name,
		llvm::ArrayRef<llvm::Type*> params)
{
	return getOrCreateAsmFunction(
			insnId,
			name,
			llvm::FunctionType::get(
					llvm::Type::getVoidTy(_module->getContext()),
					params,
					false));
}

/**
 * The same as @c getOrCreateAsmFunction(std::size_t,std::string&, llvm::FunctionType*),
 * but function type is created by this variant.
 */
llvm::Function* Capstone2LlvmIrTranslator_impl::getOrCreateAsmFunction(
		std::size_t insnId,
		const std::string& name,
		llvm::Type* retType,
		llvm::ArrayRef<llvm::Type*> params)
{
	return getOrCreateAsmFunction(
			insnId,
			name,
			llvm::FunctionType::get(retType, params, false));
}

} // namespace capstone2llvmir
} // namespace retdec
