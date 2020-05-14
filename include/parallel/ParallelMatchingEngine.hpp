/*
 * ThreadPool.hpp
 *
 *  Created on: 21 nov 2017
 *      Author: vcarletti
 */

/*
Parallel Matching Engine with global state stack only (no look-free stack)
*/

#ifndef PARALLELMATCHINGTHREADPOOL_HPP
#define PARALLELMATCHINGTHREADPOOL_HPP

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <vector>
#include <stack>
#include <cstdint>

#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#else
#include <Windows.h>
#include <stdint.h>
#endif

#include "WindowsTime.h"

#include "ARGraph.hpp"
#include "MatchingEngine.hpp"
#include "Stack.hpp"
#include "LockFreeStack.hpp"
#include "SynchronizedStack.hpp"

namespace vflib {

typedef unsigned short ThreadId;
constexpr ThreadId NULL_THREAD = (std::numeric_limits<ThreadId>::max)();

template<typename VFState>
class ParallelMatchingEngine
		: public MatchingEngine<VFState>
{

protected:
	typedef unsigned short ThreadId;
	using MatchingEngine<VFState>::solutions;
	using MatchingEngine<VFState>::visit;
	using MatchingEngine<VFState>::solCount;
	using MatchingEngine<VFState>::storeSolutions;
	using MatchingEngine<VFState>::fist_solution_time;

	struct timeval start_time;
	struct timeval pool_time;
	struct timeval exit_time;
	struct timeval eos_time; 
	std::vector<struct timeval> thEndOfSearchTime;

	std::atomic<bool> once;

	std::atomic<int16_t> endThreadCount;

	int16_t cpu;
	int16_t numThreads;
	std::vector<std::thread> pool;
	std::atomic<int32_t> statesToBeExplored;
	Stack<VFState*>* globalStateStack;
	struct timeval time;

	virtual void PreMatching(VFState* s){};
	virtual void PreprocessState(ThreadId thread_id){};
	virtual void PostprocessState(ThreadId thread_id){};
	virtual void UnprocessedState(ThreadId thread_id){};

	virtual void PutState(VFState* s, ThreadId thread_id) {
		globalStateStack->push(s);
	}

	virtual void GetState(VFState** res, ThreadId thread_id)
	{
		*res = nullptr;
		std::shared_ptr<VFState* > stackitem = globalStateStack->pop();
		if(stackitem!=nullptr)
		{
			*res = *(stackitem.get());
		}
	}

	inline unsigned GetRemainingStates() {
		return globalStateStack->size();
	}

	void Run(ThreadId thread_id) 
	{
		VFState* s = NULL;
#ifdef DEBUG
		std::cout<<"Thread["<<thread_id<<"] Started\n";
#endif
		while(statesToBeExplored>0)
		{
			GetState(&s, thread_id);
			if(s)
			{
				PreprocessState(thread_id);
				ProcessState(s, thread_id);
				statesToBeExplored--;
				delete s;
				PostprocessState(thread_id);
			}
			UnprocessedState(thread_id);
			//gettimeofday(&(thEndOfSearchTime[thread_id]),NULL);	
		}
		
		if(++endThreadCount == numThreads)
		{
			//std::cout<<"Thread["<<thread_id<<"] is the last\n";
			gettimeofday(&(eos_time),NULL);
		}


		//Last thread
#ifdef DEBUG
		std::cout<<"Thread["<<thread_id<<"] Stopped\n";
#endif
	}

	bool ProcessState(VFState *s, ThreadId thread_id)
	{
#ifdef DEBUG
		std::cout<<"Thread["<<thread_id<<"] Processing\n";
#endif
		if (s->IsGoal())
		{
			if(!once.exchange(true))
			{
				gettimeofday(&(this->fist_solution_time),NULL);
			}

			//threadSolutionCount[thread_id]++;
			solCount++;
			if(storeSolutions)
			{
				MatchingSolution sol;
				s->GetCoreSet(sol);
				solutions.push_back(sol);
			}
			if (visit)
			{
				return (*visit)(*s);
			}
			return true;
		}

		if (s->IsDead())
			return false;

		nodeID_t n1 = NULL_NODE, n2 = NULL_NODE;
		while (s->NextPair(&n1, &n2, n1, n2))
		{
			if (s->IsFeasiblePair(n1, n2))
			{
				ExploreState(s, n1, n2, thread_id);
			}
		}		
		return false;
		
	}

	virtual void ExploreState(VFState *s, nodeID_t n1, nodeID_t n2, ThreadId thread_id)
	{
		statesToBeExplored++;
		VFState* s1 = new VFState(*s);
		s1->AddPair(n1, n2);
		PutState(s1, thread_id);
	}

#ifndef WIN32
	void SetAffinity(int cpu, pthread_t handle)
	{
		cpu_set_t cpuset;
    	CPU_ZERO(&cpuset);
    	CPU_SET(cpu, &cpuset);
		int rc = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
		if (rc != 0) 
		{
			std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
		}
	}
#endif

public:
	ParallelMatchingEngine(unsigned short int numThreads, 
		bool storeSolutions=false,
		bool lockFree=false,
		short int cpu = -1,
		MatchingVisitor<VFState> *visit = NULL):
		MatchingEngine<VFState>(visit, storeSolutions),
		thEndOfSearchTime(numThreads),
		once(false),
		endThreadCount(0),
		cpu(cpu),
		numThreads(numThreads),
		pool(numThreads),
		statesToBeExplored(0){
			if(lockFree)
			{
#ifdef DEBUG
				std::cout<<"Using Lock Free Stack\n";
#endif
				globalStateStack = new LockFreeStack<VFState*>();
			}
			else
			{
				globalStateStack = new SynchronizedStack<VFState*>();
			}
#ifdef DEBUG
			std::cout<<"Started Version VF3PGSS\n";
#endif
		}

	~ParallelMatchingEngine()
	{
		delete globalStateStack;
	}

	bool FindAllMatchings(VFState& s)
	{
		statesToBeExplored++;
		PreMatching(&s);
		gettimeofday(&(this->start_time),NULL);
		StartPool();
		gettimeofday(&(this->pool_time),NULL);

		
		VFState* s0 = new VFState(s);
		PutState(s0, NULL_THREAD);
#ifdef DEBUG
		std::cout<<"First State in GGS\n";
#endif

#ifdef DEBUG
		std::cout<<"Pool Started\n";
#endif

		JoinPool();
		gettimeofday(&(this->exit_time),NULL);
#ifdef VERBOSE
		//Getting higher End Of Search Time
		/*double maxEOFTime = 0;
		double actualEOFTime = 0;
		struct timeval maxtime;
		for(int i=0; i<numThreads; i++)
		{
			actualEOFTime = GetElapsedTime(pool_time, thEndOfSearchTime[i]);
			if(actualEOFTime > maxEOFTime)
			{
				maxEOFTime = actualEOFTime;
				maxtime = thEndOfSearchTime[i];
			}
		}*/

		std::cout<<"Pool Started: "<<GetElapsedTime(start_time, pool_time)<<std::endl;
		//std::cout<<"Last Thread Stopped: "<<maxEOFTime<<std::endl;
		std::cout<<"Pool Closed: "<<GetElapsedTime(pool_time, eos_time)<<std::endl;
		std::cout<<"Pool Closed: "<<GetElapsedTime(eos_time, exit_time)<<std::endl;
#endif
		return true;
	}

	inline size_t GetThreadCount() const {
		return pool.size();
	}

	void ResetSolutionCounter()
	{
		solCount = 0;
		endThreadCount=0;
		once = false;
	}

	void StartPool()
	{
		int current_cpu = cpu;
		for (size_t i = 0; i < numThreads; ++i)
		{
			pool[i] = std::thread([this, i] { this->Run(i); });
#ifndef WIN32
			//If cpu is not -1 set the thread affinity starting from the cpu
			if (current_cpu > -1)
			{
				SetAffinity(current_cpu, pool[i].native_handle());
				current_cpu++;
			}
#endif
		}
	}

	inline void JoinPool()
	{
		//std::cout<<"Waiting for the poll to finish";
		for (auto &th : pool) {
			if (th.joinable()) {
				th.join();
			}
		}
		
		//Counting solution
		/*for (int16_t i = 0; i < numThreads; i++)
		{
			solCount += threadSolutionCount[i];
			threadSolutionCount[i]=0;
			if(storeSolutions)
			{
				std::move(threadSolutionFound[i].begin(), 
					threadSolutionFound[i].end(), std::back_inserter(solutions));
			}
		}*/
	}

};

}

#endif /* INCLUDE_PARALLEL_PARALLELMATCHINGTHREADPOOL_HPP_ */
