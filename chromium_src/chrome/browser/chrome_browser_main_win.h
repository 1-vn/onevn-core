/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_CHROMIUM_SRC_CHROME_BROWSER_CHROME_BROWSER_MAIN_WIN_H_
#define ONEVN_CHROMIUM_SRC_CHROME_BROWSER_CHROME_BROWSER_MAIN_WIN_H_

#include "onevn/browser/onevn_browser_main_parts.h"
#include "chrome/browser/chrome_browser_main.h"

#define ChromeBrowserMainParts OnevnBrowserMainParts
#include "../../../../chrome/browser/chrome_browser_main_win.h"  // NOLINT
#undef ChromeBrowserMainParts

#endif  // ONEVN_CHROMIUM_SRC_CHROME_BROWSER_CHROME_BROWSER_MAIN_WIN_H_
