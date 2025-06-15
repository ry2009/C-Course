#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <memory>

/**
 * @brief Represents a single latency measurement between two system points
 */
struct LatencyMeasurement {
    std::string sourceComponent;    // Component where measurement started
    std::string targetComponent;    // Component where measurement ended
    int64_t latencyNs;             // Latency in nanoseconds
    int64_t timestamp;             // When the measurement was taken
    std::string correlationId;     // ID to correlate related measurements
};

/**
 * @brief Represents detailed statistics about latency
 */
struct LatencyStatistics {
    int64_t minLatencyNs;
    int64_t maxLatencyNs;
    int64_t avgLatencyNs;
    int64_t p50LatencyNs;   // 50th percentile (median)
    int64_t p95LatencyNs;   // 95th percentile
    int64_t p99LatencyNs;   // 99th percentile
    double standardDeviation;
    int64_t sampleSize;
};

/**
 * @brief Class for monitoring and analyzing system latency
 * 
 * Students should implement this class to fulfill the requirements of Part 3:
 * - Measure end-to-end latency for order processing
 * - Identify bottlenecks in the system
 * - Provide statistics on system performance
 */
class LatencyMonitor {
public:
    LatencyMonitor();
    virtual ~LatencyMonitor() = default;
    
    /**
     * @brief Initialize the latency monitor with configuration parameters
     * @param params Configuration parameters
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const std::unordered_map<std::string, std::string>& params) = 0;
    
    /**
     * @brief Start a new latency measurement
     * 
     * This method should be called at the beginning of a process you want to measure.
     * It creates a measurement point that can be completed later with endMeasurement().
     * 
     * @param component Name of the component where measurement starts
     * @param correlationId ID that can be used to correlate this measurement with others
     * @return Unique identifier for this measurement
     */
    virtual std::string startMeasurement(
        const std::string& component,
        const std::string& correlationId = "") = 0;
    
    /**
     * @brief End a latency measurement
     * 
     * This method should be called at the end of a process being measured.
     * It completes the measurement started with startMeasurement().
     * 
     * @param measurementId ID returned by startMeasurement
     * @param component Name of the component where measurement ends
     * @return Measured latency in nanoseconds
     */
    virtual int64_t endMeasurement(
        const std::string& measurementId,
        const std::string& component) = 0;
    
    /**
     * @brief Record a point-to-point latency measurement
     * 
     * This method allows recording a complete latency measurement
     * without explicitly calling start and end.
     * 
     * @param source Name of the source component
     * @param target Name of the target component
     * @param latencyNs Latency in nanoseconds
     * @param correlationId Optional correlation ID
     */
    virtual void recordLatency(
        const std::string& source,
        const std::string& target,
        int64_t latencyNs,
        const std::string& correlationId = "") = 0;
    
    /**
     * @brief Get latency statistics for a specific path
     * 
     * Calculate detailed statistics for latency between two components.
     * 
     * @param source Name of the source component
     * @param target Name of the target component
     * @param timeWindowMs Time window to consider in milliseconds (0 = all time)
     * @return Latency statistics
     */
    virtual LatencyStatistics getLatencyStats(
        const std::string& source,
        const std::string& target,
        int64_t timeWindowMs = 0) = 0;
    
    /**
     * @brief Identify bottlenecks in the system
     * 
     * Analyze all collected latency measurements to find the slowest components
     * or paths in the system.
     * 
     * @param timeWindowMs Time window to consider in milliseconds (0 = all time)
     * @param maxResults Maximum number of bottlenecks to return
     * @return Map of component/path to its latency statistics, ordered by severity
     */
    virtual std::vector<std::pair<std::string, LatencyStatistics>> identifyBottlenecks(
        int64_t timeWindowMs = 0,
        int maxResults = 5) = 0;
    
    /**
     * @brief Generate a performance report
     * 
     * Create a comprehensive report of system latency performance.
     * 
     * @param timeWindowMs Time window to consider in milliseconds (0 = all time)
     * @return Map of performance metrics and their values
     */
    virtual std::unordered_map<std::string, std::string> generatePerformanceReport(
        int64_t timeWindowMs = 0) = 0;
    
protected:
    // Storage for latency measurements
    std::vector<LatencyMeasurement> measurements_;
    
    // Active measurements (started but not completed)
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> activeMeasurements_;
    
    // Configuration
    int64_t retentionPeriodMs_ = 3600000; // 1 hour default
    size_t maxMeasurements_ = 100000;     // Maximum measurements to keep
    bool enablePeriodicReporting_ = false;
    
    // Statistics
    std::atomic<int64_t> totalMeasurements_{0};
};

/**
 * @brief Factory function to create a latency monitor
 * @param monitorType Type of latency monitor to create
 * @return Shared pointer to the created latency monitor
 */
std::shared_ptr<LatencyMonitor> createLatencyMonitor(const std::string& monitorType); 