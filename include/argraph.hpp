/**
 * @file   argraph.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date   December, 2014
 * @brief  Definition of a class representing an Attributed Relational Graph (ARG).
 */

#ifndef ARGRAPH_H
#define ARGRAPH_H

#include <assert.h>
#include <stddef.h>
#include <vector>
#include "error.hpp"

using namespace std;

/*--------------------------------------------------------
 * General definitions
 -------------------------------------------------------*/
#ifdef NEED_BOOL
#ifndef BOOL_DEFINED
#define BOOL_DEFINED
typedef int bool;
const false=0;
const true=1
#endif
#endif

#ifndef BYTE_DEFINED
#define BYTE_DEFINED
typedef unsigned char byte;
#endif

typedef unsigned short node_id; /**<Type for the id of the nodes in the graph */
const node_id NULL_NODE=0xFFFF;	/**<Null node value */

/**
 * @class Empty
 * @brief Class for the default empty attribute. 
 * This class should be used for void attributes of nodes or edges.
 */
class Empty{
public:
    friend std::istream& operator>>(std::istream& is, Empty& e)
    {
        return is;
    }
    
    bool operator== (Empty &e2){
        return true;
    }
};

/**
 * @class EqualityComparator
 * @brief Default Functors for Node/Edge Attributes Equality Test.
 */
template<typename T1, typename T2>
class EqualityComparator
{
public:
    bool operator()(T1& a, T2& b)
    {
        return a == b;
    }
};

 /**
 * @class ARGLoader
 * @brief Abstract class ARGLoader. Allows to construct an ARGraph.
 * @note  The abstract class, ARGLoader, is defined to allow the
 * 	use of different file formats for loading the graphs.\n
 * 	The loader is queried by the ARGraph to acquire the information
 * 	needed for building the graph.\n
 * 	A simple ARGLoader based on iostream.h is provided in argloader.hpp;
 * @see argloader.hpp
 */
template<typename Node, typename Edge>
class ARGLoader
{ public:
    virtual ~ARGLoader() {}
    
    /**
    * @brief Counts the number of loaded nodes
    * @returns Number of loaded nodes.
    */
    virtual int NodeCount() = 0;
    /**
    * @brief Returns the attribute of a given node.
    * @param [in] node Node id.
    * @returns Attribute of the node.
    */
    virtual Node GetNodeAttr(node_id node) = 0;
    /**
    * @brief Counts the number of edges out going from a given node.
    * @param [in] node Node id.
    * @returns Number of out going edges.
    */
    virtual int OutEdgeCount(node_id node) = 0;
    /**
    * @brief Returns the End node of the i-th edge out going from a given node.
    * @param [in] node Node id.
    * @param [in] i Index of the edge.
    * @param [out] pattr Attribute of the edge.
    * @returns End node id.
    */
    virtual node_id GetOutEdge(node_id node, int i, Edge *pattr)=0;
};

 /**
 * @class ARGraph
 * @brief This is the real representation of an ARG.
 * @note The graphs, once created, are immutable, in the sense that 
 * graph-editing operations are not provided.\n
 * This allows the use of an internal representation particularly
 * suited for efficient graph matching, which is the primary target
 * of this program.
 *
 * Edges and edge attributes (pointers) are stored into sorted
 * vectors associated to each node, and are looked for
 * using binary search.
 *
 * Nodes are identified using the type node_id, which is currently
 * unsigned short; the special value NULL_NODE is used as null
 * value for this type.
 *
 * Bound checks are performed using the assert macro. They can be
 * disabled by ensuring the macro NDEBUG is defined during
 * compilation.
 *
 * Differently from the previous versions  of this library
 * (before version 2.0), there is no more an adjacency matrix to
 * check for the existence of a node.
 * @see argloader.hpp
 * @see argedit.hpp
 */
 template <typename Node, typename Edge>
class ARGraph
{ public:
    
    typedef void *param_type;			/**<Type for the generic parameter of the visitor */
    typedef void (*edge_visitor)(ARGraph *g,
    node_id n1, node_id n2, Edge *attr,
    param_type param);					/**<Type for the visitor of edges in the graph */
    
private:
    typedef vector<node_id> NodeVec;
    typedef vector<Edge> EdgeAttrVector;
    typedef vector<Node> NodeAttrVector;
    
    int n;                               /**<number of nodes  */
    int max_deg_in;                      /**<max in degree over all the nodes*/
    int max_deg_out;                     /**<max out degree over all the nodes */
    int max_degree;                      /**<max degree over all the nodes */
    NodeAttrVector attr;                 /**<node attributes  */
    vector<EdgeAttrVector> in_attr;      /**<Edge attributes for 'in' edges */
    vector<EdgeAttrVector> out_attr;     /**<Edge attributes for 'out' edges */
    vector<NodeVec> in;                  /**<nodes connected by 'in' edges to each n*/
    vector<NodeVec> out;                 /**<nodes connected by 'out' edges to each node */
    
    bool GetNodeIndex(node_id n1, node_id n2, node_id &index) const;
    
public:
    ARGraph(ARGLoader<Node, Edge> *loader);
    
    int NodeCount() const;
    
    Node& GetNodeAttr(node_id i);
    void SetNodeAttr(node_id i, Node &attr);
    
    bool HasEdge(node_id n1, node_id n2) const;
    bool HasEdge(node_id n1, node_id n2, Edge &pattr) const;
    Edge& GetEdgeAttr(node_id n1, node_id n2);
    void SetEdgeAttr(node_id n1, node_id n2, Edge &attr);
    
    int InEdgeCount(node_id node) const;
    int OutEdgeCount(node_id node) const;
    int EdgeCount(node_id node) const;
    node_id GetInEdge(node_id node, int i) const ;
    node_id GetInEdge(node_id node, int i, Edge& pattr) const;
    node_id GetOutEdge(node_id node, int i) const;
    node_id GetOutEdge(node_id node, int i, Edge& pattr) const;
    node_id* GetOutEdgeSet(node_id node);
    node_id* GetInEdgeSet(node_id node);
  
    /**
	* @brief Maximum incoming degree in the graph
	* @returns Maximum in degree
	*/
    int InMaxDegree() const { return max_deg_in; }
    /**
	* @brief Maximum out going degree in the graph
	* @returns Maximum out degree
	*/
    int OutMaxDegree() const { return max_deg_out; }
    /**
	* @brief Maximum degree in the graph
	* @returns Maximum degree
	*/
    int MaxDegree() const { return max_degree; }
    
    
    void VisitInEdges(node_id node, edge_visitor vis, param_type param);
    void VisitOutEdges(node_id node, edge_visitor vis, param_type param);
    void VisitEdges(node_id node, edge_visitor vis, param_type param);
};

/**
* @brief Returns the number of nodes in the graph
* @returns Number of nodes
*/
template <typename Node, typename Edge>
inline int ARGraph<Node, Edge>::NodeCount() const
{
    return n;
}

/**
* @brief Return the attribute of a given node.
* @param [in] id Node id.
* @returns Attribute of the node.
*/
template <typename Node, typename Edge>
inline Node& ARGraph<Node, Edge>::GetNodeAttr(node_id id)
{
    assert(id<n);
    return attr[id];
}

/**
* @brief Check the presence of an edge.
* @param [in] n1 Start node id.
* @param [in] n2 End node id.
* @retval TRUE If the edge exists.
* @retval FALSE If the edge doesn't exist.
*/
template <typename Node, typename Edge>
inline bool ARGraph<Node, Edge>::HasEdge(node_id n1, node_id n2) const
{
    node_id index;
    return GetNodeIndex(n1,n2,index);
}

/**
* @brief Gets the index of node n2 in the neighboors set of n1.
* @param [in] n1 First node id.
* @param [in] n2 Second node id.
* @param [out] index Index of the node n2 in the neighboors set of n1.
* @retval TRUE If the the nodes n2 is a neighboor of n1.
* @retval FALSE If the the nodes n2 isn't a neighboor of n1.
*/
template <typename Node, typename Edge>
inline bool ARGraph<Node, Edge>::GetNodeIndex(node_id n1, node_id n2, node_id &index) const
{
    unsigned long a, b, c;
    //NodeVec id = out[n1];
    
    assert(n1<n);
    assert(n2<n);
    
    a=0;
    b=out[n1].size();
    while (a<b)
    { c=(unsigned)(a+b)>>1;
        if (out[n1][c]<n2)
            a=c+1;
        else if (out[n1][c]>n2)
            b=c;
        else
        {
            index = c;
            return true;
        }
    }
    
    return false;
}

/**
* @brief Gets the attribute of an edge. 
* @note The method assumes that the edge exists.
* @param [in] n1 Start node id.
* @param [in] n2 End node id.
* @returns the attribute of the edge.
*/
template <typename Node, typename Edge>
inline Edge& ARGraph<Node, Edge>::GetEdgeAttr(node_id n1, node_id n2) 
{
    node_id index;
    assert(GetNodeIndex(n1,n2,index) == true);
    return out_attr[n1][index];
}

/**
* @brief Returns the number of edges going into a node. 
* @param [in] node Node id.
* @returns Number of edges going into a node.
*/
template <typename Node, typename Edge>
inline int ARGraph<Node, Edge>::InEdgeCount(node_id node) const
{
    assert(node<n);
    return (int)in[node].size();
}


/**
* @brief Returns the number of edges departing from a node. 
* @param [in] node Node id.
* @returns Number of edges departing from a node.
*/
template <typename Node, typename Edge>
inline int ARGraph<Node, Edge>::OutEdgeCount(node_id node) const
{ assert(node<n);
    return (int)out[node].size();
}

/**
* @brief Returns the full valence (in and out) of a node. 
* @param [in] node Node id.
* @returns Number of edges of a node.
*/
template <typename Node, typename Edge>
inline int ARGraph<Node, Edge>::EdgeCount(node_id node) const
{ assert(node<n);
    return in[node].size()+out[node].size();
}

/**
* @brief Gets the other end of an edge entering a node. 
* @param [in] node Node id.
* @param [in] i Index of the neighboor of the node.
* @returns Other end of the edge.
*/
template <typename Node, typename Edge>
inline node_id ARGraph<Node, Edge>::GetInEdge(node_id node, int i) const
{ assert(node<n);
    assert(i<in[node].size());
    if(node == 2 && i == 0)
      return in[node][i];
    return in[node][i];
}

/**
* @brief Gets the other end of an edge entering a node and its attribute. 
* @param [in] node Node id.
* @param [in] i Index of the neighboor of the node.
* @param [out] pattr Attribute of the edge.
* @returns Other end of the edge.
*/
template <typename Node, typename Edge>
inline node_id ARGraph<Node, Edge>::GetInEdge(node_id node, int i,
                                              Edge& pattr) const
{   assert(node<n);
    assert(i<in[node].size());
    pattr = in_attr[node][i];
    return in[node][i];
}

/**
* @brief Gets the other end of an edge leaving a node. 
* @param [in] node Node id.
* @param [in] i Index of the neighboor of the node.
* @returns Other end of the edge.
*/
template <typename Node, typename Edge>
inline node_id ARGraph<Node, Edge>::GetOutEdge(node_id node, int i) const
{ assert(node<n);
    assert(i<out[node].size());
    return out[node][i];
}

/**
* @brief Gets the other end of an edge leaving a node and its attribute. 
* @param [in] node Node id.
* @param [in] i Index of the neighboor of the node.
* @param [out] pattr Attribute of the edge.
* @returns Other end of the edge.
*/
template <typename Node, typename Edge>
inline node_id ARGraph<Node, Edge>::GetOutEdge(node_id node, int i,
                                               Edge& pattr) const
{ assert(node<n);
    assert(i<out[node].size());
    pattr = out_attr[node][i];
    return out[node][i];
}

/**
* @brief Change the attribute of a node. 
* @param [in] i Node id.
* @param [int] new_attr Attribute of the node.
*/
template <typename Node, typename Edge>
void ARGraph<Node, Edge>::SetNodeAttr(node_id i, Node &new_attr)
{
    assert(i<n);
    attr[i]=new_attr;
}

/**
 * @brief Gets the set of out edges
 * @param [in] node Node id.
 * @returns Set of out edges
 */
template <typename Node, typename Edge>
inline node_id* ARGraph<Node, Edge>::GetOutEdgeSet(node_id node)
{
  return out[node].data();
}

/**
 * @brief Gets the set of in edges
 * @param [in] node Node id.
 * @returns Set of in edges
 */
template <typename Node, typename Edge>
inline node_id* ARGraph<Node, Edge>::GetInEdgeSet(node_id node)
{
  return in[node].data();
}

/*-------------------------------------------------------------------
 * Checks the existence of an edge, and returns its attribute
 * using the parameter pattr.
 * Note: uses binary search.
 * Implem. note: Uses the out/out_attr vectors; this fact is
 *               exploited in the constructor to generate the
 *               in_attr vector
 ------------------------------------------------------------------*/
/**
* @brief Checks the existence of an edge, and returns its attribute. 
* @note The method uses binary search. \n
*	Uses the out/out_attr vectors; this fact is exploited in the 
*	constructor to generate the in_attr vector.
* @param [in] n1 Start node id.
* @param [in] n2 End node id.
* @param [out] pattr Attribute of the edge.
* @retval TRUE If the edge exists.
* @retval FALSE If the edge doesn't exist.
*/ 
template <typename Node, typename Edge>
bool ARGraph<Node, Edge>::HasEdge(node_id n1, node_id n2, Edge &pattr) const
{
    node_id index;
    if(GetNodeIndex(n1,n2,index)){
        pattr=out_attr[n1][index];
        return true;
    }
    return false;
}

/**
* @brief Change the attribute of an edge. It is an error if the edge does not exist. 
* @note The method uses binary search. 
* @param [in] n1 Start node id.
* @param [in] n2 End node id.
* @param [in] pattr Attribute of the edge.
*/
template <typename Node, typename Edge>
void  ARGraph<Node, Edge>::SetEdgeAttr(node_id n1, node_id n2, Edge& new_attr)
{
    int c;
    assert(n1<n);
    assert(n2<n);
    
    if(GetNodeIndex(n1,n2,c))
        out_attr[n1][c]=new_attr;
    
    if(GetNodeIndex(n2,n1,c))
        in_attr[n2][c]=new_attr;
}

/**
* @brief Applies the visitor to all the in edges of the node.
* @param [in] node Node id.
* @param [in] vis Edge Visitor.
* @param [in] param Generic param for the visitor.
*/
template <typename Node, typename Edge>
void ARGraph<Node, Edge>::VisitInEdges(node_id node, edge_visitor vis,
                                       param_type param)
{
    assert(node<n);
    int i;
    for(i=0; i<in[node].size(); i++)
        vis(this, in[node][i], node, in_attr[node][i], param);
}

/**
* @brief Applies the visitor to all the out edges of the node.
* @param [in] node Node id.
* @param [in] vis Edge Visitor.
* @param [in] param Generic param for the visitor.
*/
template <typename Node, typename Edge>
void ARGraph<Node, Edge>::VisitOutEdges(node_id node, edge_visitor vis,
                                        param_type param)
{
    assert(node<n);
    int i;
    for(i=0; i<out[node].size(); i++)
        vis(this, node, out[node][i], out_attr[node][i], param);
}

/**
* @brief Applies the visitor to all the edges of the node.
* @param [in] node Node id.
* @param [in] vis Edge Visitor.
* @param [in] param Generic param for the visitor.
*/
template <typename Node, typename Edge>
void ARGraph<Node, Edge>::VisitEdges(node_id node, edge_visitor vis, param_type param)
{
    VisitInEdges(node, vis, param);
    VisitOutEdges(node, vis, param);
}

/**
* @brief Constructs the graph form a loader.
* @param loader ARGLoader 
*/
template <typename Node, typename Edge>
ARGraph<Node, Edge>::ARGraph(ARGLoader<Node, Edge> *loader)
{
    n = loader->NodeCount();
    vector<int> in_node_count(n,0);
    max_deg_in = max_deg_out = max_degree = 0;
    
    int i, j;
    for(i=0; i<n; i++)
        attr.push_back(loader->GetNodeAttr(i));
    
    for(i=0; i<n; i++)
    {
        int k=loader->OutEdgeCount(i);
        
        if (k > max_deg_out)
            max_deg_out = k;
        
        NodeVec nv_out;
        EdgeAttrVector eav_out(k);
        out.push_back(nv_out);
        out_attr.push_back(eav_out);
        
        for(j=0; j<k; j++)
        {
            node_id n2 = loader->GetOutEdge(i, j, &out_attr[i][j]);
            out[i].push_back(n2);
            in_node_count[n2]++;
        }
    }
    
    for(i=0; i<n; i++)
    {
        int k = in_node_count[i];
        
        if (k > max_deg_in)
            max_deg_in = k;
        
        NodeVec nv;
        EdgeAttrVector eav;
        in.push_back(nv);
        in_attr.push_back(eav);
        
        int l=0;
        for(j=0; j<n; j++)
        {
            if (HasEdge(j,i))
            {
                in[i].push_back(j);
                in_attr[i].push_back(GetEdgeAttr(j, i));
                l++;
            }
        }
        
        assert(l==k);
    }
    
    for(i = 0; i < n; i++){
        int count = in_node_count[i] + loader->OutEdgeCount(i);
        if(count > max_degree){
            max_degree = count;
        }
    }
    
}

#endif
/* defined ARGRAPH_H */


