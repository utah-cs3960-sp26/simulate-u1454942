# Results — 2D Physics Simulator

---

# Week 8 — Physics Simulator

## Current Simulator Status

The simulator runs ~1000 balls in an 800×600 window with SDL3. Balls fall under gravity, bounce off walls and each other, and settle into a pile. The simulation runs at real-time frame rates.

### What Works
- **Gravity & falling**: Balls fall naturally at a realistic speed (GRAVITY=500, DAMPING=0.995)
- **Wall collisions**: Balls bounce off all 4 walls with restitution, never pass through
- **Ball-ball collisions**: Impulse-based resolution with position correction prevents most overlap
- **Spatial grid**: Broad-phase collision detection keeps 1000 balls performant
- **Energy dissipation**: Damping + restitution cutoff ensures balls lose energy over time
- **Sleep system**: Balls with low velocity that are supported (by floor or ball below) get zeroed
- **Forced freeze**: After 3 seconds, all physics stops and the pile is completely still

### Remaining Issues
- **Overlap in dense piles**: Balls in the bottom of the pile still visibly overlap. The position correction (1 pass per solver iteration on a stale grid) cannot fully resolve chain overlaps in a dense pile of 1000 balls. Fully eliminating overlap requires 100+ solver iterations with grid rebuilds, which destroys real-time performance.
- **The 3-second freeze is a hack**: Balls don't naturally settle to zero jitter — the simulation just stops physics entirely after 3 seconds to force stillness.

### Architecture
- Euler integration with variable timestep (capped at 20ms)
- 8 solver iterations per frame (impulse + position correction combined)
- Spatial grid with cell_size = 4× max radius, 4-neighbor pair checking
- Restitution cutoff: e=0 when relative velocity < 20 px/s (kills micro-bounces)
- Support-aware sleep: uses spatial grid to find balls below

---

## History of Prompt Changes

The user iterated through **~20 different versions** of PROMPT.md, each attempting to guide Amp toward fixing specific physics issues. Here is the chronological history:

### Prompt 1: Original Assignment
The GitHub repo's original PROMPT.md — a straightforward assignment spec: build a 2D physics sim with 1000 balls, gravity, collisions, SDL3 rendering, stable settling.

### Prompt 2: Verlet Integration Architecture
Completely rewrote the prompt to specify a **Verlet integration** approach with constraint-based separation. Included detailed pseudocode for every step: Verlet integration, hard wall constraints, ball-ball constraint solver (10 iterations), damping on implicit velocity, wall restitution, sleep.
- **Why**: The original impulse-based code had jitter and overlap issues. The user tried prescribing an entirely different physics architecture.
- **Amp behavior**: Amp rewrote the entire main.cpp to use Verlet as requested.

### Prompt 3: Back to Euler Integration
Reversed course — the prompt now said "Use standard Euler integration (NOT Verlet). The Verlet approach has been attempted and failed." Specified impulse solver (8 iterations, velocity-only) + separate hard separation pass (position-only).
- **Why**: The Verlet approach apparently didn't work well visually.
- **Amp behavior**: Amp rewrote main.cpp again, discarding all Verlet code.

### Prompt 4: "Do Not Rewrite — Fix Grid Cell Size + Separation Iterations"
First "targeted fix" prompt. Said the code was "mostly correct" and listed exactly 2 bugs: grid cell size too large (4× → 2×) and separation pass running only once (→ 5 times).
- **Why**: User realized full rewrites were wasteful and started prescribing specific fixes.
- **Amp behavior**: Applied the two changes correctly.

### Prompt 5: Three More Targeted Fixes
Cell size += 1px buffer, separation iterations 5→20, partial correction factor 0.5.
- **Amp behavior**: Applied all three correctly.

### Prompt 6: Detailed 5-Problem Fix List
Listed 5 problems: position correction mixed with impulse, weak damping, floor-only sleep, grid cell size, non-configurable restitution. Included updated parameter block and final loop structure pseudocode.
- **Why**: User tried to address multiple issues at once with a comprehensive fix list.
- **Amp behavior**: Applied parameter changes but didn't restructure the solver (it was already structured correctly from previous iterations).

### Prompt 7: "All 8 Grid Neighbors Required"
Specified checking all 8 grid neighbors instead of 4, plus moving grid build outside separation loop.
- **Why**: User suspected diagonal neighbor pairs were being missed.
- **Amp behavior**: Changed to 8 neighbors, then next prompt changed back to 4 because 8 neighbors caused doubled impulses.

### Prompt 8: Fix Doubled Impulses + Move Grid Build
Identified that 8 neighbors was wrong (doubled pairs), reverted to 4. Moved grid build outside separation loop for performance.
- **Amp behavior**: Applied both fixes.

### Prompts 9–12: Generic "Fix Overlap and Jitter" Prompts
These prompts became increasingly generic — "fix overlap", "use Baumgarte correction", "add penetration slop", "increase stability." They stopped prescribing exact code changes and instead described desired outcomes.
- **Amp behavior**: This is where Amp started iterating endlessly. It would make small parameter changes, report "builds and runs," but couldn't verify whether overlap/jitter was actually fixed since it can't see the simulation visually.

### Prompts 13–15: Forced Measurement Prompts
User required Amp to **measure MAX_PENETRATION numerically** and print it. Required multiple runs. Set a pass threshold of MAX_PENETRATION < 0.5.
- **Why**: User realized Amp was claiming success without verification. Numeric measurement forced accountability.
- **Amp behavior**: Amp added O(n²) measurement code, discovered penetration was 7+ pixels, then began the long iteration of increasing solver iterations (10→20→40→80→100→120→150) until MAX_PEN dropped below 0.5. This took ~150 solver iterations per frame which destroyed performance.

### Prompts 16–17: Performance Constraints Added
User added "must complete in under 2 seconds" and "must run at real-time." Tried to force Amp to find an efficient solution.
- **Why**: Amp's approach of brute-forcing 150 iterations per frame made the simulation run at ~1 FPS.
- **Amp behavior**: Amp oscillated between adding more iterations (improving overlap but killing performance) and reducing iterations (improving performance but increasing overlap). It never found a satisfactory middle ground.

### Prompt 18: "Start From Original, Small Fixes Only"
User told Amp to **restore the original code from git** and make only small, controlled changes: damping, restitution behavior, sleep logic.
- **Why**: After 15+ iterations of architectural rewrites and parameter thrashing, the code was a mess. Starting fresh from the original baseline was the reset needed.
- **Amp behavior**: This was the most successful approach. Amp restored the original, then applied 4 small targeted fixes (damping 0.999→0.995, restitution cutoff, support-aware sleep, wall velocity zeroing). The simulation worked reasonably well.

### Prompts 19–20: "Balls Fall Too Slowly"
User noticed damping was too aggressive (0.97) making balls float. Prompt specified damping must be 0.98–0.999.
- **Amp behavior**: Changed damping to 0.995, raised sleep velocity to 15, raised restitution cutoff to 20.

### Prompts 21–23: "Sleep After N Seconds"
Simple one-line prompts: "have balls sleep after 15 seconds," then "after 5 seconds," then "after 3 seconds."
- **Why**: User gave up on natural settling and opted for a forced freeze timer as a pragmatic solution.
- **Amp behavior**: Applied these trivially — added a timer that disables all physics after the specified time.

---

## Amp Misbehaviors Observed

### 1. "It Builds and Runs" ≠ "It Works"
The most persistent issue: Amp would make changes, verify compilation, run the simulator for 3–5 seconds via a timed kill, and declare success. Since Amp cannot see the rendered output, it had no way to verify whether balls were overlapping, jittering, or falling at the right speed. Every claim of "builds and runs successfully" was meaningless for physics correctness.

### 2. Eager Full Rewrites
When given a detailed architecture description, Amp would rewrite the entire main.cpp from scratch rather than making incremental changes. This happened with the Verlet prompt (Prompt 2) and the Euler prompt (Prompt 3). Each rewrite discarded working code and introduced new bugs.

### 3. Endless Parameter Thrashing
When given vague prompts like "fix overlap," Amp would enter a loop of changing parameters slightly (e.g., correction factor 0.5→0.7→0.8→1.0, iterations 10→15→20→30), testing each by running the simulator for a few seconds, and reporting "stable." Without visual or numeric verification, these changes were blind guesses.

### 4. Brute-Force Over Elegance
When forced to measure penetration numerically, Amp's solution was simply to increase solver iterations until the number went down: 10→20→40→80→100→120→150 iterations. It never considered architectural improvements (sub-stepping, better convergence schemes, warm-starting) that could achieve the same result with fewer iterations.

### 5. Contradictory Changes Across Iterations
Amp would make a change in one iteration (e.g., "grid cell size should be 2×max_radius") and then reverse it in the next when a different prompt said the opposite (e.g., "cell size should be 4×max_radius"). It treated each prompt as an independent instruction without maintaining a coherent design rationale.

### 6. Cannot Verify Visual Properties
Amp fundamentally cannot verify: "balls jitter," "balls fall too slowly," "pile looks stable," "no visible overlap." It can only verify: "code compiles," "process doesn't crash," "numeric measurement passes threshold." This meant all visual quality assessment had to come from the user.

### 7. Ignoring Performance Until Told
Amp would happily add 150 solver iterations per frame without considering that this made the simulation run at 1 FPS. Performance was only addressed when the prompt explicitly mentioned it, and even then Amp would oscillate between "more iterations" and "fewer iterations" without finding a balance.

---

## What Was Added to the Prompt and Why

| Addition | Why |
|----------|-----|
| Detailed pseudocode for physics steps | Amp kept inventing its own physics that didn't work |
| "DO NOT rewrite from scratch" | Amp would throw away working code |
| "DO NOT use Baumgarte/Verlet/etc" | Previous architecture attempts failed |
| Specific parameter values | Amp would pick arbitrary values |
| Numeric measurement requirements | Amp couldn't verify visual correctness |
| Performance constraints | Amp ignored performance |
| "Make ONE change at a time" | Amp would make 5 changes and not know which helped |
| "Revert if worse" | Amp would keep bad changes |
| Forced sleep timer | User gave up on natural settling |

## What Was Removed from the Prompt

| Removal | Why |
|---------|-----|
| Verlet architecture | It didn't work, switched to Euler |
| Euler architecture details | Switched back to "start from original" |
| "Eliminate ALL overlap" requirement | Impossible at real-time performance; relaxed to "small overlap acceptable" |
| High iteration count requirements | Destroyed performance |
| Complex Baumgarte formulas | Over-engineering; simple correction worked better |
| Detailed debugging guides | Amp didn't follow them anyway |

---

## Lessons Learned

1. **Start simple, iterate small**: The most effective prompt was "restore original, make 4 small fixes." Full architectural rewrites always introduced more problems than they solved.

2. **Amp needs numeric verification**: Without measurable pass/fail criteria, Amp will claim success based on compilation alone. Always require printed metrics.

3. **Visual properties require human judgment**: Amp cannot assess "looks right." The user must verify visual quality and feed back specific problems.

4. **Performance and correctness trade off**: With 1000 balls and Euler integration, achieving zero overlap at 60fps is extremely difficult. A pragmatic compromise (some overlap + forced freeze) was the realistic outcome.

5. **Prompt specificity has diminishing returns**: Very detailed prompts (pseudocode for every step) caused Amp to implement exactly what was specified, even if it was wrong. Simpler prompts with clear success criteria ("balls must settle, no jitter") worked better when combined with "start from working code."

---

# Week 9 — CSV Scene Files & Image Reveal

## What Was Built

Week 9 extended the simulator with a CSV-based scene pipeline and an image-reveal tool. The idea: balls bounce around looking random, then settle into a recognizable image.

### New Features

1. **CSV Scene Loading (`--load <file>`)**: The simulator reads an initial scene from a CSV file with columns `x,y,r,g,b[,radius]`. Each row defines one ball's starting position, color, and optional radius.

2. **CSV Scene Saving (`--save <file>`)**: After the physics settle (controlled by `--settle-time <ms>`, default 4000ms), the simulator writes the final ball positions and colors to a CSV file in the same format.

3. **`generate_scene` tool**: Standalone utility that creates an initial CSV with N randomly-positioned balls. Usage: `./generate_scene [num_balls] [output.csv]`.

4. **`assign_colors` tool**: The key tool for the image reveal effect. It takes the initial CSV (starting positions), the final CSV (resting positions), and a target image. For each ball, it samples the image color at the ball's *final* resting position and writes a new CSV with that color applied to the ball's *starting* position.

### The Image Reveal Workflow

```
1. ./generate_scene 1000 initial.csv
2. ./simulator --load initial.csv --save final.csv
3. ./assign_colors initial.csv final.csv target_image.png colored.csv
4. ./simulator --load colored.csv
```

Step 4 is the payoff: balls start scattered with seemingly random colors, bounce around chaotically, and when they settle, the colors align to form the target image. This works because `assign_colors` pre-computed which color each ball needs based on where it will end up.

### CSV Format

```csv
x,y,r,g,b,radius
333.724,222.323,183,218,115,4.46932
370.935,501.019,166,170,197,3.36443
```

- `x,y`: position in simulation coordinates (800×600 window)
- `r,g,b`: color (0–255)
- `radius`: optional; randomly generated if omitted

### Architecture Decisions

- **stb_image.h** (header-only) was used for image loading since no image library was already in the project. It supports PNG, JPG, BMP, PPM, etc.
- **Deterministic ball ordering**: The CSV row index is the ball's identity. `initial.csv` row N and `final.csv` row N are the same ball, which is what makes the color mapping work. The simulator preserves ball order throughout the simulation.
- **No changes to physics**: The Week 8 physics code was left untouched. All Week 9 changes are in the I/O layer (CSV parsing, command-line args) and the external tools.

## Prompt and Amp Behavior

### The Prompt

The Week 9 prompt was a single natural-language request: add CSV scene loading/saving, create a color-assignment tool that samples an image at final ball positions, and produce the image-reveal effect.

### What Amp Did Well

- **Understood the full pipeline**: From one prompt, Amp correctly identified all four components needed (CSV load, CSV save, scene generator, color assigner) and their relationships.
- **Preserved existing code**: The physics engine was left completely untouched. All new code was additive — new functions, new files, new CLI flags.
- **Got the color-mapping logic right on the first try**: The key insight (sample image at *final* position, write color to *starting* position) was implemented correctly without iteration.
- **Header-only library choice**: Using stb_image.h was a pragmatic choice — no build system changes beyond adding a new target, no external dependency installation required.

### What Amp Could Have Done Better

- **No visual verification possible**: As with Week 8, Amp cannot see the simulation output. The workflow was verified by checking that CSVs have correct row counts and that colors are sampled from valid image coordinates, but whether the final image actually "looks right" requires human judgment.
- **Hardcoded simulation dimensions**: The `assign_colors` tool hardcodes `SIM_W=800, SIM_H=600` to match the simulator. If the simulator's window size changes, the color assignment tool silently produces wrong results.

## What Changed in the Prompt

| Change | Why |
|--------|-----|
| Single combined request instead of step-by-step | The task was well-defined enough to describe in one go |
| No physics changes requested | The physics were "good enough" from Week 8 |
| Described desired end result (image reveal) | Gave Amp the high-level goal and let it design the pipeline |

## Lessons Learned (Week 9)

1. **Well-scoped feature requests work well**: Unlike Week 8's physics debugging (where Amp couldn't verify results), Week 9's task was mostly I/O plumbing and data transformation — things Amp can verify (files parse correctly, row counts match, colors are in range).

2. **Additive features are easier than fixing bugs**: Week 9 added new capabilities without modifying existing code. This is Amp's sweet spot — there's no risk of breaking working physics.

3. **Pipeline design from a single prompt**: When the end-to-end workflow is clearly described, Amp can correctly decompose it into separate tools and define their interfaces (CSV format, CLI arguments, file flow).

4. **Human verification still essential for the visual payoff**: The technical pipeline works — CSVs flow correctly through the tools. But whether the final animation is visually satisfying (good ball density, recognizable image, pleasing chaos-to-order transition) can only be judged by watching it.
