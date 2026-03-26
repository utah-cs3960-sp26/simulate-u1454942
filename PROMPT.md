# CS 3960 Homework 4 — Autonomous Build Prompt

You are working inside a student repository for a C++ / SDL3 physics simulator assignment.

## High-level goal

Build a **working, stable, demoable 2D physics simulator** in C++ using **SDL3** for rendering and **Box2D** for physics simulation.

This project must satisfy the homework requirements and must prioritize:
- **stability**
- **correctness**
- **clear structure**
- **successful demo behavior**

The simulator should display **~1000 circular balls** inside a **container made of fixed walls**, falling under gravity, colliding with each other and the walls, bouncing with configurable restitution, and then eventually settling down.

The biggest technical requirements are:
- balls must **not overlap badly**
- balls must **not phase through walls**
- balls must **not explode to infinite speed**
- the simulation must eventually **settle naturally**
- changing restitution should change **how bouncy / how long it takes to settle**, but should **not change the final packed shape much**

---

# IMPORTANT IMPLEMENTATION DECISION

## DO NOT build a homemade physics engine.

Use:
- **SDL3** for windowing, rendering, and input
- **Box2D** for physics simulation

This is required for stability.

### Forbidden approach
Do **not**:
- implement a custom impulse solver
- implement custom circle-circle collision resolution
- implement custom wall collision math
- add random hacks to "fix jitter"
- tie physics directly to frame rate
- repeatedly rewrite architecture from scratch

If the repository currently contains a homemade physics solver, you may **replace or remove it** if doing so improves stability and helps satisfy the assignment.

---

# Development priorities

Always prioritize in this order:

1. **A stable simulation**
2. **A clean, understandable codebase**
3. **Meeting the assignment requirements**
4. **Adding Week 11 extensions**
5. **Optional polish**

Do not sacrifice stability for cleverness.

---

# Required features

## Core simulator (Week 10 / base project)
Build a simulator that:

- opens an SDL3 window
- creates a physics world with downward gravity
- creates static wall geometry forming a visible container
- spawns about **1000 dynamic circular balls**
- simulates them falling, colliding, bouncing, and settling
- renders them in real time
- allows **restitution** to be configured
- uses a **fixed timestep**
- avoids runaway instability

## Week 11 features
After the base simulator works, add:

### 1. CSV input for initial ball scene
Support loading balls from a CSV file with at least:

- `id`
- `x`
- `y`
- `r`
- `g`
- `b`

Optional fields allowed:
- radius
- vx
- vy
- etc.

### 2. CSV output for final settled positions
Save a CSV after simulation settles (or on command) containing at least:

- `id`
- final `x`
- final `y`
- `r`
- `g`
- `b`

### 3. Color assignment tool
Create a tool (C++ or Python is acceptable if it fits the repo cleanly) that:

- reads an initial scene CSV
- reads a final positions CSV
- reads an image file
- assigns each ball’s color based on the image color at that ball’s **final resting location**
- writes a new recolored initial scene CSV

This should make it possible to:
- start with a chaotic-looking ball arrangement
- run the simulation
- have the balls settle into a recognizable image

---

# Technical constraints

## Physics architecture
Use **Box2D** correctly:

- one physics world
- dynamic bodies for balls
- static bodies for walls
- circle shapes for balls
- edge/segment/chain shapes for walls
- use friction / restitution / damping as needed
- allow bodies to sleep
- prefer stable defaults over flashy motion

## Time stepping
Use a **fixed timestep**, such as:

- `1.0 / 60.0`

Never tie simulation correctness directly to render FPS.

Use an accumulator loop if needed.

## Stability requirements
Take explicit steps to avoid:
- tunneling through walls
- overlap explosion
- infinite jitter
- energy gain over time

Use Box2D features/settings rather than ad-hoc hacks wherever possible.

## Rendering
Use SDL3 only for:
- creating a window
- drawing circles / walls
- keyboard input
- displaying the simulation

Do not put physics logic inside rendering code.

---

# Project organization requirements

Refactor into clear modules if needed. Prefer simple structure like:

- `main.cpp`
- `simulator.*`
- `physics.*`
- `render.*`
- `scene_io.*`
- `csv.*`
- `color_tool.*` or separate helper script
- `RESULTS.md`

You may choose a different structure if it is cleaner, but keep it understandable.

---

# Required workflow

## Before coding
At the start of each session:

1. Inspect the current repository state.
2. Determine what already works and what is broken.
3. Identify the **single highest-priority next milestone**.
4. Make a short plan.
5. Then implement only that milestone.

Do not wander.

---

# Milestone order (IMPORTANT)

Work in this exact order unless the repo already has a working equivalent.

## Milestone 1 — Build / dependency setup
Ensure the project can:
- compile cleanly
- link SDL3
- link Box2D
- run a windowed app

If build is broken, fix build first.

## Milestone 2 — Minimal stable simulation
Create the smallest working simulation:
- one floor
- a few balls
- gravity
- visible rendering

Verify it runs stably.

## Milestone 3 — Full container + many balls
Expand to:
- container walls
- ~1000 balls
- stable stepping
- acceptable performance

Verify balls stay in bounds and settle.

## Milestone 4 — Configurable restitution
Add an easy way to change restitution:
- constant in code
- config file
- command-line option
Any clean option is acceptable.

Verify lower restitution settles faster.

## Milestone 5 — Week 10 documentation
Create/update `RESULTS.md` with:
- current simulator status
- what works
- what bugs remain
- how the architecture works
- prompt / Amp behavior observations if relevant

## Milestone 6 — CSV input/output
Add:
- load initial balls from CSV
- save final ball positions to CSV

## Milestone 7 — Image recoloring tool
Add the tool that maps final positions to image colors and rewrites initial colors.

## Milestone 8 — Final polish
Only after all required features work:
- improve visuals
- improve controls
- improve README / comments
- improve demo experience

---

# REQUIRED TESTING BEHAVIOR

After every meaningful code change:

1. Build the project.
2. Run it.
3. Observe whether behavior improved or regressed.
4. If it regressed, fix before moving on.

Do not stack many speculative changes without testing.

When debugging, prefer:
- small controlled test scenes
- fewer balls first
- then scale back up to ~1000

Do not debug a 1000-ball failure first if a 10-ball scene already fails.

---

# REQUIRED DEBUGGING STRATEGY

If the simulation is unstable:

1. Reduce the scene size.
2. Verify fixed timestep.
3. Verify wall/body setup.
4. Verify body properties.
5. Verify sleep/damping/restitution configuration.
6. Only then scale complexity back up.

Do not repeatedly “guess and patch.”

---

# REQUIRED GIT BEHAVIOR

Commit only after a real milestone or clear improvement.

Good commit examples:
- `Set up SDL3 + Box2D build`
- `Add stable fixed-step Box2D simulation`
- `Add container walls and 1000-ball scene`
- `Implement CSV scene loading`
- `Add final position export`
- `Add image recoloring tool`
- `Update RESULTS.md with Week 10 notes`

Do not spam meaningless commits.

---

# REQUIRED DOCUMENTATION BEHAVIOR

Maintain or create `RESULTS.md`.

It should contain clearly labeled sections such as:

- `## Week 10`
- `## Week 11`

Include:
- current simulator behavior
- bugs found / fixed
- notable design decisions
- how CSV input/output works
- how the image-color tool works
- what Amp got wrong and how it was corrected

If the assignment wording references the wrong week numbers, still label your work clearly and sensibly.

---

# REQUIRED AMP BEHAVIOR CONSTRAINTS

You must avoid these common failure modes:

## Do not:
- rewrite large parts of working code without a strong reason
- delete working features casually
- “clean up” code while core requirements are still failing
- invent unnecessary abstractions
- add advanced UI before the simulator works
- over-engineer

## Do:
- prefer the smallest working implementation
- preserve working behavior
- make focused changes
- leave concise comments where useful
- keep the project demoable

---

# If the current repo is messy or broken

If the current codebase is unstable or deeply flawed, you may:

- refactor it
- replace the simulation core
- remove broken custom physics logic
- simplify the architecture

But do so **incrementally** and keep the project buildable.

Do not perform a giant destructive rewrite without first establishing a stable replacement plan.

---

# Session output expectations

At the end of each session, do all of the following:

1. Summarize what you changed.
2. State what now works.
3. State what still does not work.
4. State the next best milestone.
5. If appropriate, update `RESULTS.md`.
6. Commit if a milestone was completed.

---

# First task for this session

Start by doing the following in order:

1. Inspect the current repository.
2. Determine whether the project already uses a homemade physics solver.
3. If so, plan a migration to Box2D-based physics.
4. Get the project building and running with SDL3 + Box2D.
5. Establish a minimal stable simulation before attempting the full 1000-ball scene.

Work carefully and prioritize a stable, demoable result over cleverness.