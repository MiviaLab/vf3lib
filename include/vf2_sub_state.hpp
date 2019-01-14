/*------------------------------------------------------------
 * vf2_state.h
 * Interface of vf2_state.cc
 * Definition of a class representing a state of the matching
 * process between two ARGs.
 * See: argraph.h state.h
 *
 * Author: P. Foggia
 *-----------------------------------------------------------------*/
#ifndef VF2_SUB_STATE_H
#define VF2_SUB_STATE_H

#include "argraph.hpp"

/*----------------------------------------------------------
 * class VF2SubState
 * A representation of the SSR current state
 * See vf2_state.cc for more details.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor = EqualityComparator<Node1, Node2>,
typename EdgeComparisonFunctor = EqualityComparator<Edge1, Edge2> >
class VF2SubState
{
private:
  NodeComparisonFunctor nf;
  EdgeComparisonFunctor ef;
  int core_len, orig_core_len;
  int added_node1;
  int t1both_len, t2both_len, t1in_len, t1out_len,
  t2in_len, t2out_len; // Core nodes are also counted by these...
  
  node_id *core_1;
  node_id *core_2;
  node_id *in_1;
  node_id *in_2;
  node_id *out_1;
  node_id *out_2;

  node_id *order;
  
  ARGraph<Node1, Edge1> *g1;
  ARGraph<Node2, Edge2> *g2;
  int n1, n2;
  
  const VF2SubState* parent;
  bool used;
  
  long *share_count;
  void BackTrack();
  
public:
  static long long  instance_count;
  VF2SubState(ARGraph<Node1, Edge1> *g1, ARGraph<Node2, Edge2> *g2, node_id* order = NULL);
  VF2SubState(const VF2SubState &state);
  ~VF2SubState();
  ARGraph<Node1, Edge1> *GetGraph1() { return g1; }
  ARGraph<Node2, Edge2> *GetGraph2() { return g2; }
  bool NextPair(node_id *pn1, node_id *pn2,
                node_id prev_n1=NULL_NODE, node_id prev_n2=NULL_NODE);
  bool IsFeasiblePair(node_id n1, node_id n2);
  void AddPair(node_id n1, node_id n2);
  bool IsGoal() { return core_len==n1; };
  bool IsDead() { return n1>n2  ||
    t1both_len>t2both_len ||
    t1out_len>t2out_len ||
    t1in_len>t2in_len;
  };
  int CoreLen() { return core_len; }
  void GetCoreSet(node_id c1[], node_id c2[]);
  VF2SubState* GetParent();
  bool IsUsed();
  void SetUsed();
};

/*----------------------------------------------------------
 * Methods of the class VF2SubState
 ---------------------------------------------------------*/

/*----------------------------------------------------------
 * VF2SubState::VF2SubState(g1, g2)
 * Constructor. Makes an empty state.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::VF2SubState(ARGraph<Node1, Edge1> *ag1, ARGraph<Node2, Edge2> *ag2, node_id* order)
{
  VF2SubState::instance_count=1;
  g1=ag1;
  g2=ag2;
  n1=g1->NodeCount();
  n2=g2->NodeCount();
  
  this->order = order;
  
  core_len=orig_core_len=0;
  t1both_len=t1in_len=t1out_len=0;
  t2both_len=t2in_len=t2out_len=0;
  parent = NULL;
  used = false;
  
	added_node1=NULL_NODE;
  
  core_1=new node_id[n1];
  core_2=new node_id[n2];
  in_1=new node_id[n1];
  in_2=new node_id[n2];
  out_1=new node_id[n1];
  out_2=new node_id[n2];
	share_count = new long;
  if (!core_1 || !core_2 || !in_1 || !in_2
	    || !out_1 || !out_2 || !share_count)
    error("Out of memory");
  
  int i;
  for(i=0; i<n1; i++)
  {
    core_1[i]=NULL_NODE;
		in_1[i]=0;
		out_1[i]=0;
  }
  for(i=0; i<n2; i++)
  {
    core_2[i]=NULL_NODE;
		in_2[i]=0;
		out_2[i]=0;
  }
  
	*share_count = 1;
}


/*----------------------------------------------------------
 * VF2SubState::VF2SubState(state)
 * Copy constructor.
 ---------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::VF2SubState(const VF2SubState &state)
{ g1=state.g1;
  g2=state.g2;
  n1=state.n1;
  n2=state.n2;
  
  order=state.order;
  VF2SubState::instance_count++;
  
  core_len=orig_core_len=state.core_len;
  t1in_len=state.t1in_len;
  t1out_len=state.t1out_len;
  t1both_len=state.t1both_len;
  t2in_len=state.t2in_len;
  t2out_len=state.t2out_len;
  t2both_len=state.t2both_len;
  parent = &state;
  used = false;
  
  added_node1=NULL_NODE;
  
  core_1=state.core_1;
  core_2=state.core_2;
  in_1=state.in_1;
  in_2=state.in_2;
  out_1=state.out_1;
  out_2=state.out_2;
  share_count=state.share_count;
  
	++ *share_count;
  
}


/*---------------------------------------------------------------
 * VF2SubState::~VF2SubState()
 * Destructor.
 --------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::~VF2SubState()
{
  
  if(-- *share_count > 0)
    BackTrack();
  
  if (*share_count == 0)
  { delete [] core_1;
    delete [] core_2;
    delete [] in_1;
    delete [] out_1;
    delete [] in_2;
    delete [] out_2;
    delete share_count;
  }
}


/*--------------------------------------------------------------------------
 * bool VF2SubState::NextPair(pn1, pn2, prev_n1, prev_n2)
 * Puts in *pn1, *pn2 the next pair of nodes to be tried.
 * prev_n1 and prev_n2 must be the last nodes, or NULL_NODE (default)
 * to start from the first pair.
 * Returns false if no more pairs are available.
 -------------------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
bool VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::NextPair(node_id *pn1, node_id *pn2,node_id prev_n1, node_id prev_n2)
{
  if (prev_n1==NULL_NODE)
    prev_n1=0;
  
  if (prev_n2==NULL_NODE)
    prev_n2=0;
  else
    prev_n2++;
  
	if (t1both_len>core_len && t2both_len>core_len)
  { while (prev_n1<n1 &&
           (core_1[prev_n1]!=NULL_NODE || out_1[prev_n1]==0
            || in_1[prev_n1]==0) )
  { prev_n1++;
    prev_n2=0;
  }
  }
	else if (t1out_len>core_len && t2out_len>core_len)
  { while (prev_n1<n1 &&
           (core_1[prev_n1]!=NULL_NODE || out_1[prev_n1]==0) )
  { prev_n1++;
    prev_n2=0;
  }
  }
  else if (t1in_len>core_len && t2in_len>core_len)
  { while (prev_n1<n1 &&
           (core_1[prev_n1]!=NULL_NODE || in_1[prev_n1]==0) )
  { prev_n1++;
    prev_n2=0;
  }
  }
  else if (prev_n1==0 && order!=NULL)
  { int i=0;
    while (i<n1 && core_1[prev_n1=order[i]] != NULL_NODE)
      i++;
    if (i==n1)
      prev_n1=n1;
  }
	else
  { while (prev_n1<n1 && core_1[prev_n1]!=NULL_NODE )
  { prev_n1++;
    prev_n2=0;
  }
  }
  
  
	if (t1both_len>core_len && t2both_len>core_len)
  { while (prev_n2<n2 &&
           (core_2[prev_n2]!=NULL_NODE || out_2[prev_n2]==0
            || in_2[prev_n2]==0) )
  { prev_n2++;
  }
  }
	else if (t1out_len>core_len && t2out_len>core_len)
  { while (prev_n2<n2 &&
           (core_2[prev_n2]!=NULL_NODE || out_2[prev_n2]==0) )
  { prev_n2++;
  }
  }
  else if (t1in_len>core_len && t2in_len>core_len)
  { while (prev_n2<n2 &&
           (core_2[prev_n2]!=NULL_NODE || in_2[prev_n2]==0) )
  { prev_n2++;
  }
  }
	else
  { while (prev_n2<n2 && core_2[prev_n2]!=NULL_NODE )
  { prev_n2++;
  }
  }
  
  if (prev_n1<n1 && prev_n2<n2)
  { *pn1=prev_n1;
    *pn2=prev_n2;
    return true;
  }
  
  return false;
}


/*---------------------------------------------------------------
 * bool VF2SubState::IsFeasiblePair(node1, node2)
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
bool VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::IsFeasiblePair(node_id node1, node_id node2)
{
  
  assert(node1<n1);
  assert(node2<n2);
  assert(core_1[node1]==NULL_NODE);
  assert(core_2[node2]==NULL_NODE);
  
  if(!nf(g1->GetNodeAttr(node1), g2->GetNodeAttr(node2)))
    return false;
  
  if(g1->EdgeCount(node1) > g2->EdgeCount(node2))
    return false;
  
  int i, other1, other2;
  Edge1 eattr1;
  Edge2 eattr2;
  int termout1=0, termout2=0, termin1=0, termin2=0, new1=0, new2=0;
  
  // Check the 'out' edges of node1
  for(i=0; i<g1->OutEdgeCount(node1); i++)
  { other1=g1->GetOutEdge(node1, i, eattr1);
    if (core_1[other1] != NULL_NODE)
    { other2=core_1[other1];
      if (!g2->HasEdge(node2, other2, eattr2) ||
          !ef(eattr1, eattr2))
        return false;
    }
    else
    { if (in_1[other1])
      termin1++;
      if (out_1[other1])
        termout1++;
      if (!in_1[other1] && !out_1[other1])
        new1++;
    }
  }
  
  // Check the 'in' edges of node1
  for(i=0; i<g1->InEdgeCount(node1); i++)
  { other1=g1->GetInEdge(node1, i, eattr1);
    if (core_1[other1]!=NULL_NODE)
    { other2=core_1[other1];
      if (!g2->HasEdge(other2, node2, eattr2) ||
          !ef(eattr1, eattr2))
        return false;
    }
    else
    { if (in_1[other1])
      termin1++;
      if (out_1[other1])
        termout1++;
      if (!in_1[other1] && !out_1[other1])
        new1++;
    }
  }
  
  
  // Check the 'out' edges of node2
  for(i=0; i<g2->OutEdgeCount(node2); i++)
  { other2=g2->GetOutEdge(node2, i);
    if (core_2[other2]!=NULL_NODE)
    { other1=core_2[other2];
      if (!g1->HasEdge(node1, other1))
        return false;
    }
    else
    { if (in_2[other2])
      termin2++;
      if (out_2[other2])
        termout2++;
      if (!in_2[other2] && !out_2[other2])
        new2++;
    }
  }
  
  // Check the 'in' edges of node2
  for(i=0; i<g2->InEdgeCount(node2); i++)
  { other2=g2->GetInEdge(node2, i);
    if (core_2[other2] != NULL_NODE)
    { other1=core_2[other2];
      if (!g1->HasEdge(other1, node1))
        return false;
    }
    else
    { if (in_2[other2])
      termin2++;
      if (out_2[other2])
        termout2++;
      if (!in_2[other2] && !out_2[other2])
        new2++;
    }
  }
  
  return termin1<=termin2 && termout1<=termout2 && new1<=new2;
}



/*--------------------------------------------------------------
 * void VF2SubState::AddPair(node1, node2)
 * Adds a pair to the Core set of the state.
 * Precondition: the pair must be feasible
 -------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::AddPair(node_id node1, node_id node2)
{
  assert(node1<n1);
  assert(node2<n2);
  assert(core_len<n1);
  assert(core_len<n2);
  
  core_len++;
	added_node1=node1;
  
	if (!in_1[node1])
  { in_1[node1]=core_len;
    t1in_len++;
		if (out_1[node1])
		  t1both_len++;
  }
	if (!out_1[node1])
  { out_1[node1]=core_len;
    t1out_len++;
		if (in_1[node1])
		  t1both_len++;
  }
  
	if (!in_2[node2])
  { in_2[node2]=core_len;
    t2in_len++;
		if (out_2[node2])
		  t2both_len++;
  }
	if (!out_2[node2])
  { out_2[node2]=core_len;
    t2out_len++;
		if (in_2[node2])
		  t2both_len++;
  }
  
  core_1[node1]=node2;
  core_2[node2]=node1;
  
  
  int i, other;
  for(i=0; i<g1->InEdgeCount(node1); i++)
  { other=g1->GetInEdge(node1, i);
    if (!in_1[other])
    { in_1[other]=core_len;
      t1in_len++;
      if (out_1[other])
        t1both_len++;
    }
  }
  
  for(i=0; i<g1->OutEdgeCount(node1); i++)
  { other=g1->GetOutEdge(node1, i);
    if (!out_1[other])
    { out_1[other]=core_len;
      t1out_len++;
      if (in_1[other])
        t1both_len++;
    }
  }
  
  for(i=0; i<g2->InEdgeCount(node2); i++)
  { other=g2->GetInEdge(node2, i);
    if (!in_2[other])
    { in_2[other]=core_len;
      t2in_len++;
      if (out_2[other])
        t2both_len++;
    }
  }
  
  for(i=0; i<g2->OutEdgeCount(node2); i++)
  { other=g2->GetOutEdge(node2, i);
    if (!out_2[other])
    { out_2[other]=core_len;
      t2out_len++;
      if (in_2[other])
        t2both_len++;
    }
  }
  
}



/*--------------------------------------------------------------
 * void VF2SubState::GetCoreSet(c1, c2)
 * Reads the core set of the state into the arrays c1 and c2.
 * The i-th pair of the mapping is (c1[i], c2[i])
 --------------------------------------------------------------*/
template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::GetCoreSet(node_id c1[], node_id c2[])
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
void VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::BackTrack()
{
  assert(core_len - orig_core_len <= 1);
  assert(added_node1 != NULL_NODE);
  
  if (orig_core_len < core_len)
  { int i, node2;
    
    if (in_1[added_node1] == core_len)
		  in_1[added_node1] = 0;
    for(i=0; i<g1->InEdgeCount(added_node1); i++)
    { int other=g1->GetInEdge(added_node1, i);
      if (in_1[other]==core_len)
			  in_1[other]=0;
    }
    
		if (out_1[added_node1] == core_len)
		  out_1[added_node1] = 0;
    for(i=0; i<g1->OutEdgeCount(added_node1); i++)
    { int other=g1->GetOutEdge(added_node1, i);
      if (out_1[other]==core_len)
			  out_1[other]=0;
    }
    
		node2 = core_1[added_node1];
    
    if (in_2[node2] == core_len)
		  in_2[node2] = 0;
    for(i=0; i<g2->InEdgeCount(node2); i++)
    { int other=g2->GetInEdge(node2, i);
      if (in_2[other]==core_len)
			  in_2[other]=0;
    }
    
		if (out_2[node2] == core_len)
		  out_2[node2] = 0;
    for(i=0; i<g2->OutEdgeCount(node2); i++)
    { int other=g2->GetOutEdge(node2, i);
      if (out_2[other]==core_len)
			  out_2[other]=0;
    }
    
    core_1[added_node1] = NULL_NODE;
		core_2[node2] = NULL_NODE;
    
    core_len=orig_core_len;
		added_node1 = NULL_NODE;
  }
  
}

template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>* VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::GetParent()
{
  return (VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>*)parent;
}

template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
bool VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::IsUsed(){
  return used;
}

template <typename Node1, typename Node2,
typename Edge1, typename Edge2,
typename NodeComparisonFunctor, typename EdgeComparisonFunctor>
void VF2SubState<Node1,Node2,Edge1,Edge2,NodeComparisonFunctor,EdgeComparisonFunctor>::SetUsed()
{
  used = true;
}

#endif

