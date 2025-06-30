# # QUERY SIZE 8
# # RUN VF3-Light
# taskset -c 1 python3 run_test.py \
#     --database_foder=/dataset/DBLP \
#     --dataset_name=DBLP \
#     --light=True \
#     --query_size=8 \
#     --node_induced=0 \
#     --undirected=True \
#     --resume=True \
#     --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/DBLP_graphs_induced_True_node_induced_0_undirected_True_vf_light_True_query_size_8.json

taskset -c 2 python3 run_test.py \
    --database_foder=/dataset/EGSM_datasets_and_querysets/dblp/label_16 \
    --dataset_name=dblp \
    --light=True \
    --query_size=12 \
    --node_induced=0 \
    --undirected=True &

taskset -c 3 python3 run_test.py \
    --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
    --dataset_name=enron \
    --light=True \
    --query_size=8 \
    --node_induced=0 \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/enron_graphs_induced_True_node_induced_0_undirected_True_vf_light_True_query_size_8.json &

taskset -c 4 python3 run_test.py \
    --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
    --dataset_name=enron \
    --light=True \
    --query_size=10 \
    --node_induced=0 \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/enron_graphs_induced_True_node_induced_0_undirected_True_vf_light_True_query_size_10.json &

taskset -c 5 python3 run_test.py \
    --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
    --dataset_name=enron \
    --light=True \
    --query_size=12 \
    --node_induced=0 \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/enron_graphs_induced_True_node_induced_0_undirected_True_vf_light_True_query_size_12.json &

taskset -c 8 python3 run_test.py \
    --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
    --dataset_name=enron \
    --light=True \
    --query_size=14 \
    --node_induced=0 \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/enron_graphs_induced_True_node_induced_0_undirected_True_vf_light_True_query_size_14.json &

taskset -c 10 python3 run_test.py \
    --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
    --dataset_name=enron \
    --light=True \
    --query_size=16 \
    --node_induced=0 \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/enron_graphs_induced_True_node_induced_0_undirected_True_vf_light_True_query_size_16.json &
