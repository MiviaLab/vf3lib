
import argparse
import glob
import os
from collections import OrderedDict

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--query_folder', type=str,
                        default='/graph-matching-analysis/baseline_algorithms/Database')
    parser.add_argument('--dest_folder_path', type=str,
                        default='/graph-matching-analysis/vfgpu/test')
    parser.add_argument('--graph_path', type=str,
                        default='EGSM_datasets_and_querysets/enron/label_16/query_graph/9')
    args = parser.parse_args()