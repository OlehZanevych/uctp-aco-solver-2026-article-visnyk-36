# UCTP ACO Solver - CSV-Based Version

This version separates data generation from solving, allowing multiple runs on the same datasets.

## Quick Start

```bash
# Build everything
make

# Generate 5 datasets
./generate_data

# Solve a specific dataset
./solve_uctp 1 20 50    # dataset_id, num_ants, max_iterations

# Solve all datasets
make solve_all
```

## Programs

### 1. generate_data
Generates 5 datasets with exponentially increasing sizes:

| Dataset | Courses | Lecturers | Groups | Rooms |
|---------|---------|-----------|--------|-------|
| 1       | 25      | 5         | 8      | 5     |
| 2       | 50      | 10        | 16     | 10    |
| 3       | 100     | 20        | 32     | 20    |
| 4       | 200     | 40        | 64     | 40    |
| 5       | 400     | 80        | 128    | 80    |

Each dataset consists of 5 CSV files:
- `dataset_X_courses.csv` - Course information (id, lecturer, duration, groups)
- `dataset_X_rooms.csv` - Room capacities
- `dataset_X_groups.csv` - Student group sizes
- `dataset_X_lecturer_undesirable.csv` - Undesirable timeslots for lecturers
- `dataset_X_group_undesirable.csv` - Undesirable timeslots for student groups

### 2. solve_uctp
Solves a timetabling problem from CSV files using ACO with heuristic caching.

**Usage:**
```bash
./solve_uctp <dataset_id> [num_ants] [max_iterations]
```

**Examples:**
```bash
./solve_uctp 1              # Use defaults (20 ants, 100 iterations)
./solve_uctp 2 30 150       # Custom parameters
./solve_uctp 3 20 50        # Faster run
```

**Output:**
- `dataset_X_solution.csv` - Timetable assignments (course, room, day, period, timeslot)
- `dataset_X_solution.csv_stats.txt` - Solution statistics (cost, violations, runtime)

### 3. uctp_aco (original)
The original monolithic version with comparison mode - still available for benchmarking.

## CSV File Formats

### courses.csv
```csv
id,lecturer,duration,groups
0,2,1,"0;3"
1,1,1,"2"
```

### rooms.csv
```csv
id,capacity
0,45
1,60
```

### groups.csv
```csv
id,size
0,30
1,25
```

### lecturer_undesirable.csv
```csv
lecturer,timeslots
0,"5;12;38"
1,""
```

### group_undesirable.csv
```csv
group,timeslots
0,"0;44"
1,"15;30"
```

### solution.csv (output)
```csv
Course,Room,Day,Period,Timeslot
0,2,1,3,12
1,0,0,5,5
```

## Running Multiple Times

Since data is separated from solving, you can:

```bash
# Generate data once
./generate_data

# Run multiple times on same data
./solve_uctp 1 20 100
./solve_uctp 1 20 100  # Different random seed, different solution
./solve_uctp 1 30 150  # Different parameters

# Compare results
cat dataset_1_solution.csv_stats.txt
```

## Makefile Targets

```bash
make                # Build all programs
make generate       # Generate datasets
make solve_all      # Solve all 5 datasets
make clean          # Remove binaries and generated files
```

## Performance Expectations

| Dataset | Courses | Typical Runtime (20 ants, 50 iters) |
|---------|---------|--------------------------------------|
| 1       | 25      | ~0.15s                               |
| 2       | 50      | ~0.5s                                |
| 3       | 100     | ~2s                                  |
| 4       | 200     | ~8s                                  |
| 5       | 400     | ~30s                                 |

## Algorithm

Uses **ACO with Heuristic Caching** for 1.5x speedup:
- Pre-computes all heuristic values once
- Elite strategy (top-3 solutions + global best)
- Adaptive pheromone deposition
- Constraint-aware heuristics

See `SPEEDUP_MODIFICATION.md` and `IMPROVEMENTS_ANALYSIS.md` for details.

## For Your Article

This structure is ideal for experiments:
1. Generate datasets once
2. Run multiple trials with different parameters
3. Compare results across datasets
4. Analyze scaling behavior

The exponential growth (2x each step) clearly shows how the algorithm scales with problem size.
