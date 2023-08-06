#include <iostream>
#include <vector>

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include "our_gl.h"

Model *model = NULL;
const int width = 800;
const int height = 800;

Vec3f light_dir(1, 1, 1);
Vec3f       eye(1, 1, 3);
Vec3f        up(0, 1, 0);
Vec3f    center(0, 0, 0);

struct GouraudShader : public IShader {
    Vec3f varying_intensity;

    //着色器首先进行顶点坐标变换，然后计算顶点光照强度并保存下来，三个顶点都处理后再进行三角形绘制
    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity * bar;
        if (intensity > 0.85) intensity = 1.0;
        else if (intensity > 0.60) intensity = 0.80;
        else if (intensity > 0.45) intensity = 0.60;
        else if (intensity > 0.30) intensity = 0.45;
        else if (intensity > 0.15) intensity = 0.30;
        else intensity = 0.0;
        color = TGAColor(255, 155, 0) * intensity;
        return false;
    }
};

struct Shader : public IShader {
    // 法线贴图处理，法向量变换与模型变换矩阵互为逆转矩阵
    mat<2, 3, float> varying_uv;
    mat<4, 4, float> uniform_M;   // Projection * ModelView
    mat<4, 4, float> uniform_MIT; // (Projection * ModelView).invert_transpose()

    virtual Vec4f vertex(int iface, int nthvert) {
        // 2行3列矩阵，每一列为一个贴图坐标
        // 设置顶点贴图坐标
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv * bar;
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize(); // 法向量
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize(); // 光照向量
        Vec3f r = (n * (n * l * 2.f) - l).normalize(); // 反射向量
        float spec = pow(std::max(r.z, 0.f), model->specular(uv));
        float diff = std::max(0.f, n * l);
        color = model->diffuse(uv);
        for (int i = 0; i < 3; i++) {
            // a test config of phong lighting
            color[i] = std::min<float>(5.f + color[i] * (diff + 0.6f * spec), 255.f);
        }
        return false;
    }
};


int main(int argc, char** argv) {
    if (argc == 2) {
        model = new Model(argv[1]);
    } else {
        model = new Model("../obj/african_head.obj");
    }

    lookat(eye, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (eye - center).norm());
    light_dir.normalize();

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

//    GouraudShader shader;
    Shader shader;
    shader.uniform_M = Projection * ModelView;
    shader.uniform_MIT = (Projection * ModelView).invert_transpose();
    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.  flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.  write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
