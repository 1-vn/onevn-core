/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_page_graph/graph_item/edge/edge_attribute_set.h"
#include <string>
#include "brave/components/brave_page_graph/graph_item/edge/edge_attribute.h"
#include "brave/components/brave_page_graph/graph_item/node.h"
#include "brave/components/brave_page_graph/graph_item/node/node_actor.h"
#include "brave/components/brave_page_graph/graph_item/node/node_html_element.h"
#include "brave/components/brave_page_graph/page_graph.h"
#include "brave/components/brave_page_graph/types.h"

using ::std::string;

namespace brave_page_graph {

EdgeAttributeSet::EdgeAttributeSet(const PageGraph* graph, const PageGraphId id,
    const NodeActor* out_node, const NodeHTMLElement* in_node,
    const string& name, const string& value) :
      EdgeAttribute(graph, id, out_node, in_node, name),
      value_(value) {}

EdgeAttributeSet::~EdgeAttributeSet() {}

string EdgeAttributeSet::ItemName() const {
  return "EdgeAttributeSet#" + ::std::to_string(id_);
}

string EdgeAttributeSet::AttributeValue() const {
  return value_;
}

string EdgeAttributeSet::ToStringBody() const {
  return ItemName() + " [" + AttributeName() + "=" + AttributeValue() + "]";
}

}  // namespace brave_page_graph