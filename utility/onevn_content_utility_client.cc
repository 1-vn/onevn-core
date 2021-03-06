/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/utility/onevn_content_utility_client.h"

#include <memory>
#include <string>
#include <utility>

#include "onevn/common/tor/tor_launcher.mojom.h"
#include "onevn/components/onevn_ads/browser/buildflags/buildflags.h"
#include "onevn/components/onevn_rewards/browser/buildflags/buildflags.h"
#include "onevn/utility/tor/tor_launcher_service.h"

#if BUILDFLAG(ONEVN_ADS_ENABLED)
#include "onevn/components/services/bat_ads/bat_ads_app.h"
#include "onevn/components/services/bat_ads/public/interfaces/bat_ads.mojom.h"
#endif

#if BUILDFLAG(ONEVN_REWARDS_ENABLED)
#include "onevn/components/services/bat_ledger/bat_ledger_app.h"
#include "onevn/components/services/bat_ledger/public/interfaces/bat_ledger.mojom.h"
#endif

#include "content/public/utility/utility_thread.h"

OnevnContentUtilityClient::OnevnContentUtilityClient()
    : ChromeContentUtilityClient() {}

OnevnContentUtilityClient::~OnevnContentUtilityClient() = default;

namespace {

void RunServiceAsyncThenTerminateProcess(
    std::unique_ptr<service_manager::Service> service) {
  service_manager::Service::RunAsyncUntilTermination(
      std::move(service),
      base::BindOnce([] { content::UtilityThread::Get()->ReleaseProcess(); }));
}

std::unique_ptr<service_manager::Service> CreateTorLauncherService(
    service_manager::mojom::ServiceRequest request) {
  return std::make_unique<tor::TorLauncherService>(
      std::move(request));
}

#if BUILDFLAG(ONEVN_ADS_ENABLED)
std::unique_ptr<service_manager::Service> CreateBatAdsService(
    service_manager::mojom::ServiceRequest request) {
  return std::make_unique<bat_ads::BatAdsApp>(
      std::move(request));
}
#endif

#if BUILDFLAG(ONEVN_REWARDS_ENABLED)
std::unique_ptr<service_manager::Service> CreateBatLedgerService(
    service_manager::mojom::ServiceRequest request) {
  return std::make_unique<bat_ledger::BatLedgerApp>(
      std::move(request));
}
#endif

}  // namespace

bool OnevnContentUtilityClient::HandleServiceRequest(
    const std::string& service_name,
    service_manager::mojom::ServiceRequest request) {

  if (service_name == tor::mojom::kTorLauncherServiceName) {
    RunServiceAsyncThenTerminateProcess(
        CreateTorLauncherService(std::move(request)));
    return true;
  }

#if BUILDFLAG(ONEVN_ADS_ENABLED)
  if (service_name == bat_ads::mojom::kServiceName) {
    RunServiceAsyncThenTerminateProcess(
        CreateBatAdsService(std::move(request)));
    return true;
  }
#endif

#if BUILDFLAG(ONEVN_REWARDS_ENABLED)
  if (service_name == bat_ledger::mojom::kServiceName) {
    RunServiceAsyncThenTerminateProcess(
        CreateBatLedgerService(std::move(request)));
    return true;
  }
#endif

  return ChromeContentUtilityClient::HandleServiceRequest(
      service_name, std::move(request));
}

