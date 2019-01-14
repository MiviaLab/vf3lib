//
//  vf3_sub_state.hpp
//  VF2Plus
//
//  Created by Vincenzo Carletti on 12/11/14.
//  Copyright (c) 2014 Vincenzo Carletti. All rights reserved.
//

#ifndef VF3_STATE_HPP
#define VF3_STATE_HPP

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

static void print_core(node_id* n1, node_id* n2, int size){
  for(int k = 0; k < size; k++){
    if(n1[k] != NULL_NODE )
      std::cout<<n2[n1[k]]<<","<<n1[k]<<":";
  }
}

/*----------------------------------------------------------
 * class VF3State
 * A representation of the SSR current state
 * See vf2_state.cc for more details.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor = EqualityComparator<Node1, Node2>,
typename EdgeComparisonFunctor = EqualityComparator<Edge1, Edge2> >
class VF3State
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
  
  node_id *order;     //Order to traverse node on the first graph
  
  //CORE SET SIZES
  int core_len;       //Current lenght of the core set
  int orig_core_len;  //Core set lenght of the previous state
  int *core_len_c;    //Core set lenght for each class
  
  int added_node1;    //Last added node
  
  node_dir_t* dir;        //Node coming set. Direction into the terminal set.
  node_id* predecessors;  //Previous node in the ordered sequence connected to a node
  
  //TERMINAL SET SIZE
  //BE AWARE: Core nodes are also counted by these
  //GLOBAL SIZE
  int t2in_len, t2both_len, t2out_len; //Len of Terminal set for the second graph
  int *t1in_len, *t1both_len, *t1out_len; //Len of Terminal set for the first graph for each level
                                          //SIZE FOR EACH CLASS
  int *t2both_len_c, *t2in_len_c, *t2out_len_c;     //Len of Terminal set for the second graph for each class
  int **t1both_len_c, **t1in_len_c, **t1out_len_c;  //Len of Terminal set for the first graph for each class end level
  
  //Used for terminal set size evaluation
  int *termout2_c, *termin2_c, *new2_c;
  int **termout1_c, **termin1_c, **new1_c;
  
  int *termin1, *termout1, *new1;
  
  node_id *core_1;
  node_id *core_2;
  
  //Terminal sets of the second graph
  //TERM IN
  node_id *in_2;
  //TERMI OUT
  node_id *out_2;
  
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
  void UpdateTerminalSetSize(node_id node, node_id level, bool* in_1, bool* out_1, bool* inserted);
  void print_terminal(int c);
  
public:
  static long long  instance_count;
  VF3State(ARGraph<Node1, Edge1> *g1, ARGraph<Node2, Edge2> *g2,
                int* class_1, int* class_2, int nclass,
                node_id* order = NULL);
  VF3State(const VF3State &state);
  ~VF3State();
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
};

/*----------------------------------------------------------
 * Methods of the class VF3State
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::print_terminal(int c){
  std::cout<<"\nClass: " << c << " Core_len: " << core_len_c[c];
  std::cout<<" t1both_len: " << t1both_len_c[core_len][c] << " t2both_len " << t2both_len_c[c];
  std::cout<<" t1out_len: " << t1out_len_c[core_len][c] << " t2out_len " << t2out_len_c[c];
  std::cout<<" t1in_len: " << t1in_len_c[core_len][c] << " t2in_len " << t2in_len_c[c];
  
}

/*----------------------------------------------------------
 * VF3State::VF3State(g1, g2)
 * Constructor. Makes an empty state.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::VF3State(ARGraph<Node1, Edge1> *ag1, ARGraph<Node2, Edge2> *ag2, int* class_1, int* class_2, int nclass, node_id* order)
{
  assert(class_1!=NULL && class_2!=NULL);
  
  VF3State::instance_count=1;
  g1=ag1;
  g2=ag2;
  n1=g1->NodeCount();
  n2=g2->NodeCount();
  
  this->order = order;
  this->class_1 = class_1;
  this->class_2 = class_2;
  this->classes_count = nclass;
  
  core_len=orig_core_len=0;
  t2both_len=t2in_len=t2out_len=0;
  
  //Creazione degli insiemi
  t1both_len = new int[n1+1];
  t1in_len = new int[n1+1];
  t1out_len = new int[n1+1];
  
  termin1 = (int*) calloc(n1, sizeof(int));
  termout1 = (int*) calloc(n1, sizeof(int));
  new1 = (int*) calloc(n1, sizeof(int));
  
  t1both_len_c = (int**)malloc((n1+1)*sizeof(int*));
  t1in_len_c = (int**)malloc((n1+1)*sizeof(int*));
  t1out_len_c = (int**)malloc((n1+1)*sizeof(int*));
  
  
  termin1_c = (int**)malloc(n1*sizeof(int*));
  termout1_c = (int**)malloc(n1*sizeof(int*));
  new1_c = (int**)malloc(n1*sizeof(int*));
  
  core_len_c = (int*) calloc(classes_count, sizeof(int));
  t2both_len_c = (int*) calloc(classes_count, sizeof(int));
  t2in_len_c = (int*) calloc(classes_count, sizeof(int));
  t2out_len_c = (int*) calloc(classes_count, sizeof(int));
  termout2_c = new int[classes_count];
  termin2_c = new int[classes_count];
  new2_c = new int[classes_count];
  
  added_node1=NULL_NODE;
  
  core_1=new node_id[n1];
  core_2=new node_id[n2];
  in_2=new node_id[n2];
  out_2=new node_id[n2];
  dir = new node_dir_t[n1];
  predecessors = new node_id[n1];
  share_count = new long;
  
  int i;
  for(i=0; i<=n1; i++)
    {
    if(i<n1){
      core_1[i]=NULL_NODE;
        termin1_c[i] = (int*) calloc(classes_count, sizeof(int));
        termout1_c[i] = (int*) calloc(classes_count, sizeof(int));
        new1_c[i] = (int*) calloc(classes_count, sizeof(int));
    }
      t1both_len_c[i] = (int*) calloc(classes_count, sizeof(int));
      t1in_len_c[i] = (int*) calloc(classes_count, sizeof(int));
      t1out_len_c[i] = (int*) calloc(classes_count, sizeof(int));
    }
  
  for(i=0; i<n2; i++)
    {
    core_2[i]=NULL_NODE;
    in_2[i]=0;
    out_2[i]=0;
    }
  
  ComputeFirstGraphTraversing();
  *share_count = 1;
}


/*----------------------------------------------------------
 * VF3State::VF3State(state)
 * Copy constructor.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::VF3State(const VF3State &state)
{ g1=state.g1;
  g2=state.g2;
  n1=state.n1;
  n2=state.n2;
  
  order=state.order;
  class_1 = state.class_1;
  class_2 = state.class_2;
  classes_count = state.classes_count;
  VF3State::instance_count++;
  
  core_len=orig_core_len=state.core_len;
  
  t1in_len=state.t1in_len;
  t1out_len=state.t1out_len;
  t1both_len=state.t1both_len;
  
  t2in_len=state.t2in_len;
  t2out_len=state.t2out_len;
  t2both_len=state.t2both_len;
  
  core_len_c = state.core_len_c;
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
  
  added_node1=NULL_NODE;
  
  core_1=state.core_1;
  core_2=state.core_2;
  in_2=state.in_2;
  out_2=state.out_2;
  dir = state.dir;
  predecessors = state.predecessors;
  share_count=state.share_count;
  
  ++ *share_count;
  
}


/*---------------------------------------------------------------
 * VF3State::~VF3State()
 * Destructor.
 --------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::~VF3State()
{
  
  if(-- *share_count > 0)
    BackTrack();
  
  if (*share_count == 0)
    { delete [] core_1;
      delete [] core_2;
      delete [] in_2;
      delete [] out_2;
      delete [] dir;
      delete [] predecessors;
      delete [] t1both_len;
      delete [] t1in_len;
      delete [] t1out_len;
      delete [] termin1;
      delete [] termout1;
      delete [] new1;
      
      for(int i = 0; i <= n1; i++){
        delete [] t1both_len_c[i];
        delete [] t1in_len_c[i];
        delete [] t1out_len_c[i];
        if(i< n1){
          delete [] termin1_c[i];
          delete [] termout1_c[i];
          delete [] new1_c[i];
        }
      }
      
      delete [] t1both_len_c;
      delete [] t1in_len_c;
      delete [] t1out_len_c;
      delete [] termin1_c;
      delete [] termout1_c;
      delete [] new1_c;
      delete [] t2both_len_c;
      delete [] t2in_len_c;
      delete [] t2out_len_c;
      delete [] core_len_c;
      delete [] termin2_c;
      delete [] termout2_c;
      delete [] new2_c;
      
      delete share_count;
    }
}

template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,
EdgeComparisonFunctor>::UpdateTerminalSetSize(node_id node, node_id level, bool* in_1, bool* out_1, bool* inserted){
  node_id i, neigh, c_neigh;
  node_id in1_count, out1_count;
  
  //Updating Terminal set size count And degree
  in1_count = g1->InEdgeCount(node);
  out1_count = g1->OutEdgeCount(node);
  
  //Updating Inner Nodes not yet inserted
  for (i = 0; i < in1_count; i++)
    {
    //Getting Neighborhood
    neigh = g1->GetInEdge(node,i);
    c_neigh = class_1[neigh];
    
    if(!inserted[neigh])
      {
      if (in_1[neigh]){
        termin1[level]++;
        termin1_c[level][c_neigh]++;
      }
      if (out_1[neigh]){
        termout1[level]++;
        termout1_c[level][c_neigh]++;
      }
      if (!in_1[neigh] && !out_1[neigh]){
        new1[level]++;
        new1_c[level][c_neigh]++;
      }
      }
    }
  
  //Updating Outer Nodes not yet insered
  for (i = 0; i < out1_count; i++)
    {
    //Getting Neighborhood
    neigh = g1->GetOutEdge(node,i);
    c_neigh = class_1[neigh];
    if(!inserted[neigh])
      {
      if (in_1[neigh]){
        termin1[level]++;
        termin1_c[level][c_neigh]++;
      }
      if (out_1[neigh]){
        termout1[level]++;
        termout1_c[level][c_neigh]++;
      }
      if (!in_1[neigh] && !out_1[neigh]){
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
void VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::ComputeFirstGraphTraversing(){
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
  
  t1in_len[0] = 0;
  t1out_len[0] = 0;
  t1both_len[0] = 0;
  
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
    
    UpdateTerminalSetSize(node, depth, in, out, inserted);
    
    //Updating counters for next step
    t1in_len[depth +1] = t1in_len[depth];
    t1out_len[depth +1] = t1out_len[depth];
    t1both_len[depth +1] = t1both_len[depth];
    for (int j = 0; j < classes_count; j++)
      {
      t1in_len_c[depth +1][j] = t1in_len_c[depth][j];
      t1out_len_c[depth +1][j] = t1out_len_c[depth][j];
      t1both_len_c[depth +1][j] = t1both_len_c[depth][j];
      }
    //Inserting the node
    //Terminal set sizes depends on the depth
    // < depth non sono nell'insieme
    // >= depth sono nell'insieme
    if (!in[node])
      {
      in[node]=true;
      t1in_len[depth+1]++;
      t1in_len_c[depth+1][node_c]++;
      if (out[node]){
        t1both_len[depth+1]++;
        t1both_len_c[depth+1][node_c]++;
      }
      }
    
    if (!out[node])
      {
      out[node]=true;
      t1out_len[depth+1]++;
      t1out_len_c[depth+1][node_c]++;
      if (in[node]){
        t1both_len[depth+1]++;
        t1both_len_c[depth+1][node_c]++;
      }
      }
    
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
        t1in_len[depth+1]++;
        t1in_len_c[depth+1][other_c]++;
        if(!inserted[other])
        {
        //dir[other] = NODE_DIR_IN;
          if(predecessors[other] == NULL_NODE)
            {
            dir[other] = NODE_DIR_IN;
            predecessors[other] = node;
            }
        }
        if (out[other]){
          t1both_len[depth+1]++;
          t1both_len_c[depth+1][other_c]++;
          //if(!inserted[other])
          //dir[other] = NODE_DIR_BOTH;
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
        t1out_len[depth+1]++;
        t1out_len_c[depth+1][other_c]++;
        if(!inserted[other])
        {
        //dir[other] = NODE_DIR_OUT;
          if(predecessors[other] == NULL_NODE)
            {
            predecessors[other] = node;
            dir[other] = NODE_DIR_OUT;
            }
        }
        if (in[other]){
          t1both_len[depth+1]++;
          t1both_len_c[depth+1][other_c]++;
          //if(!inserted[other])
          //dir[other] = NODE_DIR_BOTH;
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
bool VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::NextPair(node_id *pn1, node_id *pn2,node_id prev_n1, node_id prev_n2)
{
  
  node_id curr_n1;
  node_id pred_pair; //Node mapped with the predecessor
  node_id pred_set_size = 0;
  int c = 0;
  
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
 * bool VF3State::IsFeasiblePair(node1, node2)
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
bool VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::IsFeasiblePair(node_id node1, node_id node2)
{
  std::cout<<"\nIF: " <<node1<<" " << node2;
  print_core(core_1, core_2, core_len);
  assert(node1<n1);
  assert(node2<n2);
  assert(core_1[node1]==NULL_NODE);
  assert(core_2[node2]==NULL_NODE);
  
  if(!nf(g1->GetNodeAttr(node1), g2->GetNodeAttr(node2)))
    return false;
  
  if(g1->InEdgeCount(node1) != g2->InEdgeCount(node2)
    || g1->OutEdgeCount(node1) != g2->OutEdgeCount(node2))
    return false;
  
  int i, other1, other2, c_other;
  Edge1 eattr1;
  Edge2 eattr2;
  int termout2=0, termin2=0, new2=0;
  memset(termin2_c,0,classes_count*sizeof(int));
  memset(termout2_c,0,classes_count*sizeof(int));
  memset(new2_c,0,classes_count*sizeof(int));
  
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
      else
        {
          if (in_2[other2]){
          termin2++;
          termin2_c[c_other]++;
          }
          if (out_2[other2]){
            termout2++;
            termout2_c[c_other]++;
          }
          if (!in_2[other2] && !out_2[other2]){
            new2++;
            new2_c[c_other]++;
          }
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
      else
        { if (in_2[other2]){
          termin2++;
          termin2_c[c_other]++;
        }
          if (out_2[other2]){
            termout2++;
            termout2_c[c_other]++;
          }
          if (!in_2[other2] && !out_2[other2]){
            new2++;
            new2_c[c_other]++;
          }
        }
    }
  
  //Look-ahead check
  if(termin1[core_len] == termin2 && termout1[core_len] == termout2){
    for(i = 0; i < classes_count; i++){
      if(termin1_c[core_len][i] != termin2_c[i] ||
         termout1_c[core_len][i] != termout2_c[i]){
        return false;
      }
    }
  }else return false;
  
  if(new1[core_len] == new2)
  {
      for(i = 0; i < classes_count; i++){
        if(new1_c[core_len][i] != new2_c[i])
          return false;
      }
  }
  else return false;
  
  //std::cout << "\nIs Feasible: " << node1 << " " << node2;
  return true;
  
}



/*--------------------------------------------------------------
 * void VF3State::AddPair(node1, node2)
 * Adds a pair to the Core set of the state.
 * Precondition: the pair must be feasible
 -------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::AddPair(node_id node1, node_id node2)
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
  
  //Checking if node2 is not in T2_in
  if (!in_2[node2])
    { in_2[node2]=core_len;
      //in2_set[node_c].push_back(node2);
      t2in_len++;
      t2in_len_c[node_c]++;
      if (out_2[node2]){
        t2both_len++;
        t2both_len_c[node_c]++;
      }
    }
  
  //Checking if node2 is not in T2_in
  if (!out_2[node2])
    { out_2[node2]=core_len;
      //out2_set[node_c].push_back(node2);
      t2out_len++;
      t2out_len_c[node_c]++;
      if (in_2[node2]){
        t2both_len++;
        t2both_len_c[node_c]++;
      }
    }
  
  //Inserting nodes into the core set
  core_1[node1]=node2;
  core_2[node2]=node1;
  
  //Evaluation of the neighborhood
  int i, other, other_c;
  other_c = -1;
  
  for(i=0; i<g2->InEdgeCount(node2); i++)
    { other=g2->GetInEdge(node2, i);
      if (!in_2[other])
        {
        other_c = class_2[other];
        in_2[other]=core_len;
        //in2_set[other_c].push_back(other);
        t2in_len++;
        t2in_len_c[other_c]++;
        if (out_2[other]){
          t2both_len++;
          t2both_len_c[other_c]++;
        }
        }
    }
  
  for(i=0; i<g2->OutEdgeCount(node2); i++)
    { other=g2->GetOutEdge(node2, i);
      if (!out_2[other])
        {
        other_c = class_2[other];
        out_2[other]=core_len;
        //out2_set[other_c].push_back(other);
        t2out_len++;
        t2out_len_c[other_c]++;
        if (in_2[other]){
          t2both_len++;
          t2both_len_c[other_c]++;
        }
        }
    }
  
}



/*--------------------------------------------------------------
 * void VF3State::GetCoreSet(c1, c2)
 * Reads the core set of the state into the arrays c1 and c2.
 * The i-th pair of the mapping is (c1[i], c2[i])
 --------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::GetCoreSet(node_id c1[], node_id c2[])
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
void VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::BackTrack()
{
  
  /*std::cout<<"\nBT:";
   print_core(core_1,core_2,n1);
   std::cout<<" -> "<< added_node1 <<":"<< core_1[added_node1];*/
  
  assert(core_len - orig_core_len <= 1);
  assert(added_node1 != NULL_NODE);
  
  int other_c = 0;
  int node_c = class_1[added_node1];
  
  if (orig_core_len < core_len)
    { int i, node2;
      node2 = core_1[added_node1];
      
      if (in_2[node2] == core_len){
        in_2[node2] = 0;
        //in2_set[node_c].erase(node2);
        t2in_len_c[node_c]--;
        if(out_2[node2])
          t2both_len_c[node_c]--;
      }
      
      if (out_2[node2] == core_len){
        out_2[node2] = 0;
        //out2_set[node_c].erase(node2);
        t2out_len_c[node_c]--;
        if(in_2[node2])
          t2both_len_c[node_c]--;
        
      }
      
      //Backtraking neightborhood
      for(i=0; i<g2->InEdgeCount(node2); i++)
        { int other=g2->GetInEdge(node2, i);
          other_c = class_2[other];
          if (in_2[other]==core_len){
            in_2[other]=0;
            //in2_set[other_c].erase(other);
            t2in_len_c[other_c] --;
            if(out_2[other])
              t2both_len_c[other_c]--;
          }
        }
      
      for(i=0; i<g2->OutEdgeCount(node2); i++)
        { int other=g2->GetOutEdge(node2, i);
          other_c = class_2[other];
          if (out_2[other]==core_len){
            out_2[other]=0;
            //out2_set[other_c].erase(other);
            t2out_len_c[other_c] --;
            if(in_2[other])
              t2both_len_c[other_c]--;
          }
        }
      
      core_1[added_node1] = NULL_NODE;
      core_2[node2] = NULL_NODE;
      
      core_len=orig_core_len;
      core_len_c[node_c]--;
      added_node1 = NULL_NODE;
    }
}

template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
bool VF3State<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::IsDead() {
  if (n1 != n2){
    return true;
  }
  
  if(t1both_len[core_len] != t2both_len ||
     t1out_len[core_len] != t2out_len ||
     t1in_len[core_len] != t2in_len){
    return true;
  }
  
  for(int c = 0; c < classes_count; c++){
    if(t1both_len_c[core_len][c] != t2both_len_c[c] ||
       t1out_len_c[core_len][c] != t2out_len_c[c] ||
       t1in_len_c[core_len][c] != t2in_len_c[c]){
      return true;
    }
  }
  
  return false;
}


#endif
