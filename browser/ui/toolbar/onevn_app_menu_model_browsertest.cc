/* Copyright (c) 2019 The Onevn Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/browser/ui/toolbar/onevn_app_menu_model.h"

#include <memory>

#include "onevn/app/onevn_command_ids.h"
#include "onevn/browser/ui/onevn_browser_command_controller.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_window.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/toolbar/toolbar_view.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "content/public/browser/notification_service.h"
#include "content/public/test/test_utils.h"

using OnevnAppMenuBrowserTest = InProcessBrowserTest;

IN_PROC_BROWSER_TEST_F(OnevnAppMenuBrowserTest, BasicTest) {
  auto* browser_view = BrowserView::GetBrowserViewForBrowser(browser());
  OnevnAppMenuModel normal_model(browser_view->toolbar(), browser());
  normal_model.Init();

  // Check normal window has both menu items.
  // -1 means |model| doesn't have passed command id.
  EXPECT_NE(-1, normal_model.GetIndexOfCommandId(IDC_SHOW_ONEVN_REWARDS));
  EXPECT_NE(-1, normal_model.GetIndexOfCommandId(IDC_SHOW_ONEVN_SYNC));

  auto* command_controller = browser()->command_controller();
  EXPECT_TRUE(command_controller->IsCommandEnabled(IDC_SHOW_ONEVN_REWARDS));
  EXPECT_TRUE(command_controller->IsCommandEnabled(IDC_SHOW_ONEVN_SYNC));

  // Create proviate browser.
  auto* private_browser = CreateIncognitoBrowser();
  auto* private_browser_view =
      BrowserView::GetBrowserViewForBrowser(private_browser);
  OnevnAppMenuModel private_model(private_browser_view->toolbar(),
                                  private_browser);
  private_model.Init();

  // Check private window has both menu items.
  // -1 means |model| doesn't have passed command id.
  EXPECT_NE(-1, private_model.GetIndexOfCommandId(IDC_SHOW_ONEVN_REWARDS));
  EXPECT_NE(-1, private_model.GetIndexOfCommandId(IDC_SHOW_ONEVN_SYNC));

  command_controller = private_browser->command_controller();
  EXPECT_TRUE(command_controller->IsCommandEnabled(IDC_SHOW_ONEVN_REWARDS));
  EXPECT_TRUE(command_controller->IsCommandEnabled(IDC_SHOW_ONEVN_SYNC));

  content::WindowedNotificationObserver browser_creation_observer(
      chrome::NOTIFICATION_BROWSER_OPENED,
      content::NotificationService::AllSources());
  profiles::SwitchToGuestProfile(ProfileManager::CreateCallback());

  browser_creation_observer.Wait();

  auto* browser_list = BrowserList::GetInstance();
  Browser* guest_browser = nullptr;
  for (Browser* browser : *browser_list) {
    if (browser->profile()->IsGuestSession()) {
      guest_browser = browser;
      break;
    }
  }
  DCHECK(guest_browser);

  auto* guest_browser_view =
      BrowserView::GetBrowserViewForBrowser(guest_browser);
  OnevnAppMenuModel guest_model(guest_browser_view->toolbar(), guest_browser);
  guest_model.Init();

  // Check guest window doesn't have them.
  // -1 means |model| doesn't have passed command id.
  EXPECT_EQ(-1, guest_model.GetIndexOfCommandId(IDC_SHOW_ONEVN_REWARDS));
  EXPECT_EQ(-1, guest_model.GetIndexOfCommandId(IDC_SHOW_ONEVN_SYNC));

  command_controller = guest_browser->command_controller();
  EXPECT_FALSE(command_controller->IsCommandEnabled(IDC_SHOW_ONEVN_REWARDS));
  EXPECT_FALSE(command_controller->IsCommandEnabled(IDC_SHOW_ONEVN_SYNC));
}
