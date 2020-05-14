#ifndef NODE_SORTER_HPP
#define NODE_SORTER_HPP

#include <vector>
#include <algorithm>
#include <ARGraph.hpp>

namespace vflib
{
	/*
	* @class NodeSorter
	* @brief Abstract Node Sorter Interface
	*/
	template<typename Node, typename Edge>
	class NodeSorter
	{
	public:
		virtual std::vector<nodeID_t> SortNodes(ARGraph<Node, Edge>* pattern) = 0;
	};

	/*
	* @class SimpleNodeSorter
	* @brief VF2 Basic Sorter
	* @details Mainains the node insertion order (as they have been inserted in the graph)
	*/
	template<typename Node, typename Edge>
	class SimpleNodeSorter : NodeSorter<Node, Edge>
	{
	public:
		std::vector<nodeID_t> SortNodes(ARGraph<Node, Edge>* pattern)
		{
			uint32_t i;
			std::vector<nodeID_t> sorted;
			for (i = 0; i < pattern->NodeCount(); i++)
				sorted.push_back(i);
			return sorted;
		}
	};

	/*
	* @class SortingNode
	* @bried Class used to sort the nodes of the graph
	*/
	class SortingNode
	{
	protected:
		nodeID_t id;			//< The node has been sorted
		uint32_t deg;
		bool used;
		bool inCandidate;	//< The node has to be ignored as candidate
		

	public:
		SortingNode(nodeID_t id, uint32_t deg = 0):id(id),deg(deg),used(false),inCandidate(false) {}

		inline nodeID_t GetID() const { return id; }

		inline bool IsUsed() const { return used; }
		inline void SetUsed() { used = true; }
		inline bool IsInCandidate() const { return inCandidate; }
		inline void SetInCanidate() { inCandidate = true; }
		inline uint32_t GetDegree() const { return deg; }

		bool operator== (const SortingNode& rhs) const
		{
			return id == rhs.id;
		}

		bool operator< (const SortingNode& rhs) const
		{
			return true;
		}

	};

	/*
	 * @class CompareCandidates
	 * @brief Functor used to compare the candidate node on the base of the used criteria
	 */
	template<typename Node>
	class CompareCandidates
	{
	public:
		bool operator()(Node* n1, Node* n2)
		{
			if (*n1 < *n2)
				return true;
			return false;
		}
	};

	/*
	* @class FindUnused
	* @brief Used by std::find_if to find the first unused SortingNode
	*/
	class FindUnused
	{
	public:
		bool operator()(SortingNode* node)
		{
			return !node->IsUsed();
		}
	};

	template<typename Node, typename Edge>
	class VF2NodeSorter :NodeSorter<Node, Edge>
	{
	public:
		std::vector<nodeID_t> SortNodes(ARGraph<Node, Edge>* pattern)
		{
			uint32_t nodeCount;
			nodeID_t i, k, neigh;
			nodeID_t out_count, in_count;

			nodeCount = pattern->NodeCount();
			std::vector<nodeID_t> sorted;
			std::vector<SortingNode*> nodes(nodeCount);

			for (i = 0; i < nodeCount; i++)
			{
				nodes[i]=new SortingNode(i);
			}

			for (i = 0; i < nodeCount; i++)
			{
				if (!nodes[i]->IsUsed())
				{
					nodes[i]->SetUsed();
					sorted.push_back(i);
				}

				in_count = pattern->InEdgeCount(i);
				out_count = pattern->OutEdgeCount(i);

				for (k = 0; k < in_count; k++)
				{
					neigh = pattern->GetInEdge(i, k);
					if (!nodes[neigh]->IsUsed())
					{
						nodes[neigh]->SetUsed();
						sorted.push_back(neigh);
					}
				}

				for (k = 0; k < out_count; k++)
				{
					neigh = pattern->GetOutEdge(i, k);
					if (!nodes[neigh]->IsUsed())
					{
						nodes[neigh]->SetUsed();
						sorted.push_back(neigh);
					}
				}
			}
			return sorted;
		}
	};
}

#endif
