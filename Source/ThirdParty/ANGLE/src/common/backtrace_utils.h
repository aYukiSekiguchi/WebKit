//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// backtrace_utils.h:
//   Tools to extract the backtrace from the ANGLE code during execution.
//

#ifndef COMMON_BACKTRACEUTILS_H_
#define COMMON_BACKTRACEUTILS_H_

#include <string>
#include <vector>
#include "debug.h"

namespace angle
{

// Used to store the backtrace information, such as the stack addresses and symbols.
class BacktraceInfo
{
  public:
    BacktraceInfo() {}
    ~BacktraceInfo() {}

    void clear()
    {
        mStackAddresses.clear();
        mStackSymbols.clear();
    }

    size_t getSize() const
    {
        ASSERT(mStackAddresses.size() == mStackSymbols.size());
        return mStackAddresses.size();
    }

    std::vector<void *> getStackAddresses() const { return mStackAddresses; }
    std::vector<std::string> getStackSymbols() const { return mStackSymbols; }

    void *getStackAddress(size_t index) const
    {
        ASSERT(index < mStackAddresses.size());
        return mStackAddresses[index];
    }

    std::string getStackSymbol(size_t index) const
    {
        ASSERT(index < mStackSymbols.size());
        return mStackSymbols[index];
    }

    // Used to add the stack addresses and their corresponding symbols to the object, when
    // angle_enable_unwind_backtrace_support is enabled on Android.
    void populateBacktraceInfo(void **stackAddressBuffer, size_t stackAddressCount);

  private:
    std::vector<void *> mStackAddresses;
    std::vector<std::string> mStackSymbols;
};

// Used to obtain the stack addresses and symbols from the device, when
// angle_enable_unwind_backtrace_support is enabled on Android. Otherwise , it returns an empty
// object.
BacktraceInfo getBacktraceInfo();

// Used to print the stack addresses and symbols embedded in the BacktraceInfo object.
void printBacktraceInfo(BacktraceInfo backtraceInfo);

}  // namespace angle

#endif  // COMMON_BACKTRACEUTILS_H_
