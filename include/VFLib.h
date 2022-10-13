#ifndef VFLIB_H
#define VFLIB_H

#include "ARGraph.hpp"
#include "FastCheck.hpp"
#include "MatchingEngine.hpp"
#include "NodeClassifier.hpp"
#include "NodeSorter.hpp"
#include "ProbabilityStrategy.hpp"
#include "RINodeSorter.hpp"
#include "State.hpp"
#include "VF3NodeSorter.hpp"
#include "loaders/ARGLoader.hpp"
#include "loaders/EdgeStreamARGLoader.hpp"
#include "loaders/FastStreamARGLoader.hpp"

#ifndef VF3BIO
typedef int32_t data_t;
#else
typedef std::string data_t;
#endif

#ifdef VF3
#include "VF3SubState.hpp"

typedef vflib::VF3SubState<data_t, data_t, vflib::Empty, vflib::Empty>
    noedgelabel_state_t;
typedef vflib::VF3SubState<data_t, data_t, data_t, data_t> state_t;
#elif defined(VF3L)
#include "VF3LightSubState.hpp"
typedef vflib::VF3LightSubState<data_t, data_t, vflib::Empty, vflib::Empty>
    noedgelabel_state_t;
typedef vflib::VF3LightSubState<data_t, data_t, data_t, data_t> state_t;
#elif defined(VF3P)
#include "parallel/CloneableVF3ParallelSubState.hpp"
#include "parallel/ParallelMatchingEngine.hpp"
#include "parallel/ParallelMatchingEngineWLS.hpp"

typedef vflib::CloneableVF3ParallelSubState<data_t, data_t, vflib::Empty,
                                            vflib::Empty>
    noedgelabel_state_t;
typedef vflib::CloneableVF3ParallelSubState<data_t, data_t, data_t, data_t>
    state_t;
#endif

#endif /* VFLIB_H*/
