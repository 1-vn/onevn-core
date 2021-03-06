/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/browser/net/onevn_static_redirect_network_delegate_helper.h"

#include <memory>
#include <string>

#include "onevn/browser/net/url_context.h"
#include "onevn/common/network_constants.h"
#include "onevn/common/translate_network_constants.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "components/component_updater/component_updater_url_constants.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "net/url_request/url_request_test_util.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace {

class OnevnStaticRedirectNetworkDelegateHelperTest : public testing::Test {
 public:
  OnevnStaticRedirectNetworkDelegateHelperTest()
      : thread_bundle_(content::TestBrowserThreadBundle::IO_MAINLOOP),
        context_(new net::TestURLRequestContext(true)) {}
  ~OnevnStaticRedirectNetworkDelegateHelperTest() override {}
  void SetUp() override { context_->Init(); }
  net::TestURLRequestContext* context() { return context_.get(); }

 private:
  content::TestBrowserThreadBundle thread_bundle_;
  std::unique_ptr<net::TestURLRequestContext> context_;
};

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, NoModifyTypicalURL) {
  net::TestDelegate test_delegate;
  GURL url("https://bradhatesprimes.1-vn.com/composite_numbers_ftw");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_TRUE(before_url_context->new_url_spec.empty());
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyGeoURL) {
  net::TestDelegate test_delegate;
  GURL url("https://www.googleapis.com/geolocation/v1/geolocate?key=2_3_5_7");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(GOOGLEAPIS_ENDPOINT GOOGLEAPIS_API_KEY);
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyCRLSet1) {
  net::TestDelegate test_delegate;
  GURL url(
      "https://dl.google.com/release2/chrome_component/AJ4r388iQSJq_4819/"
      "4819_all_crl-set-5934829738003798040.data.crx3");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(
      "https://crlsets.1-vn.com/release2/chrome_component/"
      "AJ4r388iQSJq_4819/4819_all_crl-set-5934829738003798040.data.crx3");
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyCRLSet2) {
  net::TestDelegate test_delegate;
  GURL url(
      "https://r2---sn-8xgp1vo-qxoe.gvt1.com/edgedl/release2/"
      "chrome_component/AJ4r388iQSJq_4819/4819_all_crl-set-5934829738003798040"
      ".data.crx3");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(
      "https://crlsets.1-vn.com/edgedl/release2/chrome_compone"
      "nt/AJ4r388iQSJq_4819/4819_all_crl-set-5934829738003798040.data.crx3");
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyCRLSet3) {
  net::TestDelegate test_delegate;
  GURL url(
      "https://www.google.com/dl/release2/chrome_component/LLjIBPPmveI_4988/"
      "4988_all_crl-set-6296993568184466307.data.crx3");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(
      "https://crlsets.1-vn.com/dl/release2/chrome_component/LLjIBPPmveI_4988/"
      "4988_all_crl-set-6296993568184466307.data.crx3");
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyCRXDownload) {
  net::TestDelegate test_delegate;
  GURL url(
      "https://clients2.googleusercontent.com/crx/blobs/QgAAAC6zw0qH2DJtn"
      "Xe8Z7rUJP1RM6lX7kVcwkQ56ujmG3AWYOAkxoNnIdnEBUz_"
      "3z4keVhjzzAF10srsaL7lrntfB"
      "IflcYIrTziwX3SUS9i_P-CAMZSmuV5tdQl-Roo6cnVC_GRzKsnZSKm1Q/"
      "extension_2_0_67"
      "3_0.crx");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(
      "https://crxdownload.1-vn.com/crx/blobs/QgAAAC6"
      "zw0qH2DJtnXe8Z7rUJP1RM6lX7kVcwkQ56ujmG3AWYOAkxoNnIdnEBUz_"
      "3z4keVhjzzAF10sr"
      "saL7lrntfBIflcYIrTziwX3SUS9i_P-CAMZSmuV5tdQl-Roo6cnVC_GRzKsnZSKm1Q/"
      "extens"
      "ion_2_0_673_0.crx");
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyCRLSet1_http) {
  net::TestDelegate test_delegate;
  GURL url(
      "http://dl.google.com/release2/chrome_component/AJ4r388iQSJq_4819/"
      "4819_all_crl-set-5934829738003798040.data.crx3");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(
      "https://crlsets.1-vn.com/release2/chrome_component/"
      "AJ4r388iQSJq_4819/4819_all_crl-set-5934829738003798040.data.crx3");
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyCRLSet2_http) {
  net::TestDelegate test_delegate;
  GURL url(
      "http://r2---sn-8xgp1vo-qxoe.gvt1.com/edgedl/release2/"
      "chrome_component/AJ4r388iQSJq_4819/4819_all_crl-set-5934829738003798040"
      ".data.crx3");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(
      "https://crlsets.1-vn.com/edgedl/release2/chrome_compone"
      "nt/AJ4r388iQSJq_4819/4819_all_crl-set-5934829738003798040.data.crx3");
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyCRLSet3_http) {
  net::TestDelegate test_delegate;
  GURL url(
      "http://www.google.com/dl/release2/chrome_component/LLjIBPPmveI_4988/"
      "4988_all_crl-set-6296993568184466307.data.crx3");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(
      "https://crlsets.1-vn.com/dl/release2/chrome_component/LLjIBPPmveI_4988/"
      "4988_all_crl-set-6296993568184466307.data.crx3");
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifyCRXDownload_http) {
  net::TestDelegate test_delegate;
  GURL url(
      "http://clients2.googleusercontent.com/crx/blobs/QgAAAC6zw0qH2DJtn"
      "Xe8Z7rUJP1RM6lX7kVcwkQ56ujmG3AWYOAkxoNnIdnEBUz_"
      "3z4keVhjzzAF10srsaL7lrntfB"
      "IflcYIrTziwX3SUS9i_P-CAMZSmuV5tdQl-Roo6cnVC_GRzKsnZSKm1Q/"
      "extension_2_0_67"
      "3_0.crx");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(
      "https://crxdownload.1-vn.com/crx/blobs/QgAAAC6"
      "zw0qH2DJtnXe8Z7rUJP1RM6lX7kVcwkQ56ujmG3AWYOAkxoNnIdnEBUz_"
      "3z4keVhjzzAF10sr"
      "saL7lrntfBIflcYIrTziwX3SUS9i_P-CAMZSmuV5tdQl-Roo6cnVC_GRzKsnZSKm1Q/"
      "extens"
      "ion_2_0_673_0.crx");
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifySafeBrowsingURLV4) {
  net::TestDelegate test_delegate;
  GURL url(
      "https://safebrowsing.googleapis.com/v4/"
      "threatListUpdates:fetch?$req=ChkKCGNocm9taXVtEg02Ni");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL::Replacements replacements;
  replacements.SetHostStr(SAFEBROWSING_ENDPOINT);
  GURL expected_url(url.ReplaceComponents(replacements));
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, ModifySafeBrowsingURLV5) {
  net::TestDelegate test_delegate;
  GURL url(
      "https://safebrowsing.googleapis.com/v5/"
      "threatListUpdates:fetch?$req=ChkKCGNocm9taXVtEg02Ni");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL::Replacements replacements;
  replacements.SetHostStr(SAFEBROWSING_ENDPOINT);
  GURL expected_url(url.ReplaceComponents(replacements));
  int ret = OnBeforeURLRequest_StaticRedirectWork(callback, before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest, RedirectTranslate) {
  net::TestDelegate test_delegate;
  std::string query_string(
      "?cb=cr.googleTranslate.onTranslateElementLoad&aus=true&"
      "clc=cr.googleTranslate.onLoadCSS&"
      "jlc=cr.googleTranslate.onLoadJavascript&hl=en&key=DUMMY_KEY");
  std::string path_string("/translate_a/element.js");
  std::string google_host_string("https://translate.googleapis.com");
  GURL url(google_host_string + path_string + query_string);
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);

  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(kOnevnTranslateServer + path_string + query_string);

  int ret = OnBeforeURLRequest_StaticRedirectWork(callback,
      before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(OnevnStaticRedirectNetworkDelegateHelperTest,
    RedirectTranslateLanguage) {
  net::TestDelegate test_delegate;
  GURL url(
      "https://translate.googleapis.com/translate_a/l?"
      "client=chrome&hl=en&key=DUMMY_KEY");
  std::unique_ptr<net::URLRequest> request = context()->CreateRequest(
      url, net::IDLE, &test_delegate, TRAFFIC_ANNOTATION_FOR_TESTS);

  std::shared_ptr<onevn::OnevnRequestInfo> before_url_context(
      new onevn::OnevnRequestInfo());
  onevn::OnevnRequestInfo::FillCTXFromRequest(request.get(),
                                              before_url_context);
  onevn::ResponseCallback callback;
  GURL expected_url(kOnevnTranslateLanguageEndpoint);

  int ret = OnBeforeURLRequest_StaticRedirectWork(callback,
      before_url_context);
  EXPECT_EQ(before_url_context->new_url_spec, expected_url);
  EXPECT_EQ(ret, net::OK);
}

}  // namespace
