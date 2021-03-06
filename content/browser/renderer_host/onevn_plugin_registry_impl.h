/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_CONTENT_BROWSER_RENDERER_HOST_PLUGIN_REGISTRY_IMPL_H_
#define ONEVN_CONTENT_BROWSER_RENDERER_HOST_PLUGIN_REGISTRY_IMPL_H_

#include "content/browser/renderer_host/plugin_registry_impl.h"

namespace content {

class ResourceContext;

class OnevnPluginRegistryImpl : public PluginRegistryImpl {
 public:
  OnevnPluginRegistryImpl(ResourceContext* resource_context);
  ~OnevnPluginRegistryImpl() override;

  void GetPlugins(bool refresh,
                  const url::Origin& main_frame_origin,
                  GetPluginsCallback callback) override;
 private:
  void GetPluginsComplete(GetPluginsCallback callback,
    std::vector<blink::mojom::PluginInfoPtr> plugins);
};

}  // namespace content

#endif  // ONEVN_CONTENT_BROWSER_RENDERER_HOST_PLUGIN_REGISTRY_IMPL_H_