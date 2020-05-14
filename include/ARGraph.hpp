/**
 * @file   ARGraph.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date   December, 2014
 * @brief  Definition of a class representing an Attributed Relational Graph (ARG).
 */

#ifndef ARGRAPH_H
#define ARGRAPH_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <limits>
#include <vector>

#include <Error.hpp>

namespace vflib
{

	typedef uint32_t nodeID_t; /**<Type for the id of the nodes in the graph */
	const nodeID_t NULL_NODE = (std::numeric_limits<uint32_t>::max)();	/**<Null node value */

	/**
	 * @class Empty
	 * @brief Class for the default empty attribute.
	 * This class should be used for void attributes of nodes or edges.
	 */
	class Empty {
		public:
			friend std::istream& operator>>(std::istream& is, Empty& e)
			{
				return is;
			}

			bool operator== (Empty &e2) {
				return true;
			}

			friend inline bool operator< (const Empty & s1, const Empty & s2)
			{
				return  false;
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
	{
	public:
		virtual ~ARGLoader() {}

		/**
		* @brief Counts the number of loaded nodes
		* @returns Number of loaded nodes.
		*/
		virtual uint32_t NodeCount() const = 0;
		/**
		* @brief Returns the attribute of a given node.
		* @param [in] node Node id.
		* @returns Attribute of the node.
		*/
		virtual Node GetNodeAttr(nodeID_t node) = 0;
		/**
		* @brief Counts the number of edges out going from a given node.
		* @param [in] node Node id.
		* @returns Number of out going edges.
		*/
		virtual uint32_t OutEdgeCount(nodeID_t node) const = 0;
		/**
		* @brief Returns the End node of the i-th edge out going from a given node.
		* @param [in] node Node id.
		* @param [in] i Index of the edge.
		* @param [out] pattr Attribute of the edge.
		* @returns End node id.
		*/
		virtual nodeID_t GetOutEdge(nodeID_t node, uint32_t i, Edge *pattr) = 0;
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
	{
	public:

		typedef void *param_type;			/**<Type for the generic parameter of the visitor */
		typedef void(*edge_visitor)(ARGraph *g,
			nodeID_t n1, nodeID_t n2, Edge *attr,
			param_type param);					/**<Type for the visitor of edges in the graph */

	private:
		typedef std::vector<nodeID_t> NodeVec;
		typedef std::vector<Edge> EdgeAttrVector;
		typedef std::vector<Node> NodeAttrVector;

		uint32_t n;                               /**<number of nodes  */
		uint32_t n_attr_count;					  /**<number of different node attributes */
		uint32_t e_attr_count;					  /**<number of different edge attributes */
		uint32_t e_count;						  /**<number of edges */
		uint32_t e_out_count;					  /**<total number of out edges */
		uint32_t e_in_count;					  /**<total number of in edges */
		uint32_t node_label_count;				  /**<number of nodes labels */
		uint32_t max_deg_in;                      /**<max in degree over all the nodes*/
		uint32_t max_deg_out;                     /**<max out degree over all the nodes */
		uint32_t max_degree;                      /**<max degree over all the nodes */
		NodeAttrVector attr;                 /**<node attributes  */
		std::vector<EdgeAttrVector> in_attr;      /**<Edge attributes for 'in' edges */
		std::vector<EdgeAttrVector> out_attr;     /**<Edge attributes for 'out' edges */
		std::vector<NodeVec> in;                  /**<nodes connected by 'in' edges to each n*/
		std::vector<NodeVec> out;                 /**<nodes connected by 'out' edges to each node */

		bool GetNodeIndex(nodeID_t n1, nodeID_t n2, nodeID_t &index) const;

	public:
		ARGraph(ARGLoader<Node, Edge> *loader);

		uint32_t NodeCount() const;
		uint32_t EdgeCount() const;
		uint32_t InEdgeCount() const;
		uint32_t OutEdgeCount() const;

		Node& GetNodeAttr(nodeID_t i);
		void SetNodeAttr(nodeID_t i, Node &attr);

		bool HasEdge(nodeID_t n1, nodeID_t n2) const;
		bool HasEdge(nodeID_t n1, nodeID_t n2, Edge &pattr) const;
		Edge& GetEdgeAttr(nodeID_t n1, nodeID_t n2);
		void SetEdgeAttr(nodeID_t n1, nodeID_t n2, Edge &attr);

		uint32_t InEdgeCount(nodeID_t node) const;
		uint32_t OutEdgeCount(nodeID_t node) const;
		uint32_t EdgeCount(nodeID_t node) const;
		nodeID_t GetInEdge(nodeID_t node, uint32_t i) const;
		nodeID_t GetInEdge(nodeID_t node, uint32_t i, Edge& pattr) const;
		nodeID_t GetOutEdge(nodeID_t node, uint32_t i) const;
		nodeID_t GetOutEdge(nodeID_t node, uint32_t i, Edge& pattr) const;
		nodeID_t* GetOutEdgeSet(nodeID_t node);
		nodeID_t* GetInEdgeSet(nodeID_t node);

		/**
		* @brief Maximum incoming degree in the graph
		* @returns Maximum in degree
		*/
		uint32_t InMaxDegree() const { return max_deg_in; }
		/**
		* @brief Maximum out going degree in the graph
		* @returns Maximum out degree
		*/
		uint32_t OutMaxDegree() const { return max_deg_out; }
		/**
		* @brief Maximum degree in the graph
		* @returns Maximum degree
		*/
		uint32_t MaxDegree() const { return max_degree; }

		/**
		* @brief Number of different node attibute in the graph
		* @returns Number of different node attibute
		*/
		uint32_t NodeAttrCount() const { return n_attr_count; }

		/**
		* @brief Number of different edge attibute in the graph
		* @returns Number of different edge attibute
		*/
		uint32_t EdgeAttrCount() const { return e_attr_count; }

		void VisitInEdges(nodeID_t node, edge_visitor vis, param_type param);
		void VisitOutEdges(nodeID_t node, edge_visitor vis, param_type param);
		void VisitEdges(nodeID_t node, edge_visitor vis, param_type param);
	};

	/**
	* @brief Returns the number of nodes in the graph
	* @returns Number of nodes
	*/
	template <typename Node, typename Edge>
	inline uint32_t ARGraph<Node, Edge>::NodeCount() const
	{
		return n;
	}

	/**
	* @brief Returns the number of edge in the graph
	* @returns Number of edge
	*/
	template <typename Node, typename Edge>
	inline uint32_t ARGraph<Node, Edge>::EdgeCount() const
	{
		return e_count;
	}

		/**
	* @brief Returns the number of in edges in the graph
	* @returns Number of in edges
	*/
	template <typename Node, typename Edge>
	inline uint32_t ARGraph<Node, Edge>::InEdgeCount() const
	{
		return e_in_count;
	}

		/**
	* @brief Returns the number of out edges in the graph
	* @returns Number of out edges
	*/
	template <typename Node, typename Edge>
	inline uint32_t ARGraph<Node, Edge>::OutEdgeCount() const
	{
		return e_out_count;
	}

	/**
	* @brief Return the attribute of a given node.
	* @param [in] id Node id.
	* @returns Attribute of the node.
	*/
	template <typename Node, typename Edge>
	inline Node& ARGraph<Node, Edge>::GetNodeAttr(nodeID_t id)
	{
		assert(id < n);
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
	inline bool ARGraph<Node, Edge>::HasEdge(nodeID_t n1, nodeID_t n2) const
	{
		nodeID_t index;
		return GetNodeIndex(n1, n2, index);
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
	inline bool ARGraph<Node, Edge>::GetNodeIndex(nodeID_t n1, nodeID_t n2, nodeID_t &index) const
	{
		unsigned long a, b, c;
		//NodeVec id = out[n1];

		assert(n1 < n);
		assert(n2 < n);

		a = 0;
		b = out[n1].size();
		while (a < b)
		{
			c = (unsigned)(a + b) >> 1;
			if (out[n1][c] < n2)
				a = c + 1;
			else if (out[n1][c] > n2)
				b = c;
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
	inline Edge& ARGraph<Node, Edge>::GetEdgeAttr(nodeID_t n1, nodeID_t n2)
	{
		nodeID_t index;
		assert(GetNodeIndex(n1, n2, index) == true);
		return out_attr[n1][index];
	}

	/**
	* @brief Returns the number of edges going into a node.
	* @param [in] node Node id.
	* @returns Number of edges going into a node.
	*/
	template <typename Node, typename Edge>
	inline uint32_t ARGraph<Node, Edge>::InEdgeCount(nodeID_t node) const
	{
		assert(node < n);
		return (int)in[node].size();
	}


	/**
	* @brief Returns the number of edges departing from a node.
	* @param [in] node Node id.
	* @returns Number of edges departing from a node.
	*/
	template <typename Node, typename Edge>
	inline uint32_t ARGraph<Node, Edge>::OutEdgeCount(nodeID_t node) const
	{
		assert(node < n);
		return (int)out[node].size();
	}

	/**
	* @brief Returns the full valence (in and out) of a node.
	* @param [in] node Node id.
	* @returns Number of edges of a node.
	*/
	template <typename Node, typename Edge>
	inline uint32_t ARGraph<Node, Edge>::EdgeCount(nodeID_t node) const
	{
		assert(node < n);
		return in[node].size() + out[node].size();
	}

	/**
	* @brief Gets the other end of an edge entering a node.
	* @param [in] node Node id.
	* @param [in] i Index of the neighboor of the node.
	* @returns Other end of the edge.
	*/
	template <typename Node, typename Edge>
	inline nodeID_t ARGraph<Node, Edge>::GetInEdge(nodeID_t node, uint32_t i) const
	{
		assert(node < n);
		assert(i < in[node].size());
		if (node == 2 && i == 0)
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
	inline nodeID_t ARGraph<Node, Edge>::GetInEdge(nodeID_t node, uint32_t i,
		Edge& pattr) const
	{
		assert(node < n);
		assert(i < in[node].size());
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
	inline nodeID_t ARGraph<Node, Edge>::GetOutEdge(nodeID_t node, uint32_t i) const
	{
		assert(node < n);
		assert(i < out[node].size());
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
	inline nodeID_t ARGraph<Node, Edge>::GetOutEdge(nodeID_t node, uint32_t i,
		Edge& pattr) const
	{
		assert(node < n);
		assert(i < out[node].size());
		pattr = out_attr[node][i];
		return out[node][i];
	}

	/**
	* @brief Change the attribute of a node.
	* @param [in] i Node id.
	* @param [int] new_attr Attribute of the node.
	*/
	template <typename Node, typename Edge>
	void ARGraph<Node, Edge>::SetNodeAttr(nodeID_t i, Node &new_attr)
	{
		assert(i < n);
		attr[i] = new_attr;
	}

	/**
	 * @brief Gets the set of out edges
	 * @param [in] node Node id.
	 * @returns Set of out edges
	 */
	template <typename Node, typename Edge>
	inline nodeID_t* ARGraph<Node, Edge>::GetOutEdgeSet(nodeID_t node)
	{
		return out[node].data();
	}

	/**
	 * @brief Gets the set of in edges
	 * @param [in] node Node id.
	 * @returns Set of in edges
	 */
	template <typename Node, typename Edge>
	inline nodeID_t* ARGraph<Node, Edge>::GetInEdgeSet(nodeID_t node)
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
	bool ARGraph<Node, Edge>::HasEdge(nodeID_t n1, nodeID_t n2, Edge &pattr) const
	{
		nodeID_t index;
		if (GetNodeIndex(n1, n2, index)) {
			pattr = out_attr[n1][index];
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
	void  ARGraph<Node, Edge>::SetEdgeAttr(nodeID_t n1, nodeID_t n2, Edge& new_attr)
	{
		uint32_t c;
		assert(n1 < n);
		assert(n2 < n);

		if (GetNodeIndex(n1, n2, c))
			out_attr[n1][c] = new_attr;

		if (GetNodeIndex(n2, n1, c))
			in_attr[n2][c] = new_attr;
	}

	/**
	* @brief Applies the visitor to all the in edges of the node.
	* @param [in] node Node id.
	* @param [in] vis Edge Visitor.
	* @param [in] param Generic param for the visitor.
	*/
	template <typename Node, typename Edge>
	void ARGraph<Node, Edge>::VisitInEdges(nodeID_t node, edge_visitor vis,
		param_type param)
	{
		assert(node < n);
		size_t i;
		for (i = 0; i < in[node].size(); i++)
			vis(this, in[node][i], node, in_attr[node][i], param);
	}

	/**
	* @brief Applies the visitor to all the out edges of the node.
	* @param [in] node Node id.
	* @param [in] vis Edge Visitor.
	* @param [in] param Generic param for the visitor.
	*/
	template <typename Node, typename Edge>
	void ARGraph<Node, Edge>::VisitOutEdges(nodeID_t node, edge_visitor vis,
		param_type param)
	{
		assert(node < n);
		size_t i;
		for (i = 0; i < out[node].size(); i++)
			vis(this, node, out[node][i], out_attr[node][i], param);
	}

	/**
	* @brief Applies the visitor to all the edges of the node.
	* @param [in] node Node id.
	* @param [in] vis Edge Visitor.
	* @param [in] param Generic param for the visitor.
	*/
	template <typename Node, typename Edge>
	void ARGraph<Node, Edge>::VisitEdges(nodeID_t node, edge_visitor vis, param_type param)
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
                attr.resize(n);
                in.resize(n);
                in_attr.resize(n);
                out.resize(n);
                out_attr.resize(n);

		e_count = 0;
		e_out_count = 0;
		e_in_count = 0;
		n_attr_count = 0;
		e_attr_count = 0;
		std::map<Node, bool> attributemap;
		std::map<Edge, bool> e_attributemap;

		max_deg_in = max_deg_out = max_degree = 0;

                std::vector< std::map< nodeID_t, Edge> > revmap;
                typename std::map< nodeID_t, Edge>::iterator rvit;
                revmap.resize(n);

		uint32_t i, j;
		for (i = 0; i < n; i++)
		{
			Node attribute = loader->GetNodeAttr(i);
			attr[i]=(attribute);

			if(!attributemap.count(attribute))
			{ 
				attributemap[attribute]=true;
				n_attr_count++;
			}
		}

		for (i = 0; i < n; i++)
		{
			uint32_t k = loader->OutEdgeCount(i);
			e_out_count += k;

			if (k > max_deg_out)
				max_deg_out = k;
                        out[i].resize(k);
                        out_attr[i].resize(k);

			for (j = 0; j < k; j++)
			{
                                nodeID_t n2 = loader->GetOutEdge(i, j,
                                                     &out_attr[i][j]);
				out[i][j]=n2;
                                revmap[n2][i]=out_attr[i][j];
			}
		}

		for (i = 0; i < n; i++)
		{
			uint32_t k = revmap[i].size();
			e_in_count += k;

			if (k > max_deg_in)
				max_deg_in = k;
                        
                        in[i].resize(k);
                        in_attr[i].resize(k);

			for (j = 0, rvit=revmap[i].begin(); j < k; j++, rvit++)
			{ in[i][j] = rvit->first;
                          Edge edge_attr=rvit->second;
                          in_attr[i][j]=edge_attr;
			  if(!e_attributemap.count(edge_attr)) { 
				e_attributemap[edge_attr]=true;
				e_attr_count++;
			  }
			}

		}

		for (i = 0; i < n; i++) {
			uint32_t count = in[i].size() + out[i].size();
			e_count += count;
			if (count > max_degree) {
				max_degree = count;
			}
		}

	}

}
#endif
/* defined ARGRAPH_H */


