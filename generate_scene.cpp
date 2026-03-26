// generate_scene: Creates an initial scene CSV with randomly positioned balls.
// This is a helper so you don't need to run the simulator just to get an
// initial CSV file.
//
// Usage: generate_scene [num_balls] [output.csv]

#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <ctime>

int main(int argc, char* argv[]) {
    int num_balls = 1000;
    const char* output = "initial.csv";

    if (argc >= 2) num_balls = atoi(argv[1]);
    if (argc >= 3) output = argv[2];

    if (num_balls <= 0) {
        fprintf(stderr, "Usage: %s [num_balls] [output.csv]\n", argv[0]);
        return 1;
    }

    constexpr int W = 800;
    constexpr int H = 600;
    constexpr float WALL = 5.0f;
    constexpr float R_MIN = 3.0f;
    constexpr float R_MAX = 5.0f;

    srand((unsigned)time(nullptr));

    std::ofstream out(output);
    out << "x,y,r,g,b,radius\n";

    for (int i = 0; i < num_balls; i++) {
        float radius = R_MIN + ((float)rand() / RAND_MAX) * (R_MAX - R_MIN);
        float x = WALL + radius + ((float)rand() / RAND_MAX) * (W - 2 * WALL - 2 * radius);
        float y = WALL + radius + ((float)rand() / RAND_MAX) * (H - 2 * WALL - 2 * radius);
        int r = 100 + rand() % 156;
        int g = 100 + rand() % 156;
        int b = 100 + rand() % 156;

        out << x << "," << y << "," << r << "," << g << "," << b << "," << radius << "\n";
    }

    printf("Generated %d balls -> %s\n", num_balls, output);
    return 0;
}
