#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// --- Configuration ---
constexpr int WINDOW_W = 800;
constexpr int WINDOW_H = 600;
constexpr int NUM_BALLS = 1000;
constexpr float GRAVITY = 500.0f;
constexpr float RESTITUTION = 0.3f;
constexpr float BALL_RADIUS_MIN = 3.0f;
constexpr float BALL_RADIUS_MAX = 5.0f;
constexpr float MAX_VELOCITY = 1500.0f;
constexpr float WALL_THICKNESS = 5.0f;
constexpr int SOLVER_ITERATIONS = 8;
constexpr float LINEAR_DAMPING = 0.995f;
constexpr float SLEEP_VELOCITY = 15.0f;

struct Ball {
    float x, y;
    float vx, vy;
    float radius;
    float mass;
    Uint8 r, g, b;
};

static void clamp_velocity(Ball& ball) {
    float speed2 = ball.vx * ball.vx + ball.vy * ball.vy;
    if (speed2 > MAX_VELOCITY * MAX_VELOCITY) {
        float speed = sqrtf(speed2);
        ball.vx = ball.vx / speed * MAX_VELOCITY;
        ball.vy = ball.vy / speed * MAX_VELOCITY;
    }
}

static void resolve_wall_collisions(Ball& ball) {
    float left = WALL_THICKNESS + ball.radius;
    float right = WINDOW_W - WALL_THICKNESS - ball.radius;
    float top = WALL_THICKNESS + ball.radius;
    float bottom = WINDOW_H - WALL_THICKNESS - ball.radius;

    if (ball.x < left) {
        ball.x = left;
        if (ball.vx < 0) ball.vx *= -RESTITUTION;
    }
    if (ball.x > right) {
        ball.x = right;
        if (ball.vx > 0) ball.vx *= -RESTITUTION;
    }
    if (ball.y < top) {
        ball.y = top;
        if (ball.vy < 0) ball.vy *= -RESTITUTION;
    }
    if (ball.y > bottom) {
        ball.y = bottom;
        if (ball.vy > 0) ball.vy *= -RESTITUTION;
    }
    if (fabsf(ball.vx) < 2.0f) ball.vx = 0;
    if (fabsf(ball.vy) < 2.0f) ball.vy = 0;
}

static void resolve_ball_collision(Ball& a, Ball& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dist2 = dx * dx + dy * dy;
    float min_dist = a.radius + b.radius;

    if (dist2 >= min_dist * min_dist || dist2 < 1e-8f)
        return;

    float dist = sqrtf(dist2);
    float nx = dx / dist;
    float ny = dy / dist;

    // Position correction - push apart equally weighted by inverse mass
    float overlap = min_dist - dist;
    float total_mass = a.mass + b.mass;
    a.x -= nx * overlap * (b.mass / total_mass);
    a.y -= ny * overlap * (b.mass / total_mass);
    b.x += nx * overlap * (a.mass / total_mass);
    b.y += ny * overlap * (a.mass / total_mass);

    // Relative velocity along collision normal
    float dvx = a.vx - b.vx;
    float dvy = a.vy - b.vy;
    float rel_vel = dvx * nx + dvy * ny;

    // Only resolve if balls are moving toward each other
    if (rel_vel <= 0)
        return;

    float e = (rel_vel < 20.0f) ? 0.0f : RESTITUTION;
    float impulse = (1.0f + e) * rel_vel / total_mass;

    a.vx -= impulse * b.mass * nx;
    a.vy -= impulse * b.mass * ny;
    b.vx += impulse * a.mass * nx;
    b.vy += impulse * a.mass * ny;
}

// Simple spatial grid for broad-phase collision detection
struct Grid {
    int cols, rows;
    float cell_size;
    std::vector<std::vector<int>> cells;

    void init(float max_radius) {
        cell_size = max_radius * 2.0f * 2.0f; // 4x max radius
        cols = (int)ceilf(WINDOW_W / cell_size) + 1;
        rows = (int)ceilf(WINDOW_H / cell_size) + 1;
        cells.resize(cols * rows);
    }

    void clear() {
        for (auto& c : cells) c.clear();
    }

    void insert(int idx, float x, float y) {
        int cx = (int)(x / cell_size);
        int cy = (int)(y / cell_size);
        cx = std::clamp(cx, 0, cols - 1);
        cy = std::clamp(cy, 0, rows - 1);
        cells[cy * cols + cx].push_back(idx);
    }

    // Call func(i, j) for each potential pair
    template<typename F>
    void for_each_pair(F func) {
        for (int cy = 0; cy < rows; cy++) {
            for (int cx = 0; cx < cols; cx++) {
                auto& cell = cells[cy * cols + cx];
                // Pairs within the same cell
                for (size_t i = 0; i < cell.size(); i++)
                    for (size_t j = i + 1; j < cell.size(); j++)
                        func(cell[i], cell[j]);

                // Pairs with neighboring cells (right, below, below-right, below-left)
                int neighbors[4][2] = {{1,0},{0,1},{1,1},{-1,1}};
                for (auto& n : neighbors) {
                    int nx = cx + n[0], ny = cy + n[1];
                    if (nx < 0 || nx >= cols || ny < 0 || ny >= rows) continue;
                    auto& ncell = cells[ny * cols + nx];
                    for (int a : cell)
                        for (int b : ncell)
                            func(a, b);
                }
            }
        }
    }
};

static void draw_filled_circle(SDL_Renderer* renderer, float cx, float cy, float r) {
    // Midpoint circle rasterization
    int ir = (int)r;
    for (int dy = -ir; dy <= ir; dy++) {
        int dx = (int)sqrtf((float)(ir * ir - dy * dy));
        SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("2D Physics Simulator", WINDOW_W, WINDOW_H, 0);
    if (!window) {
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_DestroyWindow(window);
        return 1;
    }

    // Initialize balls
    srand((unsigned)time(nullptr));
    std::vector<Ball> balls(NUM_BALLS);
    for (auto& b : balls) {
        b.radius = BALL_RADIUS_MIN + ((float)rand() / RAND_MAX) * (BALL_RADIUS_MAX - BALL_RADIUS_MIN);
        b.mass = b.radius * b.radius; // mass proportional to area
        b.x = WALL_THICKNESS + b.radius + ((float)rand() / RAND_MAX) * (WINDOW_W - 2 * WALL_THICKNESS - 2 * b.radius);
        b.y = WALL_THICKNESS + b.radius + ((float)rand() / RAND_MAX) * (WINDOW_H - 2 * WALL_THICKNESS - 2 * b.radius);
        b.vx = ((float)rand() / RAND_MAX - 0.5f) * 200.0f;
        b.vy = ((float)rand() / RAND_MAX - 0.5f) * 200.0f;
        b.r = 100 + rand() % 156;
        b.g = 100 + rand() % 156;
        b.b = 100 + rand() % 156;
    }

    Grid grid;
    grid.init(BALL_RADIUS_MAX);

    bool running = true;
    SDL_Event event;
    Uint64 last_time = SDL_GetTicks();
    Uint64 start_time = last_time;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        Uint64 now = SDL_GetTicks();
        float dt = (float)(now - last_time) / 1000.0f;
        last_time = now;
        if (dt > 0.02f) dt = 0.02f; // Cap timestep

        // --- Physics Update ---
        bool frozen = ((now - start_time) >= 4000);

        if (!frozen) {
        // Apply gravity and integrate velocity
        for (auto& b : balls) {
            b.vy += GRAVITY * dt;
            b.vx *= LINEAR_DAMPING;
            b.vy *= LINEAR_DAMPING;
            clamp_velocity(b);
        }

        // Integrate position
        for (auto& b : balls) {
            b.x += b.vx * dt;
            b.y += b.vy * dt;
        }

        // Resolve collisions with multiple iterations for stability
        for (int iter = 0; iter < SOLVER_ITERATIONS; iter++) {
            // Wall collisions
            for (auto& b : balls) {
                resolve_wall_collisions(b);
            }

            // Ball-ball collisions via spatial grid
            grid.clear();
            for (int i = 0; i < (int)balls.size(); i++) {
                grid.insert(i, balls[i].x, balls[i].y);
            }
            grid.for_each_pair([&](int i, int j) {
                resolve_ball_collision(balls[i], balls[j]);
            });
        }

        // Sleep slow balls that have support (floor or ball below)
        // Reuse the grid already built in the last solver iteration
        for (int i = 0; i < (int)balls.size(); i++) {
            Ball& b = balls[i];
            float speed2 = b.vx * b.vx + b.vy * b.vy;
            if (speed2 >= SLEEP_VELOCITY * SLEEP_VELOCITY) continue;

            float bottom = WINDOW_H - WALL_THICKNESS - b.radius;
            bool has_support = (b.y >= bottom - 1.0f);

            if (!has_support) {
                int cx = (int)(b.x / grid.cell_size);
                int cy = (int)(b.y / grid.cell_size);
                cx = std::clamp(cx, 0, grid.cols - 1);
                cy = std::clamp(cy, 0, grid.rows - 1);
                for (int dy = -1; dy <= 1 && !has_support; dy++) {
                    for (int dx = -1; dx <= 1 && !has_support; dx++) {
                        int nx = cx + dx, ny = cy + dy;
                        if (nx < 0 || nx >= grid.cols || ny < 0 || ny >= grid.rows) continue;
                        auto& cell = grid.cells[ny * grid.cols + nx];
                        for (int oidx : cell) {
                            if (oidx == i) continue;
                            Ball& other = balls[oidx];
                            if (other.y <= b.y) continue;
                            float ddx = b.x - other.x;
                            float ddy = b.y - other.y;
                            float d2 = ddx * ddx + ddy * ddy;
                            float thresh = b.radius + other.radius + 2.0f;
                            if (d2 < thresh * thresh) { has_support = true; break; }
                        }
                    }
                }
            }

            if (has_support) {
                b.vx = 0;
                b.vy = 0;
            }
        }
        } // end if (!frozen)

        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);

        // Draw walls
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        SDL_FRect walls[4] = {
            {0, 0, (float)WINDOW_W, WALL_THICKNESS},                                      // top
            {0, WINDOW_H - WALL_THICKNESS, (float)WINDOW_W, WALL_THICKNESS},              // bottom
            {0, 0, WALL_THICKNESS, (float)WINDOW_H},                                      // left
            {WINDOW_W - WALL_THICKNESS, 0, WALL_THICKNESS, (float)WINDOW_H}               // right
        };
        for (auto& w : walls) {
            SDL_RenderFillRect(renderer, &w);
        }

        // Draw balls
        for (auto& b : balls) {
            SDL_SetRenderDrawColor(renderer, b.r, b.g, b.b, 255);
            draw_filled_circle(renderer, b.x, b.y, b.radius);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
