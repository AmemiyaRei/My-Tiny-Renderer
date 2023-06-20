#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            iss >> v.x >> v.y >> v.z;
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> tf;
            std::vector<int> nf;
            int idx_n, idx, idx_t;
            iss >> trash;
            // v/vt/vn
            while (iss >> idx >> trash >> idx_t >> trash >> idx_n) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
                idx_t--;
                tf.push_back(idx_t);
                idx_n--;
                nf.push_back(idx_n);
            }
            faces_.push_back(f);
            tfaces_.push_back(tf);
            nfaces_.push_back(nf);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f tv;
            iss >> tv.x >> tv.y;
            texture_verts_.push_back(tv);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f v;
            iss >> v.x >> v.y >> v.z;
            normalverts_.push_back(v);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::ntverts() {
    return (int)texture_verts_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::normal_vert(int i) {
    return normalverts_[i];
}

Vec2f Model::tvert(int i) {
    return texture_verts_[i];
}

std::vector<int> Model::tface(int idx) {
    return tfaces_[idx];
}

std::vector<int> Model::nface(int idx) {
    return nfaces_[idx];
}
