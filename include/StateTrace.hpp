#ifndef STATETRACE_HPP
#define STATETRACE_HPP

#include <cinttypes>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <limits>
#include <vector>

/**
 * Class used to keep trace of the exploration of VF3
 * The information collected for each state are:
 * - counter
 * - Parent id
 * - is_goal
 * - depth
 * - leaf (state without descendant)
 * - number of descendants (feasible states)
 * - number of candidates
 **/
namespace vflib{

const uint64_t NULL_TRACE_ID = (std::numeric_limits<uint64_t>::max)();	/**<Null ID value */

class StateTrace
{
    private:

        typedef struct {
            uint64_t id = NULL_TRACE_ID;
            uint64_t parent = NULL_TRACE_ID;
            bool goal = false;
            bool feasible = false;
            bool leaf = true;
            uint32_t depth = 0;
            uint32_t descendants_count = 0;
            uint32_t candidates_count = 0;
        }StateInfo;

        StateInfo last_info;
        uint64_t next_id = 0;
        std::ofstream output_stream;
        

    public:

        StateTrace (std::string filename):output_stream(filename)
        {
            output_stream << "Id;Parent;Depth;Descendants;Candidates;Goal;Feasible;Leaf\n";
        }

        inline uint64_t AppendCandidateState(uint64_t parent_id, uint32_t depth)
        {   
            // Flush last info
            if (last_info.parent != last_info.id)
            {
                CSVFlush();
                last_info.id = NULL_TRACE_ID;
                last_info.parent = NULL_TRACE_ID;
                last_info.goal = false;
                last_info.feasible = false;
                last_info.leaf = true;
            }

            last_info.id = next_id++;
            last_info.parent = parent_id;
            last_info.depth = depth;
            return last_info.id;
        }

        inline uint32_t GetLastDepth(){ return last_info.depth; }

        inline void SetGoal()
        {            
            last_info.goal = true;
            last_info.leaf = true;
        }

        inline void SetFeasible()
        {
            last_info.feasible = true;
            last_info.leaf = false;
        }
        
        inline void SetLeaf(){ last_info.leaf = true; }

        void CSVFlush()
        {
            output_stream << last_info.id << ";"  
                << last_info.parent << ";"
                << last_info.depth << ";"
                << last_info.descendants_count << ";"
                << last_info.candidates_count << ";"
                << ((last_info.goal) ? "1" : "0") << ";"
                << ((last_info.feasible) ? "1" : "0") << ";"
                << ((last_info.leaf) ? "1" : "0") << std::endl;
            
        }
};

};
#endif /*STATETRACE_HPP*/
