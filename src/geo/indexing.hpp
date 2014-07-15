// Copyright 2010-2014 RethinkDB, all rights reserved.
#ifndef GEO_INDEXING_HPP_
#define GEO_INDEXING_HPP_

#include <string>
#include <vector>

#include "btree/parallel_traversal.hpp"
#include "btree/types.hpp"
#include "containers/counted.hpp"
#include "geo/s2/s2cellid.h"

namespace ql {
class datum_t;
}


std::vector<std::string> compute_index_grid_keys(
        const counted_t<const ql::datum_t> &key,
        int goal_cells);

// TODO! Support compound indexes. Somehow.
// TODO! Implement aborting the traversal early
class geo_index_traversal_helper_t : public btree_traversal_helper_t {
public:
    geo_index_traversal_helper_t(const std::vector<std::string> &query_grid_keys);

    /* Called for every pair that could potentially intersect with query_grid_keys.
    Note that this might be called multiple times for the same value. */
    virtual void on_candidate(
            const btree_key_t *key,
            const void *value,
            buf_parent_t parent)
            THROWS_ONLY(interrupted_exc_t) = 0;

    /* btree_traversal_helper_t interface */
    void process_a_leaf(
            buf_lock_t *leaf_node_buf,
            const btree_key_t *left_exclusive_or_null,
            const btree_key_t *right_inclusive_or_null,
            signal_t *interruptor,
            int *population_change_out)
            THROWS_ONLY(interrupted_exc_t);
    void postprocess_internal_node(UNUSED buf_lock_t *internal_node_buf) { }
    void filter_interesting_children(
            buf_parent_t parent,
            ranged_block_ids_t *ids_source,
            interesting_children_callback_t *cb);
    access_t btree_superblock_mode() { return access_t::read; }
    access_t btree_node_mode() { return access_t::read; }

private:
    static bool cell_intersects_with_range(
            S2CellId c, const btree_key_t *left_excl, const btree_key_t *right_incl);
    bool any_query_cell_intersects(const btree_key_t *left_excl,
                                   const btree_key_t *right_incl);

    std::vector<S2CellId> query_cells_;
};

#endif  // GEO_INDEXING_HPP_