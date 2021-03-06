/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef ONEVN_COMPONENTS_ONEVN_SYNC_ONEVN_SYNC_TOOLS_H_
#define ONEVN_COMPONENTS_ONEVN_SYNC_ONEVN_SYNC_TOOLS_H_

#include <string>

namespace base {
  class Time;
} // namespace base

namespace onevn_sync {

namespace tools {

std::string GenerateObjectId();
std::string GetPlatformName();

bool IsTimeEmpty(const base::Time &time);

} // namespace tools

} // namespace onevn_sync

#endif // ONEVN_COMPONENTS_ONEVN_SYNC_ONEVN_SYNC_TOOLS_H_
