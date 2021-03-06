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

#include "mir/graphics/buffer.h"
#include "native_buffer.h"
#include "sync_fence.h"
#include "swapping_gl_context.h"
#include "android_format_conversion-inl.h"
#include "fb_device.h"
#include "framebuffer_bundle.h"
#include "buffer.h"
#include "mir/geometry/length.h"

#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <algorithm>

namespace mg = mir::graphics;
namespace mga=mir::graphics::android;
namespace geom=mir::geometry;

mga::FbControl::FbControl(std::shared_ptr<framebuffer_device_t> const& fbdev) :
    fb_device(fbdev)
{
    if (fb_device->setSwapInterval)
        fb_device->setSwapInterval(fb_device.get(), 1);
}

void mga::FbControl::power_mode(DisplayName display, MirPowerMode mode)
{
    if (display != mga::DisplayName::primary)
        BOOST_THROW_EXCEPTION(std::runtime_error("fb device cannot activate non-primary display"));

    int enable = 0;
    if (mode == mir_power_mode_on)
        enable = 1;

    if (fb_device->enableScreen)
        fb_device->enableScreen(fb_device.get(), enable);
}

mg::DisplayConfigurationOutput mga::FbControl::active_config_for(DisplayName display_name)
{
    auto const connected = (display_name == DisplayName::primary);

    geom::Length length_x_inches{0, geom::Length::Units::inches};
    geom::Length length_y_inches{0, geom::Length::Units::inches};
    if (fb_device->xdpi != 0)
        length_x_inches = geom::Length(fb_device->width / fb_device->xdpi, geom::Length::Units::inches);
    if (fb_device->ydpi != 0)
        length_y_inches = geom::Length(fb_device->height / fb_device->ydpi, geom::Length::Units::inches);

    geom::Size display_size_mm{
        length_x_inches.as(geom::Length::Units::millimetres),
        length_y_inches.as(geom::Length::Units::millimetres),
    };

    return {
        as_output_id(display_name),
        mg::DisplayConfigurationCardId{0},
        mg::DisplayConfigurationOutputType::lvds,
        std::vector<MirPixelFormat>{mga::to_mir_format(fb_device->format)},
        std::vector<mg::DisplayConfigurationMode>{
            mg::DisplayConfigurationMode{{fb_device->width, fb_device->height}, fb_device->fps}
        },
        0,
        display_size_mm,
        connected,
        connected,
        {0,0},
        0,
        mga::to_mir_format(fb_device->format),
        mir_power_mode_on,
        mir_orientation_normal,
        1.0f,
        mir_form_factor_phone,
        mir_subpixel_arrangement_unknown,
        {},
        mir_output_gamma_unsupported,
        {},
        {}
    };
}

mga::ConfigChangeSubscription mga::FbControl::subscribe_to_config_changes(
        std::function<void()> const&,
        std::function<void(DisplayName, mg::Frame::Timestamp)> const&)
{
    return nullptr;
}

mga::FBDevice::FBDevice(std::shared_ptr<framebuffer_device_t> const& fbdev) :
    fb_device(fbdev)
{
}

void mga::FBDevice::commit(std::list<DisplayContents> const& contents)
{
    auto primary_contents = std::find_if(contents.begin(), contents.end(),
        [](mga::DisplayContents const& c) {
            return (c.name == mga::DisplayName::primary);
    });
    if (primary_contents == contents.end()) return;
    auto& context = primary_contents->context;
    auto const& buffer = context.last_rendered_buffer();
    auto native_buffer = mga::to_native_buffer_checked(buffer->native_buffer_handle());
    native_buffer->ensure_available_for(mga::BufferAccess::read);
    if (fb_device->post(fb_device.get(), native_buffer->handle()) != 0)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("error posting with fb device"));
    }
}

bool mga::FBDevice::compatible_renderlist(RenderableList const&)
{
    return false;
}

void mga::FBDevice::content_cleared()
{
}

std::chrono::milliseconds mga::FBDevice::recommended_sleep() const
{
    return std::chrono::milliseconds::zero();
}

bool mga::FBDevice::can_swap_buffers() const
{
    return true;
}
