/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "onevn/browser/ui/onevn_layout_constants.h"

#include "chrome/browser/ui/layout_constants.h"
#include "ui/base/material_design/material_design_controller.h"

// Returns a |nullopt| if the UI color is not handled by Onevn.
base::Optional<int> GetOnevnLayoutConstant(LayoutConstant constant) {
  const bool touch = ui::MaterialDesignController::touch_ui();
  // const bool hybrid = mode == ui::MaterialDesignController::MATERIAL_HYBRID;
  // const bool touch_optimized_material =
  //     ui::MaterialDesignController::touch_ui();
  // const bool newer_material = ui::MaterialDesignController::IsNewerMaterialUi();
  switch (constant) {
    case LOCATION_BAR_BUBBLE_CORNER_RADIUS:
      // Note: this is likely to be moved in to views/layout_provider.h
      // in a future chromium version.
      return 4;
    case TAB_HEIGHT: {
      return (touch ? 41 : 30) + GetLayoutConstant(TABSTRIP_TOOLBAR_OVERLAP);
    }
    default:
      break;
  }
  return base::nullopt;
}
