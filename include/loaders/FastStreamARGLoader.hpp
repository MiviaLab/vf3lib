/**
 * @file FastStreamARGLoader.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @date May 2014
 * @brief A fast implementation of the ARGLoader interface for
 *        reading from a text istream
 */

#ifndef FASTSTREAMARGLOADER_HPP
#define FASTSTREAMARGLOADER_HPP

#include <iostream>
#include <vector>
#include <map>

#include <Error.hpp>

#include "ARGraph.hpp"

namespace vflib {

template <typename Node, typename Edge>
class FastStreamARGLoader: public ARGLoader<Node, Edge> {
    public:
        /**
         * @brief Reads the graph data from a text istream
         * @param in The input stream
         * @param undirected If true, the graph is undirected
         */
        FastStreamARGLoader(std::istream &in, bool undirected=false);
        virtual uint32_t NodeCount() const;
        virtual Node GetNodeAttr(nodeID_t node);
        virtual uint32_t OutEdgeCount(nodeID_t node) const;
        virtual nodeID_t GetOutEdge(nodeID_t node, uint32_t i, Edge *pattr);

    private:
        uint32_t node_count;
        std::vector<Node> nodes;
        std::vector< std::map<nodeID_t, Edge> > edges;
        typename std::map<nodeID_t, Edge>::iterator edge_iterator;
        nodeID_t last_edge_node;
        uint32_t last_edge_index;
};



template <typename Node, typename Edge>
FastStreamARGLoader<Node,Edge>
::FastStreamARGLoader(std::istream &in, bool undirected) {
    last_edge_node = NULL_NODE;

    if (!in.good())
      error("End of file or reading error");

    in >> node_count;
    nodes.resize(node_count);
    edges.resize(node_count);
    uint32_t edge_count;
    nodeID_t i, j, n1, n2;
    for(i=0; i<node_count; i++) {
        in >> n1 >> nodes[i];
        if (n1 != i)
            error("Error in file format reading node", n1);
    }

    for(i=0; i<node_count; i++) {
        in >> edge_count;
        for(j=0; j<edge_count; j++) {
            in >> n1 >> n2;
            if (n1 != i || n2 >= node_count || n1==n2)
                error("Error in file format reading edge", n1, n2);
            in >> edges[n1][n2];
            if (undirected)
                edges[n2][n1] = edges[n1][n2];
        }
    }
}

template <typename Node, typename Edge>
uint32_t
FastStreamARGLoader<Node,Edge>
::NodeCount() const {
    return node_count;
}

template <typename Node, typename Edge>
Node
FastStreamARGLoader<Node,Edge>
::GetNodeAttr(nodeID_t node) {
    return nodes[node];
}

template <typename Node, typename Edge>
uint32_t
FastStreamARGLoader<Node,Edge>
::OutEdgeCount(nodeID_t node) const {
    return edges[node].size();
}

template <typename Node, typename Edge>
nodeID_t
FastStreamARGLoader<Node,Edge>
::GetOutEdge(nodeID_t node, uint32_t i, Edge *pattr) {
    assert (i<OutEdgeCount(node));

    if (node != last_edge_node) {
        last_edge_node = node;
        edge_iterator = edges[node].begin();
        last_edge_index = 0;
    }
    while (last_edge_index < i) {
        edge_iterator ++;
        last_edge_index ++;
    }

    while (last_edge_index > i) {
        edge_iterator --;
        last_edge_index --;
    }

    *pattr = edge_iterator->second;
    return edge_iterator->first;
}



} // End namespace vflib

#endif
