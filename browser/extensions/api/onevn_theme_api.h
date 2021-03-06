/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_BROWSER_EXTENSIONS_API_ONEVN_THEME_API_H_
#define ONEVN_BROWSER_EXTENSIONS_API_ONEVN_THEME_API_H_

#include "extensions/browser/extension_function.h"

namespace extensions {
namespace api {

class OnevnThemeGetOnevnThemeListFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("onevnTheme.getOnevnThemeList", UNKNOWN)

 protected:
  ~OnevnThemeGetOnevnThemeListFunction() override {}

  ResponseAction Run() override;
};

class OnevnThemeGetOnevnThemeTypeFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("onevnTheme.getOnevnThemeType", UNKNOWN)

 protected:
  ~OnevnThemeGetOnevnThemeTypeFunction() override {}

  ResponseAction Run() override;
};

}  // namespace api
}  // namespace extensions

#endif  // ONEVN_BROWSER_EXTENSIONS_API_ONEVN_THEME_API_H_
