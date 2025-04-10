from manim import *
import numpy as np

class DistributedSystemsVisualization(Scene):
    def construct(self):
        # Set background color to a dark blue instead of black
        self.camera.background_color = "#1a1a2e"

        # Introduction with fade in effect
        title = Text("Distributed Systems in Trading", font_size=40, color=BLUE)
        subtitle = Text("A Comprehensive Overview", font_size=30, color=LIGHT_GRAY)
        subtitle.next_to(title, DOWN)
        
        self.play(
            FadeIn(title, shift=UP),
            FadeIn(subtitle, shift=UP)
        )
        self.wait(5)  # Increased wait time
        self.play(FadeOut(subtitle))

        # Section 1: Basic Concepts with animated reveal
        section1 = Text("1. Basic Distributed Systems Concepts", font_size=30, color=YELLOW)
        section1.next_to(title, DOWN, buff=1)
        self.play(
            Write(section1, run_time=2),
            title.animate.scale(0.8).to_edge(UP)
        )
        self.wait(3)

        # CAP Theorem with dynamic build-up
        cap_title = Text("CAP Theorem", font_size=25, color=GREEN)
        cap_title.next_to(section1, DOWN, buff=1)
        self.play(Write(cap_title))
        self.wait(3)

        # CAP Triangle with animated construction
        cap_points = [
            np.array([-2, 1, 0]),  # Consistency
            np.array([0, -1, 0]),  # Availability
            np.array([2, 1, 0])    # Partition Tolerance
        ]
        
        # Create triangle sides one by one
        lines = VGroup()
        for i in range(3):
            start = cap_points[i]
            end = cap_points[(i + 1) % 3]
            line = Line(start, end, color=WHITE)
            lines.add(line)
            self.play(Create(line), run_time=1)
        
        # Add labels with explanations
        consistency_label = Text("Consistency", font_size=20, color=BLUE)
        consistency_label.move_to(cap_points[0] + np.array([0, 0.3, 0]))
        
        consistency_desc = Text("All nodes see the\nsame data at the\nsame time", 
                              font_size=16, color=LIGHT_GRAY)
        consistency_desc.next_to(consistency_label, UP)
        
        availability_label = Text("Availability", font_size=20, color=GREEN)
        availability_label.move_to(cap_points[1] + np.array([0, -0.3, 0]))
        
        availability_desc = Text("System remains\noperational\nat all times", 
                               font_size=16, color=LIGHT_GRAY)
        availability_desc.next_to(availability_label, DOWN)
        
        partition_label = Text("Partition\nTolerance", font_size=20, color=RED)
        partition_label.move_to(cap_points[2] + np.array([0, 0.3, 0]))
        
        partition_desc = Text("System continues\nto operate despite\nnetwork failures", 
                            font_size=16, color=LIGHT_GRAY)
        partition_desc.next_to(partition_label, UP)
        
        # Animate labels and descriptions
        for label, desc in [(consistency_label, consistency_desc),
                          (availability_label, availability_desc),
                          (partition_label, partition_desc)]:
            self.play(
                Write(label),
                FadeIn(desc, shift=UP),
                run_time=2
            )
            self.wait(3)

        # CAP Trade-offs with animated examples
        cap_explanation = VGroup(
            Text("You can only choose two:", font_size=20, color=YELLOW),
            Text("• CP: Consistent and Partition Tolerant", font_size=20),
            Text("• AP: Available and Partition Tolerant", font_size=20),
            Text("• CA: Consistent and Available", font_size=20)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.3)
        cap_explanation.next_to(lines, DOWN, buff=1)
        
        for i, text in enumerate(cap_explanation):
            self.play(Write(text), run_time=1)
            self.wait(2)

        # Add practical examples
        examples = VGroup(
            Text("Examples:", font_size=20, color=YELLOW),
            Text("CP: Banking transactions", font_size=18),
            Text("AP: Content delivery networks", font_size=18),
            Text("CA: Single-datacenter systems", font_size=18)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.3)
        examples.next_to(cap_explanation, DOWN, buff=0.5)
        
        self.play(Write(examples), run_time=2)
        self.wait(5)

        # Clear CAP section with fancy transition
        cap_group = VGroup(cap_title, lines, consistency_label, consistency_desc,
                          availability_label, availability_desc,
                          partition_label, partition_desc,
                          cap_explanation, examples)
        
        self.play(
            FadeOut(cap_group, shift=LEFT),
            run_time=2
        )
        
        # Clear previous sections to make space
        self.play(FadeOut(section1))
        
        # Section 2: System Architecture - move up on screen
        section2 = Text("2. Distributed Trading System Architecture", font_size=30, color=YELLOW)
        section2.next_to(title, DOWN, buff=1)
        self.play(Write(section2))
        self.wait(2)

        # Create detailed system components
        components = VGroup()
        component_texts = VGroup()
        
        # Define components with their roles
        nodes = [
            ("Market Data\nProcessor", "Processes real-time\nmarket data"),
            ("Strategy\nEngine", "Executes trading\nstrategies"),
            ("Risk\nManagement", "Monitors and\ncontrols risk"),
            ("Order\nManagement", "Handles order\nrouting")
        ]
        colors = [BLUE, GREEN, RED, YELLOW]
        
        # Position components in a more compact arrangement
        positions = [[-3, 0, 0], [-1, 0, 0], [1, 0, 0], [3, 0, 0]]
        
        for i, ((name, desc), color) in enumerate(zip(nodes, colors)):
            # Main component
            component = Circle(radius=0.5, color=color, fill_opacity=0.3)
            component.move_to(positions[i])
            
            # Component name
            name_text = Text(name, font_size=16)
            name_text.move_to(component.get_center())
            
            # Component description
            desc_text = Text(desc, font_size=12)
            desc_text.next_to(component, DOWN, buff=0.3)
            
            components.add(component)
            component_texts.add(name_text)
            component_texts.add(desc_text)

        self.play(Create(components), Write(component_texts))
        self.wait(3)

        # Add detailed connections
        connections = VGroup()
        connection_labels = VGroup()
        
        # Define connection types
        connection_types = [
            ("Order\nFlow", GREEN),
            ("Execution\nCommands", YELLOW),
            ("Market Data", BLUE),
            ("Order\nFlow", GREEN),
            ("Risk\nSignals", RED),
            ("Order\nFlow", GREEN)
        ]
        
        # Connect only adjacent components for clarity
        for i in range(3):
            # Connection line
            line = Line(components[i].get_center(), components[i+1].get_center(), color=WHITE)
            connections.add(line)
            
            # Connection label
            conn_type, color = connection_types[i]
            label = Text(conn_type, font_size=12, color=color)
            label.move_to(line.get_center())
            connection_labels.add(label)

        self.play(Create(connections), Write(connection_labels))
        self.wait(3)

        # Section 3: Data Flow
        # Move architecture components up to make room
        architecture_group = VGroup(components, component_texts, connections, connection_labels)
        self.play(architecture_group.animate.shift(UP*1.5))
        
        section3 = Text("3. Data Flow and Message Processing", font_size=30, color=YELLOW)
        section3.next_to(architecture_group, DOWN, buff=0.7)
        self.play(Write(section3))
        self.wait(2)

        # Show message flow animation
        for i in range(3):  # Just show key flows between adjacent components
            # Create message
            message = Circle(radius=0.2, color=WHITE, fill_opacity=1)
            message.move_to(components[i].get_center())
            
            # Animate message movement
            self.play(
                message.animate.move_to(components[i+1].get_center()),
                run_time=0.7
            )
            self.play(FadeOut(message))
        
        # Clear screen for next section
        self.play(
            FadeOut(architecture_group),
            FadeOut(section2),
            FadeOut(section3)
        )

        # Section 4: Performance Metrics
        section4 = Text("4. Performance Metrics and Optimization", font_size=30, color=YELLOW)
        section4.next_to(title, DOWN, buff=1)
        self.play(Write(section4))
        self.wait(2)

        # Create focused metrics table - smaller to fit screen better
        metrics = Table(
            [["Metric", "Target", "Achieved"],
             ["Order Book Sync", "< 100μs", "75μs"],
             ["Network Latency", "< 50μs", "45μs"],
             ["Time Sync", "< 1μs", "0.8μs"],
             ["Throughput", "1M+ msg/s", "1.2M msg/s"],
             ["Availability", "99.999%", "99.999%"]],
            include_outer_lines=True
        )
        metrics.scale(0.7)
        metrics.next_to(section4, DOWN, buff=0.7)
        self.play(Create(metrics))
        
        # Add importance highlights
        importance = VGroup(
            Text("Critical metrics for trading systems:", font_size=20, color=RED),
            Text("• Low latency ensures competitive execution", font_size=18),
            Text("• High throughput handles market volume", font_size=18),
            Text("• Precise time sync ensures fairness", font_size=18)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.3)
        importance.next_to(metrics, DOWN, buff=0.7)
        self.play(Write(importance))
        self.wait(4)

        # Clear screen for next section
        self.play(
            FadeOut(metrics),
            FadeOut(importance),
            FadeOut(section4)
        )

        # Section 5: Fault Tolerance
        section5 = Text("5. Fault Tolerance and Recovery", font_size=30, color=YELLOW)
        section5.next_to(title, DOWN, buff=1)
        self.play(Write(section5))
        self.wait(2)

        # Show fault tolerance mechanisms with animations
        fault_points = VGroup(
            Text("• Redundant Components", font_size=20),
            Text("• Automatic Failover", font_size=20),
            Text("• State Recovery", font_size=20),
            Text("• Network Partition Handling", font_size=20)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.5)
        fault_points.next_to(section5, DOWN, buff=0.7)
        
        for point in fault_points:
            self.play(Write(point), run_time=1)
            self.wait(1)
        
        # Animation showing failover
        # Create two nodes
        node1 = Circle(radius=0.5, color=GREEN, fill_opacity=0.5)
        node1.shift(LEFT*2.5)
        node1_label = Text("Primary", font_size=16).next_to(node1, UP)
        
        node2 = Circle(radius=0.5, color=BLUE, fill_opacity=0.3)
        node2.shift(RIGHT*2.5)
        node2_label = Text("Backup", font_size=16).next_to(node2, UP)
        
        arrow = Arrow(node1.get_right(), node2.get_left(), color=WHITE)
        replication_label = Text("Replication", font_size=14).next_to(arrow, UP, buff=0.1)
        
        failover_group = VGroup(node1, node1_label, node2, node2_label, arrow, replication_label)
        failover_group.next_to(fault_points, DOWN, buff=1)
        
        self.play(
            Create(node1),
            Write(node1_label),
            Create(node2),
            Write(node2_label),
            Create(arrow),
            Write(replication_label)
        )
        self.wait(2)
        
        # Simulate node failure
        fail_cross = VGroup(
            Line(node1.get_corner(UL), node1.get_corner(DR), color=RED),
            Line(node1.get_corner(UR), node1.get_corner(DL), color=RED)
        )
        
        self.play(
            Create(fail_cross),
            node1.animate.set_color(RED)
        )
        self.wait(1)
        
        # Switch to backup
        self.play(
            node2.animate.set_color(GREEN).set_fill(opacity=0.5),
            node2_label.animate.become(Text("New Primary", font_size=16).next_to(node2, UP))
        )
        self.wait(2)
        
        # Clear screen for next section
        self.play(
            FadeOut(fault_points),
            FadeOut(failover_group),
            FadeOut(fail_cross),
            FadeOut(section5)
        )

        # Section 6: Implementation Challenges
        section6 = Text("6. Implementation Challenges", font_size=30, color=YELLOW)
        section6.next_to(title, DOWN, buff=1)
        self.play(Write(section6))
        self.wait(2)

        # Show challenges
        challenges = VGroup(
            Text("• Maintaining Consistency", font_size=20),
            Text("• Handling Network Latency", font_size=20),
            Text("• Managing State", font_size=20),
            Text("• Ensuring Security", font_size=20),
            Text("• Scaling the System", font_size=20)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.5)
        challenges.next_to(section6, DOWN, buff=0.7)
        
        for challenge in challenges:
            self.play(Write(challenge), run_time=0.7)
            self.wait(0.5)
        
        self.wait(3)

        # Conclusion with important key takeaways
        self.play(
            FadeOut(challenges),
            FadeOut(section6)
        )
        
        conclusion_title = Text("Key Takeaways", font_size=36, color=YELLOW)
        conclusion_title.next_to(title, DOWN, buff=1)
        
        conclusion_points = VGroup(
            Text("• Distributed systems trade-offs are central to design", font_size=24),
            Text("• Low latency & high reliability are crucial for trading", font_size=24),
            Text("• Fault tolerance must be built-in, not added later", font_size=24),
            Text("• Trade-ngin architecture balances these concerns", font_size=24)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.7)
        conclusion_points.next_to(conclusion_title, DOWN, buff=0.7)
        
        self.play(Write(conclusion_title))
        self.wait(1)
        
        for point in conclusion_points:
            self.play(Write(point), run_time=1)
            self.wait(1)
        
        self.wait(3)

        # Final conclusion
        final_message = Text("Understanding these concepts is crucial\nfor building robust trading systems",
                        font_size=30, color=BLUE)
        
        self.play(
            FadeOut(conclusion_points),
            FadeOut(conclusion_title),
            Write(final_message)
        )
        self.wait(5)

        # Elegant fade out for all elements
        self.play(
            *[FadeOut(mob) for mob in self.mobjects],
            run_time=2
        ) 