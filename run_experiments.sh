#!/bin/bash

echo "=== UCTP ACO Comprehensive Experiments ==="
echo "Running 3 trials per dataset..."
echo

# Create results directory
mkdir -p experiment_results

# Run experiments
for dataset in 1 2 3 4 5; do
    echo "Dataset $dataset:"
    
    for trial in 1 2 3; do
        echo -n "  Trial $trial... "
        ./solve_uctp $dataset 20 50 > experiment_results/dataset_${dataset}_trial_${trial}.log 2>&1
        
        # Extract key metrics
        cost=$(grep "^Cost:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $2}')
        hc=$(grep "^Hard Violations:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $3}')
        runtime=$(grep "^Runtime:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $2}')
        
        echo "Cost=$cost, HC=$hc, Time=${runtime}s"
        
        # Save solution with trial number
        cp dataset_${dataset}_solution.csv experiment_results/dataset_${dataset}_trial_${trial}_solution.csv
        cp dataset_${dataset}_solution.csv_stats.txt experiment_results/dataset_${dataset}_trial_${trial}_stats.txt
    done
    echo
done

# Generate summary
echo "=== SUMMARY ===" > experiment_results/summary.txt
echo >> experiment_results/summary.txt

for dataset in 1 2 3 4 5; do
    echo "Dataset $dataset:" >> experiment_results/summary.txt
    
    costs=""
    hcs=""
    times=""
    
    for trial in 1 2 3; do
        cost=$(grep "^Cost:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $2}')
        hc=$(grep "^Hard Violations:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $3}')
        time=$(grep "^Runtime:" experiment_results/dataset_${dataset}_trial_${trial}.log | awk '{print $2}')
        
        costs="$costs $cost"
        hcs="$hcs $hc"
        times="$times $time"
    done
    
    echo "  Costs: $costs" >> experiment_results/summary.txt
    echo "  Hard Violations: $hcs" >> experiment_results/summary.txt
    echo "  Runtimes: $times" >> experiment_results/summary.txt
    echo >> experiment_results/summary.txt
done

echo "Experiments complete! Results saved in experiment_results/"
echo
cat experiment_results/summary.txt
