#!/usr/bin/env python3
import graphviz
import os

def create_threading_architecture_diagram():
    """
    Create a diagram showing common threading architecture patterns in
    quantitative finance systems.
    """
    # Create a new directed graph
    dot = graphviz.Digraph(comment='Trading System Threading Architecture', format='png')
    
    # Set graph attributes for better readability
    dot.attr(rankdir='TB', size='12,8', dpi='300', fontname='Arial', 
             ranksep='0.75', nodesep='0.5')
    
    # Define node styles
    dot.attr('node', shape='box', style='filled,rounded', fontname='Arial', 
             fontsize='12', width='2', height='0.8')
    
    # Create clusters for different components
    with dot.subgraph(name='cluster_market_data') as c:
        c.attr(label='Market Data Processing', style='filled', color='lightblue', fontsize='16')
        
        c.node('feed1', 'Exchange Feed 1\n(Thread 1)', fillcolor='skyblue')
        c.node('feed2', 'Exchange Feed 2\n(Thread 2)', fillcolor='skyblue')
        c.node('feed3', 'Exchange Feed 3\n(Thread 3)', fillcolor='skyblue')
        c.node('normalizer', 'Data Normalizer\n(Thread Pool)', fillcolor='skyblue3')
        
        c.edge('feed1', 'normalizer')
        c.edge('feed2', 'normalizer')
        c.edge('feed3', 'normalizer')
    
    with dot.subgraph(name='cluster_strategy') as c:
        c.attr(label='Strategy Execution', style='filled', color='lightgreen', fontsize='16')
        
        c.node('signal_gen', 'Signal Generation\n(Thread Pool)', fillcolor='palegreen')
        c.node('strategy1', 'Strategy 1\n(Thread)', fillcolor='palegreen3')
        c.node('strategy2', 'Strategy 2\n(Thread)', fillcolor='palegreen3')
        c.node('strategy3', 'Strategy 3\n(Thread)', fillcolor='palegreen3')
        
        c.edge('signal_gen', 'strategy1')
        c.edge('signal_gen', 'strategy2')
        c.edge('signal_gen', 'strategy3')
    
    with dot.subgraph(name='cluster_execution') as c:
        c.attr(label='Order Execution', style='filled', color='lightsalmon', fontsize='16')
        
        c.node('order_manager', 'Order Manager\n(Thread)', fillcolor='salmon')
        c.node('exec1', 'Execution 1\n(Thread)', fillcolor='salmon2')
        c.node('exec2', 'Execution 2\n(Thread)', fillcolor='salmon2')
        
        c.edge('order_manager', 'exec1')
        c.edge('order_manager', 'exec2')
    
    with dot.subgraph(name='cluster_risk') as c:
        c.attr(label='Risk Management', style='filled', color='lavender', fontsize='16')
        
        c.node('pre_trade', 'Pre-Trade Risk\n(Thread)', fillcolor='plum')
        c.node('post_trade', 'Post-Trade Risk\n(Thread)', fillcolor='plum')
        c.node('reporting', 'Risk Reporting\n(Thread)', fillcolor='plum3')
        
        c.edge('pre_trade', 'reporting')
        c.edge('post_trade', 'reporting')
    
    # Connect the components
    dot.edge('normalizer', 'signal_gen', color='blue', penwidth='2', label='Market Data Flow')
    dot.edge('strategy1', 'order_manager', color='green', penwidth='2', label='Order Flow')
    dot.edge('strategy2', 'order_manager', color='green', penwidth='2')
    dot.edge('strategy3', 'order_manager', color='green', penwidth='2')
    dot.edge('normalizer', 'pre_trade', color='purple', penwidth='1.5', style='dashed', label='Market Data Updates')
    dot.edge('exec1', 'post_trade', color='red', penwidth='1.5', style='dashed', label='Execution Updates')
    dot.edge('exec2', 'post_trade', color='red', penwidth='1.5', style='dashed')
    
    # Render the diagram
    output_file = 'threading_architecture'
    dot.render(output_file, cleanup=True)
    print(f"Threading architecture diagram saved as {output_file}.png")

def create_thread_synchronization_diagram():
    """
    Create a diagram illustrating different thread synchronization mechanisms.
    """
    # Create a new directed graph
    dot = graphviz.Digraph(comment='Thread Synchronization Mechanisms', format='png')
    
    # Set graph attributes
    dot.attr(rankdir='LR', size='14,10', dpi='300', fontname='Arial')
    
    # Define node styles
    dot.attr('node', shape='box', style='filled,rounded', fontname='Arial', 
             fontsize='12', width='2.2', height='1.0')
    
    # Create a subgraph for mutex
    with dot.subgraph(name='cluster_mutex') as c:
        c.attr(label='Mutex Synchronization', style='filled', color='lightblue', fontsize='16')
        
        c.node('thread1_m', 'Thread 1', fillcolor='skyblue')
        c.node('thread2_m', 'Thread 2', fillcolor='skyblue')
        c.node('mutex', 'Mutex', shape='circle', fillcolor='red')
        c.node('shared_m', 'Shared Resource', shape='cylinder', fillcolor='lightyellow')
        
        c.edge('thread1_m', 'mutex', label='lock()')
        c.edge('thread2_m', 'mutex', label='lock() (blocks)')
        c.edge('mutex', 'shared_m', label='access when locked')
    
    # Create a subgraph for reader-writer lock
    with dot.subgraph(name='cluster_rw') as c:
        c.attr(label='Reader-Writer Lock', style='filled', color='lightgreen', fontsize='16')
        
        c.node('reader1', 'Reader Thread 1', fillcolor='palegreen')
        c.node('reader2', 'Reader Thread 2', fillcolor='palegreen')
        c.node('writer', 'Writer Thread', fillcolor='palegreen3')
        c.node('rwlock', 'RW Lock', shape='circle', fillcolor='orange')
        c.node('shared_rw', 'Shared Resource', shape='cylinder', fillcolor='lightyellow')
        
        c.edge('reader1', 'rwlock', label='read_lock()')
        c.edge('reader2', 'rwlock', label='read_lock()')
        c.edge('writer', 'rwlock', label='write_lock() (blocks)')
        c.edge('rwlock', 'shared_rw', label='concurrent reads\nexclusive writes')
    
    # Create a subgraph for condition variables
    with dot.subgraph(name='cluster_cv') as c:
        c.attr(label='Condition Variables', style='filled', color='lavender', fontsize='16')
        
        c.node('producer', 'Producer Thread', fillcolor='plum')
        c.node('consumer', 'Consumer Thread', fillcolor='plum')
        c.node('condvar', 'Condition\nVariable', shape='circle', fillcolor='purple')
        c.node('mutex_cv', 'Mutex', shape='circle', fillcolor='red')
        c.node('queue', 'Message Queue', shape='cylinder', fillcolor='lightyellow')
        
        c.edge('producer', 'mutex_cv', label='lock()')
        c.edge('mutex_cv', 'queue', label='add data')
        c.edge('mutex_cv', 'condvar', label='notify()')
        c.edge('consumer', 'condvar', label='wait()')
        c.edge('condvar', 'queue', label='process data\nwhen signaled')
    
    # Create a subgraph for atomic operations
    with dot.subgraph(name='cluster_atomic') as c:
        c.attr(label='Atomic Operations', style='filled', color='lightsalmon', fontsize='16')
        
        c.node('thread1_a', 'Thread 1', fillcolor='salmon')
        c.node('thread2_a', 'Thread 2', fillcolor='salmon')
        c.node('counter', 'Atomic Counter\nstd::atomic<int>', shape='cylinder', fillcolor='lightyellow')
        
        c.edge('thread1_a', 'counter', label='fetch_add(1)\n(atomic operation)')
        c.edge('thread2_a', 'counter', label='fetch_add(1)\n(atomic operation)')
    
    # Render the diagram
    output_file = 'thread_synchronization'
    dot.render(output_file, cleanup=True)
    print(f"Thread synchronization diagram saved as {output_file}.png")

def create_thread_safety_issues_diagram():
    """
    Create a diagram illustrating common thread safety issues.
    """
    # Create a new directed graph
    dot = graphviz.Digraph(comment='Thread Safety Issues', format='png')
    
    # Set graph attributes
    dot.attr(rankdir='TB', size='12,10', dpi='300', fontname='Arial')
    
    # Define node styles
    dot.attr('node', shape='box', style='filled,rounded', fontname='Arial', 
             fontsize='12', width='2.5', height='1.2')
    
    # Race condition
    with dot.subgraph(name='cluster_race') as c:
        c.attr(label='Race Condition', style='filled', color='mistyrose', fontsize='16')
        
        c.node('thread1_r', 'Thread 1\nread counter = 5', fillcolor='salmon')
        c.node('thread2_r', 'Thread 2\nread counter = 5', fillcolor='salmon')
        c.node('thread1_w', 'Thread 1\nwrite counter = 6', fillcolor='salmon')
        c.node('thread2_w', 'Thread 2\nwrite counter = 6', fillcolor='salmon')
        c.node('result_r', 'Expected: counter = 7\nActual: counter = 6', shape='note', fillcolor='red')
        
        c.edge('thread1_r', 'thread2_r')
        c.edge('thread2_r', 'thread1_w')
        c.edge('thread1_w', 'thread2_w')
        c.edge('thread2_w', 'result_r')
    
    # Deadlock
    with dot.subgraph(name='cluster_deadlock') as c:
        c.attr(label='Deadlock', style='filled', color='lightblue', fontsize='16')
        
        c.node('thread1_d', 'Thread 1\nHolds Mutex A\nWaiting for Mutex B', fillcolor='skyblue')
        c.node('thread2_d', 'Thread 2\nHolds Mutex B\nWaiting for Mutex A', fillcolor='skyblue')
        c.node('deadlock', 'DEADLOCK!\nNeither thread can proceed', shape='note', fillcolor='red')
        
        c.edge('thread1_d', 'deadlock', dir='both', arrowhead='normal', arrowtail='normal', 
              style='bold', color='red')
        c.edge('thread2_d', 'deadlock', dir='both', arrowhead='normal', arrowtail='normal', 
              style='bold', color='red')
    
    # False sharing
    with dot.subgraph(name='cluster_false') as c:
        c.attr(label='False Sharing', style='filled', color='lavender', fontsize='16')
        
        c.node('thread1_f', 'Thread 1\nUpdates counter1', fillcolor='plum')
        c.node('thread2_f', 'Thread 2\nUpdates counter2', fillcolor='plum')
        c.node('cache_line', 'Cache Line (64 bytes)\ncontains both counters', shape='record', fillcolor='lightyellow')
        c.node('cpu_cache', 'CPU Cache\nInvalidation Storm', shape='note', fillcolor='orange')
        
        c.edge('thread1_f', 'cache_line')
        c.edge('thread2_f', 'cache_line')
        c.edge('cache_line', 'cpu_cache', style='bold', color='red', label='Performance\nProblem')
    
    # Data race
    with dot.subgraph(name='cluster_data_race') as c:
        c.attr(label='Data Race', style='filled', color='lightgreen', fontsize='16')
        
        c.node('thread1_dr', 'Thread 1\nReading position', fillcolor='palegreen')
        c.node('thread2_dr', 'Thread 2\nUpdating position', fillcolor='palegreen')
        c.node('memory', 'Shared Memory\nposition variable', shape='cylinder', fillcolor='lightyellow')
        c.node('corrupt', 'Memory Corruption\nTorn Read', shape='note', fillcolor='red')
        
        c.edge('thread1_dr', 'memory', label='read')
        c.edge('thread2_dr', 'memory', label='write')
        c.edge('memory', 'corrupt', style='bold', color='red')
    
    # Render the diagram
    output_file = 'thread_safety_issues'
    dot.render(output_file, cleanup=True)
    print(f"Thread safety issues diagram saved as {output_file}.png")

def create_performance_comparison_diagram():
    """
    Create a diagram comparing performance characteristics of different
    threading and synchronization approaches.
    """
    # Create a new directed graph
    dot = graphviz.Digraph(comment='Threading Performance Comparison', format='png')
    
    # Set graph attributes
    dot.attr(rankdir='TB', size='12,8', dpi='300', fontname='Arial')
    
    # Define node styles
    dot.attr('node', shape='box', style='filled,rounded', fontname='Arial', 
             fontsize='12', width='3.0', height='1.2')
    
    # Define tables as HTML-like labels
    mutex_table = '''<
    <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
      <TR>
        <TD COLSPAN="2" BGCOLOR="#6495ED"><B>Mutex-Based Synchronization</B></TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Throughput</TD>
        <TD>Medium</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Latency</TD>
        <TD>Medium-High</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Contention</TD>
        <TD>High under load</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Best Use</TD>
        <TD>Complex shared data structures</TD>
      </TR>
    </TABLE>
    >'''
    
    rw_lock_table = '''<
    <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
      <TR>
        <TD COLSPAN="2" BGCOLOR="#32CD32"><B>Reader-Writer Lock Synchronization</B></TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Throughput</TD>
        <TD>High for reads, Medium for writes</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Latency</TD>
        <TD>Low for reads, Medium for writes</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Contention</TD>
        <TD>Low for reads, Medium for writes</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Best Use</TD>
        <TD>Read-heavy workloads (e.g., order books)</TD>
      </TR>
    </TABLE>
    >'''
    
    lockfree_table = '''<
    <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
      <TR>
        <TD COLSPAN="2" BGCOLOR="#FF7F50"><B>Lock-Free Structures</B></TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Throughput</TD>
        <TD>Very High</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Latency</TD>
        <TD>Very Low</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Contention</TD>
        <TD>Minimal</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Best Use</TD>
        <TD>High-throughput queues and buffers</TD>
      </TR>
    </TABLE>
    >'''
    
    atomic_table = '''<
    <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
      <TR>
        <TD COLSPAN="2" BGCOLOR="#9370DB"><B>Atomic Operations</B></TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Throughput</TD>
        <TD>High</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Latency</TD>
        <TD>Low</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Contention</TD>
        <TD>Low</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Best Use</TD>
        <TD>Counters, flags, simple shared data</TD>
      </TR>
    </TABLE>
    >'''
    
    # Create nodes with HTML table labels
    dot.node('mutex', mutex_table, shape='plaintext')
    dot.node('rw_lock', rw_lock_table, shape='plaintext')
    dot.node('lockfree', lockfree_table, shape='plaintext')
    dot.node('atomic', atomic_table, shape='plaintext')
    
    # Create a performance label
    perf_label = '''<
    <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
      <TR>
        <TD COLSPAN="2" BGCOLOR="#FFD700"><B>Performance vs. Complexity Tradeoff</B></TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Higher Performance</TD>
        <TD>Lock-Free &gt; Atomic &gt; Reader-Writer &gt; Mutex</TD>
      </TR>
      <TR>
        <TD BGCOLOR="#D3D3D3">Higher Complexity</TD>
        <TD>Lock-Free &gt; Reader-Writer &gt; Atomic &gt; Mutex</TD>
      </TR>
    </TABLE>
    >'''
    
    # Add performance comparison node
    dot.node('performance', perf_label, shape='plaintext')
    
    # Layout the nodes
    dot.attr(rank='same')
    dot.edge('mutex', 'rw_lock', style='invis')
    dot.edge('rw_lock', 'lockfree', style='invis')
    dot.edge('lockfree', 'atomic', style='invis')
    
    # Add a constraint to keep the performance node at the bottom
    dot.edge('mutex', 'performance', style='invis', constraint='true')
    
    # Render the diagram
    output_file = 'threading_performance_comparison'
    dot.render(output_file, cleanup=True)
    print(f"Threading performance comparison diagram saved as {output_file}.png")

def main():
    """Create all diagrams for threading visualization."""
    # Ensure output directory exists
    os.makedirs('diagrams', exist_ok=True)
    
    # Change to the output directory
    os.chdir('diagrams')
    
    # Create all diagrams
    create_threading_architecture_diagram()
    create_thread_synchronization_diagram()
    create_thread_safety_issues_diagram()
    create_performance_comparison_diagram()
    
    print("\nAll diagrams created successfully in the 'diagrams' directory.")

if __name__ == '__main__':
    main() 