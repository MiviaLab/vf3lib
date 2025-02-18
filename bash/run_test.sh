#!/bin/sh
export CUDA_VISIBLE_DEVICES=0
cd ../
result1=result.log/
file1=result
rm -rf result.log/*.*

QUERY_TEST=/dataset/DBLP
TARGET_TEST=/dataset/DBLP

QUERY_TARGET_NAMES=("query_8.sub.grf data.grf" "query_16.sub.grf data.grf" "query_32.sub.grf data.grf" "query_64.sub.grf data.grf")

# QUERY_TARGET_NAMES=("query_16_uniform_label.sub.grf data_uniform_label.grf" "query_32_uniform_label.sub.grf data_uniform_label.grf" "query_64_uniform_label.sub.grf data_uniform_label.grf")

for i in "${QUERY_TARGET_NAMES[@]}"; do
    # Split the element into two parts
    QUERY_NAME=$(echo $i | awk '{print $1}')
    TARGET_NAME=$(echo $i | awk '{print $2}')

    # Print the two strings
    echo "Query: $QUERY_NAME - Target string: $TARGET_NAME"

    QUERY_TEST=/dataset/DBLP/${QUERY_NAME}
    TARGET_TEST=/dataset/DBLP/${TARGET_NAME}

    if [ $1 -eq 1 ]; then
        cuda-gdbserver localhost:1236 bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -s -v
    else
        ./bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -s -v
    fi
    echo $'\n######################################\n'
done

# if [ $1 -eq 1 ]; then
#     cuda-gdbserver localhost:1236 bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -s -v
# else
#     ./bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -s -v
# fi
cd bash
