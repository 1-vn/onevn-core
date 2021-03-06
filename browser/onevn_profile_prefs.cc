/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/browser/onevn_profile_prefs.h"

#include "onevn/browser/themes/onevn_theme_service.h"
#include "onevn/browser/tor/tor_profile_service.h"
#include "onevn/common/pref_names.h"
#include "onevn/components/onevn_rewards/browser/rewards_service.h"
#include "onevn/components/onevn_shields/browser/onevn_shields_web_contents_observer.h"
#include "chrome/browser/net/prediction_options.h"
#include "chrome/browser/prefs/session_startup_pref.h"
#include "chrome/common/pref_names.h"
#include "components/content_settings/core/common/pref_names.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/safe_browsing/common/safe_browsing_prefs.h"
#include "components/signin/core/browser/signin_pref_names.h"
#include "components/spellcheck/browser/pref_names.h"
#include "components/sync/base/pref_names.h"
#include "third_party/widevine/cdm/buildflags.h"

#if BUILDFLAG(BUNDLE_WIDEVINE_CDM)
#include "onevn/browser/widevine/onevn_widevine_bundle_manager.h"
#endif

namespace onevn {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  onevn_rewards::RewardsService::RegisterProfilePrefs(registry);
  onevn_shields::OnevnShieldsWebContentsObserver::RegisterProfilePrefs(
      registry);

  // appearance
  OnevnThemeService::RegisterProfilePrefs(registry);
  registry->RegisterBooleanPref(kLocationBarIsWide, false);
  registry->RegisterBooleanPref(kHideOnevnRewardsButton, false);

  tor::TorProfileService::RegisterProfilePrefs(registry);

  registry->RegisterBooleanPref(kWidevineOptedIn, false);
#if BUILDFLAG(BUNDLE_WIDEVINE_CDM)
  OnevnWidevineBundleManager::RegisterProfilePrefs(registry);
#endif

  // Default Onevn shields
  registry->RegisterBooleanPref(kHTTPSEVerywhereControlType, true);
  registry->RegisterBooleanPref(kNoScriptControlType, false);
  registry->RegisterBooleanPref(kGoogleLoginControlType, true);
  registry->RegisterBooleanPref(kFBEmbedControlType, true);
  registry->RegisterBooleanPref(kTwitterEmbedControlType, true);
  registry->RegisterBooleanPref(kLinkedInEmbedControlType, false);

  // WebTorrent
  registry->RegisterBooleanPref(kWebTorrentEnabled, true);

  // Hangouts
  registry->RegisterBooleanPref(kHangoutsEnabled, true);

  // No sign into Onevn functionality
  registry->SetDefaultPrefValue(prefs::kSigninAllowed, base::Value(false));

  // Restore last profile on restart
  registry->SetDefaultPrefValue(
      prefs::kRestoreOnStartup,
      base::Value(SessionStartupPref::kPrefValueLast));

  // Show download prompt by default
  registry->SetDefaultPrefValue(prefs::kPromptForDownload, base::Value(true));

  // Not using chrome's web service for resolving navigation errors
  registry->SetDefaultPrefValue(prefs::kAlternateErrorPagesEnabled,
                                base::Value(false));

  // Disable spell check service
  registry->SetDefaultPrefValue(
      spellcheck::prefs::kSpellCheckUseSpellingService, base::Value(false));

  // Disable safebrowsing reporting
  registry->SetDefaultPrefValue(
      prefs::kSafeBrowsingExtendedReportingOptInAllowed, base::Value(false));

  // Disable search suggestion
  registry->SetDefaultPrefValue(prefs::kSearchSuggestEnabled,
                                base::Value(false));

  // Disable "Use a prediction service to load pages more quickly"
  registry->SetDefaultPrefValue(
      prefs::kNetworkPredictionOptions,
      base::Value(chrome_browser_net::NETWORK_PREDICTION_NEVER));

  // Make sync managed to dsiable some UI after password saving.
  registry->SetDefaultPrefValue(syncer::prefs::kSyncManaged, base::Value(true));

  // Make sure sign into Onevn is not enabled
  // The older kSigninAllowed is deprecated and only in use in Android until
  // C71.
  registry->SetDefaultPrefValue(prefs::kSigninAllowedOnNextStartup,
                                base::Value(false));

  // Disable cloud print
  // Cloud Print: Don't allow this browser to act as Cloud Print server
  registry->SetDefaultPrefValue(prefs::kCloudPrintProxyEnabled,
                                base::Value(false));
  // Cloud Print: Don't allow jobs to be submitted
  registry->SetDefaultPrefValue(prefs::kCloudPrintSubmitEnabled,
                                base::Value(false));

  // Importer: selected data types
  registry->RegisterBooleanPref(prefs::kImportDialogCookies, true);
  registry->RegisterBooleanPref(prefs::kImportDialogStats, true);
  registry->RegisterBooleanPref(prefs::kImportDialogLedger, true);
  registry->RegisterBooleanPref(prefs::kImportDialogWindows, true);
  // Importer: ledger (used for Onevn Rewards pinned => tips)
  registry->RegisterIntegerPref(kOnevnPaymentsPinnedItemCount, 0);

  // IPFS companion extension
  registry->RegisterBooleanPref(kIPFSCompanionEnabled, false);
}

}  // namespace onevn
