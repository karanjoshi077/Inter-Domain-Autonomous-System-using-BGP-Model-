# Inter-Domain Routing Simulation — Project Specification

## Objective
Explore inter-domain connectivity and routing behavior using a reproducible ns-3 simulation environment. Evaluate packet delivery, latency, routing configuration, and behavior under network events.

## Technical scope
1. Establish a baseline multi-router topology and collect FlowMonitor metrics.
2. Exchange bidirectional application traffic and verify end-to-end delivery.
3. Compare two routing configurations and inspect their observed communication behavior.
4. Simulate network events and compare measurements before, during, and after the event.
5. Visualize traffic behavior with NetAnim and plot delay over time.

## Implementation and evidence
- C++ ns-3 simulations in `simulations/`
- Execution logs and FlowMonitor XML in `results/`
- Delay plots and NetAnim recordings in `results/`
- Setup script in `setup_ns3.sh`

## Evaluation criteria
Correct simulation execution, repeatable configuration, measured packet delivery and delay, and clear discussion of limitations.

## Important implementation note
The code models inter-domain routing scenarios; it should not be described as implementing a production BGP protocol stack unless the source explicitly supports that claim.
