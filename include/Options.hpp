#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#ifndef WIN32
#include <unistd.h>
#endif

#include <cinttypes>
#include <stdio.h>
#include <iostream>
#include "VFLib.h"

#define VF3PGSS	 (1)
#define VF3PWLS  (2)

struct OptionStructure
{
	char *pattern;
	char *target;
	bool undirected;
	bool storeSolutions;
#ifdef VF3P
	int8_t algo;
	int16_t cpu;
	int16_t numOfThreads;
	bool lockFree;
	int16_t ssrHighLimit;
	int16_t ssrLocalStackLimit;
#endif
	bool verbose;
	std::string format;
	float repetitionTimeLimit;

	OptionStructure():
		pattern(nullptr),
		target(nullptr),
		undirected(false),
		storeSolutions(false),
#ifdef VF3P
		algo(1),
		cpu(-1),
		numOfThreads(1),
		lockFree(0),
		ssrHighLimit(3),
		ssrLocalStackLimit(10),
#endif
		verbose(0),
		format("vf"),
		repetitionTimeLimit(1){}
};

typedef OptionStructure Options;


void PrintUsage()
{
	std::string outstring = "vf3 [pattern] [target] ";
#ifdef VF3P
  outstring += "-c [start cpu] -t [# of threads] -a [version id] -h [SSR high limit] -l [local stack limit] -k ";
#endif
	outstring += "-u -s -f [graph format]";
	std::cout<<outstring<<std::endl;
}


bool GetOptions(Options &opt, int argc, char **argv)
{
	/*
	* -c Start CPU for the pool allocation
	* -t Number of threads. Default [1]
	* -a Version of the matcher to use. Default -1 is VF3
	* -h SSR limit for the global stack. Default 3
	* -l Local Stack limit. Default is pattern size
	* -u Load graphs as undirected
	* -k LockFree Version
	* -r Minimum time in second for benchmark repetitions. Default 1.
	* -s Print Solutions
	* -f Graph format [vf, edge]
	* -v Verbose: show all time
	*/
#ifdef VF3P
	std::string optionstring = ":a:c:t:r:f:h:l:sukv";
#else
  std::string optionstring = ":r:f:suv";
#endif

	char option;
	while ((option = getopt (argc, argv, optionstring.c_str())) != -1)
	{
		switch (option)
		{
#ifdef VF3P
			case 'a':
				opt.algo = atoi(optarg);
				break;
			case 'c':
				opt.cpu = atoi(optarg);
				break;
			case 'k':
				opt.lockFree=1;
				break;
			case 't':
				opt.numOfThreads = atoi(optarg);
				break;
			case 'h':
				opt.ssrHighLimit = atoi(optarg);
				break;
			case 'l':
				opt.ssrLocalStackLimit = atoi(optarg);
				break;
#endif
      case 's':
				opt.storeSolutions = true;
				break;
			case 'r':
				opt.repetitionTimeLimit = atof(optarg);
        break;
      		case 'u':
				opt.undirected = true;
				break;
			case 'v':
				opt.verbose = true;
				break;
			case 'f':
				opt.format = std::string(optarg);
				break;
			case '?':
				PrintUsage();
				return false;
		}

	}
	//additional parameter
	if(argc < 2)
	{
		PrintUsage();
		return false;
	}

	opt.pattern = argv[optind];
	opt.target = argv[optind+1];
	return true;
}

template<typename Node, typename Edge>
vflib::ARGLoader<Node, Edge>* CreateLoader(const Options& opt, std::istream &in)
{

	if(opt.format == "vf")
  {
		return new vflib::FastStreamARGLoader<Node, Edge>(in, opt.undirected);
  }
	else if(opt.format == "edge")
  {
		return new vflib::EdgeStreamARGLoader<Node, Edge>(in, opt.undirected);
	}
	else
	{
		return nullptr;
	}
}

vflib::MatchingEngine<state_t>* CreateMatchingEngine(const Options& opt)
{
#ifdef VF3P
	switch(opt.algo)
	{
		case VF3PGSS:
			return new vflib::ParallelMatchingEngine<state_t >(opt.numOfThreads, opt.storeSolutions, opt.lockFree, opt.cpu);
		case VF3PWLS:
			return new vflib::ParallelMatchingEngineWLS<state_t >(opt.numOfThreads, opt.storeSolutions, opt.lockFree,
                opt.cpu, opt.ssrHighLimit, opt.ssrLocalStackLimit);
		default:
			std::cout<<"Wrong Algorithm Selected\n";
			std::cout<<"1: VF3P with GSS Only\n";
			std::cout<<"2: VF3P with Local Stack and limited depth\n";
			return nullptr;
	}
#elif defined(VF3) || defined(VF3L)
    return new vflib::MatchingEngine<state_t >(opt.storeSolutions);
#endif
}

#endif /* OPTIONS */
