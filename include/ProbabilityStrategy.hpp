#ifndef PROBABILITY_STRATEGY_HPP
#define PROBABILITY_STRATEGY_HPP

#include <map>
#include <vector>
#include <ARGraph.hpp>

namespace vflib
{
	template<typename Node, typename Edge>
	class NodeProbabilityStrategy
	{

	protected:
		double* degree; //Array of degree probability
		double* out_deg; //Array of "out" degree probability
		double* in_deg; //Array of "in" degree probability
		uint32_t degree_size;
		uint32_t out_deg_size;
		uint32_t in_deg_size;

		std::map<Node, double> labels; //Map of labels probability

		virtual void EvaluateProbabilities(ARGraph<Node, Edge>* source)
		{
			uint32_t i;
			uint32_t node_count;
			Node attr;

			std::map<Node, uint32_t> labelsmap;

			if (!source)
				return;

			out_deg_size = source->OutMaxDegree() + 1;
			in_deg_size = source->InMaxDegree() + 1;
			degree_size = source->MaxDegree() + 1;
			node_count = source->NodeCount();


			//std::cout<<"\n" <<out_deg_size<<" - " << in_deg_size << " - " << degree_size ;
			std::vector<uint32_t> out_deg_counter(out_deg_size, 0);
			std::vector<uint32_t> in_deg_counter(in_deg_size, 0);
			std::vector<uint32_t> total_deg_counter(degree_size, 0);

			out_deg = new double[out_deg_size];
			in_deg = new double[in_deg_size];
			degree = new double[degree_size];

			for (i = 0; i < node_count; i++) {
				//Reading attributes
				uint32_t out_deg = source->OutEdgeCount(i);
				uint32_t in_deg = source->InEdgeCount(i);
				uint32_t total_deg = in_deg + out_deg;
				attr = source->GetNodeAttr(i);

				//td::cout << "\nNode " << i << ": " << out_deg << " - " << in_deg << " - " << total_deg;

				out_deg_counter[out_deg]++;
				in_deg_counter[in_deg]++;
				total_deg_counter[total_deg]++;

				if (labelsmap.count(attr)) {
					labelsmap[attr]++;
				}
				else {
					labelsmap[attr] = 1;
				}
			}

			//std::cout << "\nOut Deg";
			for (i = 0; i < out_deg_size; i++) {
				out_deg[i] = ((double)out_deg_counter[i]) / node_count;
				//std::cout << "\n" << i << ": " << out_deg_counter[i];
			}

			//std::cout << "\nIn Deg";
			for (i = 0; i < in_deg_size; i++) {
				in_deg[i] = ((double)in_deg_counter[i]) / node_count;
				//std::cout << "\n" << i << ": " << in_deg_counter[i];
			}

			//std::cout << "\nTot Deg";
			for (i = 0; i < degree_size; i++) {
				degree[i] = ((double)total_deg_counter[i]) / node_count;
				//std::cout << "\n" << i << ": " << total_deg_counter[i];
			}

			typename std::map<Node, uint32_t>::iterator iter;
			for (iter = labelsmap.begin(); iter != labelsmap.end(); ++iter)
			{
				Node n = iter->first;
				labels[n] = ((double)labelsmap[n]) / node_count;
			}
		}

	public:
		NodeProbabilityStrategy(ARGraph<Node, Edge>* g2) {
			EvaluateProbabilities(g2);
		}

		~NodeProbabilityStrategy() {
			delete[] out_deg;
			delete[] in_deg;
			delete[] degree;
		}

		/*
		* @fn GetProbability
		* @param [in] g1 Graph
		* @param [in] id Node id
		* @returns The probability of the node
		*/
		virtual double GetProbability(ARGraph<Node, Edge> *g1, nodeID_t id) = 0;
	};

	template<typename Node, typename Edge>
	class UniformProbability {
	public:
		UniformProbability(ARGraph<Node, Edge>* source) {}
		double GetProbability(ARGraph<Node, Edge>* g, nodeID_t id)
		{
			return 1 / g->NodeCount();
		}
	};

	/*
	* @class IsoNodeProbability
	* @brief Compute the probability function in case of Isomorphism
	* @details We do not suggest to use it in case of subgraph isomorphism
	*/
	template<typename Node, typename Edge>
	class IsoNodeProbability : public NodeProbabilityStrategy<Node, Edge> {
	public:
		IsoNodeProbability(ARGraph<Node, Edge>* source) :NodeProbabilityStrategy<Node, Edge>(source) {}
		virtual ~IsoNodeProbability(){}
		double GetProbability(ARGraph<Node, Edge>* g, nodeID_t id)
		{
			double node_p;
			uint32_t node_out_deg, node_in_deg, node_deg;
			Node node_attr;
			node_out_deg = g->OutEdgeCount(id);
			node_in_deg = g->InEdgeCount(id);
			node_deg = node_out_deg + node_in_deg;
			node_attr = g->GetNodeAttr(id);
			node_p = this->out_deg[node_out_deg] * this->in_deg[node_in_deg] * this->degree[node_deg] * this->labels[node_attr];
			return node_p;
		}
	};

	/*
	* @class SubIsoNodeProbability
	* @brief Compute the probability function in case of Subgraph Isomorphism
	* @details We do not suggest to use it in case of graph isomorphism
	*/
	template<typename Node, typename Edge>
	class SubIsoNodeProbability : public NodeProbabilityStrategy<Node, Edge> {
	public:
		SubIsoNodeProbability(ARGraph<Node, Edge>* source) :NodeProbabilityStrategy<Node, Edge>(source) {}
		virtual ~SubIsoNodeProbability(){}
		double GetProbability(ARGraph<Node, Edge>* g, nodeID_t id)
		{
			double node_p;
			uint32_t node_out_deg, node_in_deg, node_deg;
			uint32_t i = 0;
			Node node_attr;
			node_out_deg = g->OutEdgeCount(id);
			node_in_deg = g->InEdgeCount(id);
			node_deg = node_out_deg + node_in_deg;
			node_attr = g->GetNodeAttr(id);

			//Sum of probability
			double out_deg_p_sum, in_deg_p_sum, tot_deg_p_sum;
			out_deg_p_sum = in_deg_p_sum = tot_deg_p_sum = 0;

			for (i = node_out_deg; i < this->out_deg_size; i++) {
				out_deg_p_sum += this->out_deg[i];
			}

			for (i = node_in_deg; i < this->in_deg_size; i++) {
				in_deg_p_sum += this->in_deg[i];
			}

			for (i = node_deg; i < this->degree_size; i++) {
				tot_deg_p_sum += this->degree[i];
			}

			node_p = out_deg_p_sum*in_deg_p_sum*tot_deg_p_sum*this->labels[node_attr];

			return node_p;
		}
	};


}

#endif
