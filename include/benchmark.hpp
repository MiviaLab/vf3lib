#ifndef VF_BENCHMARK_HPP
#define VF_BENCHMARK_HPP
#include <time.h>
#include "match.hpp"
#include "memory.hpp"

bool def_visitor(int n, node_id ni1[], node_id ni2[], void *usr_data)
{
  int* sol_counter = (int*)usr_data;
  (*sol_counter)++;
  return false;
}

template<typename State>
class BenchmarkAllMatchings
{
public:
  bool operator()(State &s, int& n, node_id* n1, node_id* n2, int* usr_data){
    return match<State>(s,&n,n1,n2, def_visitor,usr_data);
  }
};

template<typename State>
class BenchmarkFirstMatching
{
public:
  bool operator()(State &s, int& n, node_id* n1, node_id* n2, int* usr_data){
    if(match<State>(s,&n,n1,n2)){
      (*usr_data)++;
      return true;
    }
    return false;
  }
};

template<typename State, typename Benchmark>
double benchmark_time(State &s, int& n, node_id* n1, node_id* n2, double time_limit, int& rep, int& solutions){
  unsigned long ticks;
  double time;
  rep = 0;
	n = 0;
  solutions = 0;
  
  Benchmark bmk;
  
  ticks = clock();
	do {
		rep++;
    bmk(s, n, n1, n2, &solutions);
  }while(clock()-ticks < CLOCKS_PER_SEC*time_limit);
  time = ((double)(clock()-ticks)/CLOCKS_PER_SEC/rep);
  solutions = solutions/rep;
  return time;
}


template<typename State, typename Benchmark>
size_t benchmark_memory(State &s, int& n, node_id* n1, node_id* n2, int& solutions)
{
  solutions = 0;
  n = 0;
  size_t mem1, mem2;
  Benchmark bmk;
  mem1 = getCurrentRSS();
  bmk(s, n, n1, n2, &solutions);
  mem2 = getPeakRSS();
  //std::cout<< mem1 << " " << mem2 << "\n";
  return mem2 - mem1;
}

#endif
