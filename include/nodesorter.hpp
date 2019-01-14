#ifndef NODE_SORTER_HPP
#define NODE_SORTER_HPP

#include <vector>
#include <algorithm>
#include "probability_strategy.hpp"
#include "argraph.hpp"

//Structure used for the sorting of the pattern
struct snode_s
{
  float probability;
  int deg;
  int deg_neigh; //Summ of neighboors' degree
  int deg_m;
  bool used;
  bool in_candidate;
  node_id id;
  
  //Used for RI
  int vis_count;
  int neig_count;
  int unv_count;
};

typedef struct snode_s snode_t;

//Comparison functions for RI Sorting
class CompareNodeDegree{
public:
  bool operator()(snode_s* n1, snode_t* n2)
  {
  if (n1->deg > n2->deg) return true;
  return false;
  }
};

class CompareNodeRIScore{
public:
  bool operator()(snode_s* n1, snode_t* n2)
  {
    if(n1->used && n2->used)
      return true;
    if(n1->used && !n2->used)
      return false;
    if(!n1->used && n2->used)
      return true;
  
    if (n1->vis_count > n2->vis_count)
      return true;
    if (n1->vis_count == n2->vis_count &&
      n1->neig_count > n2->neig_count)
      return true;
    if (n1->vis_count == n2->vis_count &&
      n1->neig_count == n2->neig_count &&
      n1->unv_count > n2->unv_count)
      return true;
    return false;
  }
};


//Comparison function for the heap
class CompareNodeStar{
public:
  bool operator()(snode_t* n1, snode_t* n2)
  {
	  if (n1->deg_m > n2->deg_m)
		return true;

	  if (n1->deg_m == n2->deg_m &&
		n1->probability < n2->probability)
		  return true;

	  if (n1->deg_m == n2->deg_m &&
		n1->probability == n2->probability &&
		n1->deg > n2->deg)
		return true;

	  return false;
  }
};

class FindUnused
{
public:
	bool operator()(snode_t* n)
	{
		return !n->used;
	}
};

class CompareNodeStarProbability{
public:
  bool operator()(snode_t* n1, snode_t* n2)
  {
  if (n1->probability < n2->probability) return true;
  if (n1->probability == n2->probability && n1->deg > n2->deg) return true;
  return false;
  }
};

template<typename Node, typename Edge>
class NodeSorter
{
  public:
    virtual vector<node_id> SortNodes(ARGraph<Node,Edge>* pattern)=0;
};

template<typename Node, typename Edge>
class SimpleNodeSorter:NodeSorter<Node,Edge>
{
  public:
    vector<node_id> SortNodes(ARGraph<Node,Edge>* pattern)
    {
      int i;
      vector<node_id> sorted;
      for (i = 0; i < pattern->NodeCount(); i++)
        sorted.push_back(i);
      return sorted;
    }
};

template<typename Node, typename Edge>
class DegreeNodeSorter:NodeSorter<Node,Edge>
{
  public:
    vector<node_id> SortNodes(ARGraph<Node,Edge>* pattern);
};

template<typename Node, typename Edge>
class DegMNodeSorter:NodeSorter<Node,Edge>
{
  public:
    vector<node_id> SortNodes(ARGraph<Node,Edge>* pattern);
};

template<typename Node, typename Edge>
class VF2NodeSorter:NodeSorter<Node,Edge>
{
  public:
    vector<node_id> SortNodes(ARGraph<Node,Edge>* pattern);
};

template<typename Node, typename Edge>
class RINodeSorter:NodeSorter<Node,Edge>
{
  public:
    vector<node_id> SortNodes(ARGraph<Node,Edge>* pattern);
};

//Node sorter class used to sort the pattern
//Complete Sorter used in VF3 - Use probability and degM connections
template<typename Node, typename Edge,
typename Probability = UniformProbability<Node,Edge > >
class VF3NodeSorter:NodeSorter<Node, Edge>
{
  private:
    Probability* probability;
    void AddNodeToSortedSet(ARGraph<Node,Edge>* pattern, node_id node, node_id level,
                          snode_t* nodes, vector<snode_t*> &candidates, vector<node_id> &nodes_order);
  
  public:
    vector<node_id> SortNodes(ARGraph<Node,Edge>* pattern);
    VF3NodeSorter(ARGraph<Node,Edge>* target);
    ~VF3NodeSorter();
  
};


template<typename Node, typename Edge,
typename Probability >
VF3NodeSorter<Node,Edge,Probability >::VF3NodeSorter(ARGraph<Node,Edge>* target)
{
  probability = new Probability(target);
}


template<typename Node, typename Edge,
typename Probability >
VF3NodeSorter<Node,Edge,Probability >::~VF3NodeSorter()
{
  delete probability;
}


template<typename Node, typename Edge,
typename Probability >
vector<node_id> VF3NodeSorter<Node,Edge,Probability >::SortNodes(ARGraph<Node,Edge>* pattern)
{
  int nodeCount;
  int i;
  nodeCount = pattern->NodeCount();
  vector<node_id> nodes_order; //Output vector with sorted nodes
  
  //We use two structures the fist used to quickly edit the deg_m of a node by its index
  //The second to perform a priority queue by means a max heap
  snode_t* nodes = new snode_t[nodeCount];
  //mypriority prio; //Heap of references
  vector<snode_t*> node_star;
  vector<snode_t*> candidates; //Node candidate for the addition
  vector<snode_t*>::iterator candidate_it;
  vector<snode_t*>::iterator max_node;
  //Initializing the node vector for sorting

  for(i = 0; i < nodeCount; i++)
  {
  
    nodes[i].id = i;
    nodes[i].deg_m = 0; //Dynamically updated
    nodes[i].deg = pattern->EdgeCount(i);
    nodes[i].deg_neigh = 0;
    nodes[i].probability = probability->getProbability(pattern,i);
    nodes[i].used = false;			//The node has been sorted
    nodes[i].in_candidate = false;	//The node has to be ignored as candidate
    nodes[i].vis_count = 0;
    nodes[i].neig_count = 0;
    nodes[i].unv_count = 0;
    node_star.push_back(&nodes[i]);
  }
  
  int n = 0;
  candidate_it = min_element(node_star.begin(),node_star.end(), CompareNodeStarProbability());
  node_id top = (*candidate_it)->id;
  AddNodeToSortedSet(pattern, top, n, nodes, candidates, nodes_order);

  //Getting the first node of the heap
  for (; n < nodeCount - 1; n++) {
    candidate_it = min_element(candidates.begin(), candidates.end(), CompareNodeStar());

	//Searching for remaining user
	if ((*candidate_it)->used)
	{
		candidate_it = find_if(node_star.begin(), node_star.end(), FindUnused());
		AddNodeToSortedSet(pattern, (*candidate_it)->id, n, nodes, candidates, nodes_order);
	}
	else if (candidate_it != candidates.end())
	{
		AddNodeToSortedSet(pattern, (*candidate_it)->id, n, nodes, candidates, nodes_order);
	}
  }
  
  delete[] nodes;
  return nodes_order;
  
}

template<typename Node, typename Edge,
typename Probability >
void VF3NodeSorter<Node,Edge,Probability >::AddNodeToSortedSet(ARGraph<Node,Edge>* pattern,
                    node_id node, node_id level, snode_t* nodes, vector<snode_t*> &candidates, vector<node_id> &nodes_order)
{
  node_id i, neigh;
  node_id in1_count, out1_count;


  nodes_order.push_back(node);
  nodes[node].used = true;
  nodes[node].in_candidate = true;
  nodes[node].deg_m = 0;		//Cleaning Deg Count for inserted nodes
  
  //Updating Terminal set size count And degree
  in1_count = pattern->InEdgeCount(node);
  out1_count = pattern->OutEdgeCount(node);
  
  //Updating Inner Nodes not yet inserted
  for (i = 0; i < in1_count; i++)
  {
    //Getting Neighborhood
    neigh = pattern->GetInEdge(node,i);
    if(!nodes[neigh].used)
      nodes[neigh].deg_m++;
    if(!nodes[neigh].in_candidate){
      nodes[neigh].in_candidate = true;
      candidates.push_back(&nodes[neigh]);
    }
    
  }
  
  //Updating Outer Nodes not yet insered
  for (i = 0; i < out1_count; i++)
  {
    //Getting Neighborhood
    neigh = pattern->GetOutEdge(node,i);
    if(!nodes[neigh].used)
      nodes[neigh].deg_m++;
    if(!nodes[neigh].in_candidate){
      nodes[neigh].in_candidate = true;
      candidates.push_back(&nodes[neigh]);
    }
  }

}

/****************** RI SORTER ********************/
template<typename Node, typename Edge>
vector<node_id> RINodeSorter<Node,Edge>::SortNodes(ARGraph<Node,Edge>* pattern)
{
  int nodeCount;
  node_id i, n, k, neigh, neigh2;
  node_id out1_count, out1_count_2;

  nodeCount = pattern->NodeCount();
  vector<node_id> sorted;
  
  snode_t* nodes = new snode_t[nodeCount];
  vector<snode_t*> node_star;
  vector<snode_t*>::iterator candidate_it;
  vector<snode_t*>::iterator start_it;
  vector<snode_t*>::iterator curr_it;
  
  for(i = 0; i < nodeCount; i++)
  {
    nodes[i].id = i;
    nodes[i].deg_m = 0;
    nodes[i].deg = pattern->EdgeCount(i);
    nodes[i].deg_neigh = 0;
    nodes[i].probability = 0;
    nodes[i].used = false; //Node already sorted
    //In this case the field is used to avoid to reduce more time the unv_count
    //Indeed a node that belongs to the neig set is supposed to be already considered
    //thus when it is insertend in the solution the unv_count has not be decreased but
    //the neig_counts
    nodes[i].in_candidate = false;
    nodes[i].vis_count = 0;
    nodes[i].neig_count = 0;
    nodes[i].unv_count = pattern->OutEdgeCount(i);
    node_star.push_back(nodes+i);
  }
  
  //Getting the element with the maximum degree
  candidate_it = min_element(node_star.begin(),node_star.end(), CompareNodeDegree());
  start_it = node_star.begin();
  
  for(n = 0; n < nodeCount; n++)
  {
    (*candidate_it)->used = true;
    (*candidate_it)->vis_count = -1;
    (*candidate_it)->neig_count = -1;
    (*candidate_it)->unv_count = -1;
    node_id node = (*candidate_it)->id;
    sorted.push_back(node);
    //std::cout<<node<<"\n";
    out1_count = pattern->OutEdgeCount(node);
  
    for (i = 0; i < out1_count; i++)
    {
      //Getting Neighborhood
      neigh = pattern->GetOutEdge(node,i);
      if(!nodes[neigh].used){
        nodes[neigh].vis_count++;
        if(!nodes[node].in_candidate)
          nodes[neigh].unv_count--;
        else
          nodes[neigh].neig_count--;
        
        out1_count_2 = pattern->OutEdgeCount(neigh);
        for (k = 0; k < out1_count_2; k++)
        {
          neigh2 = pattern->GetOutEdge(neigh,k);
          if(neigh2 != node &&
            !nodes[neigh2].used &&
            pattern->HasEdge(neigh2,node))
            {
              nodes[neigh].neig_count++;
              nodes[neigh2].in_candidate = true;
              nodes[neigh].unv_count--;
            }
        }
      }
    }
  
    /*for(int m = 0; m < nodeCount; m++)
    {
      std::cout<<m<<" "<<nodes[m].vis_count<<" "<<nodes[m].neig_count<<" "<<nodes[m].unv_count<<"\n";
    }
    std::cout<<"\n\n";*/
  
    candidate_it = min_element(node_star.begin(),node_star.end(), CompareNodeRIScore());
  }
  
  return sorted;
}

/****************** VF2 SORTER ********************/
template<typename Node, typename Edge>
vector<node_id> VF2NodeSorter<Node,Edge>::SortNodes(ARGraph<Node,Edge>* pattern)
{
  int nodeCount;
  node_id i, k, neigh;
  node_id out_count, in_count;

  nodeCount = pattern->NodeCount();
  vector<node_id> sorted;
  
  snode_t* nodes = new snode_t[nodeCount];
  
  for(i = 0; i < nodeCount; i++)
  {
    nodes[i].id = i;
    nodes[i].deg_m = 0;
    nodes[i].deg = 0;
    nodes[i].deg_neigh = 0;
    nodes[i].probability = 0;
    nodes[i].used = false;
    nodes[i].in_candidate = false;
    nodes[i].vis_count = 0;
    nodes[i].neig_count = 0;
    nodes[i].unv_count = 0;
  }
  
  for(i = 0; i < nodeCount; i++)
  {
    if(!nodes[i].used)
    {
      nodes[i].used = true;
      sorted.push_back(i);
    }
  
    in_count = pattern->InEdgeCount(i);
    out_count = pattern->OutEdgeCount(i);
  
    for(k = 0; k < in_count; k++)
    {
      neigh = pattern->GetInEdge(i, k);
      if(!nodes[neigh].used)
      {
        nodes[neigh].used = true;
        sorted.push_back(neigh);
      }
    }
  
    for(k = 0; k < out_count; k++)
    {
      neigh = pattern->GetOutEdge(i, k);
      if(!nodes[neigh].used)
      {
        nodes[neigh].used = true;
        sorted.push_back(neigh);
      }
    }
  }
  return sorted;
}

/****************** DEG SORTER ********************/
/*template<typename Node, typename Edge>
vector<node_id> DegreeNodeSorter<Node,Edge>::SortNodes(ARGraph<Node,Edge>* pattern)
{

  int nodeCount;
  node_id i, n, k, neigh, neigh2;
  node_id out1_count, out1_count_2;

  nodeCount = pattern->NodeCount();
  vector<node_id> sorted;
  
  snode_t* nodes = new snode_t[nodeCount];
  vector<snode_t*> node_star;
  vector<snode_t*>::iterator candidate_it;
  vector<snode_t*>::iterator start_it;
  vector<snode_t*>::iterator curr_it;
  
  for(i = 0; i < nodeCount; i++)
  {
    nodes[i].id = i;
    nodes[i].deg_m = 0;
    nodes[i].deg = pattern->EdgeCount(i);
    nodes[i].deg_neigh = 0;
    nodes[i].probability = 0;
    nodes[i].used = false;
    nodes[i].in_candidate = false;
    nodes[i].vis_count = 0;
    nodes[i].neig_count = 0;
    nodes[i].unv_count = pattern->OutEdgeCount(i);
    node_star.push_back(nodes+i);
  }
  
  //Getting the element with the maximum degree
  candidate_it = min_element(node_star.begin(),node_star.end(), CompareNodeDegree());
  start_it = node_star.begin();

  for(n = 0; n < nodeCount; n++)
  {
    (*candidate_it)->used = true;
    node_id node = (*candidate_it)->id;
    sorted.push_back(node);
  
  }
  return sorted;
}*/

#endif
