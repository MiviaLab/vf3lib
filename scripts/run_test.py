
import argparse
import glob
import os
from collections import OrderedDict
import subprocess
import time
import json
import psutil

TIMEOUT_MAX = 3600 # 1 hrs


def run_vf3(exe_path: str, res_dir: str, log_file: str, error_file: str, query_path: str, target_path: str, results_dict: dict, args: list, query_size: str, query_indx: str, labels: str):
    
    print("Running VF3")
    command = [exe_path, query_path, target_path] + args
    print(command)
    # query_size = query_path.split('/')[3]
    # query_indx = query_path.split('/')[-1].split('.')[0].split('_')[-2] if 'original_labels' not in query_path else query_path.split('/')[-1].split('.')[0].split('_')[-1]
    # labels = query_path.split('/')[-1].split('.')[0].split('_')[-1] if 'original_labels' not in query_path else 'original_labels'
    
    if query_size not in results_dict:
        results_dict[query_size] = OrderedDict()
    
    if query_indx not in results_dict[query_size]:
        results_dict[query_size][query_indx] = OrderedDict()
        
    if labels not in results_dict[query_size][query_indx]:
        results_dict[query_size][query_indx][labels] = OrderedDict()    
    
    # print(command)
    process = subprocess.Popen(command, stdout = open(log_file, "w"), stderr = open(error_file, 'w'))

    try:
        _, _ = process.communicate(timeout=TIMEOUT_MAX)
        # print(stderr)
        if process.returncode == 0:
            print("Completed check the results")
            with open(log_file, 'r') as f:
                rows = f.readlines()
                
            for indx, row in enumerate(rows):
                if indx == 0:
                    num_sol = int(row.split(' ')[0])
                    first_solution_time = float(row.split(' ')[-1].split(' ')[0])
                    running_time = float(row.split(' ')[-1].split('\n')[0])
                
            results_dict[query_size][query_indx][labels]['success'] = 1    
            results_dict[query_size][query_indx][labels]['num_sol'] = num_sol
            results_dict[query_size][query_indx][labels]['first_solution_time'] = first_solution_time
            results_dict[query_size][query_indx][labels]['running_time'] = running_time
            
        else:
            
            with open(error_file, 'r') as f:
                rows = f.readlines()
            
            error_str = ''
            for row in rows:
                error_str += row
                
            print(f"Error info {error_str}")   
            
            results_dict[query_size][query_indx][labels]['success'] = 0
            results_dict[query_size][query_indx][labels]['error_info'] = error_str     
            
    except Exception as e:
        print(f"Error: {e}")
       
        results_dict[query_size][query_indx][labels]['success'] = 0
        results_dict[query_size][query_indx][labels]['error_info'] = 'Timeout'
        print('Terminating process')
        process = psutil.Process(process.pid)
        for proc in process.children(recursive=True):
            proc.kill()
        process.kill()
        
    # update RESULTS
    algo_prop = log_file.split('/')[-1].split('.')[0].split('log_')[-1]
    with open(os.path.join(res_dir, f"{algo_prop}.json"), 'w') as f:
        json.dump(results_dict, f)
    
    return results_dict
   
   
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--res_folder', type=str,
                        default='/graph-matching-analysis/baseline_algorithms/Database')
    parser.add_argument('--database_foder', type=str,
                        default='/dataset/DBLP')
    parser.add_argument('--dataset_name', type=str,
                        default='DBLP')
    parser.add_argument('--bin_path', type=str, default='/graph-matching-analysis/baseline_algorithms/vf3lib/bin')
    parser.add_argument('--edge_induced', type=bool, default=False)
    parser.add_argument('--graphs_induced', type=bool, default=True)
    parser.add_argument('--resume', type=bool, default=False)
    parser.add_argument('--resume_file', type=str, default='')
    parser.add_argument('--undirected', type=bool, default=False)
    parser.add_argument('--light', type=bool, default=False)
    parser.add_argument('--query_size', type=int, default=64)

    
    args = parser.parse_args()
    
    results_dict = OrderedDict()
    if args.resume:
        with open(args.resume_file, 'r') as f:
            results_dict = json.load(f)
        
    print(f"Node Induced: {not args.edge_induced} - Graphs Induced: {args.graphs_induced} - Undirected: {args.undirected} - Light: {args.light}")
    

    print(f"Testing Query Size: {args.query_size}")
    if args.dataset_name == 'DBLP':
        query_folder = f"{args.database_foder}/{args.query_size}"
        if args.graphs_induced:
            query_folder = f"{query_folder}/node_induced"
        else:
            query_folder = f"{query_folder}/edge_induced"
        # query_folder = f"{query_folder}/node_induced"
        # 'original_labels', 'label_64', 
        LABEL_SIZE = ['original_labels', 'label_64', 'label_32', 'label_16', 'label_8', 'label_4', 'label_2'] 
    elif args.dataset_name == 'enron' or args.dataset_name == 'dblp':
        query_folder = os.path.join(args.database_foder, "query_graph", f"{args.query_size}")
        LABEL_SIZE = ['-1']
    
    # # create log file 
    log_file = os.path.join(os.path.dirname(os.path.realpath(__file__)), f"log_{args.dataset_name}_graphs_induced_{args.graphs_induced}_node_induced_{not args.edge_induced}_undirected_{args.undirected}_vf_light_{args.light}_query_size_{args.query_size}.txt")

    error_file = os.path.join(os.path.dirname(os.path.realpath(__file__)), f"log_{args.dataset_name}_graphs_induced_{args.graphs_induced}_node_induced_{not args.edge_induced}_undirected_{args.undirected}_vf_light_{args.light}_query_size_{args.query_size}_graphs_error.txt")

    
    for label_indx, label_size in enumerate(LABEL_SIZE):
        print(f"\tTesting Label Size: {label_size}")
        final_query_folder = f"{query_folder}/{label_size}" if label_size != '-1' else query_folder
        print(f"\tQuery Folder: {final_query_folder}")
        query_files = glob.glob(f"{final_query_folder}/*.sub.grf")

        # order query_files based on the query indx
        query_files.sort(key=lambda x: int(x.split('/')[-1].split('.')[0].split('_')[-1]) if 'original_labels' not in x else int(x.split('/')[-1].split('.')[0].split('_')[-1]))

        print(f"\tNumber of Queries: {len(query_files)}")
        
        # data file path
        if args.dataset_name == 'DBLP':
            if 'original_labels' in label_size:
                data_path = f"{args.database_foder}/data.grf"
            else:
                label_num = int(label_size.split('_')[-1])
                data_path = f"{args.database_foder}/data_{label_num}.grf"
        else:
            data_path = f"{args.database_foder}/data.grf"
            
        for idx, query_file in enumerate(query_files):
            print(query_file)
            
            qs = query_file.split('/')[3] if args.dataset_name == 'DBLP' else query_file.split('/')[-2] 
            if args.dataset_name == 'DBLP':
                qi = query_file.split('/')[-1].split('.')[0].split('_')[-2] if 'original_labels' not in query_file else query_file.split('/')[-1].split('.')[0].split('_')[-1] 
                
                lab_size = query_file.split('/')[-1].split('.')[0].split('_')[-1] if 'original_labels' not in query_file else 'original_labels'
            elif args.dataset_name == 'enron' or args.dataset_name == 'dblp':
                qi = query_file.split('/')[-1].split('.')[0].split('_')[-1]
                lab_size = query_file.split('/')[-4].split('_')[-1]
                
            print(f"\t\tQuery: {qs} - {qi} - {lab_size}")
            
            
            # print(qs, qi, lab)
            
            if qs in results_dict.keys() and qi in results_dict[qs].keys() and lab_size in results_dict[qs][qi].keys():
                print(f"\t\tQuery: {query_file} already tested")
            else:
                # check for the previous label size
                if args.dataset_name == 'DBLP':
                    previous_label = LABEL_SIZE[label_indx - 1] if label_indx > 0 else None
                    if previous_label is not None:
                        previsous_label_size = previous_label.split('_')[-1]
                    
                    if qs in results_dict.keys() and qi in results_dict[qs].keys() and previsous_label_size in results_dict[qs][qi].keys():
                        
                        if results_dict[qs][qi][previsous_label_size]['success'] == 0 and "Timeout" in results_dict[qs][qi][previsous_label_size]['error_info']:
                            print(f"\t\t\tQuery: {query_file} failed with previous label size for timeout")
                
                            # update the results
                            print(f"\t\t\tQuery: {query_file} failed with previous label size for timeout")
                            results_dict[qs][qi][lab_size] = OrderedDict()
                            results_dict[qs][qi][lab_size]['success'] = 0
                            results_dict[qs][qi][lab_size]['error_info'] = 'Timeout'
                            
                            algo_prop = log_file.split('/')[-1].split('.')[0].split('log_')[-1]
                            with open(os.path.join(os.path.dirname(os.path.realpath(__file__)), f"{algo_prop}.json"), 'w') as f:
                                json.dump(results_dict, f)                          
                            continue
                
                
                print(f"\t\tTesting Query: {query_file}")
                print(f"\t\tData File: {data_path}")
                args_vf = []
              
                # run VF3 algorithm
                if args.light == False:
                    exe_path = f"{args.bin_path}/vf3"
                else:
                    exe_path = f"{args.bin_path}/vf3l"

                if args.edge_induced:
                    # use the flag -e for edge induced subgraph isomorphism
                    args_vf.append('-e')

                if args.undirected:
                    args_vf.append('-u')    
          
                # print(log_file)
                # print(f"\t\tAlgo prob {log_file.split('/')[-1].split('.')[0].split('log_')[-1]}")
                results_dict = run_vf3(exe_path=exe_path,
                                res_dir = os.path.dirname(os.path.realpath(__file__)),
                                log_file=log_file, 
                                error_file = error_file,
                                query_path=query_file, 
                                target_path=data_path, 
                                args=args_vf,
                                results_dict=results_dict,
                                query_size=qs, 
                                query_indx=qi,                             
                                labels=lab_size
                                )
                
                
                
                
                
            
                        
