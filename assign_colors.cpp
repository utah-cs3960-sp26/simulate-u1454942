// assign_colors: Takes an initial scene CSV and a target image, runs the
// simulator to find final ball positions, then samples the image at each
// ball's final resting position and writes a new CSV with those colors
// assigned to the balls' *starting* positions. When this colored CSV is fed
// back into the simulator, the balls start with the colors they'll display
// once they settle — forming the target image.
//
// Usage:
//   assign_colors <initial.csv> <final.csv> <image.png> <output.csv>
//
// Workflow:
//   1. Run the simulator:  ./simulator --load initial.csv --save final.csv
//   2. Assign colors:      ./assign_colors initial.csv final.csv image.png colored.csv
//   3. Watch the reveal:   ./simulator --load colored.csv

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

struct BallRecord {
    float x, y;
    int r, g, b;
    float radius;
};

static bool load_csv(const char* path, std::vector<BallRecord>& out) {
    std::ifstream file(path);
    if (!file.is_open()) {
        fprintf(stderr, "Cannot open %s\n", path);
        return false;
    }
    out.clear();
    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;
        BallRecord rec{};

        if (!std::getline(ss, token, ',')) continue;
        rec.x = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        rec.y = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        rec.r = std::stoi(token);
        if (!std::getline(ss, token, ',')) continue;
        rec.g = std::stoi(token);
        if (!std::getline(ss, token, ',')) continue;
        rec.b = std::stoi(token);
        if (std::getline(ss, token, ',') && !token.empty()) {
            rec.radius = std::stof(token);
        } else {
            rec.radius = 4.0f;
        }
        out.push_back(rec);
    }
    return !out.empty();
}

static void sample_image(const unsigned char* pixels, int img_w, int img_h, int channels,
                          float ball_x, float ball_y, int sim_w, int sim_h,
                          int& out_r, int& out_g, int& out_b) {
    // Map ball position in simulation space to image space
    int ix = (int)(ball_x / (float)sim_w * (float)img_w);
    int iy = (int)(ball_y / (float)sim_h * (float)img_h);
    ix = std::clamp(ix, 0, img_w - 1);
    iy = std::clamp(iy, 0, img_h - 1);

    int idx = (iy * img_w + ix) * channels;
    out_r = pixels[idx];
    out_g = (channels >= 2) ? pixels[idx + 1] : pixels[idx];
    out_b = (channels >= 3) ? pixels[idx + 2] : pixels[idx];
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <initial.csv> <final.csv> <image> <output.csv>\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Assigns colors from <image> to balls based on their final resting\n");
        fprintf(stderr, "positions in <final.csv>, writing the result with starting positions\n");
        fprintf(stderr, "from <initial.csv> into <output.csv>.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Workflow:\n");
        fprintf(stderr, "  1. Create initial scene:  (use simulator with --save, or manually)\n");
        fprintf(stderr, "  2. Run simulator:         ./simulator --load initial.csv --save final.csv\n");
        fprintf(stderr, "  3. Assign colors:         %s initial.csv final.csv image.png colored.csv\n", argv[0]);
        fprintf(stderr, "  4. Watch the reveal:      ./simulator --load colored.csv\n");
        return 1;
    }

    const char* initial_path = argv[1];
    const char* final_path = argv[2];
    const char* image_path = argv[3];
    const char* output_path = argv[4];

    // Load CSVs
    std::vector<BallRecord> initial, final_pos;
    if (!load_csv(initial_path, initial)) {
        fprintf(stderr, "Failed to load initial CSV: %s\n", initial_path);
        return 1;
    }
    if (!load_csv(final_path, final_pos)) {
        fprintf(stderr, "Failed to load final CSV: %s\n", final_path);
        return 1;
    }

    if (initial.size() != final_pos.size()) {
        fprintf(stderr, "Mismatch: initial has %zu balls, final has %zu balls\n",
                initial.size(), final_pos.size());
        return 1;
    }

    // Load image
    int img_w, img_h, channels;
    unsigned char* pixels = stbi_load(image_path, &img_w, &img_h, &channels, 0);
    if (!pixels) {
        fprintf(stderr, "Failed to load image: %s\n", image_path);
        return 1;
    }
    printf("Loaded image %s: %dx%d, %d channels\n", image_path, img_w, img_h, channels);

    // For each ball, sample the image at the FINAL position, then write the
    // INITIAL position with that sampled color
    constexpr int SIM_W = 800;
    constexpr int SIM_H = 600;

    std::ofstream out(output_path);
    if (!out.is_open()) {
        fprintf(stderr, "Cannot open output file: %s\n", output_path);
        stbi_image_free(pixels);
        return 1;
    }

    out << "x,y,r,g,b,radius\n";
    for (size_t i = 0; i < initial.size(); i++) {
        int cr, cg, cb;
        sample_image(pixels, img_w, img_h, channels,
                     final_pos[i].x, final_pos[i].y, SIM_W, SIM_H,
                     cr, cg, cb);

        out << initial[i].x << "," << initial[i].y << ","
            << cr << "," << cg << "," << cb << ","
            << initial[i].radius << "\n";
    }

    stbi_image_free(pixels);
    printf("Wrote %zu balls with image-sampled colors to %s\n", initial.size(), output_path);
    printf("\nTo see the reveal, run:\n  ./simulator --load %s\n", output_path);

    return 0;
}
