/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#if defined(ONEVN_CHROMIUM_BUILD)
#define GOOGLE_CHROME_BUILD
#endif

#include "../../../../components/metrics/metrics_service_accessor.cc"

#if defined(ONEVN_CHROMIUM_BUILD)
#undef GOOGLE_CHROME_BUILD
#endif
