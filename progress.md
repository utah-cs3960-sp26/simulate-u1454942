# Progress

## Completed

### Initial Setup
- SDL3 window and renderer scaffolding (pre-existing)
- Installed SDL3 and CMake via Homebrew
- Verified base project compiles and runs

### Core Physics Implementation
- Ball struct with position, velocity, radius, mass, and color
- Gravity integration with fixed timestep cap (0.02s max)
- Linear velocity damping (0.999 per frame)
- Velocity clamping (max 1500 px/s) to prevent instability

### Collision Detection & Resolution
- Ball-wall collisions with position correction and restitution
- Ball-ball collisions with impulse-based resolution and position correction weighted by inverse mass
- Spatial grid broad-phase for O(n) average-case collision detection
- Multi-iteration solver (8 passes) for stable stacking

### Rendering
- Filled circle drawing via midpoint rasterization
- Randomized ball colors for visual clarity
- Gray wall boundaries
- Dark background

### Stability
- Sleep system: balls at rest on the floor with near-zero velocity are frozen
- Overlap resolution via position correction in collision response
- Multiple solver iterations prevent tunneling and jitter

## Known Issues
- None currently observed

## Failed Approaches
- None so far
