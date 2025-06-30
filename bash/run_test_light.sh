#!/bin/bash
export CUDA_VISIBLE_DEVICES=0
cd ../
result1=result.log/
file1=result
rm -rf result.log/*.*

QUERY_TEST=/dataset/DBLP
TARGET_TEST=/dataset/DBLP

# QUERY_TARGET_NAMES=("node_induced_connected_query_32.sub.grf data.grf" "node_induced_connected_query_16.sub.grf data.grf" "node_induced_connected_query_8.sub.grf data.grf" "node_induced_connected_query_32_64.sub.grf data_64.grf" "node_induced_connected_query_16_64.sub.grf data_64.grf" "node_induced_connected_query_8_64.sub.grf data_64.grf" "node_induced_connected_query_32_32.sub.grf data_32.grf" "node_induced_connected_query_16_32.sub.grf data_32.grf" "node_induced_connected_query_8_32.sub.grf data_32.grf" "node_induced_connected_query_32_16.sub.grf data_16.grf" "node_induced_connected_query_16_16.sub.grf data_16.grf" "node_induced_connected_query_8_16.sub.grf data_16.grf" "node_induced_connected_query_32_8.sub.grf data_8.grf" "node_induced_connected_query_16_8.sub.grf data_8.grf" "node_induced_connected_query_8_8.sub.grf data_8.grf" "node_induced_connected_query_32_4.sub.grf data_4.grf" "node_induced_connected_query_16_4.sub.grf data_4.grf" "node_induced_connected_query_8_4.sub.grf data_4.grf" "node_induced_connected_query_32_2.sub.grf data_2.grf" "node_induced_connected_query_16_2.sub.grf data_2.grf" "node_induced_connected_query_8_2.sub.grf data_2.grf")
QUERY_TARGET_NAMES=("connected_query_0_64.sub.grf data_64.grf")

for i in "${QUERY_TARGET_NAMES[@]}"; do
    # Split the element into two parts
    QUERY_NAME=$(echo $i | awk '{print $1}')
    TARGET_NAME=$(echo $i | awk '{print $2}')

    # Print the two strings
    echo "Query: $QUERY_NAME - Target string: $TARGET_NAME"

    QUERY_TEST=/dataset/DBLP/64/edge_induced/label_64/${QUERY_NAME}
    TARGET_TEST=/dataset/DBLP/${TARGET_NAME}

    ./bin/vf3l ${QUERY_TEST} ${TARGET_TEST} -i -u

    echo $'\n######################################\n'
done

# if [ $1 -eq 1 ]; then
#     cuda-gdbserver localhost:1236 bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -s -v
# else
#     ./bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -s -v
# fi
cd bash
