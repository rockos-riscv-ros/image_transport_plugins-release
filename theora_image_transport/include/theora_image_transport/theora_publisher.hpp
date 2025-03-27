// Copyright (c) 2012, Willow Garage, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef THEORA_IMAGE_TRANSPORT__THEORA_PUBLISHER_HPP_
#define THEORA_IMAGE_TRANSPORT__THEORA_PUBLISHER_HPP_

#include <theora/codec.h>
#include <theora/theoraenc.h>
#include <theora/theoradec.h>

#include <memory>
#include <string>
#include <vector>

#include <rclcpp/node.hpp>

#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>

#include <image_transport/simple_publisher_plugin.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <std_msgs/msg/header.hpp>
#include <theora_image_transport/msg/packet.hpp>

#include "theora_image_transport/compression_common.hpp"

namespace theora_image_transport
{

using ParameterEvent = rcl_interfaces::msg::ParameterEvent;

class TheoraPublisher
  : public image_transport::SimplePublisherPlugin<theora_image_transport::msg::Packet>
{
public:
  TheoraPublisher();
  ~TheoraPublisher() override;

  // Return the system unique string representing the theora transport type
  std::string getTransportName() const override {return "theora";}

protected:
  void advertiseImpl(
    rclcpp::Node * node,
    const std::string & base_topic,
    rmw_qos_profile_t custom_qos,
    rclcpp::PublisherOptions options) override;

  // TODO(anyone): Callback to send header packets to new clients
  // virtual void connectCallback(const ros::SingleSubscriberPublisher& pub);

  // Main publish function
  void publish(
    const sensor_msgs::msg::Image & message,
    const PublishFn & publish_fn) const override;

  // Runtime reconfiguration support
  void refreshConfig() const;
  mutable bool refreshConfigNeeded;

  // Utility functions
  bool ensureEncodingContext(
    const sensor_msgs::msg::Image & image,
    const PublishFn & publish_fn) const;
  void oggPacketToMsg(
    const std_msgs::msg::Header & header,
    const ogg_packet & oggpacket,
    theora_image_transport::msg::Packet & msg) const;
  void updateKeyframeFrequency() const;

  // Some data is preserved across calls to publish(), but from the user's perspective publish() is
  // "logically const"
  mutable cv_bridge::CvImage img_image;
  mutable th_info encoder_setup_;
  mutable ogg_uint32_t keyframe_frequency_;
  mutable std::shared_ptr<th_enc_ctx> encoding_context_;
  mutable std::vector<theora_image_transport::msg::Packet> stream_header_;

  rclcpp::Logger logger_;
  rclcpp::Node * node_;

private:
  std::vector<std::string> parameters_;
  std::vector<std::string> deprecatedParameters_;

  rclcpp::Subscription<ParameterEvent>::SharedPtr parameter_subscription_;

  void declareParameter(
    const std::string & base_name,
    const ParameterDefinition & definition);

  void onParameterEvent(
    ParameterEvent::SharedPtr event, std::string full_name,
    std::string base_name);
};

}  // namespace theora_image_transport

#endif  // THEORA_IMAGE_TRANSPORT__THEORA_PUBLISHER_HPP_
