# 2D Physics Simulator Project

You are building a 2D physics simulator in C++ using SDL3. The goal is to simulate around 1000 circular balls inside a container made of walls. The balls should be affected by gravity, collide with each other, and collide with the walls.

## Core Requirements

- Balls are circles with position, velocity, radius, and mass
- Gravity continuously affects all balls
- Balls collide with other balls
- Balls collide with walls (walls are fixed and immovable)
- Collisions are non-elastic with a configurable restitution value

## Physics Requirements

- Implement stable collision detection and resolution
- Balls must not overlap after collisions
- Balls must not pass through walls
- Use position correction to resolve overlaps
- Prevent instability such as jittering, vibration, or exploding velocities
- Clamp velocities if needed to maintain stability

Restitution should be configurable. Lower restitution should cause the system to settle faster, but the final resting configuration should remain the same regardless of restitution.

## Performance Requirements

- The simulation should handle around 1000 balls
- It should run smoothly and be visually stable
- Avoid unnecessary recomputation

## Rendering

- Use SDL3 to render the simulation
- Display balls as circles
- Display walls clearly as boundaries
- The simulation should start with an initial scene where balls bounce around and eventually settle

## Development Instructions

- Use the existing `main.cpp` and extend it (do not rewrite everything)
- Organize code into logical components (physics, collisions, rendering)
- Work in small steps and test frequently
- Do not continue if something is broken; fix it first
- Commit after meaningful progress

## Debugging Guidance

If balls overlap:
- Fix collision detection and add position correction

If balls pass through walls:
- Improve collision handling or reduce timestep

If balls jitter or explode:
- Add damping
- Clamp velocities
- Ensure collision resolution is stable

## Project Tracking

Maintain the following files:
- `README.md` for overall description
- `progress.md` to track what you are working on
- Log bugs, failed approaches, and fixes

Always update progress.md after completing a task.

## Goal

The final result should be a stable simulation where many balls bounce, interact, and eventually settle naturally inside a container without overlapping or instability.