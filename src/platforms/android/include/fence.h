/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_GRAPHICS_ANDROID_FENCE_H_
#define MIR_GRAPHICS_ANDROID_FENCE_H_

#include <chrono>
namespace mir
{
namespace graphics
{
namespace android
{

//TODO: (kdub) remove this type in favor of mir::Fd
typedef int NativeFence;

class Fence
{
public:
    virtual ~Fence() = default;

    virtual void wait() = 0;
    virtual bool wait_for(std::chrono::milliseconds) = 0;
    virtual void reset_fence() = 0;
    //TODO: (kdub) use the Fd type instead of NativeFence
    virtual void merge_with(NativeFence& merge_fd) = 0;
    virtual NativeFence copy_native_handle() const = 0;
    virtual NativeFence native_handle() const = 0;

protected:
    Fence() = default;
    Fence(Fence const&) = delete;
    Fence& operator=(Fence const&) = delete;
};


}
}
}

#endif /* MIR_GRAPHICS_ANDROID_FENCE_H_ */
