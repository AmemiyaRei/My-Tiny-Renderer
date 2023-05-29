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

Vec3f barycentric(std::vector<Vec2i> &pts, Vec2i P) {
    Vec3f u = Vec3f(pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - P.x) ^
              Vec3f(pts[1].y - pts[0].y, pts[2].y - pts[0].y, pts[0].y - P.y);
    // use abs(u.z) < 1 to represent u.z == 0 to avoid minor error by float calculation
    if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(1.0 - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmin, bboxmax;
    bboxmin.x = std::max(0, (std::min(t0.x, std::min(t1.x, t2.x))));
    bboxmin.y = std::max(0, (std::min(t0.y, std::min(t1.y, t2.y))));
    bboxmax.x = std::max(clamp.x, (std::max(t0.x, std::max(t1.x, t2.x))));
    bboxmax.y = std::max(clamp.y, (std::max(t0.y, std::max(t1.y, t2.y))));
    Vec2i P;
    std::vector<Vec2i> pts{t0, t1, t2};
    for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
            Vec3f bc = barycentric(pts, P);
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            image.set(P.x, P.y, color);
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
