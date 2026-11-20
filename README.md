# Inter-Domain Routing and Network Resilience Simulation

A C++/ns-3 project for studying traffic delivery, routing configurations, and communication under network events across interconnected autonomous systems. The simulations use ns-3 global routing and do not implement BGP protocol message exchange.

## Highlights
- Baseline inter-domain network topology and FlowMonitor measurements
- Bidirectional client/server traffic
- Routing mechanism comparisons
- Network event and delay analysis
- NetAnim recordings, plots, execution logs, and XML metrics

## Repository structure
```text
simulations/           C++ network simulations
experiments/           Additional experiment variants
results/               Captured measurements, logs, plots and animations
docs/                  Project specification and technical analysis
setup_ns3.sh           ns-3 environment setup
bgp.cc                 Baseline simulation
```

## Getting started
1. Review [project specification](docs/project-specification.md).
2. Inspect `setup_ns3.sh` before running; it installs packages using sudo.
3. Set up ns-3.48 and copy a simulation into its `scratch/` directory.
4. From the ns-3 root, run `./ns3 run scratch/bgp` for the baseline or `./ns3 run scratch/<source-name>` for another simulation. Source filenames in `simulations/` are intended to be copied into `scratch/` before running.
5. Compare outputs with the archived evidence in `results/`.

## Simulation sources
| Focus | Source |
| --- | --- |
| Baseline | [bgp.cc](bgp.cc) |
| Bidirectional communication | [bidirectional-traffic.cc](simulations/bidirectional-traffic.cc) |
| Routing mechanisms | [routing-mechanisms.cc](simulations/routing-mechanisms.cc) |
| Network event analysis | [network-event-analysis.cc](simulations/network-event-analysis.cc) |
| Routing evaluation | [routing-evaluation.cc](simulations/routing-evaluation.cc) |

## Validation status

The archived results are included for inspection. The reorganized sources have not been compiled or executed as part of this repository cleanup; reproducing them requires a configured ns-3.48 environment. Some simulations write output files using their original internal filenames.

## Analysis
See [results and analysis](docs/results-and-analysis.md) for the experiment summary and limitations. The archived logs and XML measurements are available under `results/`.

## Technologies
C++, ns-3.48, FlowMonitor, NetAnim, XML, Bash.

**Scope:** This project studies inter-domain routing scenarios in simulation; it is not a production BGP implementation.
