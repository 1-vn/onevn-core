/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_ONEVN_CONTENT_SETTINGS_PREF_PROVIDER_H_
#define ONEVN_COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_ONEVN_CONTENT_SETTINGS_PREF_PROVIDER_H_

#include "components/content_settings/core/browser/content_settings_pref_provider.h"
#include "components/prefs/pref_change_registrar.h"

namespace content_settings {

// With this subclass, shields configuration is persisted across sessions.
// Its content type is |CONTENT_SETTINGS_TYPE_PLUGIN| and its storage option is
// ephemeral because chromium want that flash configuration shouldn't be
// persisted. (Maybe chromium assumes flash is the only one of this type).
// Because of this reasion, shields configuration was also ephemeral.
// However, we want shilelds configuration persisted. To do this, we make
// EphemeralProvider ignore shields type and this class handles.
class OnevnPrefProvider : public PrefProvider {
 public:
  OnevnPrefProvider(
      PrefService* prefs, bool incognito, bool store_last_modified);
  ~OnevnPrefProvider() override {}

 private:
  // content_settings::PrefProvider overrides:
  void ShutdownOnUIThread() override;
  bool SetWebsiteSetting(
      const ContentSettingsPattern& primary_pattern,
      const ContentSettingsPattern& secondary_pattern,
      ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      base::Value* value) override;

  // PrefProvider::pref_change_registrar_ alreay has plugin type.
  PrefChangeRegistrar onevn_pref_change_registrar_;

  DISALLOW_COPY_AND_ASSIGN(OnevnPrefProvider);
};

}  //  namespace content_settings

#endif  // ONEVN_COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_ONEVN_CONTENT_SETTINGS_PREF_PROVIDER_H_
