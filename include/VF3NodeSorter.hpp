#ifndef VF3NODE_SORTER_HPP
#define VF3NODE_SORTER_HPP

#include <vector>
#include <algorithm>

#include <ProbabilityStrategy.hpp>
#include <ARGraph.hpp>
#include <NodeSorter.hpp>

namespace vflib
{
	/*
	* @class VF3SortingNode
	* @bried Class used to sort the nodes of the graph by VF3
	*/
	class VF3SortingNode : public SortingNode
	{
	protected:
		double probability;
		uint32_t degNeigh;	//< Summ of neighboors' degree
		uint32_t degM;

	public:
		VF3SortingNode(nodeID_t id, int32_t deg, double probability)
			:SortingNode(id, deg), probability(probability)
		{
			degM = 0;
			degNeigh = 0;
		}


		inline double GetProbability() const { return probability; };

		inline uint32_t GetNeigborsDegree() const { return degNeigh; }
		inline void SetNeigborsDegree(const uint32_t& degree) { degNeigh = degree; }

		inline void ResetMatchingDegree() { degM = 0; }
		inline uint32_t GetMatchingDegree() const { return degM; }
		inline void IncreaseMatchingDegree() { degM++; }
		inline void SetMatchingDegree(const uint32_t& degree) { degM = degree; }

		bool operator< (const VF3SortingNode& rhs) const
		{
			if (degM > rhs.degM)
				return true;

			if (degM == rhs.degM &&
				probability < rhs.probability)
				return true;

			if (degM == rhs.degM &&
				probability == rhs.probability &&
				deg > rhs.deg)
				return true;

			return false;
		}

	};

	/*
	* @class CompareSortingNodeProbability
	* @brief Compares VF3SortingNode using the propability. If the probability of two nodes is equal uses the degree.
	*/
	class CompareSortingNodeProbability {
	public:
		bool operator()(VF3SortingNode* n1, VF3SortingNode* n2)
		{
			if (n1->GetProbability() < n2->GetProbability()) return true;
			if (n1->GetProbability() == n2->GetProbability() &&
				n1->GetDegree() > n2->GetDegree()) return true;
			return false;
		}
	};

	//Node sorter class used to sort the pattern
	//Complete Sorter used in VF3 - Use probability and degM connections
	template<typename Node, typename Edge,
		typename Probability = UniformProbability<Node, Edge > >
		class VF3NodeSorter : NodeSorter<Node, Edge>
	{
	private:
		Probability* probability;

		void AddNodeToSortedSet(ARGraph<Node, Edge>* pattern, nodeID_t node, nodeID_t level,
			std::vector<VF3SortingNode*>& nodes, std::vector<VF3SortingNode*> &candidates,
			std::vector<nodeID_t> &nodes_order)
		{
			nodeID_t i, neigh;
			nodeID_t in1_count, out1_count;

			nodes_order.push_back(node);
			nodes[node]->SetUsed();
			nodes[node]->SetInCanidate();
			nodes[node]->ResetMatchingDegree();

			//Updating Terminal set size count And degree
			in1_count = pattern->InEdgeCount(node);
			out1_count = pattern->OutEdgeCount(node);

			//Updating Inner Nodes not yet inserted
			for (i = 0; i < in1_count; i++)
			{
				//Getting Neighborhood
				neigh = pattern->GetInEdge(node, i);
				if (!nodes[neigh]->IsUsed())
					nodes[neigh]->IncreaseMatchingDegree();
				if (!nodes[neigh]->IsInCandidate()) {
					nodes[neigh]->SetInCanidate();
					candidates.push_back(nodes[neigh]);
				}

			}

			//Updating Outer Nodes not yet insered
			for (i = 0; i < out1_count; i++)
			{
				//Getting Neighborhood
				neigh = pattern->GetOutEdge(node, i);
				if (!nodes[neigh]->IsUsed())
					nodes[neigh]->IncreaseMatchingDegree();
				if (!nodes[neigh]->IsInCandidate()) {
					nodes[neigh]->SetInCanidate();
					candidates.push_back(nodes[neigh]);
				}
			}
		}

	public:
		VF3NodeSorter(ARGraph<Node, Edge>* target)
		{
			probability = new Probability(target);
		}

		~VF3NodeSorter()
		{
			delete probability;
		}

		std::vector<nodeID_t> SortNodes(ARGraph<Node, Edge>* pattern)
		{
			uint32_t nodeCount;
			uint32_t i;
			nodeCount = pattern->NodeCount();
			std::vector<nodeID_t> nodes_order; //Output vector with sorted nodes
										  //We use two structures the fist used to quickly edit the deg_m of a node by its index
										  //The second to perform a priority queue by means a max heap
			std::vector<VF3SortingNode*> nodes(nodeCount);
			std::vector<VF3SortingNode*> candidates; //Node candidate for the addition
			std::vector<VF3SortingNode*>::iterator candidate_it;
			std::vector<VF3SortingNode*>::iterator max_node;
			//Initializing the node vector for sorting

			for (i = 0; i < nodeCount; i++)
			{
				nodes[i] = new VF3SortingNode(i, pattern->EdgeCount(i), probability->GetProbability(pattern, i));
			}

			uint32_t n = 0;
			candidate_it = std::min_element(nodes.begin(), nodes.end(), CompareSortingNodeProbability());
			nodeID_t top = (*candidate_it)->GetID();
			AddNodeToSortedSet(pattern, top, n, nodes, candidates, nodes_order);

			//Getting the first node of the heap
			for (; n < nodeCount - 1; n++) {
				candidate_it = std::min_element(candidates.begin(), candidates.end(), CompareCandidates<VF3SortingNode>());

				//Searching for remaining user
				if ((*candidate_it)->IsUsed())
				{
					candidate_it = std::find_if(nodes.begin(), nodes.end(), FindUnused());
					AddNodeToSortedSet(pattern, (*candidate_it)->GetID(), n, nodes, candidates, nodes_order);
				}
				else if (candidate_it != candidates.end())
				{
					AddNodeToSortedSet(pattern, (*candidate_it)->GetID(), n, nodes, candidates, nodes_order);
				}
			}

			return nodes_order;
		}
	};

}

#endif
