#ifndef FASTCHECK_HPP_
#define FASTCHECK_HPP_

#include "ARGraph.hpp"

namespace vflib
{

template<typename Node1, typename Node2,
typename Edge1, typename Edge2 >
class FastCheck
{
    private:
        //Graphs to analyze
        ARGraph<Node1, Edge1> *g1;
        ARGraph<Node2, Edge2> *g2;

    public:
        FastCheck(ARGraph<Node1, Edge1> *g1,
            ARGraph<Node2, Edge2> *g2):g1(g1),g2(g2){}

        bool CheckIsomorphism(){
            if(
                //Size
                (g1->NodeCount() != g2->NodeCount()) ||
                //Edges
                (g1->EdgeCount() != g2->EdgeCount())||
                (g1->InEdgeCount() != g2->InEdgeCount())||
                (g1->OutEdgeCount() != g2->OutEdgeCount())||
                //Maximum Valence
                (g1->MaxDegree() != g2->MaxDegree()) ||
                (g1->OutMaxDegree() != g2->OutMaxDegree()) ||
                (g1->InMaxDegree() != g2->InMaxDegree()) ||
                //Labels
                (g1->NodeAttrCount() != g2->NodeAttrCount()) ||
                (g1->EdgeAttrCount() != g2->EdgeAttrCount()) )
            {
                return false;
            }
            return true;
        }

        bool CheckSubgraphIsomorphism(){
            if(
                //Size
                (g1->NodeCount() > g2->NodeCount()) ||
                //Edges
                (g1->EdgeCount() > g2->EdgeCount())||
                (g1->InEdgeCount() > g2->InEdgeCount())||
                (g1->OutEdgeCount() > g2->OutEdgeCount())||
                //Maximum Valence
                (g1->MaxDegree() > g2->MaxDegree()) ||
                (g1->OutMaxDegree() > g2->OutMaxDegree()) ||
                (g1->InMaxDegree() > g2->InMaxDegree()) ||
                //Labels
                (g1->NodeAttrCount() > g2->NodeAttrCount()) ||
                (g1->EdgeAttrCount() > g2->EdgeAttrCount()) )
            {
                return false;
            }
            return true;
        }
        
};

}

#endif