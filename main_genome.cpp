#include <iostream>
#include <fstream>
#include <ARGLoader.hpp>
#include <ARGraph.hpp>
#include <ARGEdit.hpp>
#include <NodeSorter.hpp>
#include <ProbabilityStrategy.hpp>
#include <genome/GFALoader.hpp>
#include <genome/VGraph.hpp>

int main(int argc, char** argv)
{

  char *graph;

  graph = argv[1];  
  
  std::ifstream graphIn(graph);
  vflib::StreamGFALoader graphLoader(graphIn);
  vflib::VGraph gr(&graphLoader);
  gr.Print();


}
