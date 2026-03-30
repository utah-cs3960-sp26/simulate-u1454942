# Progress

## Current Architecture
- **Box2D v3.1.1** for physics (replaced homemade Euler/impulse solver)
- SDL3 for rendering and windowing
- Fixed timestep (1/60s) with accumulator loop
- 4 Box2D sub-steps per step for stability
- PPM (pixels-per-meter) = 50 for coordinate conversion
- Segment shapes for walls, circle shapes for balls

## Parameters
- Gravity: 10 m/s² downward (Box2D meters)
- Linear damping: 0.5
- Ball density: 1.0, friction: 0.3
- Restitution: 0.3 (configurable via --restitution)
- 1000 balls, radius 3-5px
- Settle time: 8000ms (configurable via --settle-time)

## Status
- Milestones 1-4 complete (build, minimal sim, full container, configurable restitution)
- CSV load/save working (Milestone 6 complete from previous work)
- assign_colors tool working (Milestone 7 complete from previous work)
- Balls fall, collide, bounce, and settle naturally via Box2D
- No overlap, no wall phasing, no explosion — Box2D handles all constraints
