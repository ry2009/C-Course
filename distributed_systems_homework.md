# Distributed Systems in Finance: Homework Assignment

## Overview
This homework assignment focuses on implementing a simplified trading system with distributed components. You will create a basic trading system that demonstrates key concepts in distributed systems and financial markets.

## Learning Objectives
- Implement core components of a trading system
- Understand basic distributed system concepts
- Integrate trading algorithms
- Learn about system reliability and performance

## Part 1: System Components (40 points)

Implement the following core components:

1. **Order Book Processor** (15 points):
   - Handle order creation, modification, and cancellation
   - Implement price-time priority matching
   - Track order book state and market metrics

2. **Risk Calculator** (15 points):
   - Validate orders against position limits
   - Calculate basic risk metrics
   - Implement simple circuit breakers

3. **Trading Strategy** (10 points):
   - Implement a basic market-making strategy
   - Calculate optimal order sizes
   - Monitor market conditions

## Part 2: System Integration (30 points)

1. **Component Communication** (15 points):
   - Implement message passing between components
   - Handle basic error cases
   - Log system events

2. **System Reliability** (15 points):
   - Implement basic health checks
   - Handle component failures
   - Maintain system state

## Part 3: Testing and Performance (30 points)

1. **Unit Tests** (10 points):
   - Test each component in isolation
   - Verify correct behavior
   - Handle edge cases

2. **Integration Tests** (10 points):
   - Test component interactions
   - Verify system behavior
   - Test error handling

3. **Performance Testing** (10 points):
   - Measure system latency
   - Test under load
   - Identify bottlenecks

## Deliverables

1. **Code Implementation**:
   - Well-documented C++ files
   - Build instructions
   - Test cases

2. **Documentation**:
   - System architecture overview
   - Component descriptions
   - Testing results

## Submission Guidelines

1. Submit all code as well-documented C++ files
2. Include build instructions
3. Provide a README explaining your implementation
4. Include test cases
5. Submit a brief report (max 5 pages) explaining your design decisions

## Grading Criteria

- Correctness of implementation (40%)
- Code quality and documentation (30%)
- Testing completeness (20%)
- Performance considerations (10%)

## Due Date
One week from distribution

---

### Recommended Reading

- "Building Trading Systems" by Kevin Davey
- "Designing Data-Intensive Applications" by Martin Kleppmann 