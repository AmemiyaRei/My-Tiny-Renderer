#include <iostream>
#include <algorithm>
#include "tgaimage.h"
#include <vector>
#include "geometry.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width = 200;
const int height = 200;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < abs(y0 - y1)) {
        steep = true;
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; ++x) {
        if (steep) image.set(y, x, color);
        else image.set(x, y, color);
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

bool y_comp(const Vec2i &v1, const Vec2i &v2) {
    return v1.y < v2.y;
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
//    line(t0.x, t0.y, t1.x, t1.y, image, color);
//    line(t0.x, t0.y, t2.x, t2.y, image, color);
//    line(t2.x, t2.y, t1.x, t1.y, image, color);
    std::vector<Vec2i> vertices{t0, t1, t2};
    std::sort(vertices.begin(), vertices.end(), y_comp);
    for (int y = vertices[0].y; y < vertices[2].y; ++y) {
        float t1 = (float)(y - vertices[0].y) / (float)(vertices[2].y - vertices[0].y);
        int x1 = vertices[2].x * t1 + vertices[0].x * (1 - t1);
        float t2;
        int x2;
        if (y <= vertices[1].y) {
            t2 = (float)(y - vertices[0].y) / (float)(vertices[1].y - vertices[0].y);
            x2 = vertices[1].x * t2 + vertices[0].x * (1 - t2);
        } else {
            t2 = (float)(y - vertices[1].y) / (float)(vertices[2].y - vertices[1].y);
            x2 = vertices[2].x * t2 + vertices[1].x * (1 - t2);
        }
        if (x1 > x2) std::swap(x1, x2);
        for (int x = x1; x <= x2; ++x) {
            image.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    if(argc == 2) {
        model = new Model(argv[1]);
    } else {
        model = new Model("../obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
//    for (int i = 0; i < model->nfaces(); ++i) {
//        std::vector<int> face = model->face(i);
//        for (int j = 0; j < 3; ++j) {
//            Vec3f v0 = model->vert(face[j]);
//            Vec3f v1 = model->vert(face[(j + 1) % 3]);
//            // map x, y to image, -1.0 <= x, y <= 1.0
//            int x0 = (v0.x + 1.0) * width / 2.0;
//            int y0 = (v0.y + 1.0) * height / 2.0;
//            int x1 = (v1.x + 1.0) * width / 2.0;
//            int y1 = (v1.y + 1.0) * height / 2.0;
//            if (v0.z + v1.z > 0)
//                line(x0, y0, x1, y1, image, red);
//            else line(x0, y0, x1, y1, image, white);
//
//        }
//    }

    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);
    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
