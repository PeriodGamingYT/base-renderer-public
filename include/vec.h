#ifndef __VEC_H
#define __VEC_H
	#include <screen-vec.h>

	typedef struct VecStruct {
		float x, y, z, w;
	} Vec;

	#define VEC(_x, _y, _z) \
		((Vec) { (_x), (_y), (_z), 1 })

	#define VEC_ADD(_a, _b) \
		VEC((_a).x + (_b).x, (_a).y + (_b).y, (_a).z + (_b).z)

	#define VEC_SUB(_a, _b) \
		VEC((_a).x - (_b).x, (_a).y - (_b).y, (_a).z - (_b).z)

	#define VEC_MUL(_a, _b) \
		VEC((_a).x * (_b).x, (_a).y * (_b).y, (_a).z * (_b).z)

	#define VEC_DIV(_a, _b) \
		VEC((_a).x / (_b).x, (_a).y / (_b).y, (_a).z / (_b).z)

	#define VEC_NEG(_vec) \
		VEC(-(_vec).x, -(_vec).y, -(_vec).z)

	Vec VecCross(Vec, Vec);
	float VecDot(Vec, Vec);
	float VecLength(Vec);
	Vec VecNormal(Vec);

	typedef struct QuaternionStruct {
		float x, y, z, w; // w is real
	} Quaternion;

	#define QUATERNION(_x, _y, _z, _w) \
		((Quaternion) { (_x), (_y), (_z), (_w) })

	// from wikipedia
	// ZYX order: yaw, pitch, roll
	float QuaternionLength(Quaternion);
	Quaternion QuaternionNormal(Quaternion);
	Quaternion EulerToQuaternion(Vec);
	Quaternion QuaternionConjugate(Quaternion);

	// NOTE: Mat[row][column|rowInnerIndex], so row-major
	typedef float Mat[4][4];

	// a pointer is needed because msvc (and c, for
	// that matter) is stupid and can't comprehend returning
	// an array (structs are okay, though!) >:(
	void MoveMat(Mat *, Vec);
	void RotMat(Mat *, Quaternion);

	// use these matrices if you want gimbal lock
	void RotXMat(Mat *, float);
	void RotYMat(Mat *, float);
	void RotZMat(Mat *, float);
	void IdentMat(Mat *);
	void TransposeMat(Mat *);
	void MatXMat(Mat *, Mat *, Mat *);

	// w is assumed to be 1 (and as such is not multiplied by its
	// corresponding matrix)
	Vec VecXMat(Vec, Mat *);
	void VecsXMat(Vec *, Vec *, int, Mat *);

	typedef struct CameraRotStruct {
		float yaw;
		float targetY;
	} CameraRot;

	typedef struct CameraStruct {
		float zNear;
		float zFar;
		float fov; // degrees
		Mat projMat;

		Vec pos;

		CameraRot rot;
		Mat cameraMat;

		Mat finalMat;
	} Camera;

	Camera InitCamera(ScreenVec);

	// since fov can change, i will need to implement
	// this (zFar and zNear don't need change)
	void UpdateCameraFov(Camera *, float, ScreenVec);

	#define CAMERA_ROT(_yaw, _targetY) \
		((CameraRot) { _yaw, _targetY })

	void CameraRotOnlyMat(Mat *, CameraRot);
	void LookAtViewMat(Mat *, Vec, Vec, Vec);
	void CameraRotMat(Mat *, CameraRot, Vec);
	void UpdateCamera(Camera *, Vec, CameraRot);
	ScreenVec ProjPointToScreenVec(Vec, ScreenVec);
#endif

