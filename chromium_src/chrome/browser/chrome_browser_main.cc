/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/chrome_browser_main.h"
#include "onevn/browser/onevn_browser_process_impl.h"

#define BrowserProcessImpl OneVNBrowserProcessImpl
#include "../../../../chrome/browser/chrome_browser_main.cc"
#undef BrowserProcessImpl
