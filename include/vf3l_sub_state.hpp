//
//  vf3_sub_state.hpp
//  VF2Plus
//
//  Created by Vincenzo Carletti on 12/11/14.
//  Copyright (c) 2014 Vincenzo Carletti. All rights reserved.
//

#ifndef VF3_SUB_STATE_HPP
#define VF3_SUB_STATE_HPP

#include <cstring>
#include <iostream>
#include <vector>
#include "argraph.hpp"

typedef unsigned char node_dir_t;
#define NODE_DIR_NONE 0
#define NODE_DIR_IN	1
#define NODE_DIR_OUT 2
#define NODE_DIR_BOTH 3

using namespace std;

/*static void print_core(node_id* n1, node_id* n2, int size){
  for(int k = 0; k < size; k++){
    if(n1[k] != NULL_NODE )
      std::cout<<n2[n1[k]]<<","<<n1[k]<<":";
    
  }
}*/

/*----------------------------------------------------------
 * class VF3LSubState
 * A representation of the SSR current state
 * See vf2_state.cc for more details.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor = EqualityComparator<Node1, Node2>,
typename EdgeComparisonFunctor = EqualityComparator<Edge1, Edge2> >
class VF3LSubState
{
private:
  //Comparison functors for nodes and edges
  NodeComparisonFunctor nf;
  EdgeComparisonFunctor ef;
  
  //Graphs to analyze
  ARGraph<Node1, Edge1> *g1;
  ARGraph<Node2, Edge2> *g2;
  
  const VF3LSubState* parent;
  bool used;
  
  //Size of each graph
  int n1, n2;
  
  node_id *order;     //Order to traverse node on the first graph
  
  //CORE SET SIZES
  int core_len;       //Current lenght of the core set
  int orig_core_len;  //Core set lenght of the previous state
  int *core_len_c;    //Core set lenght for each class
  
  int added_node1;    //Last added node
  
  node_dir_t* dir;        //Node coming set. Direction into the terminal set.
  node_id* predecessors;  //Previous node in the ordered sequence connected to a node
  
  node_id *core_1;
  node_id *core_2;
  
  //Vector of sets used for searching the successors
  //Each class has its set
  int last_candidate_index;
  
  /* Structures for classes */
  int *class_1;       //Classes for nodes of the first graph
  int *class_2;       //Classes for nodes of the first graph
  int classes_count;  //Number of classes
  
  long *share_count;  //Count the number of instances sharing the common sets
  
  //PRIVATE METHODS
  void BackTrack();
  void ComputeFirstGraphTraversing();
  void print_terminal(int c);
  
public:
  static long long instance_count;
  VF3LSubState(ARGraph<Node1, Edge1> *g1, ARGraph<Node2, Edge2> *g2,
                int* class_1, int* class_2, int nclass,
                node_id* order = NULL);
  VF3LSubState(const VF3LSubState &state);
  ~VF3LSubState();
  ARGraph<Node1, Edge1> *GetGraph1() { return g1; }
  ARGraph<Node2, Edge2> *GetGraph2() { return g2; }
  bool NextPair(node_id *pn1, node_id *pn2,
                node_id prev_n1=NULL_NODE, node_id prev_n2=NULL_NODE);
  bool IsFeasiblePair(node_id n1, node_id n2);
  void AddPair(node_id n1, node_id n2);
  bool IsGoal() { return core_len==n1; };
  bool IsDead();
  int CoreLen() { return core_len; }
  void GetCoreSet(node_id c1[], node_id c2[]);
  VF3LSubState* GetParent();
  bool IsUsed(){return used;}
  void SetUsed(){used = true;}
};


/*----------------------------------------------------------
 * VF3LSubState::VF3LSubState(g1, g2)
 * Constructor. Makes an empty state.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::VF3LSubState(ARGraph<Node1, Edge1> *ag1, ARGraph<Node2, Edge2> *ag2, int* class_1, int* class_2, int nclass, node_id* order)
{
  assert(class_1!=NULL && class_2!=NULL);
  
  VF3LSubState::instance_count=1;
  g1=ag1;
  g2=ag2;
  n1=g1->NodeCount();
  n2=g2->NodeCount();
  last_candidate_index = 0;
  
  this->order = order;
  this->class_1 = class_1;
  this->class_2 = class_2;
  this->classes_count = nclass;
  parent = NULL;
  used = false;
  core_len=orig_core_len=0;
  
  
  core_len_c = (int*) calloc(classes_count, sizeof(int));
  
  added_node1=NULL_NODE;
  
  core_1=new node_id[n1];
  core_2=new node_id[n2];
  dir = new node_dir_t[n1];
  predecessors = new node_id[n1];
  share_count = new long;
  
  int i;
  for(i=0; i<n1; i++)
      core_1[i]=NULL_NODE;
  
  for(i=0; i<n2; i++)
    core_2[i]=NULL_NODE;
  
  ComputeFirstGraphTraversing();
  *share_count = 1;
}


/*----------------------------------------------------------
 * VF3LSubState::VF3LSubState(state)
 * Copy constructor.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::  VF3LSubState(const VF3LSubState &state)
{ g1=state.g1;
  g2=state.g2;
  n1=state.n1;
  n2=state.n2;
  
  order=state.order;
  class_1 = state.class_1;
  class_2 = state.class_2;
  classes_count = state.classes_count;
  parent = &state;
  used = false;
  VF3LSubState::instance_count++;
  
  last_candidate_index = state.last_candidate_index;
  
  core_len=orig_core_len=state.core_len;
  core_len_c = state.core_len_c; 
  added_node1=NULL_NODE;
  
  core_1=state.core_1;
  core_2=state.core_2;
  dir = state.dir;
  predecessors = state.predecessors;
  share_count=state.share_count;
  
  ++ *share_count;
  
}


/*---------------------------------------------------------------
 * VF3LSubState::~VF3LSubState()
 * Destructor.
 --------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::~VF3LSubState()
{
  
  if(-- *share_count > 0)
    BackTrack();
  
  if (*share_count == 0)
    { delete [] core_1;
      delete [] core_2;
      delete [] dir;
      delete [] predecessors;
      delete [] core_len_c;
      delete share_count;
    }
}

//Provare ad avere in1 ed ou1 predeterminati, senza doverlo calcolare ad ogni iterazione
//La loro dimensione ad ogni livello dell'albero di ricerca e' predeterminato
//In questo modo mi basta conoscere solo l'ordine di scelta e la dimensione di in1 ed out1
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::ComputeFirstGraphTraversing(){
  //The algorithm start with the node with the maximum degree
  node_id depth, i;
  node_id node;	//Current Node
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
    dir[i] = NODE_DIR_NONE;
    inserted[i] = false;
    predecessors[i] = NULL_NODE;
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
bool VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::NextPair(node_id *pn1, node_id *pn2,node_id prev_n1, node_id prev_n2)
{
  
  node_id curr_n1;
  node_id pred_pair; //Node mapped with the predecessor
  node_id pred_set_size = 0;
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
 * bool VF3LSubState::IsFeasiblePair(node1, node2)
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
bool VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::IsFeasiblePair(node_id node1, node_id node2)
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
 * void VF3LSubState::AddPair(node1, node2)
 * Adds a pair to the Core set of the state.
 * Precondition: the pair must be feasible
 -------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::AddPair(node_id node1, node_id node2)
{
  
  /*std::cout<<"\nAP:";
  print_core(core_1,core_2,n1);
  std::cout<<" <- "<< node1 <<":"<< node2;*/
  
  assert(node1<n1);
  assert(node2<n2);
  assert(core_len<n1);
  assert(core_len<n2);
  assert(class_1[node1] == class_2[node2]);
  
  //Updating the core lenght
  core_len++;
  added_node1=node1;
  int node_c = class_1[node1];
  core_len_c[node_c]++;
  
  //Inserting nodes into the core set
  core_1[node1]=node2;
  core_2[node2]=node1;
  
}



/*--------------------------------------------------------------
 * void VF3LSubState::GetCoreSet(c1, c2)
 * Reads the core set of the state into the arrays c1 and c2.
 * The i-th pair of the mapping is (c1[i], c2[i])
 --------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::GetCoreSet(node_id c1[], node_id c2[])
{
  int i,j;
  for (i=0,j=0; i<n1; i++)
    if (core_1[i] != NULL_NODE)
      { c1[j]=i;
        c2[j]=core_1[i];
        j++;
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
void VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::BackTrack()
{
  
  /*std::cout<<"\nBT:";
   print_core(core_1,core_2,n1);
   std::cout<<" -> "<< added_node1 <<":"<< core_1[added_node1];*/
  
  assert(core_len - orig_core_len <= 1);
  if(added_node1 != NULL_NODE)
  {
  int other_c = 0;
  int node_c = class_1[added_node1];
  
  if (orig_core_len < core_len)
    { int i, node2;
      node2 = core_1[added_node1];
      
      core_1[added_node1] = NULL_NODE;
      core_2[node2] = NULL_NODE;
      
      core_len=orig_core_len;
      core_len_c[node_c]--;
      added_node1 = NULL_NODE;
    }
  }
}

template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
bool VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::IsDead() {
  
  if (n1 > n2){
    return true;
  }
  return false;
}


template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>* VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::GetParent()
{
  return (VF3LSubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>*)parent;
}

#endif
