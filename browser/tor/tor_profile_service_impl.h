/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_BROWSER_TOR_TOR_PROFILE_SERVICE_IMPL_H_
#define ONEVN_BROWSER_TOR_TOR_PROFILE_SERVICE_IMPL_H_

#include "onevn/browser/tor/tor_profile_service.h"

#include <string>

#include "base/memory/scoped_refptr.h"
#include "onevn/browser/tor/tor_launcher_factory.h"
#include "onevn/browser/tor/tor_proxy_config_service.h"

class Profile;

namespace net {
class URLRequestContextGetter;
}

namespace tor {

class TorProfileServiceImpl : public TorProfileService,
                              public base::CheckedObserver {
 public:
  explicit TorProfileServiceImpl(Profile* profile);
  ~TorProfileServiceImpl() override;

  // KeyedService:
  void Shutdown() override;

  // TorProfileService:
  void LaunchTor(const TorConfig&) override;
  void ReLaunchTor(const TorConfig&) override;
  void SetNewTorCircuit(const GURL& request_url, const base::Closure&) override;
  const TorConfig& GetTorConfig() override;
  int64_t GetTorPid() override;

  int SetProxy(net::ProxyResolutionService*,
               const GURL& request_url,
               bool new_circuit) override;

  void KillTor();

  // For internal observer
  void NotifyTorLauncherCrashed();
  void NotifyTorCrashed(int64_t pid);
  void NotifyTorLaunched(bool result, int64_t pid);

 private:
  void SetNewTorCircuitOnIOThread(
      const scoped_refptr<net::URLRequestContextGetter>&, std::string);

  Profile* profile_;  // NOT OWNED
  TorLauncherFactory* tor_launcher_factory_;  // Singleton
  TorProxyConfigService::TorProxyMap tor_proxy_map_;
  DISALLOW_COPY_AND_ASSIGN(TorProfileServiceImpl);
};

}  // namespace tor

#endif  // ONEVN_BROWSER_TOR_TOR_PROFILE_SERVICE_IMPL_H_
