/* Copyright (c) 2019 The OneVN Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_CHROMIUM_SRC_GOOGLE_APIS_GOOGLE_API_KEYS_H_
#define ONEVN_CHROMIUM_SRC_GOOGLE_APIS_GOOGLE_API_KEYS_H_

#include "../../../google_apis/google_api_keys.h"
#include <string>

namespace google_apis {

void SetAPIKeyForTesting(const std::string& api_key);

}  // namespace google_apis

#endif  // ONEVN_CHROMIUM_SRC_GOOGLE_APIS_GOOGLE_API_KEYS_H_
