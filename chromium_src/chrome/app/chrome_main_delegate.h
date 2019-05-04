/* Copyright 2019 The OneVN Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_CHROMIUM_SRC_CHROME_APP_CHROME_MAIN_DELEGATE_H_
#define ONEVN_CHROMIUM_SRC_CHROME_APP_CHROME_MAIN_DELEGATE_H_

#include "../../../chrome/common/chrome_content_client.h"  // NOLINT

#include "onevn/common/onevn_content_client.h"

#define ChromeContentClient OneVNContentClient
#include "../../../chrome/app/chrome_main_delegate.h"  // NOLINT
#undef ChromeContentClient

#endif  // ONEVN_CHROMIUM_SRC_CHROME_APP_CHROME_MAIN_DELEGATE_H_
