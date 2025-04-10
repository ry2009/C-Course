from manim import *
import numpy as np

class TradeNginDemo(Scene):
    def construct(self):
        # Set up the section title
        title = Text("Threading in Trade-Ngin", font_size=40)
        title.to_edge(UP, buff=0.5)
        
        self.play(Write(title))
        self.wait(1)

        # Create a high-level architecture diagram of the Trade-Ngin system
        system_title = Text("Trade-Ngin Architecture", font_size=28)
        system_title.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(system_title))
        self.wait(1)
        
        # Create core components of Trade-Ngin
        market_data = Rectangle(height=2, width=2.5, color=BLUE, fill_opacity=0.2)
        market_data.move_to(LEFT * 3.5 + UP * 0.5)
        market_data_text = Text("Market Data\nProcessor", font_size=20).move_to(market_data)
        
        strategy = Rectangle(height=2, width=2.5, color=GREEN, fill_opacity=0.2)
        strategy.move_to(ORIGIN + UP * 0.5)
        strategy_text = Text("Strategy\nEngine", font_size=20).move_to(strategy)
        
        risk = Rectangle(height=2, width=2.5, color=RED, fill_opacity=0.2)
        risk.move_to(RIGHT * 3.5 + UP * 0.5)
        risk_text = Text("Risk\nManagement", font_size=20).move_to(risk)
        
        order = Rectangle(height=2, width=2.5, color=PURPLE, fill_opacity=0.2)
        order.move_to(ORIGIN + DOWN * 2.5)
        order_text = Text("Order\nManagement", font_size=20).move_to(order)
        
        # Create arrows connecting components
        md_to_strat = Arrow(market_data.get_right(), strategy.get_left(), color=BLUE)
        strat_to_risk = Arrow(strategy.get_right(), risk.get_left(), color=GREEN)
        strat_to_order = Arrow(strategy.get_bottom(), order.get_top(), color=GREEN)
        risk_to_order = Arrow(risk.get_bottom(), order.get_top() + RIGHT * 1.5, color=RED)
        
        # Show components and connections
        self.play(
            Create(market_data), Write(market_data_text),
            Create(strategy), Write(strategy_text),
            Create(risk), Write(risk_text),
            Create(order), Write(order_text)
        )
        self.wait(1)
        
        self.play(
            Create(md_to_strat), 
            Create(strat_to_risk), 
            Create(strat_to_order),
            Create(risk_to_order)
        )
        self.wait(1)
        
        # Show Trade-Ngin specific thread allocation for each component
        thread_allocations = VGroup(
            Text("8-16 threads", font_size=16, color=YELLOW).next_to(market_data, DOWN, buff=0.1),
            Text("10-30 threads", font_size=16, color=GREEN).next_to(strategy, DOWN, buff=0.1),
            Text("2-4 threads", font_size=16, color=RED).next_to(risk, UP, buff=0.1),
            Text("4-6 threads", font_size=16, color=PURPLE).next_to(order, UP, buff=0.1)
        )
        
        self.play(Write(thread_allocations))
        self.wait(1)
        
        # Clear the high-level diagram
        self.play(
            FadeOut(market_data), FadeOut(market_data_text),
            FadeOut(strategy), FadeOut(strategy_text),
            FadeOut(risk), FadeOut(risk_text),
            FadeOut(order), FadeOut(order_text),
            FadeOut(md_to_strat), FadeOut(strat_to_risk), 
            FadeOut(strat_to_order), FadeOut(risk_to_order),
            FadeOut(thread_allocations), FadeOut(system_title)
        )
        
        # Focus on Trade-Ngin market data processing threads
        md_detailed_title = Text("Trade-Ngin Market Data Processing", font_size=28)
        md_detailed_title.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(md_detailed_title))
        self.wait(1)
        
        # Create an exchange feed box at the top
        exchange = Rectangle(height=1, width=3, color=YELLOW, fill_opacity=0.2)
        exchange.to_edge(UP, buff=2)
        exchange_text = Text("Exchange Feed", font_size=18).move_to(exchange)
        
        self.play(Create(exchange), Write(exchange_text))
        self.wait(0.5)
        
        # Create thread pipeline visualization
        pipeline_title = Text("Trade-Ngin Thread Pipeline", font_size=20)
        pipeline_title.next_to(exchange, DOWN, buff=1)
        
        self.play(Write(pipeline_title))
        
        # Pipeline stages
        receiver = Rectangle(height=0.8, width=2, color=BLUE, fill_opacity=0.2)
        receiver.next_to(pipeline_title, DOWN, buff=0.5)
        receiver_text = Text("Receiver Threads", font_size=16).move_to(receiver)
        receiver_count = Text("8-16 threads", font_size=14).next_to(receiver, DOWN, buff=0.1)
        
        parser = Rectangle(height=0.8, width=2, color=GREEN, fill_opacity=0.2)
        parser.next_to(receiver, DOWN, buff=1)
        parser_text = Text("Parser Threads", font_size=16).move_to(parser)
        parser_count = Text("4-8 threads", font_size=14).next_to(parser, DOWN, buff=0.1)
        
        order_book = Rectangle(height=0.8, width=2, color=RED, fill_opacity=0.2)
        order_book.next_to(parser, DOWN, buff=1)
        order_book_text = Text("Order Book Threads", font_size=16).move_to(order_book)
        order_book_count = Text("8-12 threads", font_size=14).next_to(order_book, DOWN, buff=0.1)
        
        analytics = Rectangle(height=0.8, width=2, color=PURPLE, fill_opacity=0.2)
        analytics.next_to(order_book, DOWN, buff=1)
        analytics_text = Text("Analytics Threads", font_size=16).move_to(analytics)
        analytics_count = Text("2-4 threads", font_size=14).next_to(analytics, DOWN, buff=0.1)
        
        # Arrows connecting pipeline stages
        pipeline_arrows = VGroup(
            Arrow(exchange.get_bottom(), receiver.get_top(), color=YELLOW),
            Arrow(receiver.get_bottom(), parser.get_top(), color=BLUE),
            Arrow(parser.get_bottom(), order_book.get_top(), color=GREEN),
            Arrow(order_book.get_bottom(), analytics.get_top(), color=RED)
        )
        
        # Display pipeline components
        self.play(
            Create(receiver), Write(receiver_text), Write(receiver_count),
            Create(parser), Write(parser_text), Write(parser_count),
            Create(order_book), Write(order_book_text), Write(order_book_count),
            Create(analytics), Write(analytics_text), Write(analytics_count)
        )
        self.play(Create(pipeline_arrows))
        self.wait(1.5)
        
        # Trade-Ngin ring buffer visualization
        buffer_title = Text("Trade-Ngin Lock-Free Ring Buffer", font_size=20)
        buffer_title.to_edge(RIGHT, buff=1.5)
        buffer_title.shift(UP * 1)
        
        ring_buffer = Circle(radius=1.2, color=WHITE)
        ring_buffer.next_to(buffer_title, DOWN, buff=0.5)
        
        # Create slots in the ring buffer
        slots = VGroup()
        for i in range(8):
            angle = i * (2 * PI / 8)
            slot = Square(side_length=0.4, color=BLUE)
            slot.move_to(ring_buffer.get_center() + 1.2 * np.array([np.cos(angle), np.sin(angle), 0]))
            slots.add(slot)
        
        # Read/Write pointers
        read_arrow = Arrow(ring_buffer.get_center(), slots[1].get_center(), color=GREEN, buff=0.3)
        read_text = Text("read", font_size=14, color=GREEN).next_to(read_arrow.get_start(), UP, buff=0.1)
        
        write_arrow = Arrow(ring_buffer.get_center(), slots[5].get_center(), color=RED, buff=0.3)
        write_text = Text("write", font_size=14, color=RED).next_to(write_arrow.get_start(), DOWN, buff=0.1)
        
        self.play(Write(buffer_title), Create(ring_buffer))
        self.play(Create(slots))
        self.play(Create(read_arrow), Write(read_text), Create(write_arrow), Write(write_text))
        self.wait(1.5)
        
        # Clear the market data section
        self.play(
            FadeOut(exchange), FadeOut(exchange_text),
            FadeOut(pipeline_title),
            FadeOut(receiver), FadeOut(receiver_text), FadeOut(receiver_count),
            FadeOut(parser), FadeOut(parser_text), FadeOut(parser_count),
            FadeOut(order_book), FadeOut(order_book_text), FadeOut(order_book_count),
            FadeOut(analytics), FadeOut(analytics_text), FadeOut(analytics_count),
            FadeOut(pipeline_arrows),
            FadeOut(buffer_title), FadeOut(ring_buffer), FadeOut(slots),
            FadeOut(read_arrow), FadeOut(read_text), FadeOut(write_arrow), FadeOut(write_text),
            FadeOut(md_detailed_title)
        )
        
        # Trade-Ngin Strategy Execution Threading
        strategy_title = Text("Trade-Ngin Strategy Execution", font_size=28)
        strategy_title.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(strategy_title))
        self.wait(1)
        
        # Strategy thread visualization
        strategy_layout = VGroup(
            Text("Signal Generation Threads", font_size=18),
            Text("(10-30 dedicated threads)", font_size=16)
        ).arrange(DOWN, buff=0.2)
        strategy_layout.move_to(UP * 1.5)
        
        alpha_layout = VGroup(
            Text("Alpha Calculation Thread Pool", font_size=18),
            Text("(4-8 worker threads)", font_size=16)
        ).arrange(DOWN, buff=0.2)
        alpha_layout.move_to(UP * 0)
        
        position_layout = VGroup(
            Text("Position Management Thread", font_size=18),
            Text("(Single dedicated thread)", font_size=16)
        ).arrange(DOWN, buff=0.2)
        position_layout.move_to(DOWN * 1.5)
        
        # Thread pipeline
        pipeline_box = Rectangle(height=4, width=6, color=BLUE, fill_opacity=0.1)
        pipeline_box.move_to(LEFT * 4)
        
        pipeline_title = Text("Trade-Ngin Thread Pipeline", font_size=20)
        pipeline_title.next_to(pipeline_box, UP, buff=0.3)
        
        # Pipeline stages within strategy
        pipe1 = Rectangle(height=0.8, width=2, color=GREEN, fill_opacity=0.3)
        pipe1.move_to(pipeline_box.get_center() + UP * 1)
        pipe1_text = Text("Strategy Thread 1", font_size=14).move_to(pipe1)
        
        pipe2 = Rectangle(height=0.8, width=2, color=GREEN, fill_opacity=0.3)
        pipe2.move_to(pipeline_box.get_center())
        pipe2_text = Text("Strategy Thread 2", font_size=14).move_to(pipe2)
        
        pipe3 = Rectangle(height=0.8, width=2, color=PURPLE, fill_opacity=0.3)
        pipe3.move_to(pipeline_box.get_center() + DOWN * 1)
        pipe3_text = Text("Order Generation", font_size=14).move_to(pipe3)
        
        # Arrows connecting pipeline stages
        pipe_arrows = VGroup(
            Arrow(pipe1.get_bottom(), pipe2.get_top(), color=GREEN),
            Arrow(pipe2.get_bottom(), pipe3.get_top(), color=GREEN)
        )
        
        # Show the strategy thread organization
        self.play(Write(strategy_layout), Write(alpha_layout), Write(position_layout))
        self.wait(1)
        
        self.play(
            Create(pipeline_box), 
            Write(pipeline_title),
            Create(pipe1), Write(pipe1_text),
            Create(pipe2), Write(pipe2_text),
            Create(pipe3), Write(pipe3_text)
        )
        self.play(Create(pipe_arrows))
        self.wait(1.5)
        
        # Trade-Ngin thread affinity example
        affinity_text = Text("Thread Affinity in Trade-Ngin:", font_size=20)
        affinity_text.next_to(alpha_layout, RIGHT, buff=3.5)
        affinity_text.shift(UP * 1)
        
        affinity_cmd = Code(
            code_string="""numactl --cpunodebind=0 --membind=0 \\
./trade_ngin_main \\
--strategy-threads=12 \\
--market-data-threads=16""",
            background="window",
            tab_width=4
        )
        affinity_cmd.next_to(affinity_text, DOWN, buff=0.5)
        affinity_cmd.scale(0.8)
        
        self.play(Write(affinity_text), Write(affinity_cmd))
        self.wait(2)
        
        # Clear strategy section
        self.play(
            FadeOut(strategy_layout), FadeOut(alpha_layout), FadeOut(position_layout),
            FadeOut(pipeline_box), FadeOut(pipeline_title),
            FadeOut(pipe1), FadeOut(pipe1_text),
            FadeOut(pipe2), FadeOut(pipe2_text),
            FadeOut(pipe3), FadeOut(pipe3_text),
            FadeOut(pipe_arrows),
            FadeOut(affinity_text), FadeOut(affinity_cmd),
            FadeOut(strategy_title)
        )
        
        # Trade-Ngin Order Management and Hot Path
        order_title = Text("Trade-Ngin Hot Path", font_size=28)
        order_title.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(order_title))
        self.wait(1)
        
        # Create hot path illustration
        hot_path_box = Rectangle(height=2, width=8, color=RED, fill_opacity=0.1)
        hot_path_box.move_to(ORIGIN)
        
        # Hot path components
        strat_box = Rectangle(height=1.5, width=2, color=GREEN, fill_opacity=0.3)
        strat_box.move_to(hot_path_box.get_center() + LEFT * 2.5)
        strat_text = Text("Strategy\nThread", font_size=16).move_to(strat_box)
        
        order_box = Rectangle(height=1.5, width=2, color=BLUE, fill_opacity=0.3)
        order_box.move_to(hot_path_box.get_center())
        order_text = Text("Order\nThread", font_size=16).move_to(order_box)
        
        exec_box = Rectangle(height=1.5, width=2, color=PURPLE, fill_opacity=0.3)
        exec_box.move_to(hot_path_box.get_center() + RIGHT * 2.5)
        exec_text = Text("Execution\nThread", font_size=16).move_to(exec_box)
        
        # Connect hot path components
        hot_arrows = VGroup(
            Arrow(strat_box.get_right(), order_box.get_left(), color=YELLOW),
            Arrow(order_box.get_right(), exec_box.get_left(), color=YELLOW)
        )
        
        # Hot path optimizations
        hot_path_title = Text("Hot Path Optimizations", font_size=20)
        hot_path_title.next_to(hot_path_box, UP, buff=0.5)
        
        optimizations = VGroup(
            Text("• Pinned to dedicated CPU cores", font_size=16),
            Text("• Real-time scheduling priorities", font_size=16),
            Text("• Locked memory pages", font_size=16),
            Text("• No blocking system calls", font_size=16)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.3)
        optimizations.next_to(hot_path_box, DOWN, buff=0.5)
        
        # Show hot path
        self.play(Create(hot_path_box), Write(hot_path_title))
        self.play(
            Create(strat_box), Write(strat_text),
            Create(order_box), Write(order_text),
            Create(exec_box), Write(exec_text)
        )
        self.play(Create(hot_arrows))
        self.play(Write(optimizations))
        self.wait(2)
        
        # Clear hot path section
        self.play(
            FadeOut(hot_path_box), FadeOut(hot_path_title),
            FadeOut(strat_box), FadeOut(strat_text),
            FadeOut(order_box), FadeOut(order_text),
            FadeOut(exec_box), FadeOut(exec_text),
            FadeOut(hot_arrows), FadeOut(optimizations),
            FadeOut(order_title)
        )
        
        # Trade-Ngin Performance Measurement
        perf_title = Text("Trade-Ngin Performance", font_size=28)
        perf_title.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(perf_title))
        self.wait(1)
        
        # Create histogram visualization
        axes = Axes(
            x_range=[0, 100, 10],
            y_range=[0, 100, 10],
            axis_config={"include_tip": False}
        )
        axes.move_to(ORIGIN)
        
        x_label = Text("Microseconds (μs)", font_size=16)
        x_label.next_to(axes, DOWN, buff=0.3)
        
        y_label = Text("Frequency", font_size=16)
        y_label.next_to(axes, LEFT, buff=0.3)
        y_label.rotate(PI/2)
        
        # Create performance bars
        # Generate random-looking but realistic latency distribution
        def latency_dist(center, width, height, num_points=10):
            points = []
            for i in range(num_points):
                x = center - width/2 + i * width/num_points
                # Log-normal-like distribution
                h = height * np.exp(-0.5 * ((x - center) / (width/4))**2)
                points.append((x, h))
            return points
        
        # Market data latency distribution
        md_points = latency_dist(4, 8, 80)
        md_graph = VGroup()
        for i, (x, h) in enumerate(md_points):
            if i < len(md_points) - 1:
                bar = Rectangle(height=h/10, width=(md_points[i+1][0] - x)/10, fill_opacity=0.5, color=BLUE)
                bar.move_to(axes.c2p((x + md_points[i+1][0])/2, h/20))
                md_graph.add(bar)
        
        md_label = Text("Market Data: 3-5 μs", font_size=16, color=BLUE)
        md_label.next_to(axes, UP, buff=0.5)
        md_label.shift(LEFT * 2)
        
        # Strategy latency distribution
        strategy_points = latency_dist(15, 10, 60)
        strategy_graph = VGroup()
        for i, (x, h) in enumerate(strategy_points):
            if i < len(strategy_points) - 1:
                bar = Rectangle(height=h/10, width=(strategy_points[i+1][0] - x)/10, fill_opacity=0.5, color=GREEN)
                bar.move_to(axes.c2p((x + strategy_points[i+1][0])/2, h/20))
                strategy_graph.add(bar)
        
        strategy_label = Text("Strategy: 10-20 μs", font_size=16, color=GREEN)
        strategy_label.next_to(md_label, RIGHT, buff=1)
        
        # Order latency distribution
        order_points = latency_dist(6, 6, 70)
        order_graph = VGroup()
        for i, (x, h) in enumerate(order_points):
            if i < len(order_points) - 1:
                bar = Rectangle(height=h/10, width=(order_points[i+1][0] - x)/10, fill_opacity=0.5, color=RED)
                bar.move_to(axes.c2p((x + order_points[i+1][0])/2, h/20))
                order_graph.add(bar)
        
        order_label = Text("Order Generation: 5-8 μs", font_size=16, color=RED)
        order_label.next_to(strategy_label, RIGHT, buff=1)
        
        # Show the performance visualizations
        self.play(Create(axes), Write(x_label), Write(y_label))
        self.play(Create(md_graph), Write(md_label))
        self.play(Create(strategy_graph), Write(strategy_label))
        self.play(Create(order_graph), Write(order_label))
        
        # Add total latency text
        total_latency = Text("End-to-End Latency: 50-100 μs", font_size=20)
        total_latency.next_to(axes, DOWN, buff=1.5)
        
        self.play(Write(total_latency))
        self.wait(2)
        
        # Clear performance section
        self.play(
            FadeOut(axes), FadeOut(x_label), FadeOut(y_label),
            FadeOut(md_graph), FadeOut(md_label),
            FadeOut(strategy_graph), FadeOut(strategy_label),
            FadeOut(order_graph), FadeOut(order_label),
            FadeOut(total_latency),
            FadeOut(perf_title)
        )
        
        # Trade-Ngin Real-World Challenges
        challenges_title = Text("Real-World Threading Challenges in Trade-Ngin", font_size=28)
        challenges_title.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(challenges_title))
        self.wait(1)
        
        # Create challenges and solutions
        challenges = VGroup(
            Text("Problem: Exchange connection threads blocked on I/O", font_size=18),
            Text("Solution: Non-blocking I/O with event polling threads", font_size=18, color=GREEN),
            
            Text("Problem: JVM garbage collection pauses", font_size=18),
            Text("Solution: C++ rewrite with custom memory management", font_size=18, color=GREEN),
            
            Text("Problem: Priority inversion in thread waiting", font_size=18),
            Text("Solution: Priority inheritance protocol for locks", font_size=18, color=GREEN),
            
            Text("Problem: Thread startup jitter affecting latency", font_size=18),
            Text("Solution: Pre-start and warm up all threads", font_size=18, color=GREEN),
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.3)
        challenges.scale(0.8)
        challenges.move_to(ORIGIN)
        
        self.play(Write(challenges))
        self.wait(2)
        
        # Clear challenges section
        self.play(
            FadeOut(challenges), 
            FadeOut(challenges_title)
        )
        
        # Trade-Ngin Lessons from Production
        lessons_title = Text("Lessons from Trade-Ngin Production", font_size=28)
        lessons_title.next_to(title, DOWN, buff=0.5)
        
        self.play(Write(lessons_title))
        self.wait(1)
        
        # Create lessons learned
        lessons = VGroup(
            Text("1. Measure Everything", font_size=22),
            Text("Thread performance in theory vs. reality", font_size=18),
            
            Text("2. System Interactions Matter", font_size=22),
            Text("OS scheduling, interrupts impact performance", font_size=18),
            
            Text("3. Start Simple", font_size=22),
            Text("Add threading complexity only where needed", font_size=18),
            
            Text("4. Test Under Stress", font_size=22),
            Text("Thread issues appear during extreme conditions", font_size=18),
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.3)
        lessons.move_to(ORIGIN)
        
        self.play(Write(lessons))
        self.wait(3)  # Give extra time for final lessons
        
        # Closing
        self.play(FadeOut(lessons), FadeOut(lessons_title))
        
        closing_text = Text("By applying these specialized threading techniques,\nTrade-Ngin achieves consistent microsecond-level latencies", font_size=28)
        closing_text.move_to(ORIGIN)
        
        self.play(Write(closing_text))
        self.wait(2)
        
        self.play(FadeOut(closing_text), FadeOut(title)) 