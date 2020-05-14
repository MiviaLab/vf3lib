/**
 * @file EdgeStreamARGLoader.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @date May 2014
 * @brief A fast implementation of the ARGLoader interface for
 *        reading from a text istream in which the graph is represented
 *        as a list of edges;
 *        no attributes are supported, so the loader always return
 *        the default value for the edge and node attribute type.
 */

#ifndef EDGESTREAMARGLOADER_HPP
#define EDGESTREAMARGLOADER_HPP

#include <iostream>
#include <vector>
#include <map>

#include <Error.hpp>

#include "ARGraph.hpp"

namespace vflib {

template <typename Node, typename Edge>
class EdgeStreamARGLoader: public ARGLoader<Node, Edge> {
    public:
        /**
         * @brief Reads the graph data from a text istream
         * @param in The input stream
         * @param undirected If true, the graph is undirected
         * @param remove_isolated_nodes If true, the nodes that
         *        have no edges are removed from the graph
         */
        EdgeStreamARGLoader(std::istream &in, bool undirected=false,
                       bool remove_isolated_nodes=true);
        virtual uint32_t NodeCount() const;
        virtual Node GetNodeAttr(nodeID_t node);
        virtual uint32_t OutEdgeCount(nodeID_t node) const;
        virtual nodeID_t GetOutEdge(nodeID_t node, uint32_t i, Edge *pattr);
        void SetNodeAttribute(Node attr) { node_attribute=attr; }
        void SetEdgeAttribute(Edge attr) { edge_attribute=attr; }

    private:
        Node node_attribute;
        Edge edge_attribute;
        uint32_t node_count;
        std::vector<nodeID_t> forward, backward;
        std::vector< std::map<nodeID_t, Empty> > edges;
        typename std::map<nodeID_t, Empty>::iterator edge_iterator;
        nodeID_t last_edge_node;
        uint32_t last_edge_index;
        void skipHeading(std::istream &in);
};



template <typename Node, typename Edge>
EdgeStreamARGLoader<Node,Edge>
::EdgeStreamARGLoader(std::istream &in, bool undirected,
                      bool remove_isolated_nodes) {
    last_edge_node = NULL_NODE;
    uint32_t count=0;

    if (!in.good())
      error("End of file or reading error");

    skipHeading(in);
    nodeID_t n1, n2;
    while (true) {
        in >> n1 >> n2;
        n1--;
        n2--;
        if (!in.good())
            break;
        if (n1 == n2)
            error("File format error. Self edges are not allowed!", n1, n2);
        if (n1>=count)
            count = n1+1;
        if (n2>=count)
            count = n2+1;
        edges.resize(count);
        forward.resize(count, 0);
        forward[n1]=1;
        forward[n2]=1;
        edges[n1][n2] = edge_attribute;
        if (undirected)
            edges[n2][n1] = edge_attribute;
    }

    if (remove_isolated_nodes) {
        backward.reserve(count);
        node_count=0;
        for(uint32_t i=0; i<count; i++) {
            if (forward[i]>0) {
                forward[i]=node_count ++;
                backward.push_back(i);
            }
        }
    } else {
        node_count=count;
        backward.resize(count);
        for(uint32_t i=0; i<count; i++) {
            forward[i]=i;
            backward[i]=i;
        }
    }
}

template <typename Node, typename Edge>
uint32_t
EdgeStreamARGLoader<Node,Edge>
::NodeCount() const {
    return node_count;
}

template <typename Node, typename Edge>
Node
EdgeStreamARGLoader<Node,Edge>
::GetNodeAttr(nodeID_t node) {
    return node_attribute;
}

template <typename Node, typename Edge>
uint32_t
EdgeStreamARGLoader<Node,Edge>
::OutEdgeCount(nodeID_t node) const {
    return edges[backward[node]].size();
}

template <typename Node, typename Edge>
nodeID_t
EdgeStreamARGLoader<Node,Edge>
::GetOutEdge(nodeID_t node, uint32_t i, Edge *pattr) {
    assert (i<OutEdgeCount(node));

    if (node != last_edge_node) {
        last_edge_node = node;
        edge_iterator = edges[backward[node]].begin();
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

    *pattr = edge_attribute;
    return forward[edge_iterator->first];
}


template <typename Node, typename Edge>
void
EdgeStreamARGLoader<Node,Edge>
::skipHeading(std::istream & in) {
    const int maxline=256*1024;
    int c=in.peek();
    while (c!=EOF && (isspace(c) || c=='#')) {
        in.get();
        if (c=='#')
            in.ignore(maxline, '\n');
        c=in.peek();
    }
}

} // End namespace vflib

#endif
