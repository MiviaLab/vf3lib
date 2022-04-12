//
//  vf3_sub_state.hpp
//  VF2Plus
//
//  Created by Vincenzo Carletti on 12/11/14.
//  Copyright (c) 2014 Vincenzo Carletti. All rights reserved.
//

#ifndef VF3LIGHT_SUB_STATE_HPP
#define VF3LIGHT_SUB_STATE_HPP

#include <cstring>
#include <iostream>
#include <vector>
#include <ARGraph.hpp>
#include <VF3State.hpp>
#include <State.hpp>

namespace vflib
{

	/*----------------------------------------------------------
	 * class VF3LightSubState
	 * A representation of the SSR current state
	 * See vf2_state.cc for more details.
	 ---------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor = EqualityComparator<Node1, Node2>,
		typename EdgeComparisonFunctor = EqualityComparator<Edge1, Edge2> >
		class VF3LightSubState : public State
	{
	private:
		//Comparison functors for nodes and edges
		NodeComparisonFunctor nf;
		EdgeComparisonFunctor ef;

		//Graphs to analyze
		ARGraph<Node1, Edge1> *g1;
		ARGraph<Node2, Edge2> *g2;

		//CORE SET SIZES
		uint32_t *core_len_c;    //Core set length for each class

		nodeDir_t* dir;        //Node coming set. Direction into the terminal set.
		nodeID_t* predecessors;  //Previous node in the ordered sequence connected to a node

		//Vector of sets used for searching the successors
		//Each class has its set
		uint32_t last_candidate_index;

		/* Structures for classes */
		uint32_t *class_1;       //Classes for nodes of the first graph
		uint32_t *class_2;       //Classes for nodes of the first graph
		uint32_t classes_count;  //Number of classes

		//PRIVATE METHODS
		void BackTrack();
		void ComputeFirstGraphTraversing();

	public:
		static long long instance_count;
		VF3LightSubState(ARGraph<Node1, Edge1> *g1, ARGraph<Node2, Edge2> *g2,
			uint32_t* class_1, uint32_t* class_2, uint32_t nclass,
			nodeID_t* order = NULL, bool induced = true);
		VF3LightSubState(const VF3LightSubState &state);
		~VF3LightSubState();
		inline ARGraph<Node1, Edge1> *GetGraph1() const { return g1; }
		inline ARGraph<Node2, Edge2> *GetGraph2() const { return g2; }
		bool NextPair(nodeID_t *pn1, nodeID_t *pn2,
			nodeID_t prev_n1 = NULL_NODE, nodeID_t prev_n2 = NULL_NODE);
		bool IsFeasiblePair(nodeID_t n1, nodeID_t n2);
		void AddPair(nodeID_t n1, nodeID_t n2);
		inline bool IsGoal() const { return core_len == n1; };
		inline bool IsDead() const { return false; };
	};


	/*----------------------------------------------------------
	 * VF3LightSubState::VF3LightSubState(g1, g2)
	 * Constructor. Makes an empty state.
	 ---------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		VF3LightSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>
			::VF3LightSubState(ARGraph<Node1, Edge1> *ag1, ARGraph<Node2, Edge2> *ag2, 
					uint32_t* class_1, uint32_t* class_2, uint32_t nclass, nodeID_t* order, bool induced)
					:State(ag1->NodeCount(), ag2->NodeCount(), order, induced)
	{
		assert(class_1 != NULL && class_2 != NULL);

		g1 = ag1;
		g2 = ag2;
		n1 = g1->NodeCount();
		n2 = g2->NodeCount();
		last_candidate_index = 0;

		this->class_1 = class_1;
		this->class_2 = class_2;
		this->classes_count = nclass;

		core_len = orig_core_len = 0;

		core_len_c = (uint32_t*)calloc(classes_count, sizeof(uint32_t));

		dir = new nodeDir_t[n1];
		predecessors = new nodeID_t[n1];

		ComputeFirstGraphTraversing();
	}


	/*----------------------------------------------------------
	 * VF3LightSubState::VF3LightSubState(state)
	 * Copy constructor.
	 ---------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		VF3LightSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>
			::VF3LightSubState(const VF3LightSubState &state):State(state)
	{
		g1 = state.g1;
		g2 = state.g2;
		n1 = state.n1;
		n2 = state.n2;

		class_1 = state.class_1;
		class_2 = state.class_2;
		classes_count = state.classes_count;
		parent = &state;
		used = false;

		last_candidate_index = state.last_candidate_index;

		core_len = orig_core_len = state.core_len;
		core_len_c = state.core_len_c;
		added_node1 = NULL_NODE;

		dir = state.dir;
		predecessors = state.predecessors;
		share_count = state.share_count;

		++ *share_count;

	}


	/*---------------------------------------------------------------
	 * VF3LightSubState::~VF3LightSubState()
	 * Destructor.
	 --------------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		VF3LightSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>::~VF3LightSubState()
	{

		if (-- *share_count > 0)
			BackTrack();

		if (*share_count == 0)
		{
			delete[] dir;
			delete[] predecessors;
			delete[] core_len_c;
		}
	}

	//Provare ad avere in1 ed ou1 predeterminati, senza doverlo calcolare ad ogni iterazione
	//La loro dimensione ad ogni livello dell'albero di ricerca e' predeterminato
	//In questo modo mi basta conoscere solo l'ordine di scelta e la dimensione di in1 ed out1
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		void VF3LightSubState<Node1, Node2, Edge1, Edge2, 
			NodeComparisonFunctor, EdgeComparisonFunctor>::ComputeFirstGraphTraversing() {
		//The algorithm start with the node with the maximum degree
		nodeID_t depth, i;
		nodeID_t node;	//Current Node
		int32_t node_c; //Class of the current node
		bool* inserted = new bool[n1];
		bool *in, *out; //Internal Terminal Set used for updating the size of
		in = new bool[n1];
		out = new bool[n1];

		//Init vectors and variables
		node = 0;
		node_c = 0;

		for (i = 0; i < n1; i++)
		{
			in[i] = false;
			out[i] = false;
			dir[i] = NODE_DIR_NONE;
			inserted[i] = false;
			predecessors[i] = NULL_NODE;
		}

		/* Following the imposed node order */
		for (depth = 0; depth < n1; depth++)
		{
			node = order[depth];
			node_c = class_1[node];
			inserted[node] = true;

			//Inserting the node
			//Terminal set sizes depends on the depth
			// < depth non sono nell'insieme
			// >= depth sono nell'insieme
			if (!in[node])
				in[node] = true;

			if (!out[node])
				out[node] = true;

			//Updating terminal sets
			int32_t i, other, other_c;
			other_c = -1;
			for (i = 0; i < g1->InEdgeCount(node); i++)
			{
				other = g1->GetInEdge(node, i);
				if (!in[other])
				{
					other_c = class_1[other];
					in[other] = true;
					if (!inserted[other])
					{
						if (predecessors[other] == NULL_NODE)
						{
							dir[other] = NODE_DIR_IN;
							predecessors[other] = node;
						}
					}
				}
			}

			for (i = 0; i < g1->OutEdgeCount(node); i++)
			{
				other = g1->GetOutEdge(node, i);
				if (!out[other])
				{
					other_c = class_1[other];
					out[other] = true;
					if (!inserted[other])
					{
						if (predecessors[other] == NULL_NODE)
						{
							predecessors[other] = node;
							dir[other] = NODE_DIR_OUT;
						}
					}
				}
			}
		}

		delete[] in;
		delete[] out;
		delete[] inserted;
	}

	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		bool VF3LightSubState<Node1, Node2, Edge1, Edge2, 
			NodeComparisonFunctor, EdgeComparisonFunctor>::NextPair(nodeID_t *pn1, nodeID_t *pn2, nodeID_t prev_n1, nodeID_t prev_n2)
	{

		nodeID_t curr_n1;
		nodeID_t pred_pair; //Node mapped with the predecessor
		nodeID_t pred_set_size = 0;
		int32_t c = 0;
		pred_pair = NULL_NODE;

		//core_len indica la profondondita' della ricerca
		curr_n1 = order[core_len];
		c = class_1[curr_n1];

		if (predecessors[curr_n1] != NULL_NODE)
		{
			if (prev_n2 == NULL_NODE)
				last_candidate_index = 0;
			else {
				last_candidate_index++; //Next Element
			}

			pred_pair = core_1[predecessors[curr_n1]];
			switch (dir[curr_n1])
			{
			case NODE_DIR_IN:
				pred_set_size = g2->InEdgeCount(pred_pair);

				while (last_candidate_index < pred_set_size)
				{
					prev_n2 = g2->GetInEdge(pred_pair, last_candidate_index);
					if (core_2[prev_n2] != NULL_NODE || class_2[prev_n2] != c)
						last_candidate_index++;
					else
						break;
				}

				break;

			case NODE_DIR_OUT:
				pred_set_size = g2->OutEdgeCount(pred_pair);

				while (last_candidate_index < pred_set_size)
				{
					prev_n2 = g2->GetOutEdge(pred_pair, last_candidate_index);
					if (core_2[prev_n2] != NULL_NODE || class_2[prev_n2] != c)
						last_candidate_index++;
					else
						break;
				}

				break;
			}

			if (last_candidate_index >= pred_set_size)
				return false;

		}
		else
		{
			//Recupero il nodo dell'esterno
			if (prev_n2 == NULL_NODE)
				prev_n2 = 0;
			else
				prev_n2++;

			while (prev_n2 < n2 &&
				(core_2[prev_n2] != NULL_NODE
					|| class_2[prev_n2] != c))
			{
				prev_n2++;
			}
		}
		//std::cout<<curr_n1 << " " << prev_n2 << " \n";

		if (prev_n2 < n2) {
			*pn1 = curr_n1;
			*pn2 = prev_n2;
			//std::cout<<"\nNP END: " <<curr_n1<<" " << prev_n2 << "\n" ;
			return true;
		}

		return false;
	}


	/*---------------------------------------------------------------
	 * bool VF3LightSubState::IsFeasiblePair(node1, node2)
	 * Returns true if (node1, node2) can be added to the state
	 * NOTE:
	 *   The attribute compatibility check (methods CompatibleNode
	 *   and CompatibleEdge of ARGraph) is always performed
	 *   applying the method to g1, and passing the attribute of
	 *   g1 as first argument, and the attribute of g2 as second
	 *   argument. This may be important if the compatibility
	 *   criterion is not symmetric.
	 --------------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		bool VF3LightSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, 
			EdgeComparisonFunctor>::IsFeasiblePair(nodeID_t node1, nodeID_t node2)
	{
		//std::cout<<"\nIF: " <<node1<<" " << node2;
		//print_core(core_1, core_2, core_len);
		assert(node1 < n1);
		assert(node2 < n2);
		assert(core_1[node1] == NULL_NODE);
		assert(core_2[node2] == NULL_NODE);

		if (!nf(g1->GetNodeAttr(node1), g2->GetNodeAttr(node2)))
			return false;

		if (g1->InEdgeCount(node1) > g2->InEdgeCount(node2)
			|| g1->OutEdgeCount(node1) > g2->OutEdgeCount(node2))
			return false;

		uint32_t i, other1, other2, c_other;
		Edge1 eattr1;
		Edge2 eattr2;

		// Check the 'out' edges of node1
		for (i = 0; i < g1->OutEdgeCount(node1); i++)
		{
			other1 = g1->GetOutEdge(node1, i, eattr1);
			c_other = class_1[other1];
			if (core_1[other1] != NULL_NODE)
			{
				other2 = core_1[other1];
				if (!g2->HasEdge(node2, other2, eattr2) ||
					!ef(eattr1, eattr2))
					return false;
			}
		}

		// Check the 'in' edges of node1
		for (i = 0; i < g1->InEdgeCount(node1); i++)
		{
			other1 = g1->GetInEdge(node1, i, eattr1);
			c_other = class_1[other1];
			if (core_1[other1] != NULL_NODE)
			{
				other2 = core_1[other1];
				if (!g2->HasEdge(other2, node2, eattr2) ||
					!ef(eattr1, eattr2))
					return false;
			}
		}

		// Check the 'out' edges of node2
		if(induced)
		{
			for (i = 0; i < g2->OutEdgeCount(node2); i++)
			{
				other2 = g2->GetOutEdge(node2, i);
				c_other = class_2[other2];
				if (core_2[other2] != NULL_NODE)
				{
					other1 = core_2[other2];
					if (!g1->HasEdge(node1, other1))
						return false;
				}
			}

			// Check the 'in' edges of node2
			for (i = 0; i < g2->InEdgeCount(node2); i++)
			{
				other2 = g2->GetInEdge(node2, i);
				c_other = class_2[other2];
				if (core_2[other2] != NULL_NODE)
				{
					other1 = core_2[other2];
					if (!g1->HasEdge(other1, node1))
						return false;
				}
			}
		}

		//std::cout << "\nIs Feasible: " << node1 << " " << node2;
		return true;

	}



	/*--------------------------------------------------------------
	 * void VF3LightSubState::AddPair(node1, node2)
	 * Adds a pair to the Core set of the state.
	 * Precondition: the pair must be feasible
	 -------------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		void VF3LightSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, 
			EdgeComparisonFunctor>::AddPair(nodeID_t node1, nodeID_t node2)
	{

		/*std::cout<<"\nAP:";
		print_core(core_1,core_2,n1);
		std::cout<<" <- "<< node1 <<":"<< node2;*/

		assert(node1 < n1);
		assert(node2 < n2);
		assert(core_len < n1);
		assert(core_len < n2);
		assert(class_1[node1] == class_2[node2]);

		//Updating the core length
		core_len++;
		added_node1 = node1;
		int32_t node_c = class_1[node1];
		core_len_c[node_c]++;

		//Inserting nodes into the core set
		core_1[node1] = node2;
		core_2[node2] = node1;

	}

	/*----------------------------------------------------------------
	 * Undoes the changes to the shared vectors made by the
	 * current state. Assumes that at most one AddPair has been
	 * performed.
	 ----------------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		void VF3LightSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>::BackTrack()
	{

		/*std::cout<<"\nBT:";
		 print_core(core_1,core_2,n1);
		 std::cout<<" -> "<< added_node1 <<":"<< core_1[added_node1];*/

		assert(core_len - orig_core_len <= 1);
		if (added_node1 != NULL_NODE)
		{
			int32_t other_c = 0;
			int32_t node_c = class_1[added_node1];

			if (orig_core_len < core_len)
			{
				int32_t i, node2;
				node2 = core_1[added_node1];

				core_1[added_node1] = NULL_NODE;
				core_2[node2] = NULL_NODE;

				core_len = orig_core_len;
				core_len_c[node_c]--;
				added_node1 = NULL_NODE;
			}
		}
	}

}

#endif
