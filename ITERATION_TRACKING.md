# Iteration Tracking - Summary

## What Was Added

The solver now tracks and saves detailed iteration data for analysis and graphing.

## New Output File

**dataset_X_iterations.csv** - Contains data for every iteration:

```csv
Iteration,BestCost,BestHardViolations,BestGapPenalty,BestUndesirablePenalty,AvgCost,WorstCost
0,260795,26,79,1,316682,370520
1,260795,26,79,1,325658,380755
2,240835,24,82,3,319139,430720
...
```

## Data Captured

For each iteration:
- **Best solution so far** (global best): Cost and constraint breakdown
- **Average cost** of all solutions in that iteration
- **Worst cost** in that iteration

This allows you to analyze:
- Convergence speed
- Solution diversity
- Algorithm stability
- Constraint satisfaction progress

## Usage

```bash
# Run solver (automatically saves iteration data)
./solve_uctp 1 20 50

# Files created:
# - dataset_1_solution.csv (final timetable)
# - dataset_1_solution.csv_stats.txt (summary)
# - dataset_1_iterations.csv (iteration data) ← NEW

# Generate graphs (requires matplotlib)
python3 plot_results.py 1
```

## Graphs Generated

The `plot_results.py` script creates 4 publication-quality graphs:

1. **convergence.png** - Best/Average/Worst cost over iterations
2. **violations.png** - Hard constraint violations over time
3. **soft_constraints.png** - Gap and undesirable penalties
4. **comprehensive.png** - 4-panel figure with all metrics + statistics

## Alternative Tools

If matplotlib isn't available:
- **Excel/Google Sheets**: Open CSV and create charts
- **R**: Use ggplot2 (examples in GRAPHING_GUIDE.md)
- **Python**: Custom plotting code (examples provided)
- **MATLAB**: readtable() and plot()

## For Your Article

This data enables you to:

### Show Convergence Behavior
- How quickly does ACO find good solutions?
- When does it plateau?
- How much improvement per iteration?

### Demonstrate Constraint Handling
- Track progress toward feasibility
- Show hard vs soft constraint trade-offs
- Identify when feasible solutions are found

### Analyze Algorithm Stability
- Run multiple trials, compare convergence curves
- Calculate mean ± standard deviation
- Show best/average/worst ranges

### Compare Problem Sizes
- Plot convergence for datasets 1-5
- Show how difficulty scales
- Demonstrate algorithm robustness

## Example Analysis

From dataset 2 (50 courses, 30 iterations):

```
Iteration 0:  Cost = 260,795 (26 HC, 79 gaps)
Iteration 2:  Cost = 240,835 (24 HC, 82 gaps) - 7.7% improvement
Iteration 18: Cost = 220,765 (22 HC, 76 gaps) - 15.3% total improvement

Average cost: ~320,000 (shows solution diversity)
Worst cost: ~380,000 (shows search space exploration)
```

**Key Insights**:
- Rapid initial improvement (first 2 iterations)
- Gradual refinement (iterations 2-18)
- Maintained diversity (avg cost >> best cost)
- Did not reach feasibility (22 violations remain)

## Files

- **solve_uctp.cpp** - Modified to track iteration data
- **plot_results.py** - Graph generation script
- **GRAPHING_GUIDE.md** - Comprehensive graphing guide
- **ITERATION_TRACKING.md** - This file

## Next Steps

1. Run experiments on all datasets
2. Generate graphs for each
3. Compare convergence behavior
4. Include best graphs in article
5. Use data for statistical analysis

The iteration data provides everything needed for thorough algorithm analysis in your article!
