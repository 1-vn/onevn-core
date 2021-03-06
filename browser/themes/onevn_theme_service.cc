/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/browser/themes/onevn_theme_service.h"

#include <utility>

#include "base/command_line.h"
#include "base/strings/string_util.h"
#include "onevn/browser/extensions/onevn_theme_event_router.h"
#include "onevn/browser/themes/theme_properties.h"
#include "onevn/browser/themes/onevn_theme_utils.h"
#include "onevn/common/onevn_switches.h"
#include "onevn/common/pref_names.h"
#include "onevn/grit/onevn_generated_resources.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/themes/theme_service_factory.h"
#include "chrome/common/channel_info.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "components/version_info/channel.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/ui_base_features.h"
#include "ui/base/ui_base_switches.h"
#include "ui/native_theme/native_theme.h"
#include "ui/native_theme/native_theme_dark_aura.h"

namespace {
OnevnThemeType GetThemeTypeBasedOnChannel() {
  switch (chrome::GetChannel()) {
    case version_info::Channel::STABLE:
    case version_info::Channel::BETA:
      return OnevnThemeType::ONEVN_THEME_TYPE_LIGHT;
    case version_info::Channel::DEV:
    case version_info::Channel::CANARY:
    case version_info::Channel::UNKNOWN:
    default:
      return OnevnThemeType::ONEVN_THEME_TYPE_DARK;
  }
}
}  // namespace

// static
std::string OnevnThemeService::GetStringFromOnevnThemeType(
    OnevnThemeType type) {
  switch (type) {
    case OnevnThemeType::ONEVN_THEME_TYPE_LIGHT:
      return "Light";
    case OnevnThemeType::ONEVN_THEME_TYPE_DARK:
      return "Dark";
    default:
      NOTREACHED();
      return "Default";
  }
}

// static
base::Value OnevnThemeService::GetOnevnThemeList() {
  base::Value list(base::Value::Type::LIST);

  if (SystemThemeModeEnabled()) {
    base::Value system_type(base::Value::Type::DICTIONARY);
    system_type.SetKey(
        "value",
        base::Value(OnevnThemeType::ONEVN_THEME_TYPE_DEFAULT));
    system_type.SetKey(
        "name",
        base::Value(l10n_util::GetStringUTF16(IDS_ONEVN_THEME_TYPE_SYSTEM)));
    list.GetList().push_back(std::move(system_type));
  }

  base::Value dark_type(base::Value::Type::DICTIONARY);
  dark_type.SetKey("value", base::Value(OnevnThemeType::ONEVN_THEME_TYPE_DARK));
  dark_type.SetKey(
      "name",
      base::Value(l10n_util::GetStringUTF16(IDS_ONEVN_THEME_TYPE_DARK)));
  list.GetList().push_back(std::move(dark_type));

  base::Value light_type(base::Value::Type::DICTIONARY);
  light_type.SetKey("value",
                    base::Value(OnevnThemeType::ONEVN_THEME_TYPE_LIGHT));
  light_type.SetKey(
      "name",
      base::Value(l10n_util::GetStringUTF16(IDS_ONEVN_THEME_TYPE_LIGHT)));
  list.GetList().push_back(std::move(light_type));

  return list;
}

// static
void OnevnThemeService::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterIntegerPref(kOnevnThemeType, ONEVN_THEME_TYPE_DEFAULT);

  // When this is set to true, prefs is changed from default type to
  // effective type. In dtor, pref is reverted to default type if this is
  // still true. With this, we can preserve the context that user didn't touch
  // theme type yet. If it is changed to false, it means user changes system
  // theme explicitly.
  // To handle crash case, prefs is used instead of boolean flags. Recovering
  // is done in OnevnThemeService::Init().
  registry->RegisterBooleanPref(kUseOverriddenOnevnThemeType, false);
}

// static
OnevnThemeType OnevnThemeService::GetActiveOnevnThemeType(
    Profile* profile) {
  // allow override via cli flag
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kUiMode)) {
    std::string requested_theme_value =
        command_line.GetSwitchValueASCII(switches::kUiMode);
    std::string requested_theme_value_lower =
        base::ToLowerASCII(requested_theme_value);
    if (requested_theme_value_lower == "light")
      return OnevnThemeType::ONEVN_THEME_TYPE_LIGHT;
    if (requested_theme_value_lower == "dark")
      return OnevnThemeType::ONEVN_THEME_TYPE_DARK;
  }

  OnevnThemeType type = static_cast<OnevnThemeType>(
      profile->GetPrefs()->GetInteger(kOnevnThemeType));
  if (type == OnevnThemeType::ONEVN_THEME_TYPE_DEFAULT) {
    DCHECK(SystemThemeModeEnabled());
    return ui::NativeTheme::GetInstanceForNativeUi()->
         SystemDarkModeEnabled() ? OnevnThemeType::ONEVN_THEME_TYPE_DARK
                                 : OnevnThemeType::ONEVN_THEME_TYPE_LIGHT;
  }
  return type;
}

OnevnThemeService::OnevnThemeService() {}

OnevnThemeService::~OnevnThemeService() {
  // In test, kOnevnThemeType isn't registered.
  if (!profile()->GetPrefs()->FindPreference(kOnevnThemeType))
    return;

  if (profile()->GetPrefs()->GetBoolean(kUseOverriddenOnevnThemeType)) {
    onevn_theme_type_pref_.Destroy();
    profile()->GetPrefs()->SetInteger(kOnevnThemeType,
                                      OnevnThemeType::ONEVN_THEME_TYPE_DEFAULT);
  }
}

void OnevnThemeService::Init(Profile* profile) {
  // In test, kOnevnThemeType isn't registered.
  if (profile->GetPrefs()->FindPreference(kOnevnThemeType)) {
    RecoverPrefStates(profile);
    OverrideDefaultThemeIfNeeded(profile);
    if (SystemThemeModeEnabled()) {
      // Start with proper system theme to make onevn theme and
      // base ui components theme use same theme.
      SetSystemTheme(static_cast<OnevnThemeType>(
          profile->GetPrefs()->GetInteger(kOnevnThemeType)));
    }

    onevn_theme_type_pref_.Init(
      kOnevnThemeType,
      profile->GetPrefs(),
      base::Bind(&OnevnThemeService::OnPreferenceChanged,
                 base::Unretained(this)));

    onevn_theme_event_router_.reset(
        new extensions::OnevnThemeEventRouter(profile));
  }

  ThemeService::Init(profile);
}

SkColor OnevnThemeService::GetDefaultColor(int id, bool incognito) const {
  // Onevn Tor profiles are always 'incognito' (for now)
  if (!incognito && profile()->IsTorProfile())
    incognito = true;
  const OnevnThemeType theme = GetActiveOnevnThemeType(profile());
  const base::Optional<SkColor> onevnColor =
      MaybeGetDefaultColorForOnevnUi(id, incognito, theme);
  if (onevnColor)
      return onevnColor.value();
  // make sure we fallback to chrome's dark theme (incognito) for our dark theme
  if (theme == OnevnThemeType::ONEVN_THEME_TYPE_DARK)
    incognito = true;
  return ThemeService::GetDefaultColor(id, incognito);
}

void OnevnThemeService::OnPreferenceChanged(const std::string& pref_name) {
  DCHECK(pref_name == kOnevnThemeType);

  // Changing theme type means default theme is not overridden anymore.
  profile()->GetPrefs()->SetBoolean(kUseOverriddenOnevnThemeType, false);

  bool notify_theme_observer_here = true;
#if defined(OS_MACOSX)
  if (SystemThemeModeEnabled()) {
    // When system theme is changed, system theme changing observer notifies
    // proper native theme observers.
    // So, we don't need to notify again. See NotifyProperThemeObserver()
    // in chromium_src/ui/native_theme/native_theme_mac.mm.
    notify_theme_observer_here = false;
    SetSystemTheme(static_cast<OnevnThemeType>(
        profile()->GetPrefs()->GetInteger(kOnevnThemeType)));
  }
#endif
  if (notify_theme_observer_here) {
    // Notify dark (cross-platform) and light (platform-specific) variants
    // When theme is changed from light to dark, we notify to light theme
    // observer because NativeThemeObserver observes light native theme.
    GetActiveOnevnThemeType(profile()) == OnevnThemeType::ONEVN_THEME_TYPE_LIGHT
        ? ui::NativeThemeDarkAura::instance()->NotifyObservers()
        : ui::NativeTheme::GetInstanceForNativeUi()->NotifyObservers();
  }
}

void OnevnThemeService::RecoverPrefStates(Profile* profile) {
  // kUseOverriddenOnevnThemeType is true means pref states are not cleaned
  // up properly at the last running(ex, crash). Recover them here.
  if (profile->GetPrefs()->GetBoolean(kUseOverriddenOnevnThemeType)) {
    profile->GetPrefs()->SetInteger(kOnevnThemeType,
                                    OnevnThemeType::ONEVN_THEME_TYPE_DEFAULT);
  }
}

void OnevnThemeService::OverrideDefaultThemeIfNeeded(Profile* profile) {
  if (!SystemThemeModeEnabled() &&
      profile->GetPrefs()->GetInteger(kOnevnThemeType) ==
          OnevnThemeType::ONEVN_THEME_TYPE_DEFAULT) {
    profile->GetPrefs()->SetBoolean(kUseOverriddenOnevnThemeType,
                                    true);
    profile->GetPrefs()->SetInteger(kOnevnThemeType,
                                    GetThemeTypeBasedOnChannel());
  }
}

void OnevnThemeService::SetOnevnThemeEventRouterForTesting(
    extensions::OnevnThemeEventRouter* mock_router) {
  onevn_theme_event_router_.reset(mock_router);
}

// static
bool OnevnThemeService::use_system_theme_mode_in_test_ = false;
bool OnevnThemeService::is_test_ = false;

// static
bool OnevnThemeService::SystemThemeModeEnabled() {
  if (is_test_)
    return use_system_theme_mode_in_test_;

  if (!base::FeatureList::IsEnabled(features::kDarkMode))
    return false;

  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kForceDarkMode))
    return true;

  return SystemThemeSupportDarkMode();
}
