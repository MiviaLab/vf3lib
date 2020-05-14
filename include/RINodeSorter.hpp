#ifndef RI_NODE_SORTER_HPP
#define RI_NODE_SORTER_HPP

#include <vector>
#include <algorithm>

#include <ProbabilityStrategy.hpp>
#include <ARGraph.hpp>
#include <NodeSorter.hpp>

namespace vflib
{
	/*
	* @class VF3SortingNode
	* @bried Class used to sort the nodes of the graph by RI Sorting Procedure
	*/
	class RISortingNode : public SortingNode
	{
	private:
		int32_t unvCount;
		int32_t visCount;
		int32_t neigCount;
	public:
		RISortingNode(nodeID_t id, uint32_t deg, uint32_t outDeg)
			:SortingNode(id, deg), unvCount(outDeg), visCount(0), neigCount(0) {}

		inline int32_t GetVisCount() { return visCount; }
		inline int32_t GetNeigborsCount() { return neigCount; }
		inline int32_t GetUnvCount() { return unvCount; }

		inline void InvalidVisCount(){visCount = -1;}
		inline void InvalidNeigborsCount(){neigCount = -1;}
		inline void InvalidUnvCount() { unvCount = -1; }

		inline void IncreaseVisCount() { visCount++; }
		inline void DecreaseVisCount() { visCount--; }
		inline void IncreaseNeigborsCount() { neigCount++; }
		inline void DecreaseNeigborsCount() { neigCount--; }
		inline void IncreaseUnvCount() { unvCount++; }
		inline void DecreaseUnvCount() { unvCount--; }

		bool operator< (const RISortingNode& rhs) const
		{
			//In this case the field is used to avoid to reduce more time the unv_count
			//Indeed a node that belongs to the neig set is supposed to be already considered
			//thus when it is insertend in the solution the unv_count has not be decreased but
			//the neig_counts

			if (used && rhs.used)
				return true;
			if (used && !rhs.used)
				return false;
			if (!used  && rhs.used)
				return true;

			if (visCount > rhs.visCount)
				return true;
			if (visCount == rhs.visCount &&
				neigCount > rhs.neigCount)
				return true;
			if (visCount == rhs.visCount&&
				neigCount == rhs.neigCount &&
				unvCount > rhs.unvCount)
				return true;
			return false;
		}
	};

	//Comparison functions for RI Sorting
	class CompareNodeDegree {
	public:
		bool operator()(RISortingNode* n1, RISortingNode* n2)
		{
			if (n1->GetDegree() > n2->GetDegree()) return true;
			return false;
		}
	};


	template<typename Node, typename Edge>
	class RINodeSorter : NodeSorter<Node, Edge>
	{
	public:
		std::vector<nodeID_t> SortNodes(ARGraph<Node, Edge>* pattern)
		{
			uint32_t nodeCount;
			nodeID_t i, n, k, neigh, neigh2;
			nodeID_t out1_count, out1_count_2;

			nodeCount = pattern->NodeCount();
			std::vector<nodeID_t> sorted;
			std::vector<RISortingNode*> nodes(nodeCount);
			std::vector<RISortingNode*> candidates; //Node candidate for the addition
			std::vector<RISortingNode*>::iterator candidate_it;
			std::vector<RISortingNode*>::iterator max_node;
			std::vector<RISortingNode*>::iterator start_it;

			for (i = 0; i < nodeCount; i++)
			{
				nodes[i] = new RISortingNode(i, pattern->EdgeCount(i), pattern->OutEdgeCount(i));
			}

			//Getting the element with the maximum degree
			candidate_it = min_element(nodes.begin(), nodes.end(), CompareNodeDegree());
			start_it = nodes.begin();

			for (n = 0; n < nodeCount; n++)
			{
				(*candidate_it)->SetUsed();
				(*candidate_it)->InvalidVisCount();
				(*candidate_it)->InvalidNeigborsCount();
				(*candidate_it)->InvalidUnvCount();
				nodeID_t node = (*candidate_it)->GetID();
				sorted.push_back(node);
				//std::cout<<node<<"\n";
				out1_count = pattern->OutEdgeCount(node);

				for (i = 0; i < out1_count; i++)
				{
					//Getting Neighborhood
					neigh = pattern->GetOutEdge(node, i);
					if (!nodes[neigh]->IsUsed()) {
						nodes[neigh]->IncreaseVisCount();
						if (!nodes[node]->IsInCandidate())
							nodes[neigh]->DecreaseUnvCount();
						else
							nodes[neigh]->DecreaseNeigborsCount();

						out1_count_2 = pattern->OutEdgeCount(neigh);
						for (k = 0; k < out1_count_2; k++)
						{
							neigh2 = pattern->GetOutEdge(neigh, k);
							if (neigh2 != node &&
								!nodes[neigh2]->IsUsed() &&
								pattern->HasEdge(neigh2, node))
							{
								nodes[neigh]->IncreaseNeigborsCount();
								nodes[neigh2]->SetInCanidate();
								nodes[neigh]->DecreaseUnvCount();
							}
						}
					}
				}

				candidate_it = min_element(nodes.begin(), nodes.end(), CompareCandidates<RISortingNode>());
			}

			return sorted;
		}
	};

}

#endif
