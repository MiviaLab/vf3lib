/**
 * @file MatchingEngine.hpp
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date   24/10/2017
 * @brief  Declaration of the matching engine.
 */

#ifndef MATCH_H
#define MATCH_H

#include <atomic>
#include <stack>
#include <string>
#include <iostream>
#include <sstream>
#include "ARGraph.hpp"

#ifdef TRACE
#include "StateTrace.hpp"
#endif

#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#else
#include <Windows.h>
#include <stdint.h>
#include "WindowsTime.h"
#endif

namespace vflib
{
	typedef std::vector<std::pair<nodeID_t, nodeID_t> > MatchingSolution;

	template <typename VFState >

	/*
	* @class MatchingVisitor
	* @brief Functor invoked when a new solution is found
	* @details Extends this class to add action to perfom when a new solution is found
	* 	The Matching vistor must be passed to the constructor of the Matching Engine to be used
	*	MatchingEngine e = new MatchingEngine(visitor)
	*/
	class MatchingVisitor
	{
	public:
		/**
		* @brief Definition of the visitor type.\n
		* @details A match visitor is a function that is invoked for each match that has been found.
		* If the function returns FALSE, then the next match is
		* searched; else the seach process terminates.
		* @param [in] s Goal VFState Found
		* @return TRUE If the matching process must be stopped.
		* @return FALSE If the matching process must continue.
		*/
		virtual bool operator()(VFState& state) = 0;

		virtual ~MatchingVisitor();
	};

	template <typename VFState >
	class MatchingEngine
	{
	protected:
		/*
		* @note  core1 and core2 will contain the ids of the corresponding nodes
		* in the two graphs.
		*/
		std::vector<MatchingSolution> solutions;
		MatchingVisitor<VFState> *visit;
		uint32_t solCount;
		bool storeSolutions;
		struct timeval fist_solution_time;

		#ifdef TRACE
		StateTrace *trace;
		#endif

	public:
		MatchingEngine(bool storeSolutions = false): visit(NULL), 
			solCount(0), storeSolutions(storeSolutions){};

		MatchingEngine(MatchingVisitor<VFState> *visit, bool storeSolutions = false):
		 visit(visit), solCount(0), storeSolutions(storeSolutions){}

		#ifdef TRACE
		inline void FlushTrace()
		{
			trace->CSVFlush();
		}
		#endif

		#ifdef TRACE
		inline void InitTrace(std::string filename)
		{
			trace = new StateTrace(filename);
		}
		#endif

		inline size_t GetSolutionsCount() { return (size_t)solCount; }
		
		inline void GetSolutions(std::vector<MatchingSolution>& sols)
		{
			sols = solutions;
		}

		inline void EmptySolutions() {solutions.clear();}

		virtual void ResetSolutionCounter()
		{
			solCount = 0;
			solutions.clear();
		}

		inline struct timeval GetFirstSolutionTime()
		{
			return fist_solution_time;
		}

		std::string SolutionToString(MatchingSolution& sol)
		{
			MatchingSolution::iterator it;
			std::stringstream ss;
			for(it = sol.begin(); it != sol.end(); it++)
			{
				ss << it->second << "," << it->first << ":";
				/*if(it+1 != sol.end())
				{
					ss << ":";
				}*/
			}
			return ss.str();
		}

		/**
		* @brief  Finds a matching between two graph, if it exists, given the initial state of the matching process.
		* @param [in] s Initial VFState.
		* @return TRUE If the matching process finds a solution.
		* @return FALSE If the matching process doesn't find solutions.
		*/
		virtual bool FindFirstMatching(VFState &s)
		{

			#ifdef TRACE
			// Stato figlio s0
			if (s.GetTraceID()==NULL_TRACE_ID) 
			{	
				// Aggiorno id dello stato corrente
				s.SetTraceID(trace->AppendCandidateState(NULL_TRACE_ID, 0));

				trace->SetFeasible();
			}
			
			// Chiamata ricorsiva subito dopo aver aggiunto uno stato
			uint32_t current_depth = trace->GetLastDepth();

			bool one_pair = false;
			#endif

			if (s.IsGoal())
			{
				#ifdef TRACE
				// soluzione
				trace->SetGoal();
				#endif

				solCount++;
				if(storeSolutions)
				{
					MatchingSolution sol;
					s.GetCoreSet(sol);
					solutions.push_back(sol);
				}
				if (visit)
				{
					(*visit)(s);
				}
				return true;
			}

			if (s.IsDead())
			{
				#ifdef TRACE
				// soluzione
				trace->SetLeaf();
				#endif
				return false;
			}

			nodeID_t n1 = NULL_NODE, n2 = NULL_NODE;
			bool found = false;

			while (!found && s.NextPair(&n1, &n2, n1, n2))
			{
				#ifdef TRACE
				// aggiungo ogni candidato
				uint64_t child_id = trace->AppendCandidateState(s.GetTraceID(),current_depth+1);

				// tengo conto che abbia generato almeno 1 candidato
				one_pair = true;
				#endif

				if (s.IsFeasiblePair(n1, n2))
				{
					VFState s1(s);
					s1.AddPair(n1, n2);

					#ifdef TRACE
					// feasible candidate
					trace->SetFeasible();

					// Avendo generato uno stato ne aggiorno il TRACE_ID
					s1.SetTraceID(child_id);
					#endif

					found = FindFirstMatching(s1);
				}

			}

			#ifdef TRACE
			if (!one_pair)
			{
				trace->SetLeaf();
			}
			#endif

			return found;
		}

		/**
		* @brief Visits all the matchings between two graphs, starting from state s.
		* @param [in] s Initial VFState.
		* @param [in/out] usr_data User defined parameter for the visitor.
		* @return TRUE If if the caller must stop the visit.
		* @return FALSE If if the caller must continue the visit.
		*/
		virtual bool FindAllMatchings(VFState &s)
		{
			#ifdef TRACE
			// Stato figlio s0
			if (s.GetTraceID()==NULL_TRACE_ID) 
			{	
				// Aggiorno id dello stato corrente
				s.SetTraceID(trace->AppendCandidateState(NULL_TRACE_ID, 0));

				trace->SetFeasible();
			}
			
			// Chiamata ricorsiva subito dopo aver aggiunto uno stato
			uint32_t current_depth = trace->GetLastDepth();

			bool one_pair = false;
			#endif

			if (s.IsGoal())
			{
				#ifdef TRACE
				// soluzione
				trace->SetGoal();
				#endif

				if(!solCount)
					gettimeofday(&fist_solution_time, NULL);

				solCount++;
				if(storeSolutions)
				{
					MatchingSolution sol;
					s.GetCoreSet(sol);
					solutions.push_back(sol);
				}
				if (visit)
				{
					return (*visit)(s);
				}
				return false;
			}

			if (s.IsDead())
			{
				#ifdef TRACE
				// soluzione
				trace->SetLeaf();
				#endif
				return false;
			}

			nodeID_t n1 = NULL_NODE, n2 = NULL_NODE;
			while (s.NextPair(&n1, &n2, n1, n2))
			{
				#ifdef TRACE
				// aggiungo ogni candidato
				uint64_t child_id = trace->AppendCandidateState(s.GetTraceID(),current_depth+1);

				// tengo conto che abbia generato almeno 1 candidato
				one_pair = true;
				#endif

				if (s.IsFeasiblePair(n1, n2))
				{
					VFState s1(s);
					s1.AddPair(n1, n2);
					#ifdef TRACE
					// feasible candidate
					trace->SetFeasible();

					// Avendo generato uno stato ne aggiorno il TRACE_ID
					s1.SetTraceID(child_id);
					#endif

					if (FindAllMatchings(s1))
					{
						return true;
					}
				}
			}

			#ifdef TRACE
			if (!one_pair)
			{
				trace->SetLeaf();
			}
			#endif

			return false;
		}
	};

}
#endif
