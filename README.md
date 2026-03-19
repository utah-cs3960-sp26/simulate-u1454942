# 2D Physics Simulator

A 2D physics simulator built in C++ using SDL3. Simulates ~1000 circular balls bouncing inside a walled container with gravity, ball-ball collisions, and ball-wall collisions.

## Features

- **Gravity** — continuous downward acceleration on all balls
- **Ball-ball collisions** — elastic collision resolution with configurable restitution
- **Ball-wall collisions** — balls bounce off fixed container walls
- **Position correction** — prevents overlapping after collisions
- **Spatial grid** — broad-phase collision detection for performance with 1000 balls
- **Stability** — velocity clamping, linear damping, multi-iteration solver, and sleep for settled balls

## Building

```bash
mkdir build && cd build
cmake ..
make
./simulator
```

## Dependencies

- SDL3
- CMake 3.16+
- C++17 compiler

## Configuration

Key constants at the top of `main.cpp`:

| Constant | Default | Description |
|---|---|---|
| `NUM_BALLS` | 1000 | Number of balls |
| `GRAVITY` | 500 | Gravity acceleration (pixels/s²) |
| `RESTITUTION` | 0.3 | Bounciness (0 = no bounce, 1 = perfectly elastic) |
| `SOLVER_ITERATIONS` | 8 | Collision solver passes per frame |
| `LINEAR_DAMPING` | 0.999 | Velocity damping per frame |
