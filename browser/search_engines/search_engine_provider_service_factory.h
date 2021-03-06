/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_BROWSER_SEARCH_ENGINE_SEARCH_ENGINE_PROVIDER_SERVICE_FACTORY_H_
#define ONEVN_BROWSER_SEARCH_ENGINE_SEARCH_ENGINE_PROVIDER_SERVICE_FACTORY_H_

#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

// The purpose of this factory is to configure proper search engine provider to
// private/guest/tor profile before it is referenced.
// Also, this factory doesn't have public api. Instead, service is instantiated
// when profile is inistialized.
class SearchEngineProviderServiceFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  static SearchEngineProviderServiceFactory* GetInstance();

 private:
  friend
      struct base::DefaultSingletonTraits<SearchEngineProviderServiceFactory>;
  SearchEngineProviderServiceFactory();
  ~SearchEngineProviderServiceFactory() override;

  // BrowserContextKeyedServiceFactory overrides:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
  bool ServiceIsNULLWhileTesting() const override;
  bool ServiceIsCreatedWithBrowserContext() const override;
  void RegisterProfilePrefs(
      user_prefs::PrefRegistrySyncable* registry) override;

  DISALLOW_COPY_AND_ASSIGN(SearchEngineProviderServiceFactory);
};

#endif  // ONEVN_BROWSER_SEARCH_ENGINE_SEARCH_ENGINE_PROVIDER_SERVICE_FACTORY_H_
