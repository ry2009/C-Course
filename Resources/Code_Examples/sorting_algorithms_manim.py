#!/usr/bin/env python3
from manim import *
import numpy as np
import random

class SortingAlgorithm(Scene):
    def construct(self):
        self.camera.background_color = "#1E1E1E"
        self.intro_scene()
        self.quick_sort_scene()
        self.merge_sort_scene()
        self.intro_sort_scene()
        self.performance_comparison_scene()
        self.conclusion_scene()

    def intro_scene(self):
        title = Text("Sorting Algorithms in Quantitative Finance", font="Arial", color=BLUE).scale(0.8)
        subtitle = Text("Performance Comparison for Financial Data", font="Arial", color=WHITE).scale(0.6)
        subtitle.next_to(title, DOWN)
        
        self.play(Write(title))
        self.play(FadeIn(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

    def create_bars(self, values, color=BLUE, width=0.7):
        bars = VGroup()
        for i, val in enumerate(values):
            bar = Rectangle(
                height=val * 0.15,
                width=width,
                fill_color=color,
                fill_opacity=1,
                stroke_width=1,
                stroke_color=WHITE
            )
            bar.move_to(i * (width + 0.1) * RIGHT + bar.height / 2 * UP)
            bars.add(bar)
        
        bars.center()
        return bars

    def quick_sort_scene(self):
        title = Text("QuickSort", font="Arial", color=RED).scale(0.8)
        title.to_edge(UP)
        
        # Create initial array
        values = list(range(1, 16))
        random.shuffle(values)
        bars = self.create_bars(values, color=RED)
        
        self.play(Write(title))
        self.play(FadeIn(bars))
        self.wait(1)
        
        # Animate QuickSort
        self.quick_sort_animation(bars, values, 0, len(values) - 1)
        
        # Show complexity
        complexity = Text("Average: O(n log n)   Worst: O(n²)", font="Arial", color=WHITE).scale(0.5)
        complexity.next_to(bars, DOWN, buff=0.5)
        
        self.play(Write(complexity))
        self.wait(2)
        
        self.play(FadeOut(bars), FadeOut(title), FadeOut(complexity))
    
    def quick_sort_animation(self, bars, values, low, high):
        if low < high:
            # Choose pivot (last element)
            pivot_idx = high
            pivot_value = values[pivot_idx]
            
            # Highlight pivot
            pivot_bar = bars[pivot_idx]
            self.play(pivot_bar.animate.set_color(YELLOW), run_time=0.5)
            
            # Partition
            i = low - 1
            for j in range(low, high):
                if values[j] <= pivot_value:
                    i += 1
                    # Swap values[i] and values[j]
                    values[i], values[j] = values[j], values[i]
                    
                    # Animate swap
                    if i != j:
                        self.play(
                            bars[i].animate.move_to(bars[j].get_center()),
                            bars[j].animate.move_to(bars[i].get_center()),
                            run_time=0.3
                        )
                        # Update bars array
                        bars[i], bars[j] = bars[j], bars[i]
            
            # Swap values[i+1] and values[high]
            values[i+1], values[high] = values[high], values[i+1]
            
            # Animate final swap with pivot
            if i+1 != high:
                self.play(
                    bars[i+1].animate.move_to(bars[high].get_center()),
                    bars[high].animate.move_to(bars[i+1].get_center()),
                    run_time=0.3
                )
                # Update bars array
                bars[i+1], bars[high] = bars[high], bars[i+1]
            
            # Reset pivot color
            self.play(bars[i+1].animate.set_color(GREEN), run_time=0.3)
            
            # Recursively sort the sub-arrays
            # For animation purposes, we'll limit the recursion depth
            if high - low < 10:  # Only animate smaller partitions
                self.quick_sort_animation(bars, values, low, i)
                self.quick_sort_animation(bars, values, i + 2, high)

    def merge_sort_scene(self):
        title = Text("MergeSort", font="Arial", color=GREEN).scale(0.8)
        title.to_edge(UP)
        
        # Create initial array
        values = list(range(1, 16))
        random.shuffle(values)
        bars = self.create_bars(values, color=GREEN)
        
        self.play(Write(title))
        self.play(FadeIn(bars))
        self.wait(1)
        
        # Animate MergeSort
        sorted_values = values.copy()
        temp_bars = bars.copy()
        self.merge_sort_animation(temp_bars, sorted_values, 0, len(values) - 1)
        
        # Update the original bars to match the sorted values
        for i, val in enumerate(sorted_values):
            new_bar = Rectangle(
                height=val * 0.15,
                width=0.7,
                fill_color=GREEN,
                fill_opacity=1,
                stroke_width=1,
                stroke_color=WHITE
            )
            new_bar.move_to(i * 0.8 * RIGHT + new_bar.height / 2 * UP)
            self.play(
                Transform(bars[i], new_bar),
                run_time=0.1
            )
        
        bars.center()
        
        # Show complexity
        complexity = Text("Average: O(n log n)   Worst: O(n log n)   Memory: O(n)", font="Arial", color=WHITE).scale(0.5)
        complexity.next_to(bars, DOWN, buff=0.5)
        
        self.play(Write(complexity))
        self.wait(2)
        
        self.play(FadeOut(bars), FadeOut(title), FadeOut(complexity))
    
    def merge_sort_animation(self, bars, values, start, end):
        if start < end:
            mid = (start + end) // 2
            
            # Highlight the current segment
            current_bars = VGroup(*[bars[i] for i in range(start, end + 1)])
            self.play(current_bars.animate.set_color(YELLOW), run_time=0.3)
            
            # Recursively sort the sub-arrays
            self.merge_sort_animation(bars, values, start, mid)
            self.merge_sort_animation(bars, values, mid + 1, end)
            
            # Merge the sorted sub-arrays
            self.merge_animation(bars, values, start, mid, end)
            
            # Reset color
            self.play(current_bars.animate.set_color(GREEN), run_time=0.3)
    
    def merge_animation(self, bars, values, start, mid, end):
        # Create temporary arrays
        L = values[start:mid + 1]
        R = values[mid + 1:end + 1]
        
        # Merge the arrays back into values[start..end]
        i = j = 0
        k = start
        
        while i < len(L) and j < len(R):
            if L[i] <= R[j]:
                values[k] = L[i]
                i += 1
            else:
                values[k] = R[j]
                j += 1
            k += 1
        
        # Copy the remaining elements
        while i < len(L):
            values[k] = L[i]
            i += 1
            k += 1
        
        while j < len(R):
            values[k] = R[j]
            j += 1
            k += 1
        
        # Update the bars to reflect the merged array
        for i in range(start, end + 1):
            new_bar = Rectangle(
                height=values[i] * 0.15,
                width=0.7,
                fill_color=BLUE,
                fill_opacity=1,
                stroke_width=1,
                stroke_color=WHITE
            )
            new_bar.move_to(i * 0.8 * RIGHT + new_bar.height / 2 * UP)
            self.play(
                Transform(bars[i], new_bar),
                run_time=0.1
            )

    def intro_sort_scene(self):
        title = Text("IntroSort (std::sort)", font="Arial", color=YELLOW).scale(0.8)
        title.to_edge(UP)
        
        # Create initial array
        values = list(range(1, 16))
        random.shuffle(values)
        bars = self.create_bars(values, color=YELLOW)
        
        self.play(Write(title))
        self.play(FadeIn(bars))
        self.wait(1)
        
        # Explain IntroSort with text
        explanation = VGroup(
            Text("IntroSort is a hybrid sorting algorithm:", font="Arial", color=WHITE).scale(0.5),
            Text("1. Starts with QuickSort", font="Arial", color=RED).scale(0.5),
            Text("2. Switches to HeapSort if recursion depth exceeds log₂(n)", font="Arial", color=BLUE).scale(0.5),
            Text("3. Uses InsertionSort for small partitions", font="Arial", color=GREEN).scale(0.5)
        ).arrange(DOWN, aligned_edge=LEFT)
        explanation.next_to(bars, DOWN, buff=0.5)
        
        self.play(Write(explanation), run_time=2)
        self.wait(2)
        
        # Show the sorted result
        sorted_values = sorted(values)
        for i, val in enumerate(sorted_values):
            new_bar = Rectangle(
                height=val * 0.15,
                width=0.7,
                fill_color=YELLOW,
                fill_opacity=1,
                stroke_width=1,
                stroke_color=WHITE
            )
            new_bar.move_to(i * 0.8 * RIGHT + new_bar.height / 2 * UP)
            self.play(
                Transform(bars[i], new_bar),
                run_time=0.1
            )
        
        bars.center()
        
        # Show complexity
        complexity = Text("Average: O(n log n)   Worst: O(n log n)   Memory: O(log n)", font="Arial", color=WHITE).scale(0.5)
        complexity.next_to(explanation, DOWN, buff=0.3)
        
        self.play(Write(complexity))
        self.wait(2)
        
        self.play(FadeOut(bars), FadeOut(title), FadeOut(explanation), FadeOut(complexity))

    def performance_comparison_scene(self):
        title = Text("Performance Comparison", font="Arial", color=BLUE).scale(0.8)
        title.to_edge(UP)
        
        self.play(Write(title))
        
        # Create axes
        axes = Axes(
            x_range=[3, 9, 1],
            y_range=[0, 10, 1],
            axis_config={"color": WHITE},
            x_axis_config={"include_tip": False},
            y_axis_config={"include_tip": False}
        )
        
        # Add labels
        x_label = Text("Data Size (log₁₀ n)", font="Arial", color=WHITE).scale(0.4)
        y_label = Text("Time (ms, log scale)", font="Arial", color=WHITE).scale(0.4)
        x_label.next_to(axes.x_axis, DOWN)
        y_label.next_to(axes.y_axis, LEFT)
        
        # Create legend
        legend = VGroup(
            Dot(color=RED),
            Text("QuickSort", font="Arial", color=RED).scale(0.4),
            Dot(color=GREEN),
            Text("MergeSort", font="Arial", color=GREEN).scale(0.4),
            Dot(color=YELLOW),
            Text("std::sort", font="Arial", color=YELLOW).scale(0.4)
        ).arrange(RIGHT, buff=0.2)
        legend.next_to(title, DOWN)
        
        # Create graph data
        # These are simulated performance values on a log scale
        x_values = np.array([3, 4, 5, 6, 7, 8, 9])  # log10(n) where n is data size
        
        # QuickSort (good for small data, degrades for large or pathological data)
        quicksort_y = np.array([0.5, 1.0, 2.0, 4.0, 8.0, 9.5, 9.8])
        
        # MergeSort (consistent but with overhead)
        mergesort_y = np.array([0.7, 1.3, 2.2, 3.5, 5.0, 6.5, 8.0])
        
        # std::sort (best overall performance)
        stdsort_y = np.array([0.4, 0.8, 1.5, 2.5, 3.8, 5.2, 6.8])
        
        # Create the graphs
        quicksort_graph = axes.plot_line_graph(x_values, quicksort_y, line_color=RED, add_vertex_dots=True)
        mergesort_graph = axes.plot_line_graph(x_values, mergesort_y, line_color=GREEN, add_vertex_dots=True)
        stdsort_graph = axes.plot_line_graph(x_values, stdsort_y, line_color=YELLOW, add_vertex_dots=True)
        
        # Add a highlight for the large data region
        highlight = Rectangle(
            width=2,
            height=4,
            fill_color=BLUE,
            fill_opacity=0.2,
            stroke_width=0
        )
        highlight.move_to(axes.c2p(8, 7))
        
        highlight_text = Text("Critical region for\nhigh-frequency trading", font="Arial", color=WHITE).scale(0.3)
        highlight_text.next_to(highlight, UP)
        
        # Animate
        self.play(FadeIn(axes), FadeIn(x_label), FadeIn(y_label))
        self.play(FadeIn(legend))
        self.play(Create(quicksort_graph), run_time=2)
        self.play(Create(mergesort_graph), run_time=2)
        self.play(Create(stdsort_graph), run_time=2)
        self.play(FadeIn(highlight), Write(highlight_text))
        
        self.wait(2)
        
        # Add annotations
        annotation1 = Text("QuickSort degrades with\npathological inputs", font="Arial", color=RED).scale(0.3)
        annotation1.next_to(axes.c2p(8.5, 9.5), RIGHT)
        
        annotation2 = Text("std::sort maintains best\noverall performance", font="Arial", color=YELLOW).scale(0.3)
        annotation2.next_to(axes.c2p(7, 5), RIGHT)
        
        self.play(Write(annotation1), Write(annotation2))
        
        self.wait(3)
        
        self.play(
            FadeOut(axes), FadeOut(x_label), FadeOut(y_label),
            FadeOut(legend), FadeOut(quicksort_graph), FadeOut(mergesort_graph),
            FadeOut(stdsort_graph), FadeOut(highlight), FadeOut(highlight_text),
            FadeOut(annotation1), FadeOut(annotation2), FadeOut(title)
        )

    def conclusion_scene(self):
        title = Text("Key Takeaways", font="Arial", color=BLUE).scale(0.8)
        
        points = VGroup(
            Text("1. std::sort (IntroSort) provides the best overall performance", font="Arial", color=WHITE).scale(0.5),
            Text("2. Consistent performance is critical for financial applications", font="Arial", color=WHITE).scale(0.5),
            Text("3. Memory efficiency matters for parallel operations", font="Arial", color=WHITE).scale(0.5),
            Text("4. Custom comparators allow for domain-specific sorting", font="Arial", color=WHITE).scale(0.5)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.5)
        
        self.play(Write(title))
        title.to_edge(UP)
        
        points.next_to(title, DOWN, buff=0.5)
        self.play(Write(points), run_time=3)
        
        self.wait(3)
        
        final_message = Text("Use std::sort for production financial systems", font="Arial", color=YELLOW).scale(0.7)
        final_message.next_to(points, DOWN, buff=0.8)
        
        self.play(Write(final_message))
        self.wait(2)
        
        self.play(FadeOut(title), FadeOut(points), FadeOut(final_message))


if __name__ == "__main__":
    # Command to render:
    # manim -pqh sorting_algorithms_manim.py SortingAlgorithm
    pass 