//
//  vf3_sub_state.hpp
//  VF2Plus
//
//  Created by Vincenzo Carletti on 12/11/14.
//  Copyright (c) 2014 Vincenzo Carletti. All rights reserved.
//

#ifndef VF3_PARALLEL_SUB_STATE_HPP
#define VF3_PARALLEL_SUB_STATE_HPP

#include <cstring>
#include <iostream>
#include <vector>
#include "ARGraph.hpp"

typedef unsigned char node_dir_t;
#define NODE_DIR_NONE 0
#define NODE_DIR_IN	1
#define NODE_DIR_OUT 2
#define NODE_DIR_BOTH 3

namespace vflib
{

/*----------------------------------------------------------
 * class VF3ParallelSubState
 * A representation of the SSR current state
 * See vf2_state.cc for more details.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor = EqualityComparator<Node1, Node2>,
typename EdgeComparisonFunctor = EqualityComparator<Edge1, Edge2> >
class VF3ParallelSubState
{
private:
  //Comparison functors for nodes and edges
  NodeComparisonFunctor nf;
  EdgeComparisonFunctor ef;

  //Graphs to analyze
  ARGraph<Node1, Edge1> *g1;
  ARGraph<Node2, Edge2> *g2;

  //Size of each graph
  int n1, n2;

  nodeID_t *order;     //Order to traverse node on the first graph

  //CORE SET SIZES
  int core_len;       //Current length of the core set
  int orig_core_len;  //Core set length of the previous state
  //int *core_len_c;    //Core set length for each class

  int added_node1;    //Last added node

  //nodeID_t* predecessors;  //Previous node in the ordered sequence connected to a node

  std::vector<nodeID_t> core_1;
  std::vector<nodeID_t> core_2;
  std::vector<int32_t> core_len_c;
  std::vector<nodeID_t> predecessors;
  std::vector<node_dir_t> dir;

  //Vector of sets used for searching the successors
  //Each class has its set
  int last_candidate_index;

  /* Structures for classes */
  uint32_t *class_1;       //Classes for nodes of the first graph
  uint32_t *class_2;       //Classes for nodes of the first graph
  uint32_t classes_count;  //Number of classes

  //PRIVATE METHODS
  void BackTrack();
  void ComputeFirstGraphTraversing();
  void print_terminal(int c);

public:
  static long long instance_count;
  VF3ParallelSubState(){}
  VF3ParallelSubState(ARGraph<Node1, Edge1> *g1, ARGraph<Node2, Edge2> *g2,
		  uint32_t* class_1, uint32_t* class_2, uint32_t nclass,
                nodeID_t* order = NULL);
  VF3ParallelSubState(const VF3ParallelSubState &state);
  ~VF3ParallelSubState(){}
  VF3ParallelSubState& operator=(const VF3ParallelSubState& state);
  ARGraph<Node1, Edge1> *GetGraph1() { return g1; }
  ARGraph<Node2, Edge2> *GetGraph2() { return g2; }
  bool NextPair(nodeID_t *pn1, nodeID_t *pn2, nodeID_t prev_n1=NULL_NODE, nodeID_t prev_n2=NULL_NODE);
  bool IsFeasiblePair(nodeID_t n1, nodeID_t n2);
  void AddPair(nodeID_t n1, nodeID_t n2);
  inline bool IsGoal() { return core_len==n1; };
  inline bool IsDead(){return false; };

  int CoreLen() { return core_len; }
  
  inline void GetCoreSet(std::vector<std::pair<nodeID_t, nodeID_t> >& core)
	{
		uint32_t i;
		core.resize(n1);
		for (i = 0; i < n1; i++)
		{
			if (core_1[i] != NULL_NODE)
			{
				core[i] = std::pair<nodeID_t, nodeID_t>(i, core_1[i]);
			}
		}
	}
};


/*----------------------------------------------------------
 * VF3ParallelSubState::VF3ParallelSubState(g1, g2)
 * Constructor. Makes an empty state.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3ParallelSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>
	::VF3ParallelSubState(ARGraph<Node1, Edge1> *ag1, ARGraph<Node2, Edge2> *ag2,
			uint32_t* class_1, uint32_t* class_2, uint32_t nclass, nodeID_t* order):
      core_1(ag1->NodeCount()),
      core_2(ag2->NodeCount()),
      core_len_c(nclass),
      predecessors(ag1->NodeCount()),
      dir(ag1->NodeCount())
{
  assert(class_1!=NULL && class_2!=NULL);

  g1=ag1;
  g2=ag2;
  n1=g1->NodeCount();
  n2=g2->NodeCount();
  last_candidate_index = 0;

  this->order = order;
  this->class_1 = class_1;
  this->class_2 = class_2;
  this->classes_count = nclass;
  core_len=orig_core_len=0;

  added_node1=NULL_NODE;

  int i;
  for(i=0; i<n1; i++)
  {
      core_1[i]=NULL_NODE;
      dir[i] = NODE_DIR_NONE;
      predecessors[i] = NULL_NODE;
  }

  for(i=0; i<n2; i++)
    core_2[i]=NULL_NODE;

  ComputeFirstGraphTraversing();
}


/*----------------------------------------------------------
 * VF3ParallelSubState::VF3ParallelSubState(state)
 * Copy constructor.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3ParallelSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::
	VF3ParallelSubState(const VF3ParallelSubState &state):
  //Qui clono tutti gli insiemi
	core_1(state.core_1), core_2(state.core_2), core_len_c(state.core_len_c),
	predecessors(state.predecessors), dir(state.dir)
{
  //*this = state;
  g1=state.g1;
  g2=state.g2;
  n1=state.n1;
  n2=state.n2;

  order=state.order;
  class_1 = state.class_1;
  class_2 = state.class_2;
  classes_count = state.classes_count;
  last_candidate_index = state.last_candidate_index;
  core_len=orig_core_len=state.core_len;
  added_node1=NULL_NODE;
}



template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3ParallelSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>&
	VF3ParallelSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>
	::operator=(const VF3ParallelSubState &state)
{
	if(this != &state)
	{
	  core_1.empty();
	  core_2.empty();
	  core_len_c.empty();
	  predecessors.empty();
	  dir.empty();

	  core_1.insert(core_1.begin(), state.core_1.begin(), state.core_1.end());
	  core_2.insert(core_2.begin(), state.core_2.begin(), state.core_2.end());
	  core_len_c.insert(core_len_c.begin(), state.core_len_c.begin(), state.core_len_c.end());
	  predecessors.insert(predecessors.begin(), state.predecessors.begin(), state.predecessors.end());
	  dir.insert(dir.begin(), state.dir.begin(), state.dir.end());

	  g1=state.g1;
	  g2=state.g2;
	  n1=state.n1;
	  n2=state.n2;

	  order=state.order;
	  class_1 = state.class_1;
	  class_2 = state.class_2;
	  classes_count = state.classes_count;
	  last_candidate_index = state.last_candidate_index;
	  core_len=orig_core_len=state.core_len;
	}
	return *this;
}

//Provare ad avere in1 ed ou1 predeterminati, senza doverlo calcolare ad ogni iterazione
//La loro dimensione ad ogni livello dell'albero di ricerca e' predeterminato
//In questo modo mi basta conoscere solo l'ordine di scelta e la dimensione di in1 ed out1
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3ParallelSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::ComputeFirstGraphTraversing(){
  //The algorithm start with the node with the maximum degree
  nodeID_t depth, i;
  nodeID_t node;	//Current Node
  int node_c; //Class of the current node
  bool* inserted = new bool[n1];
  bool *in, *out; //Internal Terminal Set used for updating the size of
  in = new bool[n1];
  out = new bool[n1];

  //Init vectors and variables
  node = 0;
  node_c = 0;

  for(i = 0; i < n1; i++)
    {
    in[i] = false;
    out[i] = false;
    inserted[i] = false;
    }

  /* Following the imposed node order */
  for(depth = 0; depth < n1; depth++)
    {
    node = order[depth];
    node_c = class_1[node];
    inserted[node] = true;

    //Inserting the node
    //Terminal set sizes depends on the depth
    // < depth non sono nell'insieme
    // >= depth sono nell'insieme
    if (!in[node])
      in[node]=true;

    if (!out[node])
      out[node]=true;

    //Updating terminal sets
    int i, other, other_c;
    other_c = -1;
    for(i=0; i<g1->InEdgeCount(node); i++)
      {
      other=g1->GetInEdge(node, i);
      if (!in[other])
        {
        other_c = class_1[other];
        in[other]=true;
        if(!inserted[other])
        {
          if(predecessors[other] == NULL_NODE)
          {
            dir[other] = NODE_DIR_IN;
            predecessors[other] = node;
          }
        }
        }
      }

    for(i=0; i<g1->OutEdgeCount(node); i++)
      {
      other=g1->GetOutEdge(node, i);
      if (!out[other])
        {
        other_c = class_1[other];
        out[other]=true;
        if(!inserted[other])
        {
          if(predecessors[other] == NULL_NODE)
          {
            predecessors[other] = node;
            dir[other] = NODE_DIR_OUT;
          }
        }
        }
      }
    }

  delete [] in;
  delete [] out;
  delete [] inserted;
}

template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
bool VF3ParallelSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>
	::NextPair(nodeID_t *pn1, nodeID_t *pn2,nodeID_t prev_n1, nodeID_t prev_n2)
{

  nodeID_t curr_n1;
  nodeID_t pred_pair; //Node mapped with the predecessor
  nodeID_t pred_set_size = 0;
  int c = 0;
  pred_pair = NULL_NODE;

  //core_len indica la profondondita' della ricerca
  curr_n1 = order[core_len];
  c = class_1[curr_n1];

  if(predecessors[curr_n1] != NULL_NODE)
    {
    if (prev_n2 == NULL_NODE)
      last_candidate_index = 0;
    else{
      last_candidate_index++; //Next Element
    }

    pred_pair = core_1[predecessors[curr_n1]];
    switch (dir[curr_n1])
      {
        case NODE_DIR_IN:
        pred_set_size = g2->InEdgeCount(pred_pair);

        while(last_candidate_index < pred_set_size)
          {
            prev_n2 = g2->GetInEdge(pred_pair,last_candidate_index);
            if(core_2[prev_n2] != NULL_NODE || class_2[prev_n2] != c)
              last_candidate_index++;
            else
              break;
          }

        break;

        case NODE_DIR_OUT:
        pred_set_size = g2->OutEdgeCount(pred_pair);

        while(last_candidate_index < pred_set_size)
          {
            prev_n2 = g2->GetOutEdge(pred_pair,last_candidate_index);
            if(core_2[prev_n2] != NULL_NODE || class_2[prev_n2] != c)
              last_candidate_index++;
            else
              break;
          }

        break;
      }

    if(last_candidate_index >= pred_set_size)
      return false;

    }
  else
    {
    //Recupero il nodo dell'esterno
    if(prev_n2 == NULL_NODE)
      prev_n2 = 0;
    else
      prev_n2++;

    while (prev_n2<n2 &&
           (core_2[prev_n2]!=NULL_NODE
            || class_2[prev_n2] != c) )
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
 * bool VF3ParallelSubState::IsFeasiblePair(node1, node2)
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
bool VF3ParallelSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::IsFeasiblePair(nodeID_t node1, nodeID_t node2)
{
  //std::cout<<"\nIF: " <<node1<<" " << node2;
  //print_core(core_1, core_2, core_len);
  assert(node1<n1);
  assert(node2<n2);
  assert(core_1[node1]==NULL_NODE);
  assert(core_2[node2]==NULL_NODE);

  if(!nf(g1->GetNodeAttr(node1), g2->GetNodeAttr(node2)))
    return false;

  if(g1->InEdgeCount(node1) > g2->InEdgeCount(node2)
    || g1->OutEdgeCount(node1) > g2->OutEdgeCount(node2))
    return false;

  int i, other1, other2, c_other;
  Edge1 eattr1;
  Edge2 eattr2;

  // Check the 'out' edges of node1
  for(i=0; i<g1->OutEdgeCount(node1); i++)
    { other1=g1->GetOutEdge(node1, i, eattr1);
      c_other = class_1[other1];
      if (core_1[other1] != NULL_NODE)
        { other2=core_1[other1];
          if (!g2->HasEdge(node2, other2, eattr2) ||
              !ef(eattr1, eattr2))
            return false;
        }
    }

  // Check the 'in' edges of node1
  for(i=0; i<g1->InEdgeCount(node1); i++)
    { other1=g1->GetInEdge(node1, i, eattr1);
      c_other = class_1[other1];
      if (core_1[other1]!=NULL_NODE)
        { other2=core_1[other1];
          if (!g2->HasEdge(other2, node2, eattr2) ||
              !ef(eattr1, eattr2))
            return false;
        }
    }


  // Check the 'out' edges of node2
  for(i=0; i<g2->OutEdgeCount(node2); i++)
    { other2=g2->GetOutEdge(node2, i);
      c_other = class_2[other2];
      if (core_2[other2]!=NULL_NODE)
        { other1=core_2[other2];
          if (!g1->HasEdge(node1, other1))
            return false;
        }
   }

  // Check the 'in' edges of node2
  for(i=0; i<g2->InEdgeCount(node2); i++)
    { other2=g2->GetInEdge(node2, i);
      c_other = class_2[other2];
      if (core_2[other2] != NULL_NODE)
        { other1=core_2[other2];
          if (!g1->HasEdge(other1, node1))
            return false;
        }
   }

  //std::cout << "\nIs Feasible: " << node1 << " " << node2;
  return true;

}



/*--------------------------------------------------------------
 * void VF3ParallelSubState::AddPair(node1, node2)
 * Adds a pair to the Core set of the state.
 * Precondition: the pair must be feasible
 -------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3ParallelSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,
EdgeComparisonFunctor>::AddPair(nodeID_t node1, nodeID_t node2)
{

  /*std::cout<<"\nAP:";
  print_core(core_1,core_2,n1);
  std::cout<<" <- "<< node1 <<":"<< node2;*/

  assert(node1<n1);
  assert(node2<n2);
  assert(core_len<n1);
  assert(core_len<n2);
  assert(class_1[node1] == class_2[node2]);

  //Updating the core length
  core_len++;
  added_node1=node1;
  int node_c = class_1[node1];
  core_len_c[node_c]++;

  //Inserting nodes into the core set
  core_1[node1]=node2;
  core_2[node2]=node1;

}

}
#endif
