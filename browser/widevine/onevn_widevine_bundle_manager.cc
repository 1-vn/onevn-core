/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/browser/widevine/onevn_widevine_bundle_manager.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_piece.h"
#include "base/task/post_task.h"
#include "onevn/browser/widevine/onevn_widevine_bundle_unzipper.h"
#include "onevn/common/pref_names.h"
#include "onevn/grit/onevn_generated_resources.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/common/chrome_paths.h"
#include "components/prefs/pref_service.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/cdm_info.h"
#include "content/public/browser/cdm_registry.h"
#include "content/public/common/service_manager_connection.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/service_manager/public/cpp/connector.h"
#include "third_party/widevine/cdm/widevine_cdm_common.h"
#include "url/gurl.h"
#include "widevine_cdm_version.h"

namespace {

constexpr int kWidevineBackgroundUpdateDelayInMins = 5;

base::Optional<base::FilePath> GetTargetWidevineBundleDir() {
  base::FilePath widevine_cdm_dir;
  if (base::PathService::Get(chrome::DIR_USER_DATA, &widevine_cdm_dir)) {
    widevine_cdm_dir = widevine_cdm_dir.Append(
        FILE_PATH_LITERAL(kWidevineCdmBaseDirectory));
    base::CreateDirectory(widevine_cdm_dir);
    return widevine_cdm_dir;
  }

  return base::Optional<base::FilePath>();
}

void SetWidevinePrefs(bool enable) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  Profile* profile = ProfileManager::GetActiveUserProfile();
  DCHECK(profile);

  profile->GetPrefs()->SetBoolean(kWidevineOptedIn, enable);
  profile->GetPrefs()->SetString(
      kWidevineInstalledVersion,
      enable ? WIDEVINE_CDM_VERSION_STRING
             : OnevnWidevineBundleManager::kWidevineInvalidVersion);
}

}  // namespace

// static
char OnevnWidevineBundleManager::kWidevineInvalidVersion[] = "";

// static
void OnevnWidevineBundleManager::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterStringPref(kWidevineInstalledVersion,
                               kWidevineInvalidVersion);
}

OnevnWidevineBundleManager::OnevnWidevineBundleManager() : weak_factory_(this) {
}

OnevnWidevineBundleManager::~OnevnWidevineBundleManager() {
}

void OnevnWidevineBundleManager::InstallWidevineBundle(
    DoneCallback done_callback,
    bool user_gesture) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DVLOG(1) << __func__ << (user_gesture ? ": Install widevine bundle"
                                        : ": Update widevine bundle");
  DCHECK(!needs_restart());
  DCHECK(startup_checked_);

  done_callback_ = std::move(done_callback);
  set_in_progress(true);

  DownloadWidevineBundle(
      GURL(WIDEVINE_CDM_DOWNLOAD_URL_STRING),
      base::BindOnce(&OnevnWidevineBundleManager::OnBundleDownloaded,
                     base::Unretained(this)));
}

void OnevnWidevineBundleManager::DownloadWidevineBundle(
    const GURL& bundle_zipfile_url,
    network::SimpleURLLoader::DownloadToFileCompleteCallback callback) {
  if (is_test_) return;

  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("widevine_bundle_downloader", R"(
        semantics {
          sender:
            "Onevn Widevine Bundle Manager"
          description:
            "Download widevine cdm pkg"
          trigger:
            "When user accpets the use of widevine or update is started"
          data: "Widevine cdm library package"
          destination: GOOGLE_OWNED_SERVICE
        }
        policy {
          cookies_allowed: NO
          setting:
            "This feature can be disabled by disabling widevine in linux"
          policy_exception_justification:
            "Not implemented."
        })");

  auto request = std::make_unique<network::ResourceRequest>();
  request->url = bundle_zipfile_url;
  bundle_loader_ =
      network::SimpleURLLoader::Create(std::move(request), traffic_annotation);
  bundle_loader_->DownloadToTempFile(
      g_browser_process->system_network_context_manager()
          ->GetURLLoaderFactory(),
      std::move(callback)
  );
}

void OnevnWidevineBundleManager::OnBundleDownloaded(
    base::FilePath tmp_bundle_zip_file_path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DVLOG(1) << __func__;

  if (tmp_bundle_zip_file_path.empty()) {
    InstallDone("bundle file download failed");
    return;
  }

  base::PostTaskAndReplyWithResult(
      file_task_runner().get(),
      FROM_HERE,
      base::BindOnce(&GetTargetWidevineBundleDir),
      base::BindOnce(&OnevnWidevineBundleManager::OnGetTargetWidevineBundleDir,
                     weak_factory_.GetWeakPtr(),
                     tmp_bundle_zip_file_path));
}

void OnevnWidevineBundleManager::OnGetTargetWidevineBundleDir(
    const base::FilePath& tmp_bundle_zip_file_path,
    base::Optional<base::FilePath> target_bundle_dir) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DVLOG(1) << __func__;

  if (!target_bundle_dir) {
    InstallDone("getting target widevine dir failed");
    return;
  }

  Unzip(tmp_bundle_zip_file_path, *target_bundle_dir);
}

void OnevnWidevineBundleManager::Unzip(
    const base::FilePath& tmp_bundle_zip_file_path,
    const base::FilePath& target_bundle_dir) {
  if (is_test_) return;

   OnevnWidevineBundleUnzipper::Create(
      content::ServiceManagerConnection::GetForProcess()->GetConnector(),
      file_task_runner(),
      base::BindOnce(&OnevnWidevineBundleManager::OnBundleUnzipped,
                     weak_factory_.GetWeakPtr()))
          ->LoadFromZipFileInDir(tmp_bundle_zip_file_path,
                                 target_bundle_dir,
                                 true);
}

void OnevnWidevineBundleManager::OnBundleUnzipped(const std::string& error) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DVLOG(1) << __func__;
  InstallDone(error);
}

bool OnevnWidevineBundleManager::in_progress() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return in_progress_;
}

void OnevnWidevineBundleManager::set_in_progress(bool in_progress) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK_NE(in_progress_, in_progress);
  DVLOG(1) << __func__ << ": " << in_progress;

  in_progress_ = in_progress;
}

bool OnevnWidevineBundleManager::needs_restart() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return needs_restart_;
}

void OnevnWidevineBundleManager::set_needs_restart(bool needs_restart) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK_NE(needs_restart_, needs_restart);
  DVLOG(1) << __func__ << ": " << needs_restart;

  needs_restart_ = needs_restart;
}

void OnevnWidevineBundleManager::InstallDone(const std::string& error) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  set_in_progress(false);

  // On success, marks that browser needs to restart to enable widevine.
  // Otherwiase, reset prefs to initial state.
  if (error.empty())
    set_needs_restart(true);
  else
    SetWidevinePrefs(false);

  std::move(done_callback_).Run(error);
}

void OnevnWidevineBundleManager::StartupCheck() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  startup_checked_ = true;

  const std::vector<content::CdmInfo>& cdms =
      content::CdmRegistry::GetInstance()->GetAllRegisteredCdms();

  auto has_widevine = [](const content::CdmInfo& cdm) {
      return cdm.supported_key_system == kWidevineKeySystem;
  };

  // If cdms has widevine cdminfo, it means that filesystem has widevine lib.
  if (std::find_if(cdms.begin(), cdms.end(), has_widevine) == cdms.end() ) {
    DVLOG(1) << __func__ << ": reset widevine prefs state";
    // Widevine is not installed yet. Don't need to check.
    // Also reset prefs to make as initial state.
    SetWidevinePrefs(false);
    return;
  }

  Profile* profile = ProfileManager::GetActiveUserProfile();
  DCHECK(profile);

  // Although this case would be very rare, this might be happen because
  // bundle unzipping and prefs setting is done asynchronously.
  if (!profile->GetPrefs()->GetBoolean(kWidevineOptedIn)) {
    DVLOG(1) << __func__ << ": recover invalid widevine prefs state";
    SetWidevinePrefs(true);
    return;
  }

  const std::string installed_version =
      profile->GetPrefs()->GetString(kWidevineInstalledVersion);

  DVLOG(1) << __func__ << ": widevine prefs state looks fine";
  DVLOG(1) << __func__ << ": installed widevine version: " << installed_version;

  // Do delayed update if installed version and latest version are different.
  if (installed_version != WIDEVINE_CDM_VERSION_STRING) {
    update_requested_ = true;
    base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&OnevnWidevineBundleManager::DoDelayedBackgroundUpdate,
                       weak_factory_.GetWeakPtr()),
        base::TimeDelta::FromMinutes(kWidevineBackgroundUpdateDelayInMins));
  }
}

void OnevnWidevineBundleManager::DoDelayedBackgroundUpdate() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  Profile* profile = ProfileManager::GetActiveUserProfile();
  const std::string installed_version =
      profile->GetPrefs()->GetString(kWidevineInstalledVersion);

  DVLOG(1) << __func__ << ": update widevine"
           << " from " << installed_version
           << " to " << WIDEVINE_CDM_VERSION_STRING;

  InstallWidevineBundle(base::BindOnce([](const std::string& error) {
    if (!error.empty()) {
      LOG(ERROR) << __func__ << ": " << error;
      return;
    }

    DVLOG(1) << __func__ << ": Widevine update success";
  }),
  false);
}

int
OnevnWidevineBundleManager::GetWidevinePermissionRequestTextFragment() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return needs_restart() ?
      IDS_WIDEVINE_PERMISSION_REQUEST_TEXT_FRAGMENT_RESTART_BROWSER :
      IDS_WIDEVINE_PERMISSION_REQUEST_TEXT_FRAGMENT_INSTALL;
}

void OnevnWidevineBundleManager::WillRestart() const {
  DCHECK(needs_restart());
  SetWidevinePrefs(true);
  DVLOG(1) << __func__;
}

scoped_refptr<base::SequencedTaskRunner>
OnevnWidevineBundleManager::file_task_runner() {
  if (!file_task_runner_) {
    file_task_runner_ = base::CreateSequencedTaskRunnerWithTraits(
                            { base::MayBlock(),
                              base::TaskPriority::BEST_EFFORT,
                              base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN });
  }
  return file_task_runner_;
}
