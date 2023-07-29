#include "geometry.h"

// 连续两个template<>表示对类模板中的模板函数进行特化，由于是构造函数，所以没有模板类型参数
// 浮点数与整数向量转换，包含四舍五入
template <> template <> vec<3, int>  ::vec(const vec<3, float>& v) : x(int(v.x + 0.5)), y(int(v.y + 0.5)), z(int(v.z + 0.5)) {}
template <> template <> vec<3, float>::vec(const vec<3, int>& v)   : x(v.x), y(v.y), z(v.z) {}
template <> template <> vec<2, int>  ::vec(const vec<2, float>& v) : x(int(v.x + 0.5)), y(int(v.y + 0.5)) {}
template <> template <> vec<2, float>::vec(const vec<2, int>& v)   : x(v.x), y(v.y) {}

