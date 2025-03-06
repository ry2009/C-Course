#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import os
from matplotlib.colors import LinearSegmentedColormap

print("Starting script...")

# Set the style
plt.style.use('ggplot')
sns.set_style("whitegrid")
print("Set style...")

# Create the data
algorithms = ['Bubble Sort', 'Insertion Sort', 'Merge Sort', 'QuickSort', 'HeapSort', 'Introsort (std::sort)']
best_case = ['O(n)', 'O(n)', 'O(n log n)', 'O(n log n)', 'O(n log n)', 'O(n log n)']
worst_case = ['O(n²)', 'O(n²)', 'O(n log n)', 'O(n²)', 'O(n log n)', 'O(n log n)']
used_in_finance = ['No', 'Sometimes', 'Yes', 'Sometimes', 'Yes', 'Yes']
notes = [
    'Too slow for large datasets',
    'Good for small or nearly sorted datasets',
    'Used in external sorting (e.g., large-scale trading data)',
    'Fast but bad worst-case performance',
    'Good for priority queues',
    'Hybrid: QuickSort + HeapSort + InsertionSort'
]

print("Created data...")

# Create a DataFrame
df = pd.DataFrame({
    'Algorithm': algorithms,
    'Best Case': best_case,
    'Worst Case': worst_case,
    'Used in Finance': used_in_finance,
    'Notes': notes
})

print("Created DataFrame...")

# Create a figure with multiple visualizations
fig = plt.figure(figsize=(16, 12))
fig.suptitle('Sorting Algorithms in Quantitative Finance', fontsize=20, fontweight='bold')
print("Created figure...")

# 1. Create a table visualization
ax1 = plt.subplot2grid((2, 2), (0, 0), colspan=2)
ax1.axis('tight')
ax1.axis('off')
table = ax1.table(
    cellText=df.values,
    colLabels=df.columns,
    loc='center',
    cellLoc='center',
    colColours=['#E6F3FF'] * 5
)
table.auto_set_font_size(False)
table.set_fontsize(10)
table.scale(1, 1.5)
print("Created table...")

# Highlight cells based on values
for i, algorithm in enumerate(algorithms):
    # Highlight worst case complexity
    if worst_case[i] == 'O(n²)':
        table[(i+1, 2)].set_facecolor('#FFCCCC')  # Light red
    else:
        table[(i+1, 2)].set_facecolor('#CCFFCC')  # Light green
    
    # Highlight finance usage
    if used_in_finance[i] == 'Yes':
        table[(i+1, 3)].set_facecolor('#CCFFCC')  # Light green
    elif used_in_finance[i] == 'Sometimes':
        table[(i+1, 3)].set_facecolor('#FFFFCC')  # Light yellow
    else:
        table[(i+1, 3)].set_facecolor('#FFCCCC')  # Light red

print("Highlighted cells...")

# 2. Create a bar chart for algorithm performance
ax2 = plt.subplot2grid((2, 2), (1, 0))
ax2.set_title('Algorithm Performance Characteristics', fontsize=14)

# Create performance scores (simplified for visualization)
performance_scores = {
    'Time Complexity': [1, 2, 4, 3.5, 4, 5],  # Higher is better
    'Memory Efficiency': [5, 4.5, 2, 4, 3.5, 4],
    'Stability': [5, 5, 5, 1, 1, 1],
    'Adaptability': [1, 4, 1, 2, 1, 5]
}

# Create a DataFrame for the scores
score_df = pd.DataFrame(performance_scores, index=algorithms)

# Plot the scores
score_df.plot(kind='bar', ax=ax2, width=0.8)
ax2.set_ylim(0, 5.5)
ax2.set_ylabel('Score (higher is better)')
ax2.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=4)
print("Created bar chart...")

# 3. Create a pie chart for finance usage
ax3 = plt.subplot2grid((2, 2), (1, 1))
ax3.set_title('Usage in Financial Applications', fontsize=14)

# Count the usage categories
usage_counts = df['Used in Finance'].value_counts()

# Create custom colors
colors = ['#4CAF50', '#FFC107', '#F44336']  # Green, Yellow, Red

# Create the pie chart
ax3.pie(
    usage_counts, 
    labels=usage_counts.index, 
    autopct='%1.1f%%',
    startangle=90,
    colors=colors,
    explode=[0.1 if x == 'Yes' else 0 for x in usage_counts.index],
    shadow=True
)
ax3.axis('equal')
print("Created pie chart...")

# Add a note about std::sort
plt.figtext(
    0.5, 0.02, 
    "Note: Introsort (std::sort) combines the advantages of multiple algorithms, "
    "making it the preferred choice for most financial applications.",
    ha='center', 
    fontsize=12, 
    bbox=dict(boxstyle='round,pad=0.5', facecolor='#E6F3FF', alpha=0.7)
)

# Save the figure
plt.tight_layout(rect=[0, 0.03, 1, 0.95])

# Ensure the output directory exists
os.makedirs('Resources/Slides', exist_ok=True)
print("Ensured output directory exists...")

# Save as PNG
png_path = 'Resources/Slides/sorting_algorithm_comparison.png'
plt.savefig(png_path, dpi=300, bbox_inches='tight')
print(f"Saved PNG to {os.path.abspath(png_path)}")

# Save as PDF
pdf_path = 'Resources/Slides/sorting_algorithm_comparison.pdf'
plt.savefig(pdf_path, bbox_inches='tight')
print(f"Saved PDF to {os.path.abspath(pdf_path)}")

print("Graph creation completed successfully!") 