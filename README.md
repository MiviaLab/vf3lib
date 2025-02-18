# vf3lib
vf3lib is a software library containing all the currently published versions of VF3, the fastest algorithm to solve subgraph isomorphism on large and dense graphs. *Extremely efficient in time and memory!* 

This library, written in C++11, contains the official implementations of VF2-Plus, VF3, VF3L, VF3P realized by the authors.
The latest version of vf3lib includes new graph loaders and the parallel versions of VF3L (VF3P), designed to effectively speed-up the algorithm on multicore architectures.

The library is allows to solve: 
* Graph Isomorphism
* Subgraph Isomorphism
* Monomorphism (Non-induced subgraph isomorphism) - *Very Soon!*

## References

If you use VF3 please don't forget to cite us!

1. Challenging the time complexity of exact subgraph isomorphism for huge and dense graphs with VF3 - Carletti V., Foggia P., Saggese A., Vento M. - IEEE transactions on pattern analysis and machine intelligence - 2018
2. Introducing VF3: A new algorithm for subgraph isomorphism - Carletti V., Foggia P., Saggese A., Vento M. - International Workshop on Graph-Based Representations in Pattern Recognition - 2017
3. Comparing performance of graph matching algorithms on huge graphs - Carletti V., Foggia P., Saggese A., Vento M. - Pattern Recognition Letters - 2018
4. A Parallel Algorithm for Subgraph Isomorphism - V. Carletti, P. Foggia, P. Ritrovato, M. Vento, V. Vigilante - International Workshop on Graph-Based Representations in Pattern Recognition - 2019

## How To Use It
The provided Makefile will produce three different executables:
- VF3: The algorith whit all the heustics
- VF3L: A lightweight version, where the look-ahead is deactivated. This version fit for sparse or small graphs.
- VF3P: A parallel version of VF3L, to be used when the problem is really hard! 

If you wish to use the sequential version of VF (VF3 or VF3L) execute the following commandline: 

> vf3 [pattern] [target]

```bash
$./bin/vf3 ./test/bvg1.sub.grf ./test/bvg1.grf
8 6.34141e-06 1.27038e-05
```

The standard output provided by the algorithm is: [number of solutions found] [time to find the first solution] [time to find all the solutions]

The following additional parameters can be added to the commandline:
* -r Repetition time limit in seconds. The matching is repeted multiple times until the overall execution time breaches the given repetition time limit. The proposed execution time provided by the executable is the average value among all the executions performed. To be used when you wish to benchmark the algorithm on very small graphs, where the execution time of a single run is extremely small (eg. milliseconds), on order to get the execution time properly. (Default: 1 sec)
* -u Force the loader to read the graphs as undirected. (Default: false)
* -v Verbose mode. Additional time information are provides, such as loading time. (Default: false)
* -s Print all the solutions (not only the number of solutions found) (Default: false)
* -f Loader file format. Using this parameter you can specify the format of the graphs to be loaded: (Default: vf)
  * vf: stanard VF file format. Commonl used by the MIVIA Graph datasets
  * edge: Edge file format commonly used on VLDB datasets such as (Patents, WebGoogle, etc...)

### VF3P additional parameters
The parallel version has the following extra parameters:
* -a Version of the paralle strategy to be used: (Mandatory)
  1. (1) Parallel Version using the Global State Stack (GSS) only
  2. (2) Parallel Version using the additional Local State Stack (LSS).
* -t Number of thread to be used (Mandatory)
* -c First CPU to be used when the thread are pinned on CPUs. The threads are pinned on different successive CPUs starting from the one has been specified. If 0 the pinning is disabled. (Default: 0)
* -l LSS size limit. Maximum number of states in the LSS. Value 0 correspond to the pattern size (Default: 0)
* -h GSS depth limit. The states belonging to the first "h" levels of the State Space are forced to be put in the GSS. 0 means only the first state is put in the GSS (Default 3)
* -k Use a lock-free stack as GSS

## Datasets
VF3 has been benchmarked on the following databases of graphs:
* [MIVIA ARG](https://mivia.unisa.it/datasets/graph-database/arg-database/)
* [MIVIA LDG](https://mivia.unisa.it/datasets/graph-database/mivia2-graph-database/)
* [MIVIA Biological](https://drive.google.com/file/d/0B2AvWp2gRW0QU2JJdGhkSFUyT3M/view?usp=sharing)

## VF File Formats

### Binary
The file is composed by a sequence of 16-bit words; the words are encoded in little-endian format (e.g., LSB first).
The first word represents the number of nodes in the graph. Then, for each node, there is a word encoding the number of edges coming out of that node, followed by a sequence of words encoding the endpoints of those edges.
An example, represented in hexadecimal, follows:

```
03 00     Number of nodes (3)
00 00     Number of edges out of node 0 (0)
02 00     Number of edges out of node 1 (2)
00 00     Target of the first edge of node 1 (edge 1 -> 0)
02 00     Target of the second edge of node 1 (edge 1 -> 2)
01 00     Number of edges out of node 2 (1)
00 00     Target of the first (and only) edge of node 2 (edge 2 -> 0)
```
    
### Text
On the first line there must be the number of nodes; subsequent lines will contain the node attributes, one node per line, preceded by the node id; node ids must be in the range from 0 to the number of nodes - 1.
Then, for each node there is the number of edges coming out of the node, followed by a line for each edge containing the ids of the edge ends and the edge attribute. 
Blank lines, and lines starting with #, are ignored.
An example file, where both node and edge attributes are ints, could be the following:

```
# Number of nodes
3

# Node attributes
0 27
1 42
2 13	 

# Edges coming out of node 0\n
2
0 1  24
0 2  73

# Edges coming out of node 1
1
1 3  66

# Edges coming out of node 2
0
```
