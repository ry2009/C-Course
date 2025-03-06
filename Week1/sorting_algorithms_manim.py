from manim import *
import numpy as np
import random

config.background_color = "#0F111A"  # Dark background similar to trading terminals

class SortingAlgorithmsForFinance(Scene):
    def construct(self):
        self.intro_scene()
        self.algorithm_comparison_scene()
        self.std_sort_internals_scene()
        self.financial_data_scene()
        self.conclusion_scene()

    def intro_scene(self):
        # Title and introduction
        title = Text("Sorting Algorithms in Quantitative Finance", font_size=48)
        subtitle = Text("Why std::sort is Preferred in Financial Applications", font_size=32)
        subtitle.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(title))
        self.play(Write(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))
        
        # Financial data pipeline visualization
        pipeline_title = Text("Financial Data Pipeline", font_size=36).to_edge(UP)
        
        steps = ["Market Data", "Collection", "Preprocessing", "Sorting", "Analysis", "Trading Decision"]
        colors = [BLUE_B, BLUE_C, BLUE_D, RED, BLUE_E, GREEN_E]
        
        pipeline = VGroup()
        arrows = VGroup()
        
        for i, (step, color) in enumerate(zip(steps, colors)):
            box = Rectangle(height=0.7, width=2.5, fill_opacity=0.7, fill_color=color, stroke_color=WHITE)
            text = Text(step, font_size=20).move_to(box)
            process = VGroup(box, text)
            
            if i == 0:
                process.to_edge(LEFT).shift(DOWN)
            else:
                process.next_to(pipeline[i-1], RIGHT, buff=0.5)
                arrow = Arrow(pipeline[i-1].get_right(), process.get_left(), color=WHITE)
                arrows.add(arrow)
            
            pipeline.add(process)
        
        self.play(Write(pipeline_title))
        
        for i in range(len(steps)):
            if i == 0:
                self.play(Create(pipeline[i]))
            else:
                self.play(Create(arrows[i-1]), Create(pipeline[i]))
                if i == 3:  # Highlight sorting step
                    self.play(Indicate(pipeline[i], color=YELLOW, scale_factor=1.2))
                    self.wait(0.5)
        
        # Add critical path highlight
        time_text = Text("Time = Money in HFT", color=YELLOW, font_size=32).to_edge(DOWN)
        time_arrow = Arrow(time_text.get_top(), pipeline[3].get_bottom(), color=YELLOW)
        
        self.play(Write(time_text), Create(time_arrow))
        self.wait(2)
        
        self.play(
            *[FadeOut(obj) for obj in [pipeline_title, pipeline, arrows, time_text, time_arrow]]
        )

    def create_bars(self, values, width=0.5, max_height=4, color=BLUE):
        bars = VGroup()
        for i, val in enumerate(values):
            height = (val / max(values)) * max_height
            bar = Rectangle(height=height, width=width, fill_opacity=0.8, fill_color=color, stroke_width=1)
            bar.move_to(i * (width + 0.1) * RIGHT + height/2 * UP)
            bars.add(bar)
        
        # Center the bars
        bars.move_to(ORIGIN)
        return bars

    def algorithm_comparison_scene(self):
        # Sorting algorithm comparison
        title = Text("Sorting Algorithm Comparison", font_size=40)
        title.to_edge(UP)
        self.play(Write(title))
        
        # Algorithm complexity table
        table_data = [
            ["Algorithm", "Average", "Worst", "Memory", "Stable", "Use in Finance"],
            ["Quick Sort", "O(n log n)", "O(n²)", "O(log n)", "No", "Risky"],
            ["Merge Sort", "O(n log n)", "O(n log n)", "O(n)", "Yes", "When stability matters"],
            ["Heap Sort", "O(n log n)", "O(n log n)", "O(1)", "No", "Memory-constrained"],
            ["Introsort", "O(n log n)", "O(n log n)", "O(log n)", "No", "Production systems"]
        ]
        
        table = Table(
            table_data,
            col_labels=None,
            include_outer_lines=True,
            line_config={"stroke_width": 1},
            v_buff=0.3,
            h_buff=0.6
        )
        table.scale(0.6)
        table.next_to(title, DOWN, buff=0.5)
        
        # Highlight std::sort (Introsort) row
        introsort_row = table.get_rows()[4]
        
        self.play(Create(table))
        self.play(Indicate(introsort_row, color=YELLOW))
        
        # Algorithm visualization setup
        self.play(table.animate.scale(0.8).to_edge(UP+RIGHT))
        
        # Create datasets for visualization
        np.random.seed(42)  # For reproducibility
        data_size = 15
        data_quicksort = list(range(1, data_size + 1))
        random.shuffle(data_quicksort)
        data_mergesort = data_quicksort.copy()
        data_introsort = data_quicksort.copy()
        
        visualization_area = Rectangle(height=5, width=10, stroke_opacity=0, fill_opacity=0)
        visualization_area.to_edge(LEFT).shift(DOWN)
        
        # Algorithm labels
        quicksort_label = Text("Quick Sort", font_size=24, color=RED)
        mergesort_label = Text("Merge Sort", font_size=24, color=GREEN)
        introsort_label = Text("std::sort (Introsort)", font_size=24, color=YELLOW)
        
        labels = VGroup(quicksort_label, mergesort_label, introsort_label)
        labels.arrange(DOWN, buff=0.5)
        labels.to_edge(LEFT)
        
        self.play(Create(visualization_area), Write(labels))
        
        # Show visualization of each algorithm
        self.visualize_quicksort(data_quicksort, quicksort_label)
        self.wait(1)
        self.visualize_mergesort(data_mergesort, mergesort_label)
        self.wait(1)
        self.visualize_introsort(data_introsort, introsort_label)
        
        self.wait(2)
        self.play(
            *[FadeOut(obj) for obj in [title, table, visualization_area, labels]]
        )

    def visualize_quicksort(self, data, label):
        # Simplified quicksort visualization
        bars = self.create_bars(data, color=RED)
        bars.next_to(label, RIGHT, buff=1)
        
        self.play(Create(bars))
        
        def quick_sort_animation(arr, left, right, bars, scene):
            if left < right:
                # Choose pivot (last element)
                pivot_idx = right
                pivot_value = arr[pivot_idx]
                
                # Highlight pivot
                pivot_bar = bars[pivot_idx]
                scene.play(pivot_bar.animate.set_color(YELLOW), run_time=0.3)
                
                # Partition
                i = left - 1
                for j in range(left, right):
                    if arr[j] <= pivot_value:
                        i += 1
                        arr[i], arr[j] = arr[j], arr[i]
                        
                        # Swap bars
                        if i != j:
                            scene.play(
                                bars[i].animate.set_color(GREEN), 
                                bars[j].animate.set_color(GREEN),
                                run_time=0.2
                            )
                            bars[i], bars[j] = bars[j], bars[i]
                            scene.play(
                                bars[i].animate.move_to(i * 0.6 * RIGHT + bars[i].get_height()/2 * UP),
                                bars[j].animate.move_to(j * 0.6 * RIGHT + bars[j].get_height()/2 * UP),
                                run_time=0.2
                            )
                            scene.play(
                                bars[i].animate.set_color(RED),
                                bars[j].animate.set_color(RED),
                                run_time=0.2
                            )
                
                # Place pivot in correct position
                arr[i+1], arr[right] = arr[right], arr[i+1]
                if i+1 != right:
                    scene.play(
                        bars[i+1].animate.set_color(BLUE),
                        bars[right].animate.set_color(BLUE),
                        run_time=0.2
                    )
                    bars[i+1], bars[right] = bars[right], bars[i+1]
                    scene.play(
                        bars[i+1].animate.move_to((i+1) * 0.6 * RIGHT + bars[i+1].get_height()/2 * UP),
                        bars[right].animate.move_to(right * 0.6 * RIGHT + bars[right].get_height()/2 * UP),
                        run_time=0.2
                    )
                    scene.play(
                        bars[i+1].animate.set_color(RED),
                        bars[right].animate.set_color(RED),
                        run_time=0.2
                    )
                
                # Mark pivot as sorted
                scene.play(bars[i+1].animate.set_color(GREEN_E), run_time=0.2)
                
                # Only recurse on shorter partitions for demo
                if right - (i+1) < 3 and i+1 - left < 3:
                    quick_sort_animation(arr, left, i, bars, scene)
                    quick_sort_animation(arr, i + 2, right, bars, scene)
        
        # Run quicksort animation on a subset to keep it brief
        quick_sort_animation(data, 0, len(data) - 1, bars, self)
        
        # Show final sorted state
        sorted_bars = self.create_bars(sorted(data), color=GREEN_E)
        sorted_bars.move_to(bars.get_center())
        
        self.play(Transform(bars, sorted_bars))
        self.wait(1)
        
        # Add financial context
        worst_case = Text("Worst case: O(n²) - Dangerous for time-critical systems", font_size=20, color=RED)
        worst_case.next_to(bars, DOWN)
        self.play(Write(worst_case))
        self.wait(1)
        
        self.play(FadeOut(bars), FadeOut(worst_case))

    def visualize_mergesort(self, data, label):
        # Simplified mergesort visualization
        bars = self.create_bars(data, color=GREEN)
        bars.next_to(label, RIGHT, buff=1)
        
        self.play(Create(bars))
        
        # Just show the divide and conquer concept
        divider1 = DashedLine(
            start=bars.get_top() + UP * 0.5,
            end=bars.get_bottom() + DOWN * 0.5,
            color=WHITE
        )
        divider1.move_to(bars.get_center())
        
        self.play(Create(divider1))
        
        divider2 = DashedLine(
            start=bars.get_top() + UP * 0.5,
            end=bars.get_bottom() + DOWN * 0.5,
            color=WHITE
        )
        divider2.move_to(bars.get_left() + bars.get_width() * 0.25 * RIGHT)
        
        divider3 = DashedLine(
            start=bars.get_top() + UP * 0.5,
            end=bars.get_bottom() + DOWN * 0.5,
            color=WHITE
        )
        divider3.move_to(bars.get_left() + bars.get_width() * 0.75 * RIGHT)
        
        self.play(Create(divider2), Create(divider3))
        
        # Show merging step
        merge_text = Text("Merge step...", font_size=20)
        merge_text.next_to(bars, DOWN)
        
        self.play(Write(merge_text))
        self.wait(1)
        
        # Show sorted result
        sorted_bars = self.create_bars(sorted(data), color=GREEN_E)
        sorted_bars.move_to(bars.get_center())
        
        self.play(
            Transform(bars, sorted_bars),
            FadeOut(divider1),
            FadeOut(divider2),
            FadeOut(divider3),
            FadeOut(merge_text)
        )
        
        # Add financial context
        stable_text = Text("Stable, O(n log n) worst case, but O(n) memory overhead", font_size=20, color=GREEN)
        stable_text.next_to(bars, DOWN)
        self.play(Write(stable_text))
        self.wait(1)
        
        self.play(FadeOut(bars), FadeOut(stable_text))

    def visualize_introsort(self, data, label):
        # Introsort visualization
        bars = self.create_bars(data, color=YELLOW)
        bars.next_to(label, RIGHT, buff=1)
        
        self.play(Create(bars))
        
        # Show hybrid nature with switching
        introsort_title = Text("Introsort: Hybrid Algorithm", font_size=24)
        introsort_title.next_to(bars, UP)
        
        self.play(Write(introsort_title))
        
        # Show the algorithm switching
        algo_box = Rectangle(height=1.5, width=3, fill_color=DARK_BLUE, fill_opacity=0.8, stroke_color=WHITE)
        algo_box.next_to(bars, DOWN, buff=0.5)
        
        quick_text = Text("QuickSort", font_size=20, color=RED)
        heap_text = Text("HeapSort", font_size=20, color=GREEN)
        insert_text = Text("InsertionSort", font_size=20, color=BLUE)
        
        quick_text.move_to(algo_box)
        heap_text.move_to(algo_box)
        insert_text.move_to(algo_box)
        
        self.play(Create(algo_box), Write(quick_text))
        
        # Decision points
        decision1 = Text("If recursion depth > log₂n", font_size=18)
        decision1.next_to(algo_box, LEFT, buff=0.5)
        arrow1 = Arrow(decision1.get_right(), algo_box.get_left(), color=WHITE)
        
        decision2 = Text("If partition size < 16", font_size=18)
        decision2.next_to(algo_box, RIGHT, buff=0.5)
        arrow2 = Arrow(decision2.get_left(), algo_box.get_right(), color=WHITE)
        
        self.play(
            Write(decision1),
            Create(arrow1)
        )
        self.wait(1)
        
        # Switch to heapsort
        self.play(Transform(quick_text, heap_text))
        self.wait(1)
        
        self.play(
            Write(decision2),
            Create(arrow2)
        )
        self.wait(1)
        
        # Switch to insertion sort
        self.play(Transform(quick_text, insert_text))
        self.wait(1)
        
        # Show sorted result
        sorted_bars = self.create_bars(sorted(data), color=YELLOW_E)
        sorted_bars.move_to(bars.get_center())
        
        self.play(Transform(bars, sorted_bars))
        
        # Highlight best of all worlds
        best_text = Text("Best of all worlds: Speed, Consistency, Low Memory", font_size=20, color=YELLOW)
        best_text.next_to(algo_box, DOWN, buff=0.5)
        
        self.play(Write(best_text))
        self.wait(2)
        
        self.play(
            FadeOut(bars),
            FadeOut(introsort_title),
            FadeOut(algo_box),
            FadeOut(quick_text),
            FadeOut(decision1),
            FadeOut(decision2),
            FadeOut(arrow1),
            FadeOut(arrow2),
            FadeOut(best_text)
        )

    def financial_data_scene(self):
        # Show real-world performance with financial data
        title = Text("Financial Application Performance", font_size=40)
        title.to_edge(UP)
        
        self.play(Write(title))
        
        # Financial dataset description
        dataset_desc = VGroup(
            Text("Typical Financial Datasets:", font_size=28),
            Text("- Order Book (10⁵-10⁷ entries)", font_size=24),
            Text("- Price Time Series (10⁶-10⁹ points)", font_size=24),
            Text("- Portfolio Securities (10²-10⁴ items)", font_size=24)
        )
        dataset_desc.arrange(DOWN, aligned_edge=LEFT)
        dataset_desc.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(dataset_desc))
        self.wait(1)
        
        # Performance graph
        axes = Axes(
            x_range=[2, 9, 1],
            y_range=[0, 5, 1],
            x_length=6,
            y_length=4,
            axis_config={"include_tip": False, "include_numbers": False}
        )
        
        x_label = Text("log₁₀(Data Size)", font_size=20)
        y_label = Text("Time (ms)", font_size=20)
        
        x_label.next_to(axes, DOWN)
        y_label.next_to(axes, LEFT).rotate(PI/2)
        
        graph_group = VGroup(axes, x_label, y_label)
        graph_group.to_edge(DOWN)
        
        # Data points
        data_sizes = [10**i for i in range(3, 10)]
        log_sizes = [math.log10(size) for size in data_sizes]
        
        # Generate realistic time curves
        # These are approximations to show the algorithmic complexity
        quicksort_times = [0.1, 0.5, 2, 10, 100, 800, 4500]  # Shows poor worst-case
        mergesort_times = [0.15, 0.7, 3, 15, 75, 350, 1600]  # Consistent but larger constant
        introsort_times = [0.1, 0.5, 2, 10, 50, 240, 1200]   # Best overall
        
        # Normalize for graph display
        max_time = max(max(quicksort_times), max(mergesort_times), max(introsort_times))
        quicksort_points = [(x, y/max_time*4) for x, y in zip(log_sizes, quicksort_times)]
        mergesort_points = [(x, y/max_time*4) for x, y in zip(log_sizes, mergesort_times)]
        introsort_points = [(x, y/max_time*4) for x, y in zip(log_sizes, introsort_times)]
        
        # Create dot sets for the graphs
        quicksort_dots = VGroup(*[Dot(axes.c2p(x, y), color=RED) for x, y in quicksort_points])
        mergesort_dots = VGroup(*[Dot(axes.c2p(x, y), color=GREEN) for x, y in mergesort_points])
        introsort_dots = VGroup(*[Dot(axes.c2p(x, y), color=YELLOW) for x, y in introsort_points])
        
        # Create lines connecting the dots
        quicksort_line = VMobject(color=RED)
        quicksort_line.set_points_as_corners([axes.c2p(x, y) for x, y in quicksort_points])
        
        mergesort_line = VMobject(color=GREEN)
        mergesort_line.set_points_as_corners([axes.c2p(x, y) for x, y in mergesort_points])
        
        introsort_line = VMobject(color=YELLOW)
        introsort_line.set_points_as_corners([axes.c2p(x, y) for x, y in introsort_points])
        
        # X-axis labels
        x_labels = VGroup()
        for i in range(3, 10):
            label = Text(f"10^{i}", font_size=16)
            label.next_to(axes.c2p(i, 0), DOWN, buff=0.5)
            x_labels.add(label)
        
        # Legend
        legend = VGroup(
            VGroup(Line(color=RED), Text("QuickSort", font_size=16, color=RED)),
            VGroup(Line(color=GREEN), Text("MergeSort", font_size=16, color=GREEN)),
            VGroup(Line(color=YELLOW), Text("std::sort", font_size=16, color=YELLOW))
        )
        
        for item in legend:
            item[0].set_length(0.5)
            item[1].next_to(item[0], RIGHT, buff=0.2)
            
        legend.arrange(DOWN, aligned_edge=LEFT)
        legend.to_edge(RIGHT).shift(UP)
        
        # Display the graph
        self.play(Create(axes), Write(x_label), Write(y_label))
        self.play(Write(x_labels))
        
        self.play(
            Create(quicksort_line),
            Create(quicksort_dots),
            Create(mergesort_line),
            Create(mergesort_dots),
            Create(introsort_line),
            Create(introsort_dots),
            Write(legend)
        )
        
        # Highlight performance difference at large scale
        large_scale = Ellipse(width=1.5, height=1, color=WHITE)
        large_scale.move_to(axes.c2p(9, 3.5))
        
        self.play(Create(large_scale))
        
        large_scale_label = Text("Critical performance\ndifference at scale", font_size=16)
        large_scale_label.next_to(large_scale, UP)
        
        self.play(Write(large_scale_label))
        self.wait(2)
        
        # Financial impact
        impact_text = Text("Real-world impact on HFT:", font_size=24)
        impact_points = VGroup(
            Text("- 1ms delay = $100K lost per day", font_size=20, color=RED),
            Text("- Consistent performance critical during volatility", font_size=20),
            Text("- Memory efficiency important for parallel operations", font_size=20)
        )
        
        impact_points.arrange(DOWN, aligned_edge=LEFT)
        impact_group = VGroup(impact_text, impact_points)
        impact_group.arrange(DOWN, aligned_edge=LEFT)
        impact_group.to_edge(LEFT + UP).shift(DOWN * 2)
        
        self.play(
            FadeOut(dataset_desc),
            Write(impact_text),
            Write(impact_points)
        )
        
        self.wait(2)
        
        self.play(
            *[FadeOut(obj) for obj in [
                title, axes, x_label, y_label, x_labels, quicksort_line, 
                quicksort_dots, mergesort_line, mergesort_dots, introsort_line, 
                introsort_dots, legend, large_scale, large_scale_label, 
                impact_text, impact_points
            ]]
        )

    def conclusion_scene(self):
        # Summary and conclusion
        title = Text("Why std::sort for Financial Applications?", font_size=40)
        title.to_edge(UP)
        
        self.play(Write(title))
        
        benefits = VGroup(
            Text("1. Guaranteed O(n log n) worst-case complexity", font_size=28),
            Text("2. Adaptive algorithm selection based on data characteristics", font_size=28),
            Text("3. Small constant factors - optimized for real hardware", font_size=28),
            Text("4. Low memory overhead (in-place sorting)", font_size=28),
            Text("5. Extensively tested in production environments", font_size=28)
        )
        
        benefits.arrange(DOWN, aligned_edge=LEFT, buff=0.4)
        benefits.next_to(title, DOWN, buff=0.7)
        
        for benefit in benefits:
            self.play(Write(benefit))
            self.wait(0.5)
        
        self.wait(1)
        
        # Final takeaway
        takeaway = Text("In quantitative finance, algorithm selection is a critical performance decision", 
                       font_size=32)
        takeaway.to_edge(DOWN, buff=1)
        
        self.play(Write(takeaway))
        self.wait(2)
        
        # Code example
        code = """
// C++ code example
#include <algorithm>
#include <vector>

void process_order_book(std::vector<Order>& orders) {
    // std::sort - optimal choice for financial data
    std::sort(orders.begin(), orders.end(), 
              [](const Order& a, const Order& b) {
                  return a.price > b.price;  // Sort by price (descending)
              });
    
    // Process sorted orders...
}
        """
        
        code_text = Code(
            code=code,
            tab_width=4,
            language="cpp",
            font="Monospace",
            font_size=20
        )
        
        code_text.to_edge(RIGHT).shift(DOWN)
        
        self.play(
            FadeOut(benefits),
            FadeOut(takeaway),
            Write(code_text)
        )
        
        self.wait(2)
        
        final_text = Text("Use std::sort for production financial systems", 
                         font_size=32, color=YELLOW)
        final_text.to_edge(DOWN, buff=1)
        
        self.play(Write(final_text))
        self.wait(3)
        
        self.play(
            *[FadeOut(obj) for obj in [title, code_text, final_text]]
        )

# To render this animation, run:
# manim -pql sorting_algorithms_manim.py SortingAlgorithmsForFinance 