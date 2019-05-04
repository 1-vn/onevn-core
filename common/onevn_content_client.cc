/* Copyright 2019 The OneVN Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/common/onevn_content_client.h"

#include "content/public/common/url_constants.h"

OneVNContentClient::OneVNContentClient() {}

OneVNContentClient::~OneVNContentClient() {}

void OneVNContentClient::AddAdditionalSchemes(Schemes* schemes) {
  ChromeContentClient::AddAdditionalSchemes(schemes);
  schemes->standard_schemes.push_back(content::kOneVNUIScheme);
  schemes->secure_schemes.push_back(content::kOneVNUIScheme);
  schemes->cors_enabled_schemes.push_back(content::kOneVNUIScheme);
  schemes->savable_schemes.push_back(content::kOneVNUIScheme);
}
