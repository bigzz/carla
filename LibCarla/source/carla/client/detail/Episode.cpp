// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Episode.h"

#include "carla/client/detail/Client.h"

namespace carla {
namespace client {
namespace detail {

  static auto &CastData(const sensor::SensorData &data) {
    using target_t = const sensor::data::RawEpisodeState;
    DEBUG_ASSERT(dynamic_cast<target_t *>(&data) != nullptr);
    return static_cast<target_t &>(data);
  }

  Episode::Episode(Client &client)
    : _client(client),
      _description(client.GetEpisodeInfo()),
      _state(std::make_shared<EpisodeState>()) {}

  Episode::~Episode() {
    _client.UnSubscribeFromStream(_description.token);
  }

  void Episode::Listen() {
    std::weak_ptr<Episode> weak = shared_from_this();
    _client.SubscribeToStream(_description.token, [weak](auto data) {
      auto self = weak.lock();
      if (self != nullptr) {
        /// @todo This is not atomic.
        auto prev = self->_state.load();
        self->_state = prev->DeriveNextStep(CastData(*data));
      }
    });
  }

} // namespace detail
} // namespace client
} // namespace carla