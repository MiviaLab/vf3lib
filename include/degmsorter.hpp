#ifndef DEGM_NODE_SORTER_HPP
#define DEGM_NODE_SORTER_HPP

#include "argraph.hpp"

template<typename Node, typename Edge,
typename ProbabilitySorter >
class DegMsorter
{
private:
  vector<node_id> nodes_order;	//Node ids for next pairs
  
  void AddCoverageTreeNode(ARGraph<Node,Edge>* g1, node_id node, node_id level,
                      node_id *m_deg_count, bool* inserted);
  
  node_id GetMaxDegNode(ARGraph<Node,Edge>* g1, bool* inserted);
  
public:
  //Sort g1 using the info evalutated on g2
  vector<node_id> operator()(ARGraph<Node,Edge>* g1, ARGraph<Node,Edge>* g2);
};


template<typename Node, typename Edge,
typename ProbabilitySorter >
node_id DegMsorter<Node,Edge,ProbabilitySorter>::GetMaxDegNode(ARGraph<Node,Edge>* g1, bool* inserted) {
  node_id i;
  int curr_deg, max_deg;
  node_id node = NULL_NODE;
  
  curr_deg = 0;
  max_deg = 0;
  
  int node_count  = g1->NodeCount();
  for (i = 0; i < node_count; i++) {
    if (!inserted[i]) {
      curr_deg = g1->EdgeCount(i);
      if (curr_deg > max_deg) {
        max_deg = curr_deg;
        node = i;
      }
    }
  }
  
  return node;
}

/*
 * Procedure to insert a node into the output set and
 * updating the M degrees of the other nodes
 * */
template<typename Node, typename Edge,
typename ProbabilitySorter >
void DegMsorter<Node,Edge,ProbabilitySorter>::AddCoverageTreeNode(ARGraph<Node,Edge>* g1, node_id node, node_id level,
                                               node_id *m_deg_count, bool* inserted)
{
  node_id i, neigh;
  node_id in1_count, out1_count;
  
  nodes_order.push_back(node);
  m_deg_count[node] = 0;		//Cleaning Deg Count for inserted nodes
  inserted[node] = true;
  
  //Updating Terminal set size count And degree
  in1_count = g1->InEdgeCount(node);
  out1_count = g1->OutEdgeCount(node);
  
  //Updating Inner Nodes not yet inserted
  for (i = 0; i < in1_count; i++)
    {
    //Getting Neighborhood
    neigh = g1->GetInEdge(node,i);
    if(!inserted[neigh])
      m_deg_count[neigh]++;
    }
  
  //Updating Outer Nodes not yet insered
  for (i = 0; i < out1_count; i++)
    {
    //Getting Neighborhood
    neigh = g1->GetOutEdge(node,i);
    if(!inserted[neigh])
      m_deg_count[neigh]++;
    }
}

template<typename Node, typename Edge,
typename ProbabilitySorter >
vector<node_id> DegMsorter<Node,Edge,ProbabilitySorter>::operator()(ARGraph<Node,Edge>* g1, ARGraph<Node,Edge>* g2)
{
  //The algorithm start with the node with the maximum degree
  node_id i,k;
  node_id n;	//Tree State Level
  bool *in, *out; //Internal Terminal Set used for updating the size of
  node_id node;	//Current Node
  node_id max_deg, max_node;	//Used for searching the node with max M degree
  bool* inserted;	//Set of nodes just inserted into the output set
  node_id *m_deg_count;	//M degree for each node in the terminal set
  float* probabilities;
  vector<node_id> candidate_set;
  
  int n1 = g1->NodeCount();
  
  //Init vectors and variables
  node = 0;
  n = 0;
  
  in = new bool[n1];
  out = new bool[n1];
  m_deg_count = new node_id[n1];
  inserted = new bool[n1];  //node already used into the cadidate set
  
  for(i = 0; i < n1; i++)
  {
    m_deg_count[i] = 0;
    inserted[i] = false;  }
  
  ProbabilitySorter prob_sorter(g2);
  vector<node_id> order = prob_sorter(g1);
  probabilities = prob_sorter.getProbabilities();

  /*
   * The algorithm get the less probabile node, if two or more
   * nodes shares the same probability, we selecte those have the max degree.
   * Then add the node in the solution.
   */
  k = 0;
  node = order[k];
  float prob = probabilities[node];
  int deg = g1->EdgeCount(node);
  
  //Searching for other nodes with the same probability
  while(probabilities[node] == probabilities[order[++k]]){
    if(g1->EdgeCount(order[k]) > deg){
      node = order[k];
      prob = probabilities[node];
      deg = g1->EdgeCount(node);
    }
  }
  
  AddCoverageTreeNode(g1, node, n, m_deg_count, inserted);
  n++; 	//Increasing tree level
  
  /*
   * The algorithm always select the node with the maximum number of connection into the core
   * If two or more nodes have the same number of connections 
   * the algoritm is going to select the less probabile
   */
  for (; n < n1; n++) {
    //Searching for max M degree
    //Note that just nodes inserted in terminal sets and not yet inserted in M have M degree > 0
    max_deg = 0;
    max_node = NULL_NODE;
    
    for (i = 0; i < n1; i++) {
      if(!inserted[i]){
        if(m_deg_count[i] > max_deg)
          {
            max_deg = m_deg_count[i];
            max_node = i;
          }
        //If the have the same degM select the less probabile
        else if (max_node != NULL_NODE
                 && m_deg_count[i] == max_deg)
          {
            //Comparing node order
            if(probabilities[i] < probabilities[max_node])
              {
                max_deg = m_deg_count[i];
                max_node = i;
              }
          }
        //Select the one having the maximum degree
        /*else if (max_node != NULL_NODE)
          {
          //Comparing degree
          if(g1->EdgeCount(i) > g1->EdgeCount(max_node))
            {
            max_deg = m_deg_count[i];
            max_node = i;
            }
          }*/
      }
    }
    
    //If Getting nodes outside
    if(max_deg == 0 || max_node == NULL_NODE)
      max_node = GetMaxDegNode(g1,inserted);
    
    AddCoverageTreeNode(g1, max_node, n, m_deg_count, inserted);
  }
  
  delete [] in;
  delete [] out;
  delete [] inserted;
  delete [] m_deg_count;
  
  return nodes_order;
}


#endif
