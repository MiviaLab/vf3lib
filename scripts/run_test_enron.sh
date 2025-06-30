# QUERY SIZE 8
# RUN VF3-Light

# taskset -c 9 python3 run_test.py \
#     --dataset_name=enron \
#     --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
#     --light=True \
#     --query_size=8 \
#     --node_induced=1 \
#     --undirected=True

# taskset -c 10 python3 run_test.py \
#     --dataset_name=enron \
#     --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
#     --light=True \
#     --query_size=10 \
#     --node_induced=1 \
#     --undirected=True

# taskset -c 11 python3 run_test.py \
#     --dataset_name=enron \
#     --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
#     --light=True \
#     --query_size=12 \
#     --node_induced=1 \
#     --undirected=True

# taskset -c 12 python3 run_test.py \
#     --dataset_name=enron \
#     --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
#     --light=True \
#     --query_size=14 \
#     --node_induced=1 \
#     --undirected=True

taskset -c 13 python3 run_test.py \
    --dataset_name=enron \
    --database_foder=/dataset/EGSM_datasets_and_querysets/enron/label_16 \
    --light=True \
    --query_size=16 \
    --node_induced=1 \
    --undirected=True
