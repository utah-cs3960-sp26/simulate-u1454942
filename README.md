# 2D Physics Simulator

A 2D physics simulator built in C++ using SDL3. Simulates ~1000 circular balls bouncing inside a walled container with gravity, impulse-based collisions, and configurable restitution.

## Features

- **Euler integration** with fixed 1/60s timestep
- **Impulse-based ball-ball collisions** — velocity only, no position changes
- **Hard separation pass** — position only, runs once after solver
- **Wall collisions** — positional clamp with velocity reflection
- **Damping** — 0.97× per frame
- **Sleep system** — support-aware (floor or ball below)
- **Spatial grid** — broad-phase collision detection for 1000 balls

## Building

```bash
mkdir build && cd build
cmake ..
make
./simulator
```

## Controls

- `[` — decrease restitution by 0.05
- `]` — increase restitution by 0.05
- Restitution displayed on screen

## Dependencies

- SDL3
- CMake 3.16+
- C++17 compiler

## Parameters

| Constant | Default | Description |
|---|---|---|
| `NUM_BALLS` | 1000 | Number of balls |
| `GRAVITY` | 980 | Gravity (px/s²) |
| `RESTITUTION` | 0.4 | Bounciness (runtime adjustable) |
| `SOLVER_ITERS` | 8 | Impulse solver iterations per frame |
| `DAMPING` | 0.97 | Velocity damping per frame |
| `SLEEP_SPEED` | 3.0 | Speed threshold for sleep (px/s) |
| `MAX_SPEED` | 2000 | Velocity clamp |
| `BALL_RADIUS` | 5–8px | Random radius per ball |
