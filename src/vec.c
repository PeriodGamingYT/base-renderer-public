#include <vec.h>

Vec VecCross(Vec a, Vec b) {
	return VEC(
		(a.y * b.z) - (a.z * b.y),
		(a.z * b.x) - (a.x * b.z),
		(a.x * b.y) - (a.y * b.x)
	);
}

float VecDot(Vec a, Vec b) {
	return (
		(a.x * b.x) +
		(a.y * b.y) +
		(a.z * b.z)
	);
}

float VecLength(Vec vec) {
	return sqrtf(VecDot(vec, vec));
}

Vec VecNormal(Vec vec) {
	float length = VecLength(vec);
	if(length == 0) {
		length = 1;
	}

	return VEC_DIV(vec, VEC(length, length, length));
}

float QuaternionLength(Quaternion quat) {
	return sqrtf(
		(quat.x * quat.x) +
		(quat.y * quat.y) +
		(quat.z * quat.z) +
		(quat.w * quat.w)
	);
}

Quaternion QuaternionNormal(Quaternion quat) {
	float length = QuaternionLength(quat);
	return QUATERNION(
		quat.x / length,
		quat.y / length,
		quat.z / length,
		quat.w / length
	);
}

// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
Quaternion EulerToQuaternion(Vec angles) {
	float sinRoll = sin(angles.x / 2);
	float sinPitch = sin(angles.y / 2);
	float sinYaw = sin(angles.z / 2);

	float cosRoll = cos(angles.x / 2);
	float cosPitch = cos(angles.y / 2);
	float cosYaw = cos(angles.z / 2);

	return QuaternionNormal(QUATERNION(
		(sinRoll * cosPitch * cosYaw) - (cosRoll * sinPitch * sinYaw),
		(cosRoll * sinPitch * cosYaw) + (sinRoll * cosPitch * sinYaw),
		(cosRoll * cosPitch * sinYaw) - (sinRoll * sinPitch * cosYaw),
		(cosRoll * cosPitch * cosYaw) + (sinRoll * sinPitch * sinYaw)
	));
}

Quaternion QuaternionConjugate(Quaternion quat) {
	return QUATERNION(-quat.x, -quat.y, -quat.z, quat.w);
}

void MoveMat(Mat *mat, Vec moveBy) {
	memcpy(&mat[0][0], (Mat) {
		{ 1, 0, 0, moveBy.x },
		{ 0, 1, 0, moveBy.y },
		{ 0, 0, 1, moveBy.z },
		{ 0, 0, 0, 1        }
	}, sizeof(Mat));
}

// https://en.wikipedia.org/wiki/Rotation_matrix
void RotXMat(Mat *mat, float rotX) {
	float sinX = sin(rotX);
	float cosX = cos(rotX);
	memcpy(mat, (Mat) {
		{ 1, 0,     0,    0 },
		{ 0, cosX, -sinX, 0 },
		{ 0, sinX,  cosX, 0 },
		{ 0, 0,     0,    1 }
	}, sizeof(Mat));
}

void RotYMat(Mat *mat, float rotY) {
	float sinY = sin(rotY);
	float cosY = cos(rotY);
	memcpy(&mat[0][0], (Mat) {
		{  cosY, 0, sinY, 0 },
		{  0,    1, 0,    0 },
		{ -sinY, 0, cosY, 0 },
		{  0,    0, 0,    1 }
	}, sizeof(Mat));
}

void RotZMat(Mat *mat, float rotZ) {
	float sinZ = sin(rotZ);
	float cosZ = cos(rotZ);
	memcpy(&mat[0][0], (Mat) {
		{ cosZ, -sinZ, 0, 0 },
		{ sinZ,  cosZ, 0, 0 },
		{ 0,     0,    1, 0 },
		{ 0,     0,    0, 1 }
	}, sizeof(Mat));
}

void RotMat(Mat *mat, Quaternion rot) {
	float xx = rot.x * rot.x;
	float xy = rot.x * rot.y;
	float xz = rot.x * rot.z;
	float xw = rot.x * rot.w;

	// float yx = rot.y * rot.x; // == xy
	float yy = rot.y * rot.y;
	float yz = rot.y * rot.z;
	float yw = rot.y * rot.w;

	// float zx = rot.z * rot.x; // == xz
	// float zy = rot.z * rot.y; // == yz
	float zz = rot.z * rot.z;
	float zw = rot.z * rot.w;

	// float wx = rot.w * rot.x; // == xw
	// float wy = rot.w * rot.y; // == yw
	// float wz = rot.w * rot.z; // == zw
	// float ww = rot.w * rot.w; // unused
	memcpy(mat, (Mat) {
		{ 1 - (2 * (yy + zz)),      2 * (xy - zw) ,      2 * (xz + yw) , 0 },
		{      2 * (xy + zw) , 1 - (2 * (xx + zz)),      2 * (yz - xw) , 0 },
		{      2 * (xz - yw) ,      2 * (yz + xw) , 1 - (2 * (xx + yy)), 0 },
		{                  0,                   0,                   0,  1 }
	}, sizeof(Mat));
}

void IdentMat(Mat *mat) {
	memcpy(&mat[0][0], (Mat) {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	}, sizeof(Mat));
}

void TransposeMat(Mat *mat) {
	#define M(_col, _row) \
		(*mat)[_row][_col]
		memcpy(&mat[0][0], (Mat) {
			{ M(0, 0), M(0, 1), M(0, 2), M(0, 3) },
			{ M(1, 0), M(1, 1), M(1, 2), M(1, 3) },
			{ M(2, 0), M(2, 1), M(2, 2), M(2, 3) },
			{ M(3, 0), M(3, 1), M(3, 2), M(3, 3) }
		}, sizeof(Mat));
	#undef M
}

void MatXMat(
	Mat *matA,
	Mat *matB,
	Mat *toMat
) {

	// need result because toMat can be matA or matB
	Mat result = { 0 };
	for(int col = 0; col < 4; col++) {
		for(int row = 0; row < 4; row++) {
			result[row][col] = (
				((*matA)[row][0] * (*matB)[0][col]) +
				((*matA)[row][1] * (*matB)[1][col]) +
				((*matA)[row][2] * (*matB)[2][col]) +
				((*matA)[row][3] * (*matB)[3][col])
			);
		}
	}

	memcpy(&toMat[0][0], &result, sizeof(Mat));
}

Vec VecXMat(
	Vec vec,
	Mat *mat
) {
	Vec result = { 0 };
	#define M(_row, _col, _coord) \
		(vec._coord * (*mat)[_row][_col])

		result.x = M(0, 0, x) + M(0, 1, y) + M(0, 2, z) + (*mat)[0][3];
		result.y = M(1, 0, x) + M(1, 1, y) + M(1, 2, z) + (*mat)[1][3];
		result.z = M(2, 0, x) + M(2, 1, y) + M(2, 2, z) + (*mat)[2][3];

		result.w = M(3, 0, x) + M(3, 1, y) + M(3, 2, z) + (*mat)[3][3];
		if(result.w != 0) {
			result.x /= result.w;
			result.y /= result.w;
			result.z /= result.w;
		}
	#undef M

	return result;
}

void VecsXMat(
	Vec *fromPoints,
	Vec *toPoints,
	int pointsSize,
	Mat *mat
) {
	int i = 0;
	Vec *currentFromPoint = fromPoints;
	Vec *currentToPoint = toPoints;
	for(i = 0; i <= pointsSize - 8; i += 8) {
		__m256 fromPointX = _mm256_setr_ps(
			currentFromPoint[0].x, currentFromPoint[1].x, currentFromPoint[2].x, currentFromPoint[3].x,
			currentFromPoint[4].x, currentFromPoint[5].x, currentFromPoint[6].x, currentFromPoint[7].x
		);

		__m256 fromPointY = _mm256_setr_ps(
			currentFromPoint[0].y, currentFromPoint[1].y, currentFromPoint[2].y, currentFromPoint[3].y,
			currentFromPoint[4].y, currentFromPoint[5].y, currentFromPoint[6].y, currentFromPoint[7].y
		);

		__m256 fromPointZ = _mm256_setr_ps(
			currentFromPoint[0].z, currentFromPoint[1].z, currentFromPoint[2].z, currentFromPoint[3].z,
			currentFromPoint[4].z, currentFromPoint[5].z, currentFromPoint[6].z, currentFromPoint[7].z
		);

		// _mm256_fmadd_ps isn't supported on my machine :(
		__m256 toPointX = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(fromPointX, _mm256_set1_ps((*mat)[0][0])),
				_mm256_mul_ps(fromPointY, _mm256_set1_ps((*mat)[0][1]))
			),

			_mm256_add_ps(
				_mm256_mul_ps(fromPointZ, _mm256_set1_ps((*mat)[0][2])),
				_mm256_set1_ps((*mat)[0][3])
			)
		);

		__m256 toPointY = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(fromPointX, _mm256_set1_ps((*mat)[1][0])),
				_mm256_mul_ps(fromPointY, _mm256_set1_ps((*mat)[1][1]))
			),

			_mm256_add_ps(
				_mm256_mul_ps(fromPointZ, _mm256_set1_ps((*mat)[1][2])),
				_mm256_set1_ps((*mat)[1][3])
			)
		);

		__m256 toPointZ = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(fromPointX, _mm256_set1_ps((*mat)[2][0])),
				_mm256_mul_ps(fromPointY, _mm256_set1_ps((*mat)[2][1]))
			),

			_mm256_add_ps(
				_mm256_mul_ps(fromPointZ, _mm256_set1_ps((*mat)[2][2])),
				_mm256_set1_ps((*mat)[2][3])
			)
		);

		__m256 toPointW = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(fromPointX, _mm256_set1_ps((*mat)[3][0])),
				_mm256_mul_ps(fromPointY, _mm256_set1_ps((*mat)[3][1]))
			),

			_mm256_add_ps(
				_mm256_mul_ps(fromPointZ, _mm256_set1_ps((*mat)[3][2])),
				_mm256_set1_ps((*mat)[3][3])
			)
		);

		__m256 mask = _mm256_cmp_ps(toPointW, _mm256_set1_ps(0), _CMP_NEQ_OQ);
		__m256 invPointW = _mm256_rcp_ps(toPointW);
		toPointX = _mm256_blendv_ps(toPointX, _mm256_mul_ps(toPointX, invPointW), mask);
		toPointY = _mm256_blendv_ps(toPointY, _mm256_mul_ps(toPointY, invPointW), mask);
		toPointZ = _mm256_blendv_ps(toPointZ, _mm256_mul_ps(toPointZ, invPointW), mask);

		// i need this to convert SoA (toPointX, ..., toPointZ) --> AoS (toPoints)
		__m128 x0To3 = _mm256_castps256_ps128(toPointX);
		__m128 y0To3 = _mm256_castps256_ps128(toPointY);
		__m128 xy0To1 = _mm_unpacklo_ps(x0To3, y0To3);
		__m128 xy2To3 = _mm_unpackhi_ps(x0To3, y0To3);

		__m128 x4To7 = _mm256_extractf128_ps(toPointX, 1);
		__m128 y4To7 = _mm256_extractf128_ps(toPointY, 1);
		__m128 xy4To5 = _mm_unpacklo_ps(x4To7, y4To7);
		__m128 xy6To7 = _mm_unpackhi_ps(x4To7, y4To7);

		__m128 z0To3 = _mm256_castps256_ps128(toPointZ);
		__m128 w0To3 = _mm256_castps256_ps128(toPointW);
		__m128 zw0To1 = _mm_unpacklo_ps(z0To3, w0To3);
		__m128 zw2To3 = _mm_unpackhi_ps(z0To3, w0To3);

		__m128 z4To7 = _mm256_extractf128_ps(toPointZ, 1);
		__m128 w4To7 = _mm256_extractf128_ps(toPointW, 1);
		__m128 zw4To5 = _mm_unpacklo_ps(z4To7, w4To7);
		__m128 zw6To7 = _mm_unpackhi_ps(z4To7, w4To7);

		_mm_storeu_ps((float *)(&currentToPoint[0]), _mm_movelh_ps(xy0To1, zw0To1));
		_mm_storeu_ps((float *)(&currentToPoint[1]), _mm_movehl_ps(zw0To1, xy0To1));

		_mm_storeu_ps((float *)(&currentToPoint[2]), _mm_movelh_ps(xy2To3, zw2To3));
		_mm_storeu_ps((float *)(&currentToPoint[3]), _mm_movehl_ps(zw2To3, xy2To3));

		_mm_storeu_ps((float *)(&currentToPoint[4]), _mm_movelh_ps(xy4To5, zw4To5));
		_mm_storeu_ps((float *)(&currentToPoint[5]), _mm_movehl_ps(zw4To5, xy4To5));

		_mm_storeu_ps((float *)(&currentToPoint[6]), _mm_movelh_ps(xy6To7, zw6To7));
		_mm_storeu_ps((float *)(&currentToPoint[7]), _mm_movehl_ps(zw6To7, xy6To7));

		currentFromPoint += 8;
		currentToPoint += 8;
	}

	for(; i < pointsSize; i++) {

		// one of the ONLY things I like about C#, the array[x, y, ...] syntax.
		// but not in c :(
		#define M(_row, _col, _coord) \
			(currentFromPoint->_coord * (*mat)[_row][_col])

			currentToPoint->x = M(0, 0, x) + M(0, 1, y) + M(0, 2, z) + (*mat)[0][3];
			currentToPoint->y = M(1, 0, x) + M(1, 1, y) + M(1, 2, z) + (*mat)[1][3];
			currentToPoint->z = M(2, 0, x) + M(2, 1, y) + M(2, 2, z) + (*mat)[2][3];

			currentToPoint->w = M(3, 0, x) + M(3, 1, y) + M(3, 2, z) + (*mat)[3][3];
			if(currentToPoint->w != 0) {
				currentToPoint->x /= currentToPoint->w;
				currentToPoint->y /= currentToPoint->w;
				currentToPoint->z /= currentToPoint->w;
			}
		#undef M

		currentFromPoint++;
		currentToPoint++;
	}
}

void UpdateCameraFov(Camera *camera, float fov, ScreenVec screenSize) {
	float fovScale = 1 / (tanf(
		(fov / 2) *
		(M_PI / 180)
	));

	float aspectRatio = (float)(screenSize.x) / (float)(screenSize.y);
	camera->projMat[0][0] = fovScale / aspectRatio;
	camera->projMat[1][1] = fovScale;
}

void CameraRotOnlyMat(Mat *mat, CameraRot rot) {
	Vec up = VEC(0, 1, 0);
	Vec target = VecNormal(VEC(0, rot.targetY, 1));

	Mat yawMat = { 0 };
	RotYMat(&yawMat, rot.yaw);

	Vec lookDir = VecXMat(target, &yawMat);
	Vec right = VecNormal(VecCross(up, lookDir));
	Vec newUp = VecCross(lookDir, right);
	memcpy(&mat[0][0], (Mat){
		{ right.x, newUp.x, lookDir.x, 0 },
		{ right.y, newUp.y, lookDir.y, 0 },
		{ right.z, newUp.z, lookDir.z, 0 },
		{       0,       0,         0, 1 }
	}, sizeof(Mat));
}

void LookAtViewMat(
	Mat *mat,
	Vec pos,
	Vec target,
	Vec up
) {
	Vec forward = VecNormal(VEC_SUB(target, pos));
	Vec right = VecNormal(VecCross(up, forward));
	Vec newUp = VecCross(forward, right);
	memcpy(&mat[0][0], (Mat) {
		{   right.x,   right.y,   right.z,   -VecDot(right, pos)  },
		{   newUp.x,   newUp.y,   newUp.z,   -VecDot(newUp, pos)  },
		{ forward.x, forward.y, forward.z, -VecDot(forward, pos)  },
		{         0,         0,         0,                     1  }
	}, sizeof(Mat));
}

void CameraRotMat(
	Mat *mat,
	CameraRot rot,
	Vec moveBy
) {
	Vec up = VEC(0, 1, 0);
	Vec target = VEC(0, rot.targetY, 1);

	Mat yawMat = { 0 };
	RotYMat(&yawMat, rot.yaw);

	Vec lookDir = VecXMat(target, &yawMat);
	target = VEC_ADD(moveBy, lookDir);

	Mat lookAtMat = { 0 };
	LookAtViewMat(&lookAtMat, moveBy, target, up);
	memcpy(&mat[0][0], &lookAtMat[0][0], sizeof(Mat));
}

void UpdateCamera(Camera *camera, Vec pos, CameraRot rot) {
	camera->pos = pos;
	camera->rot = rot;
	CameraRotMat(
		&camera->cameraMat,
		rot,
		pos
	);
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix.html
#define DEFAULT_Z_NEAR (float)(1)
#define DEFAULT_Z_FAR (float)(1000)
#define DEFAULT_FOV (float)(90)
Camera InitCamera(ScreenVec screenSize) {
	float fovScale = 1 / (tanf(
		(DEFAULT_FOV / 2) *
		(M_PI / 180)
	));

	float farNearCalc = DEFAULT_Z_FAR / (DEFAULT_Z_FAR - DEFAULT_Z_NEAR);
	float aspectRatio = (float)(screenSize.x) / (float)(screenSize.y);
	Mat projMat = {
		{ fovScale / aspectRatio, 0,        0,                            0 },
		{ 0,                      fovScale, 0,                            0 },
		{ 0,                      0,        farNearCalc,                  1 },
		{ 0,                      0,        DEFAULT_Z_NEAR * farNearCalc, 0 }
	};

	Camera result = {
		.zNear = DEFAULT_Z_NEAR,
		.zFar = DEFAULT_Z_FAR,
		.fov = DEFAULT_FOV,
	};

	memcpy(&result.projMat[0][0], projMat, sizeof(Mat));
	UpdateCamera(&result, VEC(0, 0, 0), CAMERA_ROT(0, 0));
	return result;
}
