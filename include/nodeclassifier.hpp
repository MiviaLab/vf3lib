#ifndef NODECLASSIFIER_HPP
#define NODECLASSIFIER_HPP

#include <map>
#include "argraph.hpp"

template<typename Node, typename Edge>
class NodeClassifier{
protected:
  map<Node,int> classmap; //Map Of classes
  ARGraph<Node,Edge>* g1;
private:
  void virtual GenerateClasses(ARGraph<Node,Edge>* g);
  
public:
    //Used for first class generation
    NodeClassifier(ARGraph<Node,Edge>* g)
    {
      g1 = g;
      GenerateClasses(g);
    }
  
    //Used to inherit classes
  NodeClassifier(ARGraph<Node,Edge>* g, const NodeClassifier<Node,Edge>& classifier)
  {
    g1 = g;
    classmap.insert(classifier.classmap.begin(), classifier.classmap.end());
    GenerateClasses(g);
  }
  
  int CountClasses()
  {
    return classmap.size();
  }
  
  vector<int> GetClasses()
  {
    Node attr;
    
    int count = g1->NodeCount();
    vector<int> classes(count);
    
    for (node_id i = 0; i < count; i++)
    {
      attr = g1->GetNodeAttr(i);
      classes[i] = classmap[attr];
    }
    
    return classes;
  }
  
};

template<typename Node, typename Edge>
class ModuleNodeClassifier
{
  private:
    int classcount;
    ARGraph<Node,Edge>* g1;
  
  public:
  
    ModuleNodeClassifier(ARGraph<Node,Edge>* g, int classcount)
    {
      g1 = g;
      this->classcount = classcount;
    }
  
    int CountClasses(){return classcount;}
  
    vector<int> GetClasses()
    {
      Node attr;
      int count = g1->NodeCount();
      vector<int> classes(count);
    
      for (node_id i = 0; i < count; i++)
      {
        attr = g1->GetNodeAttr(i);
        classes[i] = (int)attr%classcount;
      }
      return classes;
  }
};

template<typename Node, typename Edge>
void NodeClassifier<Node,Edge>::GenerateClasses(ARGraph<Node,Edge>* g){
  int c;
  int classes_count = 0;
  node_id n;
  Node attr;
  int count = g->NodeCount();
  
  for(n = 0; n < count; n++)
  {
    attr = g->GetNodeAttr(n);
    if(classmap.count(attr))
    {
      c = classmap[attr];
    }
    else
    {
      c = classes_count++;
      classmap[attr] = c;
      
    }
  }
}

#endif
