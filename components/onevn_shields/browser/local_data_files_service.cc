/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/components/onevn_shields/browser/local_data_files_service.h"

#include <algorithm>
#include <utility>

#include "base/base_paths.h"
#include "base/bind.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "onevn/browser/onevn_browser_process_impl.h"
#include "onevn/components/onevn_shields/browser/base_local_data_files_observer.h"

namespace onevn_shields {

std::string LocalDataFilesService::g_local_data_files_component_id_(
  kLocalDataFilesComponentId);
std::string LocalDataFilesService::g_local_data_files_component_base64_public_key_(
  kLocalDataFilesComponentBase64PublicKey);

LocalDataFilesService::LocalDataFilesService()
  : initialized_(false),
    observers_already_called_(false),
    weak_factory_(this) {
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

LocalDataFilesService::~LocalDataFilesService() { }

bool LocalDataFilesService::Start() {
  if (initialized_)
    return true;
  Register(kLocalDataFilesComponentName,
           g_local_data_files_component_id_,
           g_local_data_files_component_base64_public_key_);
  initialized_ = true;
  return true;
}

void LocalDataFilesService::AddObserver(BaseLocalDataFilesObserver* observer) {
  DCHECK(!observers_already_called_);
  observers_.push_back(observer);
}

void LocalDataFilesService::OnComponentReady(
    const std::string& component_id,
    const base::FilePath& install_dir,
    const std::string& manifest) {
  observers_already_called_ = true;
  for (BaseLocalDataFilesObserver* observer : observers_)
    observer->OnComponentReady(component_id, install_dir, manifest);
}

// static
void LocalDataFilesService::SetComponentIdAndBase64PublicKeyForTest(
    const std::string& component_id,
    const std::string& component_base64_public_key) {
  g_local_data_files_component_id_ = component_id;
  g_local_data_files_component_base64_public_key_ = component_base64_public_key;
}

///////////////////////////////////////////////////////////////////////////////

// The onevn shields factory. Using the Onevn Shields as a singleton
// is the job of the browser process.
std::unique_ptr<LocalDataFilesService> LocalDataFilesServiceFactory() {
  return std::make_unique<LocalDataFilesService>();
}

}  // namespace onevn_shields
