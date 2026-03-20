# Progress

## Current Architecture
- Euler integration with fixed 1/60s timestep
- Combined impulse + position solver (5 iterations, grid rebuilt each)
- Support-aware sleep system (sleeping balls skip gravity/integration)
- Wake-on-impact for sleeping balls
- Spatial grid collision detection (cell_size = 2 * max_radius)
- Baumgarte-style position correction with wall clamp per iteration

## Parameters
- GRAVITY=980, DAMPING=0.96, SLEEP_SPEED=15
- RESTIT_CUTOFF=5.0, RESTITUTION=0.3 (adjustable)
- 1000 balls, radius 5-8px

## Measurements
- 5 solver iters + grid rebuild: MAX_PEN ~7.0, 500+ sleeping after 10s
- More iterations reduce overlap but increase frame time
- 100 iterations on stale grid: MAX_PEN ~1.0 but slow
- 150 iterations on stale grid: MAX_PEN ~0.44 (passes <0.5 threshold) but very slow
