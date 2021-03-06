/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ONEVN_BROWSER_NET_URL_CONTEXT_H_
#define ONEVN_BROWSER_NET_URL_CONTEXT_H_

#include <memory>
#include <string>

#include "chrome/browser/net/chrome_network_delegate.h"
#include "content/public/common/resource_type.h"
#include "net/url_request/url_request.h"
#include "url/gurl.h"

class OnevnNetworkDelegateBase;

namespace onevn {

struct OnevnRequestInfo;
using ResponseCallback = base::Callback<void()>;

}  // namespace onevn

namespace onevn_rewards {
  int OnBeforeURLRequest(
      const onevn::ResponseCallback& next_callback,
      std::shared_ptr<onevn::OnevnRequestInfo> ctx);
}  // namespace onevn_rewards

namespace onevn {

enum OnevnNetworkDelegateEventType {
  kOnBeforeRequest,
  kOnBeforeStartTransaction,
  kOnHeadersReceived,
  kOnCanGetCookies,
  kOnCanSetCookies,
  kUnknownEventType
};

enum BlockedBy {
  kNotBlocked ,
  kAdBlocked,
  kTrackerBlocked,
  kOtherBlocked
};

struct OnevnRequestInfo {
  OnevnRequestInfo();
  ~OnevnRequestInfo();
  GURL request_url;
  GURL tab_origin;
  GURL tab_url;
  GURL initiator_url;
  std::string new_url_spec;
  bool allow_onevn_shields = true;
  bool allow_ads = false;
  bool allow_http_upgradable_resource = false;
  bool allow_1p_cookies = true;
  bool allow_3p_cookies = false;
  bool allow_google_auth = true;
  int render_process_id = 0;
  int render_frame_id = 0;
  int frame_tree_node_id = 0;
  uint64_t request_identifier = 0;
  size_t next_url_request_index = 0;
  net::HttpRequestHeaders* headers = nullptr;
  const net::HttpResponseHeaders* original_response_headers = nullptr;
  scoped_refptr<net::HttpResponseHeaders>* override_response_headers = nullptr;
  GURL* allowed_unsafe_redirect_url = nullptr;
  OnevnNetworkDelegateEventType event_type = kUnknownEventType;
  const base::ListValue* referral_headers_list = nullptr;
  BlockedBy blocked_by = kNotBlocked;
  bool cancel_request_explicitly = false;
  // Default to invalid type for resource_type, so delegate helpers
  // can properly detect that the info couldn't be obtained.
  content::ResourceType resource_type = content::RESOURCE_TYPE_LAST_TYPE;

  static void FillCTXFromRequest(const net::URLRequest* request,
    std::shared_ptr<onevn::OnevnRequestInfo> ctx);

 private:
  // Please don't add any more friends here if it can be avoided.
  // We should also remove the ones below.
  friend int OnBeforeURLRequest_SiteHacksWork(
      const ResponseCallback& next_callback,
      std::shared_ptr<OnevnRequestInfo> ctx);
  friend int onevn_rewards::OnBeforeURLRequest(
      const onevn::ResponseCallback& next_callback,
      std::shared_ptr<onevn::OnevnRequestInfo> ctx);
  friend int OnBeforeURLRequest_TorWork(
      const ResponseCallback& next_callback,
      std::shared_ptr<OnevnRequestInfo> ctx);
  friend class ::OnevnNetworkDelegateBase;

  // Don't use this directly after any dispatch
  // request is deprecated, do not use it.
  const net::URLRequest* request;
  GURL* new_url = nullptr;

  DISALLOW_COPY_AND_ASSIGN(OnevnRequestInfo);
};

// ResponseListener
using OnBeforeURLRequestCallback =
    base::Callback<int(const ResponseCallback& next_callback,
        std::shared_ptr<OnevnRequestInfo> ctx)>;
using OnBeforeStartTransactionCallback =
    base::Callback<int(net::URLRequest* request,
        net::HttpRequestHeaders* headers,
        const ResponseCallback& next_callback,
        std::shared_ptr<OnevnRequestInfo> ctx)>;
using OnHeadersReceivedCallback =
    base::Callback<int(net::URLRequest* request,
        const net::HttpResponseHeaders* original_response_headers,
        scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
        GURL* allowed_unsafe_redirect_url,
        const ResponseCallback& next_callback,
        std::shared_ptr<OnevnRequestInfo> ctx)>;
using OnCanGetCookiesCallback =
    base::Callback<bool(std::shared_ptr<OnevnRequestInfo> ctx)>;
using OnCanSetCookiesCallback =
    base::Callback<bool(std::shared_ptr<OnevnRequestInfo> ctx)>;

}  // namespace onevn


#endif  // ONEVN_BROWSER_NET_URL_CONTEXT_H_
