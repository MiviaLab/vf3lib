/*----------------------------------------------------
 * @file   VF2SubState.hpp
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date  November, 2017 
 * @brief Definition of the base class representing a state of the matching
 * process between two ARGs.
 *--------------------------------------------------*/

#ifndef STATE_HPP
#define STATE_HPP

#include <cstring>
#include <iostream>
#include <vector>
#include <Error.hpp>
#include <ARGraph.hpp>

#ifdef TRACE
#include <StateTrace.hpp>
#endif

namespace vflib
{
	class State
	{
	protected:
		const State* parent;
		bool used;

		bool induced;
		nodeID_t *order;

		#ifdef TRACE
		uint64_t trace_id = NULL_TRACE_ID;
		#endif

		//Size of each graph
		uint32_t n1, n2;

		//CORE SET SIZES
		uint32_t core_len;       //Current length of the core set
		uint32_t orig_core_len;  //Core set length of the previous state

		nodeID_t added_node1;    //Last added node

		nodeID_t *core_1;
		nodeID_t *core_2;

		int64_t *share_count;  //Count the number of instances sharing the common sets

		//PRIVATE METHODS
		virtual void BackTrack() = 0;

	public:
		State(uint32_t n1, uint32_t n2, nodeID_t* order, bool induced)
		{

			this->order = order;
			this->induced = induced;
			this->n1 = n1;
			this->n2 = n2;

			core_1 = new nodeID_t[n1];
			core_2 = new nodeID_t[n2];
			core_len = orig_core_len = 0;
			share_count = new int64_t;
			added_node1 = NULL_NODE;

			used = false;
			parent = NULL;

			if (!core_1 || !core_2 || !share_count)
			{
				error("Out of memory");
			}

			uint32_t i;
			for (i = 0; i < n1; i++)
			{
				core_1[i] = NULL_NODE;
			}

			for (i = 0; i < n2; i++)
			{
				core_2[i] = NULL_NODE;
			}

			*share_count = 1;
		}

		State(const State& state)
		{
			order = state.order;
			induced = state.induced;

			n1 = state.n1;
			n2 = state.n2;
			core_1 = state.core_1;
			core_2 = state.core_2;

			added_node1 = NULL_NODE;
			core_len = orig_core_len = state.core_len;

			parent = &state;
			used = false;

			share_count = state.share_count;
			++ *share_count;

		}

		~State() 
		{
			if (*share_count == 0)
			{
				delete[] core_1;
				delete[] core_2;
				delete share_count;
			}
		}

		virtual bool NextPair(nodeID_t *pn1, nodeID_t *pn2, 
			nodeID_t prev_n1 = NULL_NODE, nodeID_t prev_n2 = NULL_NODE) = 0;
		virtual bool IsFeasiblePair(nodeID_t n1, nodeID_t n2) = 0;
		virtual void AddPair(nodeID_t n1, nodeID_t n2) = 0;
		virtual bool IsGoal() const = 0;
		virtual bool IsDead() const = 0;
		
		inline uint32_t CoreLen() const { return core_len; }
		inline const State* GetParent() const { return parent; }
		inline bool IsUsed() const { return used; }
		inline void SetUsed() { used = true; }

		#ifdef TRACE
		inline uint64_t GetTraceID(){return trace_id;};
		inline void SetTraceID(uint64_t new_trace_id){trace_id=new_trace_id;};
		#endif

		inline void GetCoreSet(std::vector<std::pair<nodeID_t, nodeID_t> >& core)
		{
			uint32_t i;
			core.resize(n1);
			for (i = 0; i < n1; i++)
			{
				if (core_1[i] != NULL_NODE)
				{
					core[i] = std::pair<nodeID_t, nodeID_t>(i, core_1[i]);
				}
			}
		}
	};
}
#endif

