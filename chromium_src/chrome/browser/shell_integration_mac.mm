/* Copyright (c) 2019 The OneVN Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#define GetDefaultWebClientSetPermission GetDefaultWebClientSetPermission_Unused
#include "../../../../chrome/browser/shell_integration_mac.mm"  // NOLINT
#undef GetDefaultWebClientSetPermission

namespace shell_integration {

DefaultWebClientSetPermission GetDefaultWebClientSetPermission() {
  return SET_DEFAULT_UNATTENDED;
}

}  // shell_integration
