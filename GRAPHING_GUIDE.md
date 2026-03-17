# Iteration Data and Graphing Guide

## Overview

The solver now saves detailed iteration data to CSV files that can be used to generate graphs for your article.

## Output Files

After running the solver, you get:

1. **dataset_X_solution.csv** - Final timetable assignments
2. **dataset_X_solution.csv_stats.txt** - Summary statistics
3. **dataset_X_iterations.csv** - **NEW: Iteration-by-iteration data**

## Iteration CSV Format

```csv
Iteration,BestCost,BestHardViolations,BestGapPenalty,BestUndesirablePenalty,AvgCost,WorstCost
0,30230,3,23,0,59277.8,100240
1,30230,3,23,0,73807.5,140165
2,20235,2,23,1,71793.5,140255
3,10380,1,38,0,55798.5,130300
...
```

### Columns

- **Iteration**: Iteration number (0 to max_iterations-1)
- **BestCost**: Best solution cost found so far (global best)
- **BestHardViolations**: Hard constraint violations in best solution
- **BestGapPenalty**: Gap penalty in best solution
- **BestUndesirablePenalty**: Undesirable timeslot penalty in best solution
- **AvgCost**: Average cost of all solutions in this iteration
- **WorstCost**: Worst cost among all solutions in this iteration

## Generating Graphs

### Option 1: Using the Provided Script (Requires matplotlib)

```bash
# Install dependencies (if needed)
pip3 install matplotlib

# Generate all graphs for a dataset
python3 plot_results.py 1
```

This creates 4 PNG files:
1. **dataset_1_convergence.png** - Convergence curve (best, average, worst)
2. **dataset_1_violations.png** - Hard constraint violations over time
3. **dataset_1_soft_constraints.png** - Gap and undesirable penalties
4. **dataset_1_comprehensive.png** - 4-panel comprehensive figure

### Option 2: Using Excel/Google Sheets

1. Open `dataset_X_iterations.csv` in Excel/Google Sheets
2. Create charts:
   - **Line chart**: Iteration vs BestCost (convergence)
   - **Line chart**: Iteration vs BestHardViolations (feasibility)
   - **Line chart**: Iteration vs BestGapPenalty, BestUndesirablePenalty (quality)
   - **Area chart**: Iteration vs BestCost/AvgCost/WorstCost (range)

### Option 3: Using R

```r
library(ggplot2)

# Read data
data <- read.csv("dataset_1_iterations.csv")

# Convergence plot
ggplot(data, aes(x=Iteration)) +
  geom_line(aes(y=BestCost, color="Best"), size=1.2) +
  geom_line(aes(y=AvgCost, color="Average"), size=1) +
  geom_ribbon(aes(ymin=BestCost, ymax=WorstCost), alpha=0.2) +
  labs(title="ACO Convergence", x="Iteration", y="Cost") +
  theme_minimal()

# Hard violations plot
ggplot(data, aes(x=Iteration, y=BestHardViolations)) +
  geom_line(color="red", size=1.2) +
  geom_hline(yintercept=0, linetype="dashed", color="green") +
  labs(title="Hard Constraint Satisfaction", x="Iteration", y="Violations") +
  theme_minimal()
```

### Option 4: Using Python (Custom)

```python
import csv
import matplotlib.pyplot as plt

# Read data
iterations = []
best_costs = []
with open('dataset_1_iterations.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        iterations.append(int(row['Iteration']))
        best_costs.append(float(row['BestCost']))

# Plot
plt.figure(figsize=(10, 6))
plt.plot(iterations, best_costs, 'b-', linewidth=2)
plt.xlabel('Iteration')
plt.ylabel('Best Cost')
plt.title('ACO Convergence')
plt.grid(True, alpha=0.3)
plt.savefig('convergence.png', dpi=300)
```

## Recommended Graphs for Article

### 1. Convergence Curve
**Purpose**: Show how solution quality improves over iterations

**Data**: Iteration vs BestCost, AvgCost, WorstCost

**Interpretation**:
- Steep initial drop = fast convergence
- Plateau = algorithm has converged
- Gap between best and average = solution diversity

### 2. Hard Constraint Satisfaction
**Purpose**: Show progress toward feasibility

**Data**: Iteration vs BestHardViolations

**Interpretation**:
- Reaching 0 = feasible solution found
- Staying above 0 = problem is highly constrained
- Oscillation = difficulty finding feasible solutions

### 3. Soft Constraint Optimization
**Purpose**: Show quality improvement after feasibility

**Data**: Iteration vs BestGapPenalty, BestUndesirablePenalty

**Interpretation**:
- Decreasing trends = improving schedule quality
- Trade-offs between different objectives

### 4. Comprehensive 4-Panel Figure
**Purpose**: Complete performance overview

**Panels**:
- (a) Convergence curve
- (b) Hard violations
- (c) Soft constraints
- (d) Summary statistics

## Example Analysis

From dataset 1 (25 courses, 50 iterations):

```
Initial Cost: 30,230
Final Cost: 10,225
Improvement: 66.2%
Hard Violations: 1 (near-feasible)
Iterations to best: 11
```

**Observations**:
- Rapid convergence in first 10 iterations (66% improvement)
- Achieved near-feasible solution (1 violation)
- Minimal improvement after iteration 11 (local optimum)

## Comparing Multiple Runs

To analyze algorithm stability:

```bash
# Run 5 times on same dataset
for i in 1 2 3 4 5; do
    ./solve_uctp 1 20 50
    cp dataset_1_iterations.csv trial_${i}_iterations.csv
done

# Compare convergence curves
python3 plot_multiple_trials.py
```

## Scaling Analysis

Compare datasets to show algorithm scaling:

```bash
# Solve all datasets
for d in 1 2 3 4 5; do
    ./solve_uctp $d 20 50
done

# Create comparison chart
python3 plot_scaling.py
```

Expected results:
- Dataset 1 (25 courses): ~0.15s, often feasible
- Dataset 2 (50 courses): ~0.5s, sometimes feasible
- Dataset 3 (100 courses): ~2s, rarely feasible
- Dataset 4 (200 courses): ~8s, infeasible
- Dataset 5 (400 courses): ~30s, highly infeasible

## Data for Tables

Extract key metrics for article tables:

```bash
# Summary table
echo "Dataset,Courses,FinalCost,HardViolations,Runtime"
for d in 1 2 3 4 5; do
    cost=$(tail -1 dataset_${d}_iterations.csv | cut -d',' -f2)
    hc=$(tail -1 dataset_${d}_iterations.csv | cut -d',' -f3)
    runtime=$(grep "Runtime:" dataset_${d}_solution.csv_stats.txt | awk '{print $2}')
    courses=$(grep "Loaded:" solve_log_${d}.txt | awk '{print $2}')
    echo "$d,$courses,$cost,$hc,$runtime"
done
```

## Tips for Article Figures

1. **Use high DPI**: 300 DPI for publication quality
2. **Label axes clearly**: Include units
3. **Add legends**: Explain all lines/colors
4. **Use consistent colors**: Same meaning = same color across figures
5. **Include error bars**: If showing multiple trials
6. **Annotate key points**: Mark when feasibility is achieved
7. **Use log scale**: If cost range is very large

## Troubleshooting

**Q: Graphs look flat?**
A: Increase iterations or use smaller dataset

**Q: No improvement after iteration X?**
A: Algorithm converged, consider parameter tuning

**Q: High variance between runs?**
A: Normal for metaheuristics, report average ± std dev

**Q: Can't reach feasibility?**
A: Problem may be over-constrained, try more ants/iterations
