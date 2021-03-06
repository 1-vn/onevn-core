/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/components/onevn_shields/browser/ad_block_regional_service_manager.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(AdBlockRegionalServiceTest, SupportedLocales) {
  std::vector<std::string> locales({ "fr", "fR", "fr-FR", "fr-ca" });
  std::for_each(locales.begin(), locales.end(), [](const std::string& locale) {
    EXPECT_TRUE(onevn_shields::AdBlockRegionalServiceManager::IsSupportedLocale(
        locale));
  });
}
