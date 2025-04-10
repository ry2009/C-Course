from manim import *
import numpy as np

class TradeNginVisualization(Scene):
    def construct(self):
        # Set up the section title
        title = Text("Trade-Ngin Distributed System Architecture", font_size=40)
        title.to_edge(UP, buff=0.5)
        
        self.play(Write(title))
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

        # Add distributed system elements
        # Create a network cloud
        network_cloud = Circle(radius=1.5, color=WHITE, fill_opacity=0.1)
        network_cloud.move_to(ORIGIN)
        network_text = Text("Distributed\nNetwork", font_size=16).move_to(network_cloud)
        
        # Create nodes
        node1 = Circle(radius=0.3, color=YELLOW, fill_opacity=0.5)
        node1.move_to(LEFT * 2 + UP * 1)
        node1_text = Text("Node 1", font_size=12).next_to(node1, DOWN)
        
        node2 = Circle(radius=0.3, color=YELLOW, fill_opacity=0.5)
        node2.move_to(RIGHT * 2 + UP * 1)
        node2_text = Text("Node 2", font_size=12).next_to(node2, DOWN)
        
        node3 = Circle(radius=0.3, color=YELLOW, fill_opacity=0.5)
        node3.move_to(LEFT * 2 + DOWN * 1)
        node3_text = Text("Node 3", font_size=12).next_to(node3, UP)
        
        node4 = Circle(radius=0.3, color=YELLOW, fill_opacity=0.5)
        node4.move_to(RIGHT * 2 + DOWN * 1)
        node4_text = Text("Node 4", font_size=12).next_to(node4, UP)
        
        # Show network and nodes
        self.play(
            Create(network_cloud), Write(network_text),
            Create(node1), Write(node1_text),
            Create(node2), Write(node2_text),
            Create(node3), Write(node3_text),
            Create(node4), Write(node4_text)
        )
        self.wait(1)
        
        # Add network connections
        connections = VGroup(
            Line(node1.get_center(), node2.get_center(), color=WHITE),
            Line(node1.get_center(), node3.get_center(), color=WHITE),
            Line(node2.get_center(), node4.get_center(), color=WHITE),
            Line(node3.get_center(), node4.get_center(), color=WHITE)
        )
        
        self.play(Create(connections))
        self.wait(1)
        
        # Add latency measurements
        latency_text = Text("Latency: 50-100μs", font_size=16, color=YELLOW)
        latency_text.next_to(network_cloud, DOWN, buff=0.5)
        
        self.play(Write(latency_text))
        self.wait(1)
        
        # Add fault tolerance elements
        redundancy_text = Text("Redundancy & Failover", font_size=16, color=RED)
        redundancy_text.next_to(latency_text, DOWN, buff=0.3)
        
        self.play(Write(redundancy_text))
        self.wait(1)
        
        # Add time synchronization
        sync_text = Text("Time Sync: NTP/PTP", font_size=16, color=GREEN)
        sync_text.next_to(redundancy_text, DOWN, buff=0.3)
        
        self.play(Write(sync_text))
        self.wait(1)
        
        # Add serialization info
        serialization_text = Text("Optimized Serialization", font_size=16, color=BLUE)
        serialization_text.next_to(sync_text, DOWN, buff=0.3)
        
        self.play(Write(serialization_text))
        self.wait(2)
        
        # Clear everything
        self.play(
            FadeOut(market_data), FadeOut(market_data_text),
            FadeOut(strategy), FadeOut(strategy_text),
            FadeOut(risk), FadeOut(risk_text),
            FadeOut(order), FadeOut(order_text),
            FadeOut(md_to_strat), FadeOut(strat_to_risk),
            FadeOut(strat_to_order), FadeOut(risk_to_order),
            FadeOut(network_cloud), FadeOut(network_text),
            FadeOut(node1), FadeOut(node1_text),
            FadeOut(node2), FadeOut(node2_text),
            FadeOut(node3), FadeOut(node3_text),
            FadeOut(node4), FadeOut(node4_text),
            FadeOut(connections),
            FadeOut(latency_text),
            FadeOut(redundancy_text),
            FadeOut(sync_text),
            FadeOut(serialization_text),
            FadeOut(title)
        )
        
        # Show performance metrics
        metrics_title = Text("Trade-Ngin Performance Metrics", font_size=40)
        metrics_title.to_edge(UP, buff=0.5)
        
        self.play(Write(metrics_title))
        self.wait(1)
        
        # Create performance table
        table = Table(
            [["Component", "Latency", "Throughput"],
             ["Market Data", "3-5μs", "1M+ msg/s"],
             ["Strategy", "10-20μs", "100K+ sig/s"],
             ["Order Mgmt", "5-8μs", "50K+ ord/s"],
             ["Risk Check", "2-4μs", "200K+ chk/s"]],
            include_outer_lines=True
        )
        table.scale(0.8)
        table.next_to(metrics_title, DOWN, buff=1)
        
        self.play(Create(table))
        self.wait(2)
        
        # Add availability metrics
        availability_text = Text("System Availability: 99.999%", font_size=24, color=GREEN)
        availability_text.next_to(table, DOWN, buff=1)
        
        self.play(Write(availability_text))
        self.wait(2)
        
        # Clear final scene
        self.play(
            FadeOut(metrics_title),
            FadeOut(table),
            FadeOut(availability_text)
        ) 