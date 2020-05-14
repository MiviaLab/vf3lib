/*----------------------------------------------------
 * @file   VF3KSubState.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date  December, 2014 
 * @brief Definition of a class representing a state of the matching
 * process between two ARGs.
 *--------------------------------------------------*/

#ifndef VF3K_SUB_STATE_HPP
#define VF3K_SUB_STATE_HPP

#include <cstring>
#include <iostream>
#include <vector>
#include <ARGraph.hpp>
#include <VF3State.hpp>
#include <State.hpp>

namespace vflib
{

	/*----------------------------------------------------------
	 * @class VF3KSubState
	 * @brief A representation of the SSR current state
	 * @details The class is used for subgraph isomorphism.
	 * 	In this version the algorithm will stop using look-ahead when the
	 * 	level K is reached
	 ---------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor = EqualityComparator<Node1, Node2>,
		typename EdgeComparisonFunctor = EqualityComparator<Edge1, Edge2> >
		class VF3KSubState : public State
	{
	private:
		//Comparison functors for nodes and edges
		NodeComparisonFunctor nf;
		EdgeComparisonFunctor ef;

		//Graphs to analyze
		ARGraph<Node1, Edge1> *g1;
		ARGraph<Node2, Edge2> *g2;

		uint32_t limit_level;

		//CORE SET SIZES
		uint32_t *core_len_c;    //Core set length for each class

		nodeDir_t* dir;        //Node coming set. Direction into the terminal set.
		nodeID_t* predecessors;  //Previous node in the ordered sequence connected to a node

		//TERMINAL SET SIZE
		//BE AWARE: Core nodes are also counted by these
		//GLOBAL SIZE
		uint32_t t2in_len, t2both_len, t2out_len; //Len of Terminal set for the second graph
		uint32_t *t1in_len, *t1both_len, *t1out_len; //Len of Terminal set for the first graph for each level
												//SIZE FOR EACH CLASS
		uint32_t *t2both_len_c, *t2in_len_c, *t2out_len_c;     //Len of Terminal set for the second graph for each class
		uint32_t **t1both_len_c, **t1in_len_c, **t1out_len_c;  //Len of Terminal set for the first graph for each class end level

		//Used for terminal set size evaluation
		uint32_t *termout2_c, *termin2_c, *new2_c;
		uint32_t **termout1_c, **termin1_c, **new1_c;
		uint32_t *termin1, *termout1, *new1;

		//Terminal sets of the second graph
		//TERM IN
		nodeID_t *in_2;
		//TERMI OUT
		nodeID_t *out_2;

		//Vector of sets used for searching the successors
		//Each class has its set
		int64_t last_candidate_index;

		/* Structures for classes */
		uint32_t *class_1;       //Classes for nodes of the first graph
		uint32_t *class_2;       //Classes for nodes of the first graph
		uint32_t classes_count;  //Number of classes

		//PRIVATE METHODS
		void BackTrack();
		void ComputeFirstGraphTraversing();
		void UpdateTerminalSetSize(nodeID_t node, nodeID_t level, bool* in_1, bool* out_1, bool* inserted);

	public:
		static long long instance_count;
		VF3KSubState(ARGraph<Node1, Edge1> *g1, ARGraph<Node2, Edge2> *g2,
			uint32_t* class_1, uint32_t* class_2, uint32_t nclass, uint32_t k=0,
			nodeID_t* order = NULL, bool induced = true);
		VF3KSubState(const VF3KSubState &state);
		~VF3KSubState();
		inline ARGraph<Node1, Edge1> *GetGraph1() const { return g1; }
		inline ARGraph<Node2, Edge2> *GetGraph2() const { return g2; }
		bool NextPair(nodeID_t *pn1, nodeID_t *pn2,
			nodeID_t prev_n1 = NULL_NODE, nodeID_t prev_n2 = NULL_NODE);
		bool IsFeasiblePair(nodeID_t n1, nodeID_t n2);
		void AddPair(nodeID_t n1, nodeID_t n2);
		inline bool IsGoal() const { return core_len == n1; };
		bool IsDead() const;
	};

	/*----------------------------------------------------------
	 * VF3KSubState::VF3KSubState(g1, g2)
	 * Constructor. Makes an empty state.
	 ---------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>
		::VF3KSubState(ARGraph<Node1, Edge1> *ag1, ARGraph<Node2, Edge2> *ag2,
			uint32_t* class_1, uint32_t* class_2, uint32_t nclass, uint32_t k, nodeID_t* order, bool induced)
		:State(ag1->NodeCount(), ag2->NodeCount(), order, induced)
	{
		assert(class_1 != NULL && class_2 != NULL);

		g1 = ag1;
		g2 = ag2;
		n1 = g1->NodeCount();
		n2 = g2->NodeCount();
		last_candidate_index = 0;

		//The limit cannot be higher
		//than the size of the pattern
		if(k > n1)
		{
			k = n1;
		}
		limit_level = k;

		this->class_1 = class_1;
		this->class_2 = class_2;
		classes_count = nclass;
		t2both_len = t2in_len = t2out_len = 0;

		dir = new nodeDir_t[n1];
		predecessors = new nodeID_t[n1];
		core_len_c = new uint32_t[classes_count];

		//Creating VF Sets
		//Only if limit_level > 0
		if(limit_level > 0)
		{
			t1both_len = new uint32_t[limit_level + 1];
			t1in_len = new uint32_t[limit_level + 1];
			t1out_len = new uint32_t[limit_level + 1];

			termin1 = new uint32_t[limit_level];
			termout1 = new uint32_t[limit_level];
			new1 = new uint32_t[limit_level];

			t1both_len_c = (uint32_t**)malloc((limit_level + 1) * sizeof(uint32_t*));
			t1in_len_c = (uint32_t**)malloc((limit_level + 1) * sizeof(uint32_t*));
			t1out_len_c = (uint32_t**)malloc((limit_level + 1) * sizeof(uint32_t*));

			termin1_c = (uint32_t**)malloc(limit_level * sizeof(uint32_t*));
			termout1_c = (uint32_t**)malloc(limit_level * sizeof(uint32_t*));
			new1_c = (uint32_t**)malloc(limit_level * sizeof(uint32_t*));

			t2both_len_c = new uint32_t[classes_count];
			t2in_len_c = new uint32_t[classes_count];
			t2out_len_c = new uint32_t[classes_count];
			termout2_c = new uint32_t[classes_count];
			termin2_c = new uint32_t[classes_count];
			new2_c = new uint32_t[classes_count];

			in_2 = new nodeID_t[n2];
			out_2 = new nodeID_t[n2];

			uint32_t i;
			for (i = 0; i <= limit_level; i++)
			{
				if (i < limit_level) {
					termin1_c[i] = (uint32_t*)calloc(classes_count, sizeof(uint32_t));
					termout1_c[i] = (uint32_t*)calloc(classes_count, sizeof(uint32_t));
					new1_c[i] = (uint32_t*)calloc(classes_count, sizeof(uint32_t));
				}
				t1both_len_c[i] = (uint32_t*)calloc(classes_count, sizeof(uint32_t));
				t1in_len_c[i] = (uint32_t*)calloc(classes_count, sizeof(uint32_t));
				t1out_len_c[i] = (uint32_t*)calloc(classes_count, sizeof(uint32_t));
			}

			for (i = 0; i < n2; i++)
			{
				in_2[i] = 0;
				out_2[i] = 0;
			}
		}
		ComputeFirstGraphTraversing();
	}


	/*----------------------------------------------------------
	 * VF3KSubState::VF3KSubState(state)
	 * Copy constructor.
	 ---------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>
			::VF3KSubState(const VF3KSubState &state):State(state)
	{
		g1 = state.g1;
		g2 = state.g2;

		class_1 = state.class_1;
		class_2 = state.class_2;
		classes_count = state.classes_count;

		last_candidate_index = state.last_candidate_index;

		limit_level = state.limit_level;
		dir = state.dir;
		predecessors = state.predecessors;
		core_len_c = state.core_len_c;

		if(limit_level>0)
		{
			t1in_len = state.t1in_len;
			t1out_len = state.t1out_len;
			t1both_len = state.t1both_len;

			t2in_len = state.t2in_len;
			t2out_len = state.t2out_len;
			t2both_len = state.t2both_len;


			t1both_len_c = state.t1both_len_c;
			t2both_len_c = state.t2both_len_c;
			t1in_len_c = state.t1in_len_c;
			t2in_len_c = state.t2in_len_c;
			t1out_len_c = state.t1out_len_c;
			t2out_len_c = state.t2out_len_c;

			termout1_c = state.termout1_c;
			termout2_c = state.termout2_c;
			termin1_c = state.termin1_c;
			termin2_c = state.termin2_c;
			new1_c = state.new1_c;
			new2_c = state.new2_c;

			termin1 = state.termin1;
			termout1 = state.termout1;
			new1 = state.new1;

			in_2 = state.in_2;
			out_2 = state.out_2;
		}

	}


	/*---------------------------------------------------------------
	 * VF3KSubState::~VF3KSubState()
	 * Destructor.
	 --------------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>::~VF3KSubState()
	{

		if (-- *share_count > 0)
			BackTrack();

		if (*share_count == 0)
		{
			delete[] dir;
			delete[] predecessors;
			delete[] core_len_c;

			if(limit_level>0)
			{
				delete[] in_2;
				delete[] out_2;
				delete[] t1both_len;
				delete[] t1in_len;
				delete[] t1out_len;
				delete[] termin1;
				delete[] termout1;
				delete[] new1;
				delete[] t2both_len_c;
				delete[] t2in_len_c;
				delete[] t2out_len_c;
				delete[] termin2_c;
				delete[] termout2_c;
				delete[] new2_c;

				for (uint32_t i = 0; i <= limit_level; i++) {
					free(t1both_len_c[i]);
					free(t1in_len_c[i]);
					free(t1out_len_c[i]);
					if (i < limit_level) {
						free(termin1_c[i]);
						free(termout1_c[i]);
						free(new1_c[i]);
					}
				}
				free(t1both_len_c);
				free(t1in_len_c);
				free(t1out_len_c);
				free(termin1_c);
				free(termout1_c);
				free(new1_c);
			}
		}
	}

	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		void VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor,
		EdgeComparisonFunctor>::UpdateTerminalSetSize(nodeID_t node, nodeID_t level, bool* in_1, bool* out_1, bool* inserted) {
		nodeID_t i, neigh, c_neigh;
		nodeID_t in1_count, out1_count;

		//Updating Terminal set size count And degree
		in1_count = g1->InEdgeCount(node);
		out1_count = g1->OutEdgeCount(node);

		//Updating Inner Nodes not yet inserted
		for (i = 0; i < in1_count; i++)
		{
			//Getting Neighborhood
			neigh = g1->GetInEdge(node, i);
			c_neigh = class_1[neigh];

			if (!inserted[neigh])
			{
				if (in_1[neigh]) {
					termin1[level]++;
					termin1_c[level][c_neigh]++;
				}
				if (out_1[neigh]) {
					termout1[level]++;
					termout1_c[level][c_neigh]++;
				}
				if (!in_1[neigh] && !out_1[neigh]) {
					new1[level]++;
					new1_c[level][c_neigh]++;
				}
			}
		}

		//Updating Outer Nodes not yet insered
		for (i = 0; i < out1_count; i++)
		{
			//Getting Neighborhood
			neigh = g1->GetOutEdge(node, i);
			c_neigh = class_1[neigh];
			if (!inserted[neigh])
			{
				if (in_1[neigh]) {
					termin1[level]++;
					termin1_c[level][c_neigh]++;
				}
				if (out_1[neigh]) {
					termout1[level]++;
					termout1_c[level][c_neigh]++;
				}
				if (!in_1[neigh] && !out_1[neigh]) {
					new1[level]++;
					new1_c[level][c_neigh]++;
				}
			}
		}
	}


	//Provare ad avere in1 ed ou1 predeterminati, senza doverlo calcolare ad ogni iterazione
	//La loro dimensione ad ogni livello dell'albero di ricerca e' predeterminato
	//In questo modo mi basta conoscere solo l'ordine di scelta e la dimensione di in1 ed out1
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		void VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>
		::ComputeFirstGraphTraversing() {
		//The algorithm start with the node with the maximum degree
		nodeID_t depth, i;
		nodeID_t node;	//Current Node
		uint32_t node_c; //Class of the current node
		bool* inserted = new bool[n1];
		bool *in, *out; //Internal Terminal Set used for updating the size of
		in = new bool[n1];
		out = new bool[n1];

		//Init vectors and variables
		node = 0;
		node_c = 0;


		if(limit_level > 0)
		{
			t1in_len[0] = 0;
			t1out_len[0] = 0;
			t1both_len[0] = 0;
		}

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

			if(depth < limit_level)
			{
				UpdateTerminalSetSize(node, depth, in, out, inserted);

				//Updating counters for next step
				t1in_len[depth + 1] = t1in_len[depth];
				t1out_len[depth + 1] = t1out_len[depth];
				t1both_len[depth + 1] = t1both_len[depth];
				for (uint32_t j = 0; j < classes_count; j++)
				{
					t1in_len_c[depth + 1][j] = t1in_len_c[depth][j];
					t1out_len_c[depth + 1][j] = t1out_len_c[depth][j];
					t1both_len_c[depth + 1][j] = t1both_len_c[depth][j];
				}
				//Inserting the node
				//Terminal set sizes depends on the depth
				// < depth non sono nell'insieme
				// >= depth sono nell'insieme
				if (!in[node])
				{
					in[node] = true;
					t1in_len[depth + 1]++;
					t1in_len_c[depth + 1][node_c]++;
					if (out[node]) {
						t1both_len[depth + 1]++;
						t1both_len_c[depth + 1][node_c]++;
					}
				}

				if (!out[node])
				{
					out[node] = true;
					t1out_len[depth + 1]++;
					t1out_len_c[depth + 1][node_c]++;
					if (in[node]) {
						t1both_len[depth + 1]++;
						t1both_len_c[depth + 1][node_c]++;
					}
				}
			}

			//Updating terminal sets
			uint32_t i, other, other_c;
			for (i = 0; i < g1->InEdgeCount(node); i++)
			{
				other = g1->GetInEdge(node, i);
				if (!in[other])
				{
					other_c = class_1[other];
					in[other] = true;
					if(depth < limit_level)
					{
						t1in_len[depth + 1]++;
						t1in_len_c[depth + 1][other_c]++;
					}
					if (!inserted[other])
					{
						if (predecessors[other] == NULL_NODE)
						{
							dir[other] = NODE_DIR_IN;
							predecessors[other] = node;
						}
					}
					if (out[other] && (depth < limit_level)) {
						t1both_len[depth + 1]++;
						t1both_len_c[depth + 1][other_c]++;
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
					if(depth < limit_level)
					{
						t1out_len[depth + 1]++;
						t1out_len_c[depth + 1][other_c]++;
					}
					if (!inserted[other])
					{
						if (predecessors[other] == NULL_NODE)
						{
							predecessors[other] = node;
							dir[other] = NODE_DIR_OUT;
						}
					}
					if (in[other] && (depth < limit_level)) {
						t1both_len[depth + 1]++;
						t1both_len_c[depth + 1][other_c]++;
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
		bool VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>::
			NextPair(nodeID_t *pn1, nodeID_t *pn2, nodeID_t prev_n1, nodeID_t prev_n2)
	{

		nodeID_t curr_n1;
		nodeID_t pred_pair; //Node mapped with the predecessor
		nodeID_t pred_set_size = 0;
		uint32_t c = 0;
		pred_pair = NULL_NODE;

		//core_len indica la profondondita' della ricerca
		curr_n1 = order[core_len];
		c = class_1[curr_n1];

		nodeID_t pred = predecessors[curr_n1];

		if (pred != NULL_NODE)
		{
			if (prev_n2 == NULL_NODE)
				last_candidate_index = 0;
			else {
				last_candidate_index++; //Next Element
			}

			pred_pair = core_1[pred];
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
	 * bool VF3KSubState::IsFeasiblePair(node1, node2)
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
		bool VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>
			::IsFeasiblePair(nodeID_t node1, nodeID_t node2)
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
		uint32_t termout2 = 0, termin2 = 0, new2 = 0;
		if(core_len < limit_level)
		{
			memset(termin2_c, 0, classes_count * sizeof(uint32_t));
			memset(termout2_c, 0, classes_count * sizeof(uint32_t));
			memset(new2_c, 0, classes_count * sizeof(uint32_t));
		}

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
		for (i = 0; i < g2->OutEdgeCount(node2); i++)
		{
			other2 = g2->GetOutEdge(node2, i);
			c_other = class_2[other2];
			if (core_2[other2] != NULL_NODE)
			{
				if(induced)
				{
					other1 = core_2[other2];
					if (!g1->HasEdge(node1, other1))
						return false;
				}
			}
			else
			{
				if(core_len < limit_level)
				{
					if (in_2[other2]) {
						termin2++;
						termin2_c[c_other]++;
					}
					if (out_2[other2]) {
						termout2++;
						termout2_c[c_other]++;
					}
					if (!in_2[other2] && !out_2[other2]) {
						new2++;
						new2_c[c_other]++;
					}
				}
			}
		}

		// Check the 'in' edges of node2
		for (i = 0; i < g2->InEdgeCount(node2); i++)
		{
			other2 = g2->GetInEdge(node2, i);
			c_other = class_2[other2];
			if (core_2[other2] != NULL_NODE)
			{
				if(induced)
				{
					other1 = core_2[other2];
					if (!g1->HasEdge(other1, node1))
						return false;
				}
			}
			else
			{
				if(core_len < limit_level)
				{
					if (in_2[other2]) {
						termin2++;
						termin2_c[c_other]++;
					}
					if (out_2[other2]) {
						termout2++;
						termout2_c[c_other]++;
					}
					if (!in_2[other2] && !out_2[other2]) {
						new2++;
						new2_c[c_other]++;
					}
				}
			}
		}

		//Look-ahead check
		if(core_len < limit_level)
		{
			if (termin1[core_len] <= termin2 && termout1[core_len] <= termout2) {
				for (i = 0; i < classes_count; i++) {
					if (termin1_c[core_len][i] > termin2_c[i] ||
						termout1_c[core_len][i] > termout2_c[i]) {
						return false;
					}
				}
			}
			else return false;

			if (new1[core_len] <= new2)
			{
				for (i = 0; i < classes_count; i++) {
					if (new1_c[core_len][i] > new2_c[i])
						return false;
				}
			}
			else return false;
		}

		//std::cout << "\nIs Feasible: " << node1 << " " << node2;
		return true;

	}



	/*--------------------------------------------------------------
	 * void VF3KSubState::AddPair(node1, node2)
	 * Adds a pair to the Core set of the state.
	 * Precondition: the pair must be feasible
	 -------------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		void VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>::
		AddPair(nodeID_t node1, nodeID_t node2)
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
		uint32_t node_c = class_1[node1];
		core_len_c[node_c]++;

		//Inserting nodes into the core set
		core_1[node1] = node2;
		core_2[node2] = node1;


		//Checking if node2 is not in T2_in
		if(core_len < limit_level)
		{
			if (!in_2[node2])
			{
				in_2[node2] = core_len;
				t2in_len++;
				t2in_len_c[node_c]++;
				if (out_2[node2]) {
					t2both_len++;
					t2both_len_c[node_c]++;
				}
			}

			//Checking if node2 is not in T2_in
			if (!out_2[node2])
			{
				out_2[node2] = core_len;
				t2out_len++;
				t2out_len_c[node_c]++;
				if (in_2[node2]) {
					t2both_len++;
					t2both_len_c[node_c]++;
				}
			}

			//Evaluation of the neighborhood
			uint32_t i, other, other_c;
			other_c = -1;

			for (i = 0; i < g2->InEdgeCount(node2); i++)
			{
				other = g2->GetInEdge(node2, i);
				if (!in_2[other])
				{
					other_c = class_2[other];
					in_2[other] = core_len;
					//in2_set[other_c].push_back(other);
					t2in_len++;
					t2in_len_c[other_c]++;
					if (out_2[other]) {
						t2both_len++;
						t2both_len_c[other_c]++;
					}
				}
			}

			for (i = 0; i < g2->OutEdgeCount(node2); i++)
			{
				other = g2->GetOutEdge(node2, i);
				if (!out_2[other])
				{
					other_c = class_2[other];
					out_2[other] = core_len;
					//out2_set[other_c].push_back(other);
					t2out_len++;
					t2out_len_c[other_c]++;
					if (in_2[other]) {
						t2both_len++;
						t2both_len_c[other_c]++;
					}
				}
			}
		}

	}

	/*----------------------------------------------------------------
	 * Undoes the changes to the shared vectors made by the
	 * current state. Assumes that at most one AddPair has been
	 * performed.
	 ----------------------------------------------------------------*/
	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		void VF3KSubState<Node1, Node2, Edge1, Edge2, NodeComparisonFunctor, EdgeComparisonFunctor>::BackTrack()
	{

		/*std::cout<<"\nBT:";
		 print_core(core_1,core_2,n1);
		 std::cout<<" -> "<< added_node1 <<":"<< core_1[added_node1];*/

		assert(core_len - orig_core_len <= 1);
		assert(added_node1 != NULL_NODE);

		uint32_t other_c = 0;
		uint32_t node_c = class_1[added_node1];

		if (orig_core_len < core_len)
		{
			uint32_t i, node2;
			node2 = core_1[added_node1];

			if(core_len < limit_level)
			{
				if (in_2[node2] == core_len) {
					in_2[node2] = 0;
					//in2_set[node_c].erase(node2);
					t2in_len_c[node_c]--;
					if (out_2[node2])
						t2both_len_c[node_c]--;
				}

				if (out_2[node2] == core_len) {
					out_2[node2] = 0;
					//out2_set[node_c].erase(node2);
					t2out_len_c[node_c]--;
					if (in_2[node2])
						t2both_len_c[node_c]--;

				}

				//Backtraking neightborhood
				for (i = 0; i < g2->InEdgeCount(node2); i++)
				{
					uint32_t other = g2->GetInEdge(node2, i);
					other_c = class_2[other];
					if (in_2[other] == core_len) {
						in_2[other] = 0;
						//in2_set[other_c].erase(other);
						t2in_len_c[other_c] --;
						if (out_2[other])
							t2both_len_c[other_c]--;
					}
				}

				for (i = 0; i < g2->OutEdgeCount(node2); i++)
				{
					uint32_t other = g2->GetOutEdge(node2, i);
					other_c = class_2[other];
					if (out_2[other] == core_len) {
						out_2[other] = 0;
						//out2_set[other_c].erase(other);
						t2out_len_c[other_c] --;
						if (in_2[other])
							t2both_len_c[other_c]--;
					}
				}
			}

			core_1[added_node1] = NULL_NODE;
			core_2[node2] = NULL_NODE;

			core_len = orig_core_len;
			core_len_c[node_c]--;
			added_node1 = NULL_NODE;
		}
	}

	template <typename Node1, typename Node2,
		typename Edge1, typename Edge2,
		typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
		bool VF3KSubState<Node1, Node2, Edge1, Edge2,
		NodeComparisonFunctor, EdgeComparisonFunctor>::IsDead() const {

		if(core_len < limit_level)
		{
			if (t1both_len[core_len] > t2both_len ||
				t1out_len[core_len] > t2out_len ||
				t1in_len[core_len] > t2in_len) {
				return true;
			}

			for (uint32_t c = 0; c < classes_count; c++) {
				if (t1both_len_c[core_len][c] > t2both_len_c[c] ||
					t1out_len_c[core_len][c] > t2out_len_c[c] ||
					t1in_len_c[core_len][c] > t2in_len_c[c]) {
					return true;
				}
			}
		}

		return false;
	}
}
#endif
