# vf3lib
VF3 algorithm is the fastest sequential algorithm to solve subgraph isomorphism on large and dense graphs.

Extremely efficient in time and memory! 

This library, written in C++11, contains the official implementations of VF2-Plus, VF3, VF3L realized by the authors.
The latest version of vf3lib includes new graph loaders and the latest parallel versions of VF3L, designed to speed-up the
algorithm on multicore architectures.

The library contains solvers for: 
* Graph Isomorphism
* Subgraph Isomorphism

If you use VF3 please don't forget to cite us!

References.
1. Challenging the time complexity of exact subgraph isomorphism for huge and dense graphs with VF3 - Carletti V., Foggia P., Saggese A., Vento M. - IEEE transactions on pattern analysis and machine intelligence - 2018
2. Introducing VF3: A new algorithm for subgraph isomorphism - Carletti V., Foggia P., Saggese A., Vento M. - International Workshop on Graph-Based Representations in Pattern Recognition - 2017
3. Comparing performance of graph matching algorithms on huge graphs - Carletti V., Foggia P., Saggese A., Vento M. - Pattern Recognition Letters - 2018
4. A Parallel Algorithm for Subgraph Isomorphism - V. Carletti, P. Foggia, P. Ritrovato, M. Vento, V. Vigilante - International Workshop on Graph-Based Representations in Pattern Recognition - 2019

## How To Use It
The provided Makefile will produce three different executables:
- VF3: The algorith whit all the heustics
- VF3L: A lightweight version, where the look-ahead is deactivated. This version fit for sparse or small graphs.
- VF3P: A parallel version of VF3L, to be used when the problem is really hard! 

If you wish to use the sequential version of VF (VF3 or VF3L) 

> vf3 [pattern] [target]

'''
$./bin/vf3 ./test/bvg1.sub.grf ./test/bvg1.grf
'''bash
