// Copyright (c) 2019 The Onevn Authors. All rights reserved.

#include "onevn/browser/extensions/updater/onevn_update_client_config.h"

#include <algorithm>

#include "base/command_line.h"
#include "base/containers/flat_map.h"
#include "base/no_destructor.h"
#include "base/version.h"
#include "chrome/browser/component_updater/component_updater_utils.h"
#include "chrome/browser/extensions/updater/extension_update_client_command_line_config_policy.h"
#include "chrome/browser/google/google_brand.h"
#include "chrome/browser/update_client/chrome_update_query_params_delegate.h"
#include "chrome/common/channel_info.h"
#include "components/prefs/pref_service.h"
#include "components/update_client/activity_data_service.h"
#include "components/update_client/net/network_chromium.h"
#include "components/update_client/protocol_handler.h"
#include "components/update_client/update_query_params.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/service_manager_connection.h"
#include "extensions/browser/extension_prefs.h"
#include "services/service_manager/public/cpp/connector.h"

namespace extensions {

namespace {

using FactoryCallback = OnevnUpdateClientConfig::FactoryCallback;

// static
static FactoryCallback& GetFactoryCallback() {
  static base::NoDestructor<FactoryCallback> factory;
  return *factory;
}

class ExtensionActivityDataService final
    : public update_client::ActivityDataService {
 public:
  explicit ExtensionActivityDataService(ExtensionPrefs* extension_prefs);
  ~ExtensionActivityDataService() override {}

  // update_client::ActivityDataService:
  bool GetActiveBit(const std::string& id) const override;
  int GetDaysSinceLastActive(const std::string& id) const override;
  int GetDaysSinceLastRollCall(const std::string& id) const override;
  void ClearActiveBit(const std::string& id) override;

 private:
  // This member is not owned by this class, it's owned by a profile keyed
  // service.
  ExtensionPrefs* extension_prefs_;

  DISALLOW_COPY_AND_ASSIGN(ExtensionActivityDataService);
};

// Calculates the value to use for the ping days parameter.
int CalculatePingDays(const base::Time& last_ping_day) {
  return last_ping_day.is_null()
             ? update_client::kDaysFirstTime
             : std::max((base::Time::Now() - last_ping_day).InDays(), 0);
}

ExtensionActivityDataService::ExtensionActivityDataService(
    ExtensionPrefs* extension_prefs)
    : extension_prefs_(extension_prefs) {
  DCHECK(extension_prefs_);
}

bool ExtensionActivityDataService::GetActiveBit(const std::string& id) const {
  return extension_prefs_->GetActiveBit(id);
}

int ExtensionActivityDataService::GetDaysSinceLastActive(
    const std::string& id) const {
  return CalculatePingDays(extension_prefs_->LastActivePingDay(id));
}

int ExtensionActivityDataService::GetDaysSinceLastRollCall(
    const std::string& id) const {
  return CalculatePingDays(extension_prefs_->LastPingDay(id));
}

void ExtensionActivityDataService::ClearActiveBit(const std::string& id) {
  extension_prefs_->SetActiveBit(id, false);
}

}  // namespace

// For privacy reasons, requires encryption of the component updater
// communication with the update backend.
OnevnUpdateClientConfig::OnevnUpdateClientConfig(
    content::BrowserContext* context)
    : context_(context),
      impl_(ExtensionUpdateClientCommandLineConfigPolicy(
                base::CommandLine::ForCurrentProcess()),
            /*require_encryption=*/true),
      pref_service_(ExtensionPrefs::Get(context)->pref_service()),
      activity_data_service_(std::make_unique<ExtensionActivityDataService>(
          ExtensionPrefs::Get(context))) {
  DCHECK(pref_service_);
}

int OnevnUpdateClientConfig::InitialDelay() const {
  return impl_.InitialDelay();
}

int OnevnUpdateClientConfig::NextCheckDelay() const {
  return impl_.NextCheckDelay();
}

int OnevnUpdateClientConfig::OnDemandDelay() const {
  return impl_.OnDemandDelay();
}

int OnevnUpdateClientConfig::UpdateDelay() const {
  return impl_.UpdateDelay();
}

std::vector<GURL> OnevnUpdateClientConfig::UpdateUrl() const {
  return impl_.UpdateUrl();
}

std::vector<GURL> OnevnUpdateClientConfig::PingUrl() const {
  return impl_.PingUrl();
}

std::string OnevnUpdateClientConfig::GetProdId() const {
  return update_client::UpdateQueryParams::GetProdIdString(
      update_client::UpdateQueryParams::ProdId::CRX);
}

base::Version OnevnUpdateClientConfig::GetBrowserVersion() const {
  return impl_.GetBrowserVersion();
}

std::string OnevnUpdateClientConfig::GetChannel() const {
  return std::string("stable");;
}

std::string OnevnUpdateClientConfig::GetBrand() const {
  std::string brand;
  google_brand::GetBrand(&brand);
  return brand;
}

std::string OnevnUpdateClientConfig::GetLang() const {
  return ChromeUpdateQueryParamsDelegate::GetLang();
}

std::string OnevnUpdateClientConfig::GetOSLongName() const {
  return impl_.GetOSLongName();
}

base::flat_map<std::string, std::string>
OnevnUpdateClientConfig::ExtraRequestParams() const {
  return impl_.ExtraRequestParams();
}

std::string OnevnUpdateClientConfig::GetDownloadPreference() const {
  return std::string();
}

scoped_refptr<update_client::NetworkFetcherFactory>
OnevnUpdateClientConfig::GetNetworkFetcherFactory() {
  if (!network_fetcher_factory_) {
    network_fetcher_factory_ =
        base::MakeRefCounted<update_client::NetworkFetcherChromiumFactory>(
            content::BrowserContext::GetDefaultStoragePartition(context_)
                ->GetURLLoaderFactoryForBrowserProcess());
  }
  return network_fetcher_factory_;
}

std::unique_ptr<service_manager::Connector>
OnevnUpdateClientConfig::CreateServiceManagerConnector() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return content::ServiceManagerConnection::GetForProcess()
      ->GetConnector()
      ->Clone();
}

bool OnevnUpdateClientConfig::EnabledDeltas() const {
  return impl_.EnabledDeltas();
}

bool OnevnUpdateClientConfig::EnabledComponentUpdates() const {
  return impl_.EnabledComponentUpdates();
}

bool OnevnUpdateClientConfig::EnabledBackgroundDownloader() const {
  return impl_.EnabledBackgroundDownloader();
}

// Disabling cup signing
bool OnevnUpdateClientConfig::EnabledCupSigning() const {
  return false;
}

PrefService* OnevnUpdateClientConfig::GetPrefService() const {
  return pref_service_;
}

update_client::ActivityDataService*
OnevnUpdateClientConfig::GetActivityDataService() const {
  return activity_data_service_.get();
}

bool OnevnUpdateClientConfig::IsPerUserInstall() const {
  return component_updater::IsPerUserInstall();
}

std::vector<uint8_t> OnevnUpdateClientConfig::GetRunActionKeyHash() const {
  return impl_.GetRunActionKeyHash();
}

std::string OnevnUpdateClientConfig::GetAppGuid() const {
  return impl_.GetAppGuid();
}

// Always use XML ProtocolHandler
std::unique_ptr<update_client::ProtocolHandlerFactory>
OnevnUpdateClientConfig::GetProtocolHandlerFactory() const {
  return std::make_unique<update_client::ProtocolHandlerFactoryXml>();
}

update_client::RecoveryCRXElevator
OnevnUpdateClientConfig::GetRecoveryCRXElevator() const {
#if defined(GOOGLE_CHROME_BUILD) && defined(OS_WIN)
  return base::BindOnce(&RunRecoveryCRXElevated);
#else
  return {};
#endif
}

OnevnUpdateClientConfig::~OnevnUpdateClientConfig() {}

// static
scoped_refptr<OnevnUpdateClientConfig> OnevnUpdateClientConfig::Create(
    content::BrowserContext* context) {
  FactoryCallback& factory = GetFactoryCallback();
  return factory.is_null() ? scoped_refptr<OnevnUpdateClientConfig>(
                                 new OnevnUpdateClientConfig(context))
                           : factory.Run(context);
}

// static
void OnevnUpdateClientConfig::SetOnevnUpdateClientConfigFactoryForTesting(
    FactoryCallback factory) {
  DCHECK(!factory.is_null());
  GetFactoryCallback() = factory;
}

}  // namespace extensions
