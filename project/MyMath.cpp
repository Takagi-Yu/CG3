#include "MyMath.h"

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return result;
}

Matrix4x4 Multiply(const Matrix4x4 &m1, const Matrix4x4 m2) {
    Matrix4x4 result{};

    result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] +
        m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
    result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] +
        m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
    result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] +
        m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
    result.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] +
        m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];
    result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] +
        m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
    result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] +
        m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
    result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] +
        m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
    result.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] +
        m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];
    result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] +
        m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
    result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] +
        m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
    result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] +
        m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
    result.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] +
        m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];
    result.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] +
        m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
    result.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] +
        m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
    result.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] +
        m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
    result.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] +
        m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];

    return result;
}

Matrix4x4 MakeRotateXMatrix(float radian) {
    Matrix4x4 result{
        1, 0, 0, 0,
        0, cosf(radian), sinf(radian), 0,
        0, -sinf(radian), cosf(radian), 0,
        0, 0, 0, 1
    };
    return result;
}

Matrix4x4 MakeRotateYMatrix(float radian) {
    Matrix4x4 result{
        cosf(radian), 0, -sinf(radian), 0,
        0, 1, 0, 0,
        sinf(radian), 0, cosf(radian),  0,
        0, 0, 0, 1
    };
    return result;
}

Matrix4x4 MakeRotateZMatrix(float radian) {
    Matrix4x4 result{
        cosf(radian), sinf(radian), 0, 0,
        -sinf(radian), cosf(radian), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    return result;
}

Matrix4x4 MakeScaleMatrix(Vector3 scale) {
    Matrix4x4 result{
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return result;
}

Matrix4x4 MakeTranslateMatrix(Vector3 translate) {
    Matrix4x4 result{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        translate.x, translate.y, translate.z, 1.0f
    };
    return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Vector3 &rotate,
    const Vector3 &translate) {
    Matrix4x4 result;

    Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);

    Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);

    Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

    Matrix4x4 rotateXYZMatrix =
        Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

    for (int i = 0; i < 3; i++) {
        result.m[0][i] = scale.x * rotateXYZMatrix.m[0][i];
    }
    result.m[0][3] = 0;

    for (int i = 0; i < 3; i++) {
        result.m[1][i] = scale.y * rotateXYZMatrix.m[1][i];
    }
    result.m[1][3] = 0;

    for (int i = 0; i < 3; i++) {
        result.m[2][i] = scale.z * rotateXYZMatrix.m[2][i];
    }
    result.m[2][3] = 0;

    result.m[3][0] = translate.x;
    result.m[3][1] = translate.y;
    result.m[3][2] = translate.z;
    result.m[3][3] = 1;

    return result;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio,
    float nearClip, float farClip) {
    Matrix4x4 result;

    result.m[0][0] = (1.0f / aspectRatio) * (1.0f / tanf(fovY / 2.0f));
    result.m[0][1] = 0.0f;
    result.m[0][2] = 0.0f;
    result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f;
    result.m[1][1] = 1.0f / tanf(fovY / 2.0f);
    result.m[1][2] = 0.0f;
    result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f;
    result.m[2][1] = 0.0f;
    result.m[2][2] = farClip / (farClip - nearClip);
    result.m[2][3] = 1.0f;
    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
    result.m[3][3] = 0.0f;

    return result;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right,
    float bottom, float nearClip, float farClip) {
    Matrix4x4 result;

    result.m[0][0] = 2.0f / (right - left);
    result.m[0][1] = 0.0f;
    result.m[0][2] = 0.0f;
    result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f;
    result.m[1][1] = 2.0f / (top - bottom);
    result.m[1][2] = 0.0f;
    result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f;
    result.m[2][1] = 0.0f;
    result.m[2][2] = 1.0f / (farClip - nearClip);
    result.m[2][3] = 0.0f;
    result.m[3][0] = (left + right) / (left - right);
    result.m[3][1] = (top + bottom) / (bottom - top);
    result.m[3][2] = nearClip / (nearClip - farClip);
    result.m[3][3] = 1.0f;

    return result;
}

// 逆行列
Matrix4x4 Inverse(const Matrix4x4 &m) {
    Matrix4x4 result;
    float A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] +
        m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] +
        m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2] - // 3
        m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] -
        m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] -
        m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2] - // 6
        m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] -
        m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] -
        m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2] + // 9
        m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] +
        m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] +
        m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2] + // 12
        m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] +
        m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] +
        m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2] - // 15
        m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] -
        m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] -
        m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2] - // 18
        m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] -
        m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] -
        m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0] + // 21
        m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] +
        m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] +
        m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0]; // 24

    result.m[0][0] =
        1 / A *
        (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] +
            m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] -
            m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]);
    result.m[0][1] =
        1 / A *
        (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] -
            m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] +
            m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]);
    result.m[0][2] =
        1 / A *
        (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] +
            m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] -
            m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]);
    result.m[0][3] =
        1 / A *
        (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] -
            m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] +
            m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]);

    result.m[1][0] =
        1 / A *
        (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] -
            m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] +
            m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]);
    result.m[1][1] =
        1 / A *
        (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] +
            m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] -
            m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]);
    result.m[1][2] =
        1 / A *
        (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] -
            m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] +
            m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]);
    result.m[1][3] =
        1 / A *
        (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] +
            m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] -
            m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]);

    result.m[2][0] =
        1 / A *
        (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] +
            m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] -
            m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]);
    result.m[2][1] =
        1 / A *
        (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] -
            m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] +
            m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]);
    result.m[2][2] =
        1 / A *
        (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] +
            m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] -
            m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]);
    result.m[2][3] =
        1 / A *
        (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] -
            m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] +
            m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]);

    result.m[3][0] =
        1 / A *
        (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] -
            m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] +
            m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]);
    result.m[3][1] =
        1 / A *
        (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] +
            m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] -
            m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]);
    result.m[3][2] =
        1 / A *
        (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] -
            m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] +
            m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]);
    result.m[3][3] =
        1 / A *
        (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] +
            m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] -
            m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]);

    return result;
}
