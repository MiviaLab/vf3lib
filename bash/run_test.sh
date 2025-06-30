#!/bin/bash
export CUDA_VISIBLE_DEVICES=0
cd ../
result1=result.log/
file1=result
rm -rf result.log/*.*

QUERY_TARGET_NAMES=("/graph-matching-analysis/baseline_algorithms/tdfs/data/pattern/12_vf.sub.grf /graph-matching-analysis/baseline_algorithms/tdfs/convertion_scripts/com-youtube.ungraph_vf.grf")

for i in "${QUERY_TARGET_NAMES[@]}"; do
    # Split the element into two parts
    QUERY_NAME=$(echo $i | awk '{print $1}')
    TARGET_NAME=$(echo $i | awk '{print $2}')

    # Print the two strings
    echo "Query: $QUERY_NAME - Target string: $TARGET_NAME"

    QUERY_TEST=/${QUERY_NAME}
    TARGET_TEST=/${TARGET_NAME}

    if [ $1 -eq 1 ]; then
        gdbserver localhost:1236 bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -i -u
    else
        ./bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -u
    fi

    echo $'\n#####################################\n'
done

# if [ $1 -eq 1 ]; then
#     cuda-gdbserver localhost:1236 bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -s -v -u
# else
#     ./bin/vf3 ${QUERY_TEST} ${TARGET_TEST} -s -v
# fi
cd bash
/user/frosa/graph_matching
