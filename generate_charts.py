#!/usr/bin/env python3
"""
Generate comparison charts for ACO speedup analysis.
Requires: matplotlib, pandas
Install: pip install matplotlib pandas
"""

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Experimental data
data = {
    'Problem Size': [50, 100, 150],
    'Standard (s)': [1.67, 3.28, 4.99],
    'Cached (s)': [1.11, 2.14, 3.12],
    'Speedup': [1.50, 1.53, 1.60],
    'Quality Ratio': [0.94, 0.92, 1.02]
}

df = pd.DataFrame(data)

# Create figure with subplots
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))
fig.suptitle('ACO Heuristic Caching: Performance Analysis', fontsize=16, fontweight='bold')

# 1. Runtime Comparison
x = np.arange(len(df['Problem Size']))
width = 0.35
ax1.bar(x - width/2, df['Standard (s)'], width, label='Standard ACO', color='#3498db', alpha=0.8)
ax1.bar(x + width/2, df['Cached (s)'], width, label='Cached ACO', color='#2ecc71', alpha=0.8)
ax1.set_xlabel('Problem Size (courses)', fontweight='bold')
ax1.set_ylabel('Runtime (seconds)', fontweight='bold')
ax1.set_title('Runtime Comparison')
ax1.set_xticks(x)
ax1.set_xticklabels(df['Problem Size'])
ax1.legend()
ax1.grid(axis='y', alpha=0.3)

# 2. Speedup vs Problem Size
ax2.plot(df['Problem Size'], df['Speedup'], marker='o', linewidth=2, 
         markersize=8, color='#e74c3c', label='Observed Speedup')
ax2.axhline(y=1.0, color='gray', linestyle='--', alpha=0.5, label='No speedup')
ax2.fill_between(df['Problem Size'], 1.0, df['Speedup'], alpha=0.2, color='#e74c3c')
ax2.set_xlabel('Problem Size (courses)', fontweight='bold')
ax2.set_ylabel('Speedup Factor', fontweight='bold')
ax2.set_title('Speedup Scaling')
ax2.legend()
ax2.grid(alpha=0.3)
ax2.set_ylim([0.8, 1.8])

# 3. Quality Ratio
ax3.bar(df['Problem Size'], df['Quality Ratio'], color='#9b59b6', alpha=0.8)
ax3.axhline(y=1.0, color='red', linestyle='--', linewidth=2, label='Equal quality')
ax3.set_xlabel('Problem Size (courses)', fontweight='bold')
ax3.set_ylabel('Quality Ratio (Cached/Standard)', fontweight='bold')
ax3.set_title('Solution Quality Comparison')
ax3.legend()
ax3.grid(axis='y', alpha=0.3)
ax3.set_ylim([0.8, 1.2])

# Add annotations
for i, v in enumerate(df['Quality Ratio']):
    label = "Better" if v < 1.0 else "Same" if abs(v - 1.0) < 0.05 else "Worse"
    ax3.text(i, v + 0.02, f'{v:.2f}\n({label})', ha='center', fontweight='bold')

# 4. Time Savings
time_savings = df['Standard (s)'] - df['Cached (s)']
percent_savings = (time_savings / df['Standard (s)']) * 100
ax4.bar(df['Problem Size'], time_savings, color='#f39c12', alpha=0.8)
ax4.set_xlabel('Problem Size (courses)', fontweight='bold')
ax4.set_ylabel('Time Saved (seconds)', fontweight='bold')
ax4.set_title('Absolute Time Savings')
ax4.grid(axis='y', alpha=0.3)

# Add percentage labels
for i, (saving, pct) in enumerate(zip(time_savings, percent_savings)):
    ax4.text(i, saving + 0.05, f'{saving:.2f}s\n({pct:.1f}%)', 
             ha='center', fontweight='bold')

plt.tight_layout()
plt.savefig('aco_speedup_analysis.png', dpi=300, bbox_inches='tight')
print("Chart saved as 'aco_speedup_analysis.png'")

# Generate summary statistics
print("\n=== Summary Statistics ===")
print(f"Average Speedup: {df['Speedup'].mean():.2f}x")
print(f"Average Quality Ratio: {df['Quality Ratio'].mean():.2f}")
print(f"Total Time Saved (150 courses): {time_savings.iloc[-1]:.2f}s ({percent_savings.iloc[-1]:.1f}%)")
print(f"Speedup Trend: {'Increasing' if df['Speedup'].iloc[-1] > df['Speedup'].iloc[0] else 'Decreasing'}")

# Create data table for LaTeX
print("\n=== LaTeX Table ===")
print("\\begin{table}[h]")
print("\\centering")
print("\\begin{tabular}{|c|c|c|c|c|}")
print("\\hline")
print("\\textbf{Courses} & \\textbf{Standard (s)} & \\textbf{Cached (s)} & \\textbf{Speedup} & \\textbf{Quality} \\\\")
print("\\hline")
for _, row in df.iterrows():
    print(f"{row['Problem Size']} & {row['Standard (s)']:.2f} & {row['Cached (s)']:.2f} & {row['Speedup']:.2f}x & {row['Quality Ratio']:.2f} \\\\")
print("\\hline")
print("\\end{tabular}")
print("\\caption{Performance comparison of standard ACO vs. heuristic caching ACO}")
print("\\label{tab:aco_comparison}")
print("\\end{table}")
