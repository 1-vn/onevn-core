/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/browser/version_info.h"

#define GetVersionNumber GetOnevnVersionNumberForDisplay
#include "../../../../../../../chrome/browser/ui/webui/version_ui.cc"
#undef GetVersionNumber
