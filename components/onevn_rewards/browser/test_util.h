/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_COMPONENTS_ONEVN_REWARDS_BROWSER_TEST_UTIL_H_
#define ONEVN_COMPONENTS_ONEVN_REWARDS_BROWSER_TEST_UTIL_H_

#include <memory>

#include "base/files/file_path.h"

class KeyedService;
class Profile;

namespace onevn_rewards {

std::unique_ptr<Profile> CreateOnevnRewardsProfile(const base::FilePath& path);

}  // namespace onevn_rewards

#endif  // ONEVN_COMPONENTS_ONEVN_REWARDS_BROWSER_TEST_UTIL_H_
