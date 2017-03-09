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

/**
* @brief PageRank computation: Transition function
*/
DECLARE_UDF(graph, compute_pagerank_transition)

/**
* @brief PageRank computation: Merge function
*/
DECLARE_UDF(graph, compute_pagerank_merge_states)

/**
* @brief PageRank computation: Final function
*/
DECLARE_UDF(graph, compute_pagerank_final)

/**
* @brief PageRank convergence test
*/
DECLARE_UDF(graph, test_pagerank_convergence)