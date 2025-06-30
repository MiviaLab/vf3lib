#!/bin/bash
export CUDA_VISIBLE_DEVICES=0
cd ../
result1=result.log/
file1=result
rm -rf result.log/*.*

QUERY_FOLDER=[PATH_TO_FOLDER_THAT_CONTAINS_QUERY_GRAPHS]
TARGET_FOLDER=[PATH_TO_FOLDER_THAT_CONTAINS_TARGET_GRAPHS]

# Define the array of query and target graph names
QUERY_TARGET_NAMES=("[NAME_OF_QUERY_GRAPH] [NAME_OF_TARGET_GRAPH]")

# Example of QUERY_TARGET_NAMES:
# QUERY_TARGET_NAMES=("bvg1.sub.grf bvg1.grf" "bvg1_2.sub.grf bvg1.grf" "bvg1_3.sub.grf bvg1.grf" "bvg1_4.sub.grf bvg1.grf" "bvg2.sub.grf bvg2.grf" "bvg3.sub.grf bvg3.grf" "m2d1.sub.grf m2d1.grf" "m2d2.sub.grf m2d2.grf" "rand1.sub.grf rand1.grf" "rand2.sub.grf rand2.grf" "rand3.sub.grf rand3.grf" "rand4.sub.grf rand4.grf" "rnd_ldg1.sub.grf rnd_ldg1.grf" "si2_b03_m400_37.sub.grf si2_b03_m400_37.grf" "si2_b03_m1000_00.sub.grf si2_b03_m1000_00.grf" "si2_b03m_m800_22.sub.grf si2_b03m_m800_22.grf" "si2_b06m_m400_96.sub.grf si2_b06m_m400_96.grf" "si2_b09_m400_37.sub.grf si2_b09_m400_01.grf" "si2_r01_m200_00.sub.grf si2_r01_m200_00.grf" "si2_r01_m1000_30.sub.grf si2_r01_m1000_30.grf" "si2_rnd_eta04_m750_20.sub.grf si2_rnd_eta04_m750_20.grf" "si2_rnd_eta04_m1000_00.sub.grf si2_rnd_eta04_m1000_00.grf")

for i in "${QUERY_TARGET_NAMES[@]}"; do
    # Split the element into two parts
    QUERY_NAME=$(echo $i | awk '{print $1}')
    TARGET_NAME=$(echo $i | awk '{print $2}')

    # Print the two strings
    echo "Query: $QUERY_NAME - Target string: $TARGET_NAME"

    QUERY_TEST=${QUERY_FOLDER}/${QUERY_NAME}
    TARGET_TEST=${TARGET_FOLDER}/${TARGET_NAME}

    if [ $1 -eq 1 ]; then
        gdbserver localhost:1236 bin/vf3 ${QUERY_TEST} ${TARGET_TEST}
    else
        ./bin/vf3 ${QUERY_TEST} ${TARGET_TEST}
    fi

    echo $'\n#####################################\n'
done

cd bash
