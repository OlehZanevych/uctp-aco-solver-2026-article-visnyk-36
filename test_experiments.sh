#!/bin/bash
echo "Testing experiments on datasets 1-2..."
mkdir -p experiment_results

for dataset in 1 2; do
    echo "Dataset $dataset:"
    for trial in 1 2; do
        echo -n "  Trial $trial... "
        ./solve_uctp $dataset 20 30 > experiment_results/dataset_${dataset}_trial_${trial}.log 2>&1
        cost=$(grep "^Cost:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $2}')
        hc=$(grep "^Hard Violations:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $3}')
        runtime=$(grep "^Runtime:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $2}')
        echo "Cost=$cost, HC=$hc, Time=${runtime}s"
    done
    echo
done
