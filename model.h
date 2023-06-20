#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
    std::vector<Vec3f> normalverts_;
	std::vector<std::vector<int> > faces_;
    std::vector<std::vector<int> > tfaces_;
    std::vector<std::vector<int> > nfaces_;
    std::vector<Vec2f> texture_verts_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
    int ntverts();
	Vec3f vert(int i);
    Vec2f tvert(int i);
    Vec3f normal_vert(int i);
	std::vector<int> face(int idx);
    std::vector<int> tface(int idx);
    std::vector<int> nface(int idx);
};

#endif //__MODEL_H__
