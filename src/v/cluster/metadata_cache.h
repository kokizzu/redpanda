#pragma once

#include "cluster/types.h"
#include "model/metadata.h"
#include "seastarx.h"

#include <seastar/core/future.hh>

#include <absl/container/flat_hash_map.h>

namespace cluster {

class metadata_cache {
public:
    // struct holding the cache content
    struct partition {
        model::partition_metadata p_md;
        model::term_id term_id{};
    };
    struct topic_metadata {
        std::vector<partition> partitions;
    };
    using broker_cache_t = absl::flat_hash_map<model::node_id, broker_ptr>;
    using cache_t = absl::flat_hash_map<model::topic, topic_metadata>;

    metadata_cache() = default;
    ss::future<> stop() { return ss::make_ready_future<>(); }

    /// Returns list of all topics that exists in the cluster.
    std::vector<model::topic> all_topics() const;

    ///\brief Returns metadata of single topic.
    ///
    /// If topic does not exists it returns an empty optional
    std::optional<model::topic_metadata>
      get_topic_metadata(model::topic_view) const;

    /// Returns metadata of all topics.
    std::vector<model::topic_metadata> all_topics_metadata() const;

    /// Returns all brokers, returns copy as the content of broker can change
    std::vector<broker_ptr> all_brokers() const;

    /// Returns all broker ids
    std::vector<model::node_id> all_broker_ids() const;

    /// Returns single broker if exists in cache,returns copy as the content of
    /// broker can change
    std::optional<broker_ptr> get_broker(model::node_id) const;

    /// Constructs the cache from the content of vector containing available
    /// brokers
    void update_brokers_cache(std::vector<model::broker>&&);

    ///\brief Add empty model::topic_metadata entry to cache
    ///
    /// This api is used when controller is recovering (or is notified)
    /// topic_configuration record type
    void add_topic(model::topic_view);

    ///\brief Removes the topic from cache
    ///
    /// Not yet used by the controller as removing topics is not yet supported
    void remove_topic(model::topic_view);

    ///\brief Updates the assignment of topic partion
    ///
    /// It is used by the controller when processing partition_assignment record
    /// types
    void update_partition_assignment(const partition_assignment&);

    ///\brief Updates leader of topic partition.
    ///
    /// It is not yet used by the controller, it will be used when controller
    /// will process leadership change notifications
    void update_partition_leader(
      model::topic_view,
      model::partition_id,
      model::term_id,
      std::optional<model::node_id>);

    bool contains(const model::topic&, model::partition_id) const;

    /// Returns metadata of all topics in cache internal format
    const cache_t& all_metadata() const { return _cache; }

    /// Directly inserts topic_metadata
    void insert_topic(model::topic_metadata);

private:
    broker_cache_t _brokers_cache;
    cache_t _cache;
    cache_t::iterator find_topic_metadata(model::topic_view);
};

model::topic_metadata
create_topic_metadata(const metadata_cache::cache_t::value_type&);

/// Looks for partition with requested id in topic_metadata type.
metadata_cache::partition*
find_partition(metadata_cache::topic_metadata&, model::partition_id);

} // namespace cluster
