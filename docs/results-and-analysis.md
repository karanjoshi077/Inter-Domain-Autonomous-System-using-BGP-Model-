# Simulation Results and Analysis

## Baseline network
The baseline experiment establishes connectivity between autonomous-system routers and records transmitted and received packets, loss, and end-to-end delay using FlowMonitor.

## Bidirectional traffic
A request-response exchange is used to verify communication in both directions. Inspect the corresponding XML and execution log to confirm delivery and timing.

## Routing mechanism comparison
Two mechanism configurations are provided as separate C++ simulations. Their logs and XML outputs allow comparison of routing outcomes.

## Network event analysis
The network-event experiment captures measurements at multiple stages, including before, during, and after the simulated event. The delay plot and animation are supplied as complementary evidence.

## Routing evaluation
The final experiment includes an additional routing simulation, XML output, delay plot, and NetAnim recording.

## Reproducibility
Run each C++ simulation within a compatible ns-3.48 installation and compare generated outputs against the archived measurements. Configuration, random seeds, and platform details may affect repeatability.

## Limitations
These are simulation results, not measurements from a live inter-domain network. Interpret conclusions in light of the topology and code implementation.
