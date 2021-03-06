/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#define FixupBrowserAboutURL FixupBrowserAboutURL_ChromiumImpl
#include "../../../../chrome/browser/browser_about_handler.cc"  // NOLINT
#undef FixupBrowserAboutURL

#include "onevn/common/url_constants.h"
#include "onevn/common/webui_url_constants.h"

bool FixupBrowserAboutURL(GURL* url,
                          content::BrowserContext* browser_context) {
  bool result = FixupBrowserAboutURL_ChromiumImpl(url, browser_context);

  // no special win10 welcome page
  if (url->host() == chrome::kChromeUIWelcomeWin10Host)
    *url = GURL(chrome::kChromeUIWelcomeURL);

  // redirect sync-internals
  if (url->host() == chrome::kChromeUISyncInternalsHost) {
    GURL::Replacements replacements;
    replacements.SetHostStr(chrome::kChromeUISyncHost);
    *url = url->ReplaceComponents(replacements);
  }

  if (url->SchemeIs(kOnevnUIScheme)) {
    GURL::Replacements replacements;
    replacements.SetSchemeStr(content::kChromeUIScheme);
    *url = url->ReplaceComponents(replacements);
  }

  return result;
}
