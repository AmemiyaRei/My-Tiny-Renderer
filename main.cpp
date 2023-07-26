#include <iostream>
#include <algorithm>
#include "tgaimage.h"
#include <vector>
#include "geometry.h"
#include "model.h"
#include <limits>

Model *model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

Vec3f light_dir = Vec3f(-1, -1, -4).normalize();
Vec3f Eye(1, 1, 3);
Vec3f Up(0, 1, 0);
Vec3f Center(0, 0, 0);

Vec3f m2v(Matrix m) {
    // convert 1 * 4 matrix to 3D vector
    return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

Matrix viewport(float x, float y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    Matrix res = Matrix::identity(4);
    for (int i = 0; i < 3; ++i) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
}

Vec3f barycentric(Vec3f* pts, Vec3f P) {
    Vec3f u = Vec3f(pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - P.x) ^
              Vec3f(pts[1].y - pts[0].y, pts[2].y - pts[0].y, pts[0].y - P.y);
    // use abs(u.z) < 1 to represent u.z == 0 to avoid minor error by float calculation
    if (std::abs(u.z) < 1e-2) return Vec3f(-1, 1, 1);
    return Vec3f(1.0 - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);
}

void triangle(Vec3f *pts, Vec3f *npts, float *zbuffer, TGAImage &image, Vec2f *tvs, TGAImage& texture) {
    if (pts[0].y == pts[1].y && pts[1].y == pts[2].y) return;
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; ++i) {
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
            // barycentric coordinates in x-y plane
            Vec3f bc = barycentric(pts, P);
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            P.z = bc * Vec3f(pts[0].z, pts[1].z, pts[2].z);
            float intensities[3];
            for (int i = 0; i < 3; ++i) {
                intensities[i] = light_dir * npts[i].normalize() * -1.f;
            }
            if (P.z > zbuffer[int(P.x + P.y * width)]) {
                Vec2f texture_coordinate = tvs[0] * bc.x + tvs[1] * bc.y + tvs[2] * bc.z;
                texture_coordinate.x *= texture.get_width();
                texture_coordinate.y *= texture.get_height();
//                std::cout << texture_coordinate;
                TGAColor color = texture.get(int(texture_coordinate.x), int(texture_coordinate.y));
                float intensity = intensities[0] * bc.x + intensities[1] * bc.y + intensities[2] * bc.z;
                zbuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color * intensity);
            }
        }
    }
}


Vec3f & round(Vec3f &v) {
    v.x = int(v.x + 0.5);
    v.y = int(v.y + 0.5);
    v.z = int(v.z + 0.5);
    return v;
}

int main(int argc, char** argv) {
    if(argc == 2) {
        model = new Model(argv[1]);
    } else {
        model = new Model("../obj/african_head.obj");
    }

    char* text_file_path = "../obj/african_head_diffuse.tga";
    TGAImage texture;
    texture.read_tga_file(text_file_path);
    texture.flip_vertically();

    int zbuffer_size = width * height;
    auto zbuffer = new float[zbuffer_size];
    for (int i = 0; i < zbuffer_size; ++i) zbuffer[i] = std::numeric_limits<float>::min();

    {
        // draw the model
        Matrix Projection = Matrix::identity(4);
        Matrix ModelView = lookat(Eye, Center, Up);
        Projection[3][2] = -1.f / (Eye - Center).norm();
        Matrix ViewPort = viewport(0, 0, width, height);
        Matrix Tr = ViewPort * Projection * ModelView;

        TGAImage image(width, height, TGAImage::RGB);
        for (int i = 0; i < model->nfaces(); ++i) {
            // for each face
            std::vector<int> face = model->face(i);
            std::vector<int> tface = model->tface(i);
            std::vector<int> nface = model->nface(i);
            Vec3f pts[3];
            Vec2f tvs[3];
            Vec3f npts[3];
            for (int j = 0; j < 3; ++j) {
//                pts[j] = world2screen(model->vert(face[j]));
                pts[j] = m2v(Tr * v2m(model->vert(face[j])));
                pts[j] = round(pts[j]);
                tvs[j] = model->tvert(tface[j]);
                npts[j] = model->normal_vert(nface[j]);
            }
            Vec3f n = (model->vert(face[2]) - model->vert(face[1])) ^ (model->vert(face[1]) - model->vert(face[0]));
            n.normalize();
            // face light intensity
            float intensity = n * light_dir;
            if (intensity > 0) {
                triangle(pts, npts, zbuffer, image, tvs, texture);
            }
        }
        image.flip_vertically();
        image.write_tga_file("output.tga");
    }


    delete model;
    delete[] zbuffer;
    return 0;
}
