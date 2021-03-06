/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_BROWSER_UI_WEBUI_ONEVN_MD_SETTINGS_UI_H_
#define ONEVN_BROWSER_UI_WEBUI_ONEVN_MD_SETTINGS_UI_H_

#include <memory>

#include "chrome/browser/ui/webui/settings/md_settings_ui.h"

namespace content {
class WebUIDataSource;
}

class Profile;

class OnevnMdSettingsUI : public settings::MdSettingsUI {
 public:
  OnevnMdSettingsUI(content::WebUI* web_ui, const std::string& host);
  ~OnevnMdSettingsUI() override;

  static void AddResources(content::WebUIDataSource* html_source, Profile* profile);

  DISALLOW_COPY_AND_ASSIGN(OnevnMdSettingsUI);
};

#endif  // ONEVN_BROWSER_UI_WEBUI_ONEVN_MD_SETTINGS_UI_H_
