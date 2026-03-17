#!/usr/bin/env python3
"""
Generate graphs from ACO iteration data for article.
Usage: python3 plot_results.py <dataset_id>
"""

import sys
import csv
import matplotlib.pyplot as plt

def read_csv(filename):
    """Read CSV file and return data as dict of lists"""
    data = {}
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)
        for key in reader.fieldnames:
            data[key] = []
        for row in reader:
            for key in row:
                data[key].append(float(row[key]))
    return data

def plot_convergence(data, dataset_id, output_prefix):
    """Plot convergence curve showing best cost over iterations"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    ax.plot(data['Iteration'], data['BestCost'], 'b-', linewidth=2, label='Best Cost')
    ax.plot(data['Iteration'], data['AvgCost'], 'g--', linewidth=1.5, alpha=0.7, label='Average Cost')
    ax.fill_between(data['Iteration'], data['BestCost'], data['WorstCost'], alpha=0.2, color='gray', label='Range')
    
    ax.set_xlabel('Iteration', fontsize=12, fontweight='bold')
    ax.set_ylabel('Cost', fontsize=12, fontweight='bold')
    ax.set_title(f'ACO Convergence - Dataset {dataset_id}', fontsize=14, fontweight='bold')
    ax.legend(loc='upper right')
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_convergence.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_convergence.png")
    plt.close()

def plot_constraint_violations(data, dataset_id, output_prefix):
    """Plot hard constraint violations over iterations"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    ax.plot(data['Iteration'], data['BestHardViolations'], 'r-', linewidth=2, marker='o', markersize=3)
    ax.axhline(y=0, color='green', linestyle='--', linewidth=2, label='Feasible (0 violations)')
    
    ax.set_xlabel('Iteration', fontsize=12, fontweight='bold')
    ax.set_ylabel('Hard Constraint Violations', fontsize=12, fontweight='bold')
    ax.set_title(f'Hard Constraint Satisfaction - Dataset {dataset_id}', fontsize=14, fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_violations.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_violations.png")
    plt.close()

def plot_soft_constraints(data, dataset_id, output_prefix):
    """Plot soft constraint penalties over iterations"""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Gap penalty
    ax1.plot(data['Iteration'], data['BestGapPenalty'], 'b-', linewidth=2)
    ax1.set_xlabel('Iteration', fontsize=11, fontweight='bold')
    ax1.set_ylabel('Gap Penalty', fontsize=11, fontweight='bold')
    ax1.set_title('Schedule Gap Minimization', fontsize=12, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    
    # Undesirable penalty
    ax2.plot(data['Iteration'], data['BestUndesirablePenalty'], 'orange', linewidth=2)
    ax2.set_xlabel('Iteration', fontsize=11, fontweight='bold')
    ax2.set_ylabel('Undesirable Penalty', fontsize=11, fontweight='bold')
    ax2.set_title('Undesirable Timeslot Avoidance', fontsize=12, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    
    fig.suptitle(f'Soft Constraint Optimization - Dataset {dataset_id}', fontsize=14, fontweight='bold')
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_soft_constraints.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_soft_constraints.png")
    plt.close()

def plot_all_in_one(data, dataset_id, output_prefix):
    """Create comprehensive 4-panel figure for article"""
    fig = plt.figure(figsize=(14, 10))
    gs = fig.add_gridspec(2, 2, hspace=0.3, wspace=0.3)
    
    # Panel 1: Convergence
    ax1 = fig.add_subplot(gs[0, 0])
    ax1.plot(data['Iteration'], data['BestCost'], 'b-', linewidth=2, label='Best')
    ax1.plot(data['Iteration'], data['AvgCost'], 'g--', linewidth=1.5, alpha=0.7, label='Average')
    ax1.set_xlabel('Iteration', fontweight='bold')
    ax1.set_ylabel('Cost', fontweight='bold')
    ax1.set_title('(a) Convergence Curve', fontweight='bold')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Panel 2: Hard Violations
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.plot(data['Iteration'], data['BestHardViolations'], 'r-', linewidth=2)
    ax2.axhline(y=0, color='green', linestyle='--', linewidth=2, alpha=0.5)
    ax2.set_xlabel('Iteration', fontweight='bold')
    ax2.set_ylabel('Hard Violations', fontweight='bold')
    ax2.set_title('(b) Constraint Satisfaction', fontweight='bold')
    ax2.grid(True, alpha=0.3)
    
    # Panel 3: Soft Constraints
    ax3 = fig.add_subplot(gs[1, 0])
    ax3_twin = ax3.twinx()
    l1 = ax3.plot(data['Iteration'], data['BestGapPenalty'], 'b-', linewidth=2, label='Gap Penalty')
    l2 = ax3_twin.plot(data['Iteration'], data['BestUndesirablePenalty'], 'orange', linewidth=2, label='Undesirable Penalty')
    ax3.set_xlabel('Iteration', fontweight='bold')
    ax3.set_ylabel('Gap Penalty', fontweight='bold', color='b')
    ax3_twin.set_ylabel('Undesirable Penalty', fontweight='bold', color='orange')
    ax3.set_title('(c) Soft Constraint Penalties', fontweight='bold')
    ax3.tick_params(axis='y', labelcolor='b')
    ax3_twin.tick_params(axis='y', labelcolor='orange')
    ax3.grid(True, alpha=0.3)
    
    # Panel 4: Statistics
    ax4 = fig.add_subplot(gs[1, 1])
    ax4.axis('off')
    
    final_cost = data['BestCost'][-1]
    initial_cost = data['BestCost'][0]
    final_hc = data['BestHardViolations'][-1]
    final_gap = data['BestGapPenalty'][-1]
    final_undes = data['BestUndesirablePenalty'][-1]
    improvement = ((initial_cost - final_cost) / initial_cost * 100)
    
    stats_text = f"""
    Final Statistics:
    
    Best Cost: {final_cost:.0f}
    Hard Violations: {final_hc:.0f}
    Gap Penalty: {final_gap:.0f}
    Undesirable Penalty: {final_undes:.0f}
    
    Improvement: {improvement:.1f}%
    Iterations: {len(data['Iteration'])}
    
    Initial Cost: {initial_cost:.0f}
    Final Cost: {final_cost:.0f}
    """
    
    ax4.text(0.1, 0.9, stats_text, transform=ax4.transAxes, fontsize=11,
             verticalalignment='top', fontfamily='monospace',
             bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.3))
    ax4.set_title('(d) Summary Statistics', fontweight='bold')
    
    fig.suptitle(f'ACO Performance Analysis - Dataset {dataset_id}', 
                 fontsize=16, fontweight='bold')
    
    plt.savefig(f'{output_prefix}_comprehensive.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_comprehensive.png")
    plt.close()

def generate_summary_stats(data, dataset_id):
    """Generate summary statistics"""
    final_cost = data['BestCost'][-1]
    initial_cost = data['BestCost'][0]
    
    improvements = sum(1 for i in range(1, len(data['BestCost'])) 
                      if data['BestCost'][i] < data['BestCost'][i-1])
    improvement_pct = ((initial_cost - final_cost) / initial_cost * 100)
    
    print(f"\n=== Summary Statistics for Dataset {dataset_id} ===")
    print(f"Initial Cost: {initial_cost:.0f}")
    print(f"Final Cost: {final_cost:.0f}")
    print(f"Improvement: {improvement_pct:.1f}%")
    print(f"Total Improvements: {improvements}")
    print(f"Hard Violations: {data['BestHardViolations'][-1]:.0f}")
    print(f"Gap Penalty: {data['BestGapPenalty'][-1]:.0f}")
    print(f"Undesirable Penalty: {data['BestUndesirablePenalty'][-1]:.0f}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 plot_results.py <dataset_id>")
        print("Example: python3 plot_results.py 1")
        sys.exit(1)
    
    dataset_id = sys.argv[1]
    input_file = f"dataset_{dataset_id}_iterations.csv"
    output_prefix = f"dataset_{dataset_id}"
    
    try:
        data = read_csv(input_file)
    except FileNotFoundError:
        print(f"Error: {input_file} not found. Run solver first:")
        print(f"  ./solve_uctp {dataset_id} 20 50")
        sys.exit(1)
    
    print(f"Generating graphs for dataset {dataset_id}...")
    
    plot_convergence(data, dataset_id, output_prefix)
    plot_constraint_violations(data, dataset_id, output_prefix)
    plot_soft_constraints(data, dataset_id, output_prefix)
    plot_all_in_one(data, dataset_id, output_prefix)
    
    generate_summary_stats(data, dataset_id)
    
    print(f"\nAll graphs generated successfully!")
    print(f"Files: {output_prefix}_*.png")

if __name__ == "__main__":
    main()

    """Plot convergence curve showing best cost over iterations"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    ax.plot(df['Iteration'], df['BestCost'], 'b-', linewidth=2, label='Best Cost')
    ax.plot(df['Iteration'], df['AvgCost'], 'g--', linewidth=1.5, alpha=0.7, label='Average Cost')
    ax.fill_between(df['Iteration'], df['BestCost'], df['WorstCost'], alpha=0.2, color='gray', label='Range')
    
    ax.set_xlabel('Iteration', fontsize=12, fontweight='bold')
    ax.set_ylabel('Cost', fontsize=12, fontweight='bold')
    ax.set_title(f'ACO Convergence - Dataset {dataset_id}', fontsize=14, fontweight='bold')
    ax.legend(loc='upper right')
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_convergence.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_convergence.png")
    plt.close()

def plot_constraint_violations(df, dataset_id, output_prefix):
    """Plot hard constraint violations over iterations"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    ax.plot(df['Iteration'], df['BestHardViolations'], 'r-', linewidth=2, marker='o', markersize=3)
    ax.axhline(y=0, color='green', linestyle='--', linewidth=2, label='Feasible (0 violations)')
    
    ax.set_xlabel('Iteration', fontsize=12, fontweight='bold')
    ax.set_ylabel('Hard Constraint Violations', fontsize=12, fontweight='bold')
    ax.set_title(f'Hard Constraint Satisfaction - Dataset {dataset_id}', fontsize=14, fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_violations.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_violations.png")
    plt.close()

def plot_soft_constraints(df, dataset_id, output_prefix):
    """Plot soft constraint penalties over iterations"""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Gap penalty
    ax1.plot(df['Iteration'], df['BestGapPenalty'], 'b-', linewidth=2)
    ax1.set_xlabel('Iteration', fontsize=11, fontweight='bold')
    ax1.set_ylabel('Gap Penalty', fontsize=11, fontweight='bold')
    ax1.set_title('Schedule Gap Minimization', fontsize=12, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    
    # Undesirable penalty
    ax2.plot(df['Iteration'], df['BestUndesirablePenalty'], 'orange', linewidth=2)
    ax2.set_xlabel('Iteration', fontsize=11, fontweight='bold')
    ax2.set_ylabel('Undesirable Penalty', fontsize=11, fontweight='bold')
    ax2.set_title('Undesirable Timeslot Avoidance', fontsize=12, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    
    fig.suptitle(f'Soft Constraint Optimization - Dataset {dataset_id}', fontsize=14, fontweight='bold')
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_soft_constraints.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_soft_constraints.png")
    plt.close()

def plot_improvement_rate(df, dataset_id, output_prefix):
    """Plot rate of improvement over iterations"""
    improvements = []
    for i in range(1, len(df)):
        if df.iloc[i]['BestCost'] < df.iloc[i-1]['BestCost']:
            improvements.append(i)
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    ax.plot(df['Iteration'], df['BestCost'], 'b-', linewidth=2, alpha=0.5)
    ax.scatter(improvements, df.iloc[improvements]['BestCost'], 
               color='red', s=50, zorder=5, label='Improvement')
    
    ax.set_xlabel('Iteration', fontsize=12, fontweight='bold')
    ax.set_ylabel('Best Cost', fontsize=12, fontweight='bold')
    ax.set_title(f'Solution Improvements - Dataset {dataset_id}', fontsize=14, fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # Add text with improvement count
    ax.text(0.02, 0.98, f'Total improvements: {len(improvements)}', 
            transform=ax.transAxes, fontsize=11, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_improvements.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_improvements.png")
    plt.close()

def plot_all_in_one(df, dataset_id, output_prefix):
    """Create comprehensive 4-panel figure for article"""
    fig = plt.figure(figsize=(14, 10))
    gs = fig.add_gridspec(2, 2, hspace=0.3, wspace=0.3)
    
    # Panel 1: Convergence
    ax1 = fig.add_subplot(gs[0, 0])
    ax1.plot(df['Iteration'], df['BestCost'], 'b-', linewidth=2, label='Best')
    ax1.plot(df['Iteration'], df['AvgCost'], 'g--', linewidth=1.5, alpha=0.7, label='Average')
    ax1.set_xlabel('Iteration', fontweight='bold')
    ax1.set_ylabel('Cost', fontweight='bold')
    ax1.set_title('(a) Convergence Curve', fontweight='bold')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Panel 2: Hard Violations
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.plot(df['Iteration'], df['BestHardViolations'], 'r-', linewidth=2)
    ax2.axhline(y=0, color='green', linestyle='--', linewidth=2, alpha=0.5)
    ax2.set_xlabel('Iteration', fontweight='bold')
    ax2.set_ylabel('Hard Violations', fontweight='bold')
    ax2.set_title('(b) Constraint Satisfaction', fontweight='bold')
    ax2.grid(True, alpha=0.3)
    
    # Panel 3: Soft Constraints
    ax3 = fig.add_subplot(gs[1, 0])
    ax3_twin = ax3.twinx()
    l1 = ax3.plot(df['Iteration'], df['BestGapPenalty'], 'b-', linewidth=2, label='Gap Penalty')
    l2 = ax3_twin.plot(df['Iteration'], df['BestUndesirablePenalty'], 'orange', linewidth=2, label='Undesirable Penalty')
    ax3.set_xlabel('Iteration', fontweight='bold')
    ax3.set_ylabel('Gap Penalty', fontweight='bold', color='b')
    ax3_twin.set_ylabel('Undesirable Penalty', fontweight='bold', color='orange')
    ax3.set_title('(c) Soft Constraint Penalties', fontweight='bold')
    ax3.tick_params(axis='y', labelcolor='b')
    ax3_twin.tick_params(axis='y', labelcolor='orange')
    ax3.grid(True, alpha=0.3)
    
    # Panel 4: Statistics
    ax4 = fig.add_subplot(gs[1, 1])
    ax4.axis('off')
    
    final = df.iloc[-1]
    initial = df.iloc[0]
    improvement = ((initial['BestCost'] - final['BestCost']) / initial['BestCost'] * 100)
    
    stats_text = f"""
    Final Statistics:
    
    Best Cost: {final['BestCost']:.0f}
    Hard Violations: {final['BestHardViolations']:.0f}
    Gap Penalty: {final['BestGapPenalty']:.0f}
    Undesirable Penalty: {final['BestUndesirablePenalty']:.0f}
    
    Improvement: {improvement:.1f}%
    Iterations: {len(df)}
    
    Initial Cost: {initial['BestCost']:.0f}
    Final Cost: {final['BestCost']:.0f}
    """
    
    ax4.text(0.1, 0.9, stats_text, transform=ax4.transAxes, fontsize=11,
             verticalalignment='top', fontfamily='monospace',
             bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.3))
    ax4.set_title('(d) Summary Statistics', fontweight='bold')
    
    fig.suptitle(f'ACO Performance Analysis - Dataset {dataset_id}', 
                 fontsize=16, fontweight='bold')
    
    plt.savefig(f'{output_prefix}_comprehensive.png', dpi=300, bbox_inches='tight')
    print(f"Saved: {output_prefix}_comprehensive.png")
    plt.close()

def generate_summary_stats(df, dataset_id):
    """Generate summary statistics"""
    final = df.iloc[-1]
    initial = df.iloc[0]
    
    improvements = sum(1 for i in range(1, len(df)) if df.iloc[i]['BestCost'] < df.iloc[i-1]['BestCost'])
    improvement_pct = ((initial['BestCost'] - final['BestCost']) / initial['BestCost'] * 100)
    
    print(f"\n=== Summary Statistics for Dataset {dataset_id} ===")
    print(f"Initial Cost: {initial['BestCost']:.0f}")
    print(f"Final Cost: {final['BestCost']:.0f}")
    print(f"Improvement: {improvement_pct:.1f}%")
    print(f"Total Improvements: {improvements}")
    print(f"Hard Violations: {final['BestHardViolations']:.0f}")
    print(f"Gap Penalty: {final['BestGapPenalty']:.0f}")
    print(f"Undesirable Penalty: {final['BestUndesirablePenalty']:.0f}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 plot_results.py <dataset_id>")
        print("Example: python3 plot_results.py 1")
        sys.exit(1)
    
    dataset_id = sys.argv[1]
    input_file = f"dataset_{dataset_id}_iterations.csv"
    output_prefix = f"dataset_{dataset_id}"
    
    try:
        df = pd.read_csv(input_file)
    except FileNotFoundError:
        print(f"Error: {input_file} not found. Run solver first:")
        print(f"  ./solve_uctp {dataset_id} 20 50")
        sys.exit(1)
    
    print(f"Generating graphs for dataset {dataset_id}...")
    
    plot_convergence(df, dataset_id, output_prefix)
    plot_constraint_violations(df, dataset_id, output_prefix)
    plot_soft_constraints(df, dataset_id, output_prefix)
    plot_improvement_rate(df, dataset_id, output_prefix)
    plot_all_in_one(df, dataset_id, output_prefix)
    
    generate_summary_stats(df, dataset_id)
    
    print(f"\nAll graphs generated successfully!")
    print(f"Files: {output_prefix}_*.png")

if __name__ == "__main__":
    main()
