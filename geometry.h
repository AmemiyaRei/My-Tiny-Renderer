#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

// 矩阵类模板
template<size_t DimCols, size_t DimRows, typename T> class mat;
// 具有DIM个T类型的元素的向量类模板
template<size_t DIM, typename T> struct vec{
    vec() {
        for (size_t i = 0; i < DIM; i++) {
            data_[i] = int();
        }
    }
    // 重载两次保证常量对象和非常量对象都可以使用[]读取数据，同时常量对象不允许修改数据
    T& operator[](const size_t i) {
        assert(i < DIM);
        return data_[i];
    }
    const T& operator[](const size_t i) const {
        assert(i < DIM);
        return data_[i];
    }
private:
    T data_[DIM];
};

// 模板具体化
template <typename T> struct vec<2, T> {
    vec(): x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    // 拷贝构造函数
    template<class U> vec<2, T> (const vec<2, U> &v);
          T& operator[](const size_t i)       { assert(i < 2); return i <= 0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }
    // 模板特化修改数据成员定义
    T x, y;
};

template <typename T> struct vec<3, T> {
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    template<class U> vec<3, T>(const vec<3, U> &v);
          T& operator[](const size_t i)       { assert(i < 3); return i <= 0 ? x : (i == 1 ? y : z); }
    const T& operator[](const size_t i) const { assert(i < 3); return i <= 0 ? x : (i == 1 ? y : z); }
    float norm() { return std::sqrt(x * x + y * y + z * z); }
    vec<3, T> & normalize(T l = 1) { *this = (*this) * (l / norm()); return *this; }

    T x, y ,z;
};

/////////////////////////////////////////////////////////////////////////////

// 重载*运算符，实现向量模板类点乘计算
template<size_t DIM, typename T> T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
    T ret = T();
    for (size_t i = 0; i < DIM; i++) {
        ret += lhs[i] * rhs[i];
    }
    return ret;
}

// 向量加法
template<size_t DIM, typename T>vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = 0; i < DIM; ++i) {
        lhs[i] += rhs[i];
    }
    return lhs;
}

// 向量减法
template<size_t DIM, typename T>vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = 0; i < DIM; ++i) {
        lhs[i] -= rhs[i];
    }
    return lhs;
}

// 向量标量乘
template<size_t DIM, typename T, typename U>vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = 0; i < DIM; ++i) {
        lhs[i] *= rhs;
    }
    return lhs;
}

// 向量标量除
template<size_t DIM, typename T, typename U>vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = 0; i < DIM; ++i) {
        lhs[i] /= rhs;
    }
    return lhs;
}

// 向量嵌入
template<size_t LEN, size_t DIM, typename T> vec<LEN, T> embed(const vec<DIM, T> &v, T fill=1) {
    vec<LEN, T> ret;
    for (size_t i = 0; i < LEN; ++i) {
        ret[i] = i < DIM ? v[i] : fill;
    }
    return ret;
}

// 向量投影
template<size_t LEN, size_t DIM, typename T> vec<LEN, T> proj(const vec<DIM, T> &v) {
    vec<LEN, T> ret;
    for (size_t i = 0 ;i < LEN; ++i) {
        ret[i] = v[i];
    }
    return ret;
}

// 3维向量叉积
template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2) {
    return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

// 重载<<运算符使得可以cout向量
template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM, T>& v) {
    for (unsigned int i = 0; i < DIM; ++i) {
        out << v[i] << " ";
    }
    return out;
}

/////////////////////////////////////////////////////////////////////////

template<size_t DIM, typename T> struct dt {
    static T det(const mat<DIM, DIM, T>& src) {
        T ret = 0;
        for (size_t i = 0; i < DIM; ++i) {
            ret += src[0][i] * src.cofactor(0, i);
        }
        return ret;
    }
};

template<typename T> struct dt<1, T> {
    static T det(const mat<1, 1, T>& src) {
        return src[0][0];
    }
};

//////////////////////////////////////////////////////////////////////////

template<size_t DimRows, size_t DimCols, typename T> class mat {
    vec<DimCols, T> rows[DimRows];
public:
    mat() {}

    vec<DimCols, T>& operator[] (const size_t idx) {
        assert(idx < DimRows);
        return rows[idx];
    }

    const vec<DimCols, T>& operator[] (const size_t idx) const {
        assert(idx < DimRows);
        return rows[idx];
    }

    vec<DimRows, T> col(const size_t idx) const {
        assert(idx < DimCols);
        vec<DimRows, T> ret;
        for (size_t i = 0; i < DimRows; ++i) {
            ret[i] = rows[i][idx];
        }
        return ret;
    }

    void set_col(size_t idx, vec<DimRows, T> v) {
        assert(idx < DimCols);
        for (size_t i = 0; i < DimRows; ++i) {
            rows[i][idx] = v[i];
        }
    }

    static mat<DimRows, DimCols, T> identity() {
        mat<DimRows, DimCols, T> ret;
        for (size_t i = 0; i < DimRows; ++i) {
            for (size_t j = 0; j < DimCols; ++j) {
                ret[i][j] = (i == j);
            }
        }
        return ret;
    }

    T det() const {
        return dt<DimCols, T>::det(*this);
    }

    // 获取矩阵的子阵，原矩阵去掉row行和col列
    mat<DimRows - 1, DimCols - 1, T> get_minor(size_t row, size_t col) const {
        mat<DimRows - 1, DimCols - 1, T> ret;
        for (size_t i = 0; i < DimRows; ++i) {
            for (size_t j = 0; j < DimCols; ++j) {
                ret[i][j] = rows[i < row? i : i + 1][j < col ? j : j + 1];
            }
        }
        return ret;
    }

    // 计算代数余子式，递归展开行列式
    T cofactor(size_t row, size_t col) const {
        return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
    }

    // 伴随矩阵，代数余子式构成的矩阵的转置，这里没有转置，其实是伴随矩阵的转置矩阵
    mat<DimRows, DimCols, T> adjugate() const {
        mat<DimRows, DimCols, T> ret;
        for (size_t i = 0; i < DimRows; ++i) {
            for (size_t j = 0; j < DimCols; ++j) {
                ret[i][j] = cofactor(i, j);
            }
        }
        return ret;
    }

    // 转置逆矩阵
    mat<DimRows, DimCols, T> invert_transpose() {
        mat<DimRows, DimCols, T> ret = adjugate();
        // tmp为矩阵行列式的值，通过展开矩阵计算得到
        T tmp = ret[0] * rows[0];
        return ret / tmp;
    }
};

///////////////////////////////////////////////////////////////////////////////

// 矩阵向量乘
template<size_t DimRows, size_t DimCols, typename T> vec<DimRows, T> operator*(const mat<DimRows, DimCols, T>& lhs,
        const vec<DimCols, T>& rhs) {
    vec<DimRows, T> ret;
    for (size_t i = 0; i < DimRows; ++i) {
        ret[i] = lhs[i] * rhs;
    }
    return ret;
}

// 矩阵乘
template<size_t R1, size_t C1, size_t C2, typename T>mat<R1, C2, T> operator*(const mat<R1, C1, T>& lhs,
        const mat<C1, C2, T>& rhs) {
    mat<R1, C2, T> result;
    for (size_t i = 0; i < R1; ++i) {
        for (size_t j = 0; j < C2; ++j) {
            result[i][j] = lhs[i] * rhs.col(j);
        }
    }
    return result;
}

// 矩阵变量除
template<size_t DimRows, size_t DimCols, typename T>mat<DimCols, DimRows, T> operator/(mat<DimRows, DimCols, T> lhs,
        const T& rhs) {
    for (size_t i = 0; i < DimRows; ++i) {
        lhs[i] = lhs[i] / rhs;
    }
    return lhs;
}

// cout输出矩阵
template<size_t DimRows, size_t DimCols, class T> std::ostream& operator<<(std::ostream& out, mat<DimRows, DimCols, T>& m) {
    for (size_t i = 0; i < DimRows; ++i) {
        out << m[i] << std::endl;
    }
    return out;
}

//////////////////////////////////////////////////////////////////////

typedef vec<2, float> Vec2f;
typedef vec<2, int>   Vec2i;
typedef vec<3, float> Vec3f;
typedef vec<3, int>   Vec3i;
typedef vec<4, float> Vec4f;
typedef mat<4, 4, float> Matrix;

// old codes
//template <class t> struct Vec2 {
//    t x, y;
//    Vec2<t>() : x(t()), y(t()) {}
//    Vec2<t>(t _x, t _y) : x(_x), y(_y) {}
//    Vec2<t>(const Vec2<t> &v) : x(t()), y(t()) { *this = v; }
//    Vec2<t> & operator =(const Vec2<t> &v) {
//        if (this != &v) {
//            x = v.x;
//            y = v.y;
//        }
//        return *this;
//    }
//    Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(x+V.x, y+V.y); }
//    Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(x-V.x, y-V.y); }
//    Vec2<t> operator *(float f)          const { return Vec2<t>(x*f, y*f); }
//    t& operator[](const int i) { if (x<=0) return x; else return y; }
//    template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
//};
//
//template <class t> struct Vec3 {
//    t x, y, z;
//    Vec3<t>() : x(t()), y(t()), z(t()) { }
//    Vec3<t>(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
//    template <class u> Vec3<t>(const Vec3<u> &v);
//    Vec3<t>(const Vec3<t> &v) : x(t()), y(t()), z(t()) { *this = v; }
//    Vec3<t> & operator =(const Vec3<t> &v) {
//        if (this != &v) {
//            x = v.x;
//            y = v.y;
//            z = v.z;
//        }
//        return *this;
//    }
//    Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
//    Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
//    Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
//    Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
//    t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
//    float norm () const { return std::sqrt(x*x+y*y+z*z); }
//    Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
//    t& operator[](const int i) { if (i<=0) return x; else if (i==1) return y; else return z; }
//    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
//};
//
//typedef Vec2<float> Vec2f;
//typedef Vec2<int>   Vec2i;
//typedef Vec3<float> Vec3f;
//typedef Vec3<int>   Vec3i;
//
//template <> template <> Vec3<int>::Vec3(const Vec3<float> &v);
//template <> template <> Vec3<float>::Vec3(const Vec3<int> &v);
//
//
//template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
//    s << "(" << v.x << ", " << v.y << ")\n";
//    return s;
//}
//
//template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
//    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
//    return s;
//}
//
////////////////////////////////////////////////////////////////////////////////////////////////
//
//const int DEFAULT_ALLOC=4;
//
//class Matrix {
//    std::vector<std::vector<float> > m;
//    int rows, cols;
//public:
//    Matrix(int r=DEFAULT_ALLOC, int c=DEFAULT_ALLOC);
//    inline int nrows();
//    inline int ncols();
//
//    static Matrix identity(int dimensions);
//    std::vector<float>& operator[](const int i);
//    Matrix operator*(const Matrix& a);
//    Matrix transpose();
//    Matrix inverse();
//
//    friend std::ostream& operator<<(std::ostream& s, Matrix& m);
//};

/////////////////////////////////////////////////////////////////////////////////////////////


#endif //__GEOMETRY_H__
