/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_BROWSER_ONEVN_TAB_HELPERS_H_
#define ONEVN_BROWSER_ONEVN_TAB_HELPERS_H_

namespace content {
class WebContents;
}

namespace onevn {

void AttachTabHelpers(content::WebContents* web_contents);

}  // namespace onevn

#endif  // ONEVN_BROWSER_ONEVN_TAB_HELPERS_H_

