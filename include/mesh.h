#ifndef __MESH_H
#define __MESH_H
	#include <tri.h>

	typedef struct MeshStruct {
		Vec pos;
		Quaternion rot;

		TriIndices *triIndices;
		int triIndicesSize;

		Vec *meshPoints;
		Vec *worldPoints;
		int pointsSize;

		Mat mat;
	} Mesh;

	Mesh InitMesh(
		TriIndices *, int,
		Vec *, int,
		ProgramState *
	);

	// the int is to make converting from .obj more tolerable
	// because point indices in .obj files start at 1
	Mesh LoadMeshFromText(char *, ProgramState *, int);
	Mesh LoadMeshFromBinary(char *, ProgramState *);
	void WriteBinaryFromMesh(Mesh *, char *);
	void UpdateMesh(Mesh *, Vec, Quaternion);
#endif
