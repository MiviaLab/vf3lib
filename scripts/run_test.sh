#### ---- Node Induced ---- ####

# QUERY SIZE 64
# RUN VF3-Light
taskset -c 1 python3 run_test.py \
    --light=True \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_True_query_size_64.json \
    --query_size=64

# RUN VF3
taskset -c 1 python3 run_test.py \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_False_query_size_64.json \
    --query_size=64

# QUERY SIZE 32
# RUN VF3-Light
taskset -c 7 python3 run_test.py \
    --light=True \
    --undirected=True \
    --query_size=32 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_True_query_size_32.json

# RUN VF3
taskset -c 8 python3 run_test.py \
    --undirected=True \
    --query_size=32 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_False_query_size_32.json

# QUERY SIZE 16
# RUN VF3-Light
taskset -c 3 python3 run_test.py \
    --light=True \
    --undirected=True \
    --query_size=16 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_True_query_size_16.json

# RUN VF3
taskset -c 4 python3 run_test.py \
    --undirected=True \
    --query_size=16 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_False_query_size_16.json

# QUERY SIZE 8
# RUN VF3-Light
taskset -c 1 python3 run_test.py \
    --light=True \
    --undirected=True \
    --query_size=8 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_True_query_size_8.json

# RUN VF3
taskset -c 2 python3 run_test.py \
    --undirected=True \
    --query_size=8 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_False_query_size_8.json

#### ---- Edge Induced ---- ####
# QUERY SIZE 64
# RUN VF3-Light
taskset -c 1 python3 run_test.py \
    --edge_induced=True \
    --light=True \
    --undirected=True \
    --query_size=64 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_True_query_size_64.json

# RUN VF3
taskset -c 3 python3 run_test.py \
    --query_size=64 \
    --edge_induced=True \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_0_undirected_True_vf_light_False_query_size_64.json

# QUERY SIZE 32
# RUN VF3-Light
taskset -c 7 python3 run_test.py \
    --light=True \
    --undirected=True \
    --query_size=32 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_True_query_size_32.json

# RUN VF3
taskset -c 4 python3 run_test.py \
    --query_size=32 \
    --edge_induced=True \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_0_undirected_True_vf_light_False_query_size_32.json

# QUERY SIZE 16
# RUN VF3-Light
taskset -c 3 python3 run_test.py \
    --light=True \
    --undirected=True \
    --query_size=16 \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_True_query_size_16.json

# RUN VF3
taskset -c 5 python3 run_test.py \
    --query_size=16 \
    --edge_induced=True \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_0_undirected_True_vf_light_False_query_size_16.json

# QUERY SIZE 8
# RUN VF3-Light
taskset -c 1 python3 run_test.py \
    --light=True \
    --query_size=8 \
    --edge_induced=True \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_True_undirected_True_vf_light_True_query_size_8.json

# RUN VF3
taskset -c 6 python3 run_test.py \
    --query_size=8 \
    --edge_induced=True \
    --undirected=True \
    --resume=True \
    --resume_file=/graph-matching-analysis/baseline_algorithms/vf3lib/scripts/graphs_induced_True_node_induced_0_undirected_True_vf_light_False_query_size_8.json
