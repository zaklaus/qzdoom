/*
**  SSA vec4 float pointer
**  Copyright (c) 2016 Magnus Norddahl
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
*/

#pragma once

#include "ssa_int.h"
#include "ssa_vec4f.h"

namespace llvm { class Value; }
namespace llvm { class Type; }

class SSAVec4fPtr
{
public:
	SSAVec4fPtr();
	explicit SSAVec4fPtr(llvm::Value *v);
	static SSAVec4fPtr from_llvm(llvm::Value *v) { return SSAVec4fPtr(v); }
	static llvm::Type *llvm_type();
	SSAVec4fPtr operator[](SSAInt index) const;
	SSAVec4f load(bool constantScopeDomain) const;
	SSAVec4f load_unaligned(bool constantScopeDomain) const;
	void store(const SSAVec4f &new_value);
	void store_unaligned(const SSAVec4f &new_value);

	llvm::Value *v;
};