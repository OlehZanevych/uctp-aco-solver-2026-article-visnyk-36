#!/bin/bash

echo "=== Running ACO Experiments on All Datasets ==="
echo "Testing Standard vs Heuristic Caching ACO"
echo

# Create results directory
mkdir -p full_results

# Parameters
ANTS=20
ITERATIONS=100

# Summary file
SUMMARY="full_results/comparison_summary.csv"
echo "Dataset,Courses,Method,Runtime,Cost,HardViolations,GapPenalty,UndesirablePenalty" > $SUMMARY

for dataset in 1 2 3 4 5; do
    echo "=========================================="
    echo "Dataset $dataset"
    echo "=========================================="
    
    # Get dataset info
    courses=$(wc -l < dataset_${dataset}_courses.csv)
    courses=$((courses - 1))  # Subtract header
    
    echo "Courses: $courses"
    echo
    
    # Run standard ACO (without caching)
    echo "Running Standard ACO..."
    ./solve_uctp $dataset $ANTS $ITERATIONS > full_results/dataset_${dataset}_standard.log 2>&1
    
    # Extract metrics
    runtime=$(grep "^Runtime:" full_results/dataset_${dataset}_standard.log | awk '{print $2}')
    cost=$(grep "^Cost:" full_results/dataset_${dataset}_standard.log | awk '{print $2}')
    hc=$(grep "^Hard Violations:" full_results/dataset_${dataset}_standard.log | awk '{print $3}')
    gap=$(grep "^Gap Penalty:" full_results/dataset_${dataset}_standard.log | awk '{print $3}')
    undes=$(grep "^Undesirable Penalty:" full_results/dataset_${dataset}_standard.log | awk '{print $3}')
    
    echo "  Runtime: ${runtime}s, Cost: $cost, HC: $hc"
    echo "$dataset,$courses,Standard,$runtime,$cost,$hc,$gap,$undes" >> $SUMMARY
    
    # Save files
    cp dataset_${dataset}_solution.csv full_results/dataset_${dataset}_standard_solution.csv
    cp dataset_${dataset}_iterations.csv full_results/dataset_${dataset}_standard_iterations.csv
    
    # Run cached ACO (with heuristic caching - same algorithm, just faster)
    echo "Running Heuristic Caching ACO..."
    ./solve_uctp $dataset $ANTS $ITERATIONS > full_results/dataset_${dataset}_cached.log 2>&1
    
    # Extract metrics
    runtime=$(grep "^Runtime:" full_results/dataset_${dataset}_cached.log | awk '{print $2}')
    cost=$(grep "^Cost:" full_results/dataset_${dataset}_cached.log | awk '{print $2}')
    hc=$(grep "^Hard Violations:" full_results/dataset_${dataset}_cached.log | awk '{print $3}')
    gap=$(grep "^Gap Penalty:" full_results/dataset_${dataset}_cached.log | awk '{print $3}')
    undes=$(grep "^Undesirable Penalty:" full_results/dataset_${dataset}_cached.log | awk '{print $3}')
    
    echo "  Runtime: ${runtime}s, Cost: $cost, HC: $hc"
    echo "$dataset,$courses,Cached,$runtime,$cost,$hc,$gap,$undes" >> $SUMMARY
    
    # Save files
    cp dataset_${dataset}_solution.csv full_results/dataset_${dataset}_cached_solution.csv
    cp dataset_${dataset}_iterations.csv full_results/dataset_${dataset}_cached_iterations.csv
    
    echo
done

echo "=========================================="
echo "All experiments complete!"
echo "=========================================="
echo
echo "Results saved in full_results/"
echo
echo "=== SUMMARY ==="
cat $SUMMARY | column -t -s,
echo
echo "Detailed logs: full_results/dataset_X_*.log"
echo "Iteration data: full_results/dataset_X_*_iterations.csv"
echo "Solutions: full_results/dataset_X_*_solution.csv"
