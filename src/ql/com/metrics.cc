/** \file
 * Utility functions for extracting statistics/metrics from programs and
 * kernels.
 */

#include "ql/com/metrics.h"

namespace ql {
namespace com {
namespace metrics {

/**
 * Classical operation counting metric.
 */
void ClassicalOperationCount::process_gate(const ir::compat::GateRef &gate) {
    if (gate->type() == ir::compat::GateType::CLASSICAL) {
        value++;
    }
}

/**
 * Quantum gate counting metric.
 */
void QuantumGateCount::process_gate(const ir::compat::GateRef &gate) {
    switch (gate->type()) {
        case ir::compat::GateType::CLASSICAL:
        case ir::compat::GateType::WAIT:
            break;
        default:
            value++;
            break;
    }
}

/**
 * Multi-qubit gate counting metric.
 */
void MultiQubitGateCount::process_gate(const ir::compat::GateRef &gate) {
    switch (gate->type()) {
        case ir::compat::GateType::CLASSICAL:
        case ir::compat::GateType::WAIT:
            break;
        default:
            if (gate->operands.size() > 1) {
                value++;
            }
            break;
    }
}

/**
 * Qubit usage counting metric.
 */
void QubitUsageCount::process_gate(const ir::compat::GateRef &gate) {
    switch (gate->type()) {
        case ir::compat::GateType::CLASSICAL:
        case ir::compat::GateType::WAIT:
            break;
        default:
            for (auto v : gate->operands) {
                value[v]++;
            }
            break;
    }
}

/**
 * Qubit cycle usage counting metric.
 */
void QubitUsedCycleCount::process_kernel(const ir::compat::KernelRef &kernel) {
    for (auto &gp : kernel->gates) {
        switch (gp->type()) {
            case ir::compat::GateType::CLASSICAL:
            case ir::compat::GateType::WAIT:
                break;
            default:
                for (auto v : gp->operands) {
                    value[v] += utils::div_ceil(
                        gp->duration,
                        kernel->platform->cycle_time
                    );
                }
                break;
        }
    }
}

/**
 * Kernel duration metric.
 */
void Latency::process_kernel(const ir::compat::KernelRef &kernel) {
    if (!kernel->gates.empty() && kernel->gates.back()->cycle != ir::compat::MAX_CYCLE) {
        // NOTE JvS: this used to just check the last gate in the circuit, but
        // that isn't sufficient. Worst case the first gate could be setting the
        // kernel duration, even if issued in the first cycle, due to it just
        // having a very long duration itself.
        for (const auto &gate : kernel->gates) {
            value = utils::max(
                value,
                gate->cycle + utils::div_ceil(
                    gate->duration,
                    kernel->platform->cycle_time
                )
            );
        }
    }
}

} // namespace metrics
} // namespace com
} // namespace ql
