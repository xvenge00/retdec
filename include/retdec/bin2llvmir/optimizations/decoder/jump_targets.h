/**
* @file include/retdec/bin2llvmir/optimizations/decoder/jump_targets.h
* @brief Jump targets representation.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#ifndef RETDEC_BIN2LLVMIR_OPTIMIZATIONS_DECODER_JUMP_TARGETS_H
#define RETDEC_BIN2LLVMIR_OPTIMIZATIONS_DECODER_JUMP_TARGETS_H

#include <set>

#include "retdec/capstone2llvmir/capstone2llvmir.h"
#include "retdec/capstone2llvmir/x86/x86.h"
#include "retdec/utils/address.h"

namespace retdec {
namespace bin2llvmir {

class JumpTarget
{
	public:
		/**
		 * Jump target type and its priority.
		 * Lower number -> higher priority.
		 */
		enum class eType
		{
			ENTRY_POINT = 0,
			CONTROL_FLOW_CALL,
			UNKNOWN,
		};

	public:
		JumpTarget();
		JumpTarget(
				retdec::utils::Address a,
				eType t,
				cs_mode m,
				retdec::utils::Address f = retdec::utils::Address::getUndef,
				const std::string& n = "");

		bool operator<(const JumpTarget& o) const;
		bool createFunction() const;

		bool hasName() const;
		std::string getName() const;
		void setName(const std::string& n) const;

		bool isKnownMode() const;
		bool isUnknownMode() const;

	friend std::ostream& operator<<(std::ostream &out, const JumpTarget& jt);

	public:
		retdec::utils::Address address;
		/// If jump target is code pointer, this is an address where
		/// it was found;
		retdec::utils::Address from;
		eType type = eType::UNKNOWN;
		cs_mode mode = CS_MODE_BIG_ENDIAN;

	private:
		mutable std::string name;
};

class JumpTargets
{
	public:
		auto begin();
		auto end();

		bool empty();
		std::size_t size() const;
		void clear();
		const JumpTarget& top();
		void pop();
		bool wasAlreadyPoped(JumpTarget& ct) const;

		void push(const JumpTarget& jt);
		void push(
				retdec::utils::Address a,
				JumpTarget::eType t,
				cs_mode m);
		void push(
				retdec::utils::Address a,
				JumpTarget::eType t,
				cs_mode m,
				retdec::utils::Address f);
		void push(
				retdec::utils::Address a,
				JumpTarget::eType t,
				cs_mode m,
				const std::string name);

	friend std::ostream& operator<<(std::ostream &out, const JumpTargets& jts);

	private:
		std::set<JumpTarget> _data;
		std::set<retdec::utils::Address> _poped;
};

} // namespace bin2llvmir
} // namespace retdec

#endif
