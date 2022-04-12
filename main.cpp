#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <algorithm>
#ifndef WIN32
#include <csignal>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#else
#include <Windows.h>
#include <stdint.h>
#endif

#include "WindowsTime.h"
#include "VFLib.h"
#include "Options.hpp"

using namespace vflib;

#ifndef WIN32
void sig_handler(int sig) {
    switch (sig) {
		case SIGKILL:
			std::cout << "Killed \n";
			exit(-1);
		case SIGABRT:
			std::cout << "Aborted \n";
			exit(-1);
		case SIGTERM:
			std::cout << "Terminated \n";
			exit(-1);
		case SIGSEGV:
			std::cout << "Segmentation fault \n";
			exit(-1);
		case SIGSTOP:
			std::cout << "Stopped \n";
			exit(-1);
		case SIGCHLD:
			std::cout << "Child stopped or terminated \n";
			exit(-1);
		default:
			std::cout << "Unexpected signal " <<  sig <<"\n";
			exit(-1);
		}
}
#endif

#ifdef TRACE
std::string& remove_chars(std::string& s, const std::string& chars) {
    s.erase(remove_if(s.begin(), s.end(), [&chars](const char& c) {
        return chars.find(c) != std::string::npos;
    }), s.end());
    return s;
}
#endif

static long long state_counter = 0;

int32_t main(int32_t argc, char** argv)
{
	Options opt;

	uint32_t n1, n2;
	double timeAll = 0;
	double totalExecTime= 0;
	double timeFirst = 0;
	double timeLast  = 0;
	double timeLoad = 0;
	int rep = 0;
	struct timeval start, loading, fastcheck, classification, iter, end;
	std::vector<MatchingSolution> solutions;
	std::vector<uint32_t> class_patt;
	std::vector<uint32_t> class_targ;
	uint32_t classes_count;

	state_counter = 0;
	size_t sols = 0;

#ifndef WIN32
	std::signal(SIGKILL, sig_handler);
	std::signal(SIGABRT, sig_handler);
	std::signal(SIGTERM, sig_handler);
	std::signal(SIGSEGV, sig_handler);
	std::signal(SIGSTOP, sig_handler);
	std::signal(SIGCHLD, sig_handler);
#endif

	if(!GetOptions(opt, argc, argv))
	{
		exit(-1);
	}

#ifdef TRACE
	int status;
	std::string pattern_name(opt.pattern);
	std::string target_name(opt.target);

	pattern_name = pattern_name.substr(pattern_name.find_last_of("/\\") + 1);
	target_name = target_name.substr(target_name.find_last_of("/\\") + 1);

	pattern_name = remove_chars(pattern_name, "./");
	target_name = remove_chars(target_name, "./");

	std::string outpath(argv[3]);

	//mkdir -p foo/bar/zoo/andsoforth
	status = mkdir(outpath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	//std::cout<<status<<" "<<errno<<std::endl;
	std::string outfilename = outpath+"/"+pattern_name+target_name+".cvs";
	//std::cout << outfilename;
#endif

	gettimeofday(&start, NULL);

	std::ifstream graphInPat(opt.pattern);
	std::ifstream graphInTarg(opt.target);

	ARGLoader<data_t, Empty>* pattloader = CreateLoader<data_t, Empty>(opt, graphInPat);
	ARGLoader<data_t, Empty>* targloader = CreateLoader<data_t, Empty>(opt, graphInTarg);

	ARGraph<data_t, Empty> patt_graph(pattloader);
	ARGraph<data_t, Empty> targ_graph(targloader);

	if(opt.verbose)
	{
		gettimeofday(&loading, NULL);
		timeLoad = GetElapsedTime(start, loading);
		std::cout<<"Loaded in: "<<timeLoad<<std::endl;
	}

	n1 = patt_graph.NodeCount();
	n2 = targ_graph.NodeCount();

	MatchingEngine<state_t >* me = CreateMatchingEngine(opt);

	if(!me)
	{
		exit(-1);
	}

	gettimeofday(&start, NULL);
	FastCheck<data_t, data_t, Empty, Empty > check(&patt_graph, &targ_graph);
	if(check.CheckSubgraphIsomorphism())
	{
		if(opt.verbose)
		{
			gettimeofday(&fastcheck, NULL);
			timeLoad = GetElapsedTime(start, fastcheck);
			std::cout<<"FastCheck in: "<<timeLoad<<std::endl;
		}

		gettimeofday(&start, NULL);
		NodeClassifier<data_t, Empty> classifier(&targ_graph);
		NodeClassifier<data_t, Empty> classifier2(&patt_graph, classifier);
		class_patt = classifier2.GetClasses();
		class_targ = classifier.GetClasses();
		classes_count = classifier.CountClasses();

		if(opt.verbose)
		{
			gettimeofday(&classification, NULL);
			timeLoad = GetElapsedTime(start, classification);
			std::cout<<"Classification in: "<<timeLoad<<std::endl;
		}
	}

	#ifndef TRACE
	do
	{
		rep++;
		me->ResetSolutionCounter();

		gettimeofday(&iter, NULL);
	#endif
		if(check.CheckSubgraphIsomorphism())
		{
			VF3NodeSorter<data_t, Empty, SubIsoNodeProbability<data_t, Empty> > sorter(&targ_graph);
			std::vector<nodeID_t> sorted = sorter.SortNodes(&patt_graph);

			#ifdef TRACE
			me->InitTrace(outfilename);
			#endif

			state_t s0(&patt_graph, &targ_graph, class_patt.data(), class_targ.data(), classes_count, sorted.data());
			me->FindAllMatchings(s0);
			#ifdef TRACE
			me->FlushTrace();
			#endif

		}

	#ifndef TRACE
		gettimeofday(&end,NULL);

		totalExecTime += GetElapsedTime(iter, end);
		end = me->GetFirstSolutionTime();
		timeFirst += GetElapsedTime(iter, end);
	} while (totalExecTime < opt.repetitionTimeLimit);
	timeAll = totalExecTime/rep;
	timeFirst /= rep;

	if(opt.storeSolutions)
	{
		me->GetSolutions(solutions);
		std::cout << "Solution Found" << std::endl;
		std::vector<MatchingSolution>::iterator it;
		for(it = solutions.begin(); it != solutions.end(); it++)
		{
			std::cout<< me->SolutionToString(*it) << std::endl;
		}
	}

	#endif
	sols = me->GetSolutionsCount();
	if(opt.verbose)
	{
		std::cout<<"First Solution in: "<<timeFirst<<std::endl;
		std::cout<<"Matching Finished in: "<<timeAll<<std::endl;
		std::cout<<"Solutions: "<<sols<<std::endl;
	}else
	{
		std::cout << sols << " " << timeFirst << " " << timeAll <<std::endl;
	}
	delete me;
  	delete pattloader;
  	delete targloader;
	return 0;
}
