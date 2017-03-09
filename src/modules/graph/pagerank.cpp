/* ----------------------------------------------------------------------- *//**
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *
 * @file graph.sql_in
 *
 * @brief SQL functions for graph analytics
 * @date Nov 2016
 *
 * @sa Provides various graph algorithms.
 *
 *//* ----------------------------------------------------------------------- */

/* ------------------------------------------------------
 *
 * @file pagerank.cpp
 *
 * @brief PageRank computation functions
 *
 *//* ----------------------------------------------------------------------- */
#include <dbconnector/dbconnector.hpp>
#include <modules/shared/HandleTraits.hpp>
#include "pagerank.hpp"

namespace madlib {
namespace modules {
namespace graph {

using madlib::dbconnector::postgres::madlib_get_typlenbyvalalign;
using madlib::dbconnector::postgres::madlib_construct_array;
using madlib::dbconnector::postgres::madlib_construct_md_array;

typedef struct __type_info{
    Oid oid;
    int16_t len;
    bool    byval;
    char    align;

    __type_info(Oid oid):oid(oid)
    {
        madlib_get_typlenbyvalalign(oid, &len, &byval, &align);
    }
} type_info;
static type_info FLOAT8TI(FLOAT8OID);
static type_info INT4TI(INT4OID);

AnyType test_pagerank_convergence::run(AnyType& args) {
    MutableArrayHandle<double> state1 = args[0].getAs<MutableArrayHandle<double> >();
    MutableArrayHandle<double> state2 = args[1].getAs<MutableArrayHandle<double> >();
    double threshold = args[2].getAs<double> ();
    if (state1.size() == 0) {
        // The state is empty for the first iteration. Ignore this
        return false;
    }
    if (state1.size() != state2.size()) {
        throw std::invalid_argument(
            "Previous and current states dimensions differ, cannot compute convergence.");
    }
    for (int32_t i=0; i<state1.size(); i++) {
        if(std::abs(state1[i]-state2[i]) > threshold)
            return false;
    }
    return true;
}

AnyType compute_pagerank_final::run(AnyType& args) {
    return args[0].getAs<MutableArrayHandle<double> >();
}

AnyType compute_pagerank_merge_states::run(AnyType& args) {
     MutableArrayHandle<double> state1 = args[0].getAs<MutableArrayHandle<double> >();
     MutableArrayHandle<double> state2 = args[1].getAs<MutableArrayHandle<double> >();
     if (state1.size() == 0) {
        // The state is empty for the first iteration. Ignore this
        return state2;
    }
     if (state1.size() != state2.size()) {
        throw std::invalid_argument(
            "Problem while merging states, the dimensions don't match!");
     }
     for (int32_t i=0; i<state1.size(); i++) {
        state1[i] += state2[i];
     }
     return state1;
}

AnyType compute_pagerank_transition::run(AnyType &args) {
    int32_t nvertices = args[6].getAs<int32_t> ();
    // previous_state is set to NULL for the very first iteration. Initialize PageRank values to 1/n
    MutableArrayHandle<double> previous_state = args[4].getAs<MutableArrayHandle<double> >();
    if (previous_state.isNull() || previous_state.size() == 0) {
        previous_state = madlib_construct_array(
            NULL, nvertices, FLOAT8TI.oid, FLOAT8TI.len, FLOAT8TI.byval,
            FLOAT8TI.align);
        double init_val = 1.0/nvertices;
        for(int32_t i=0; i<nvertices; i++) {
            previous_state[i] = init_val;
        }
    }

    // args[0] will be null for the very first row processed in this segment.
    MutableArrayHandle<double> state = args[0].getAs<MutableArrayHandle<double> >();
    if (state.isNull() || state.size() == 0) {
        // note that this also initializes all values to 0.0
        state =  madlib_construct_array(
            NULL, nvertices, FLOAT8TI.oid, FLOAT8TI.len, FLOAT8TI.byval,
            FLOAT8TI.align);
    }
    ArrayHandle<int32_t> inedges = args[2].getAs<ArrayHandle<int32_t> >();
    ArrayHandle<int32_t> outdegrees = args[3].getAs<ArrayHandle<int32_t> >();
    if (inedges.size() != outdegrees.size()) {
        throw std::invalid_argument(
            "The number of incoming edges and the number of out-degrees do not match.");
    }
    double damping_factor = args[5].getAs<double>();
    int32_t vertex = args[1].getAs<int32_t> ();

    double random_prob = (1.0-damping_factor)/(nvertices*1.0);
    if (inedges.size() == 0) {
        // Vertices that have no incoming edges.
        state[vertex] = random_prob;
    } else {
        for (int32_t i=0; i<inedges.size(); i++) {
            state[vertex] += (previous_state[inedges[i]])/(1.0*outdegrees[i]);
        }
        state[vertex] = state[vertex]*damping_factor + random_prob;
    }
    return state;
}

} // namespace graph
} // namespace modules
} // namespace madlib
