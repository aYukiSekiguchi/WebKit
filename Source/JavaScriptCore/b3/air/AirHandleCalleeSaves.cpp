/*
 * Copyright (C) 2015-2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "AirHandleCalleeSaves.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInstInlines.h"
#include "RegisterSet.h"

namespace JSC { namespace B3 { namespace Air {

void handleCalleeSaves(Code& code)
{
    RegisterSetBuilder usedCalleeSaves;

    for (BasicBlock* block : code) {
        for (Inst& inst : *block) {
            inst.forEachTmpFast(
                [&] (Tmp& tmp) {
                    // At first we just record all used regs.
                    usedCalleeSaves.add(tmp.reg(), IgnoreVectors);
                });

            if (inst.kind.opcode == Patch) {
                usedCalleeSaves.merge(inst.extraClobberedRegs());
                usedCalleeSaves.merge(inst.extraEarlyClobberedRegs());
            }
        }
    }

    handleCalleeSaves(code, WTFMove(usedCalleeSaves));
}

void handleCalleeSaves(Code& code, RegisterSetBuilder usedCalleeSaves)
{
    // We filter to really get the callee saves.
    usedCalleeSaves.filter(RegisterSetBuilder::vmCalleeSaveRegisters());
    usedCalleeSaves.filter(code.mutableRegs());
    usedCalleeSaves.exclude(RegisterSetBuilder::stackRegisters()); // We don't need to save FP here.

    auto calleSavesToSave = usedCalleeSaves.buildWithLowerBits();

    if (!calleSavesToSave.numberOfSetRegisters())
        return;

    RegisterAtOffsetList calleeSaveRegisters = RegisterAtOffsetList(calleSavesToSave);

    size_t byteSize = 0;
    for (const RegisterAtOffset& entry : calleeSaveRegisters)
        byteSize = std::max(static_cast<size_t>(-entry.offset()), byteSize);
    ASSERT(calleeSaveRegisters.sizeOfAreaInBytes() == byteSize);

    code.setCalleeSaveRegisterAtOffsetList(
        WTFMove(calleeSaveRegisters),
        code.addStackSlot(byteSize, StackSlotKind::Locked));
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
