#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

// --- Configuration ---
constexpr int WINDOW_W = 800;
constexpr int WINDOW_H = 600;
constexpr int NUM_BALLS = 1000;
constexpr float RESTITUTION = 0.3f;
constexpr float BALL_RADIUS_MIN = 3.0f;
constexpr float BALL_RADIUS_MAX = 5.0f;
constexpr float WALL_THICKNESS = 5.0f;

// Box2D uses meters; we define a pixels-per-meter scale
constexpr float PPM = 50.0f;

// Fixed timestep for Box2D
constexpr float FIXED_DT = 1.0f / 60.0f;
constexpr int SUB_STEPS = 4;

struct Ball {
    float radius;   // in pixels
    Uint8 r, g, b;
    b2BodyId bodyId;
};

static bool load_scene_csv(const char* path, std::vector<Ball>& balls,
                           b2WorldId worldId, float restitution) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    balls.clear();
    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;

        float x = 0, y = 0;
        Uint8 cr = 200, cg = 200, cb = 200;
        float radius = BALL_RADIUS_MIN + ((float)rand() / RAND_MAX) * (BALL_RADIUS_MAX - BALL_RADIUS_MIN);

        if (!std::getline(ss, token, ',')) continue;
        x = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        y = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        cr = (Uint8)std::stoi(token);
        if (!std::getline(ss, token, ',')) continue;
        cg = (Uint8)std::stoi(token);
        if (!std::getline(ss, token, ',')) continue;
        cb = (Uint8)std::stoi(token);
        if (std::getline(ss, token, ',') && !token.empty()) {
            radius = std::stof(token);
        }

        // Create Box2D body
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {x / PPM, y / PPM};
        bodyDef.linearDamping = 0.5f;
        b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.3f;
        shapeDef.material.restitution = restitution;
        b2Circle circle = {{0.0f, 0.0f}, radius / PPM};
        b2CreateCircleShape(bodyId, &shapeDef, &circle);

        Ball ball;
        ball.radius = radius;
        ball.r = cr;
        ball.g = cg;
        ball.b = cb;
        ball.bodyId = bodyId;
        balls.push_back(ball);
    }
    return !balls.empty();
}

static void save_scene_csv(const char* path, const std::vector<Ball>& balls) {
    std::ofstream file(path);
    if (!file.is_open()) {
        SDL_Log("Failed to open %s for writing", path);
        return;
    }
    file << "x,y,r,g,b,radius\n";
    for (auto& b : balls) {
        b2Vec2 pos = b2Body_GetPosition(b.bodyId);
        float px = pos.x * PPM;
        float py = pos.y * PPM;
        file << px << "," << py << ","
             << (int)b.r << "," << (int)b.g << "," << (int)b.b << ","
             << b.radius << "\n";
    }
    SDL_Log("Saved %zu balls to %s", balls.size(), path);
}

static void draw_filled_circle(SDL_Renderer* renderer, float cx, float cy, float r) {
    int ir = (int)r;
    for (int dy = -ir; dy <= ir; dy++) {
        int dx = (int)sqrtf((float)(ir * ir - dy * dy));
        SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

static void create_walls(b2WorldId worldId) {
    b2BodyDef wallBodyDef = b2DefaultBodyDef();
    wallBodyDef.type = b2_staticBody;
    b2BodyId wallBody = b2CreateBody(worldId, &wallBodyDef);

    b2ShapeDef wallShapeDef = b2DefaultShapeDef();
    wallShapeDef.material.friction = 0.3f;
    wallShapeDef.material.restitution = 0.2f;

    float t = WALL_THICKNESS / PPM;
    float w = (float)WINDOW_W / PPM;
    float h = (float)WINDOW_H / PPM;

    // Walls at inner edges of the visible wall thickness
    b2Segment bottom = {{t, h - t}, {w - t, h - t}};
    b2CreateSegmentShape(wallBody, &wallShapeDef, &bottom);

    b2Segment top = {{t, t}, {w - t, t}};
    b2CreateSegmentShape(wallBody, &wallShapeDef, &top);

    b2Segment left = {{t, t}, {t, h - t}};
    b2CreateSegmentShape(wallBody, &wallShapeDef, &left);

    b2Segment right = {{w - t, t}, {w - t, h - t}};
    b2CreateSegmentShape(wallBody, &wallShapeDef, &right);
}

static void print_usage(const char* prog) {
    SDL_Log("Usage: %s [--load <scene.csv>] [--save <output.csv>] [--settle-time <ms>] [--restitution <0-1>]", prog);
    SDL_Log("  --load <file>         Load initial scene from CSV");
    SDL_Log("  --save <file>         Save final positions to CSV on quit");
    SDL_Log("  --settle-time <ms>    Time in ms before saving (default: 8000)");
    SDL_Log("  --restitution <val>   Bounce coefficient 0-1 (default: 0.3)");
}

int main(int argc, char* argv[]) {
    const char* load_path = nullptr;
    const char* save_path = nullptr;
    Uint64 settle_time_ms = 8000;
    float restitution = RESTITUTION;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--load") == 0 && i + 1 < argc) {
            load_path = argv[++i];
        } else if (strcmp(argv[i], "--save") == 0 && i + 1 < argc) {
            save_path = argv[++i];
        } else if (strcmp(argv[i], "--settle-time") == 0 && i + 1 < argc) {
            settle_time_ms = (Uint64)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--restitution") == 0 && i + 1 < argc) {
            restitution = (float)atof(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("2D Physics Simulator (Box2D)", WINDOW_W, WINDOW_H, 0);
    if (!window) {
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_DestroyWindow(window);
        return 1;
    }

    // Create Box2D world with gravity
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, 10.0f}; // downward in screen coords (y-down)
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Create container walls
    create_walls(worldId);

    // Initialize balls
    srand((unsigned)time(nullptr));
    std::vector<Ball> balls;

    if (load_path) {
        if (!load_scene_csv(load_path, balls, worldId, restitution)) {
            SDL_Log("Failed to load scene from %s, using random balls", load_path);
            load_path = nullptr;
        } else {
            SDL_Log("Loaded %zu balls from %s", balls.size(), load_path);
        }
    }

    if (!load_path) {
        balls.reserve(NUM_BALLS);
        for (int i = 0; i < NUM_BALLS; i++) {
            float radius = BALL_RADIUS_MIN + ((float)rand() / RAND_MAX) * (BALL_RADIUS_MAX - BALL_RADIUS_MIN);
            float x = WALL_THICKNESS + radius + ((float)rand() / RAND_MAX) * (WINDOW_W - 2 * WALL_THICKNESS - 2 * radius);
            float y = WALL_THICKNESS + radius + ((float)rand() / RAND_MAX) * (WINDOW_H - 2 * WALL_THICKNESS - 2 * radius);

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_dynamicBody;
            bodyDef.position = {x / PPM, y / PPM};
            bodyDef.linearDamping = 0.5f;
            b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.density = 1.0f;
            shapeDef.material.friction = 0.3f;
            shapeDef.material.restitution = restitution;
            b2Circle circle = {{0.0f, 0.0f}, radius / PPM};
            b2CreateCircleShape(bodyId, &shapeDef, &circle);

            Ball ball;
            ball.radius = radius;
            ball.r = (Uint8)(100 + rand() % 156);
            ball.g = (Uint8)(100 + rand() % 156);
            ball.b = (Uint8)(100 + rand() % 156);
            ball.bodyId = bodyId;
            balls.push_back(ball);
        }
    }

    bool running = true;
    SDL_Event event;
    Uint64 last_time = SDL_GetTicks();
    Uint64 start_time = last_time;
    bool saved = false;
    float accumulator = 0.0f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        Uint64 now = SDL_GetTicks();
        float frame_dt = (float)(now - last_time) / 1000.0f;
        last_time = now;
        if (frame_dt > 0.1f) frame_dt = 0.1f; // cap

        bool frozen = ((now - start_time) >= settle_time_ms);

        if (frozen && save_path && !saved) {
            save_scene_csv(save_path, balls);
            saved = true;
        }

        if (!frozen) {
            accumulator += frame_dt;
            while (accumulator >= FIXED_DT) {
                b2World_Step(worldId, FIXED_DT, SUB_STEPS);
                accumulator -= FIXED_DT;
            }
        }

        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);

        // Draw walls
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        SDL_FRect walls[4] = {
            {0, 0, (float)WINDOW_W, WALL_THICKNESS},
            {0, WINDOW_H - WALL_THICKNESS, (float)WINDOW_W, WALL_THICKNESS},
            {0, 0, WALL_THICKNESS, (float)WINDOW_H},
            {WINDOW_W - WALL_THICKNESS, 0, WALL_THICKNESS, (float)WINDOW_H}
        };
        for (auto& w : walls) {
            SDL_RenderFillRect(renderer, &w);
        }

        // Draw balls
        for (auto& ball : balls) {
            b2Vec2 pos = b2Body_GetPosition(ball.bodyId);
            float px = pos.x * PPM;
            float py = pos.y * PPM;
            SDL_SetRenderDrawColor(renderer, ball.r, ball.g, ball.b, 255);
            draw_filled_circle(renderer, px, py, ball.radius);
        }

        SDL_RenderPresent(renderer);
    }

    // Save on exit if not already saved
    if (save_path && !saved) {
        save_scene_csv(save_path, balls);
    }

    b2DestroyWorld(worldId);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
