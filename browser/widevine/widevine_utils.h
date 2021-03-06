/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_BROWSER_WIDEVINE_WIDEVINE_UTILS_H_
#define ONEVN_BROWSER_WIDEVINE_WIDEVINE_UTILS_H_

#include "third_party/widevine/cdm/buildflags.h"

namespace content {
class WebContents;
}

int GetWidevinePermissionRequestTextFrangmentResourceId();
void RequestWidevinePermission(content::WebContents* web_contents);

#if BUILDFLAG(BUNDLE_WIDEVINE_CDM)
void InstallBundleOrRestartBrowser();
#endif

#if BUILDFLAG(ENABLE_WIDEVINE_CDM_COMPONENT)
void EnableWidevineCdmComponent(content::WebContents* web_contents);
#endif

#endif  // ONEVN_BROWSER_WIDEVINE_WIDEVINE_UTILS_H_
