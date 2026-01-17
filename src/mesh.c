#include <base-renderer.h>

Mesh InitMesh(
	TriIndices *triIndices, int triIndicesSize,
	Vec *points, int pointsSize,
	ProgramState *state
) {
	Mesh result = { 0 };

	TriIndices *arenaTriIndices = (TriIndices *)(AppendToArena(
		&state->triIndices,
		triIndicesSize
	));

	if(triIndices != NULL) {
		memcpy(
			arenaTriIndices,
			triIndices,
			triIndicesSize * sizeof(TriIndices)
		);
	}

	int startPointsIndex = state->meshPoints.size;
	for(int i = 0; i < triIndicesSize; i++) {
		TriIndices *triIndices = &arenaTriIndices[i];
		triIndices->a += startPointsIndex;
		triIndices->b += startPointsIndex;
		triIndices->c += startPointsIndex;
	}

	result.triIndices = arenaTriIndices;
	result.triIndicesSize = triIndicesSize;

	Vec *arenaMeshPoints = (Vec *)(AppendToArena(
		&state->meshPoints,
		pointsSize
	));

	if(points != NULL) {
		memcpy(
			arenaMeshPoints,
			points,
			pointsSize * sizeof(Vec)
		);
	}

	Vec *worldPoints = &state->worldPoints[startPointsIndex];
	if(points != NULL) {
		memcpy(
			worldPoints,
			points,
			pointsSize * sizeof(Vec)
		);
	}

	result.meshPoints = arenaMeshPoints;
	result.worldPoints = worldPoints;
	result.pointsSize = pointsSize;
	return result;
}

static inline BOOL IsWhitespace(uint8_t input) {
	return (
		input ==  ' ' ||
		input == '\t' ||
		input == '\r' ||
		input == '\n' ||

		// for comments, needs to be specially handled (entire rest of the
		// line must be skipped)
		input ==  '#' ||

		// symbols are only for readabiliy and don't really
		// need to be taken seriously
		input ==  ',' ||
		input ==  ':'
	);
}

static void SkipLine(FileMap *fileMap) {
	for(
		;
		*fileMap->current != '\n' &&
		*fileMap->current != '\r' &&
		fileMap->current < fileMap->end;
		fileMap->current++
	);
}

static void SkipWhitespace(FileMap *fileMap) {
	for(
		;
		IsWhitespace(*fileMap->current) &&
		fileMap->current < fileMap->end;
		fileMap->current++
	) {
		if(*fileMap->current == '#') {
			SkipLine(fileMap);
		}
	}
}

typedef struct LiteralStruct {
	uint8_t *name;
	int size;
} Literal;

static inline IsAlphabet(uint8_t input) {
	return input >= 'a' && input <= 'z';
}

static Literal GetLiteral(FileMap *fileMap) {
	if(!IsAlphabet(*fileMap->current)) {
		return (Literal) {
			.name = NULL,
			.size = 0
		};
	}

	Literal result = {
		.name = fileMap->current,
		.size = 0
	};

	for(
		;
		IsAlphabet(*fileMap->current) &&
		fileMap->current < fileMap->end;
		fileMap->current++
	) {
		result.size++;
	}

	SkipWhitespace(fileMap);
	return result;
}

static inline BOOL IsLiteralEqual(Literal literal, char *name) {
	if(strlen(name) != literal.size) {
		return FALSE;
	}

	int i = 0;
	for(
		i = 0;
		literal.name[i] == name[i] &&
		i < literal.size;
		i++
	);

	return i == literal.size;
}

static inline BOOL IsIntChar(uint8_t input) {

	// if the first digit(s) of the number is a zero
	// then that needs to be ignored
	return input >= '0' && input <= '9';
}

static int GetInt(FileMap *fileMap) {
	// doesn't reasonably need to be errored, just ignored.
	// also kinda stupid lol
	for(
		;
		*fileMap->current == '0' &&
		fileMap->current < fileMap->end;
		fileMap->current++
	);

	if(!IsIntChar(*fileMap->current)) {
		fileMap->current--;
	}

	int result = 0;
	for(
		;
		IsIntChar(*fileMap->current) &&
		fileMap->current < fileMap->end;
		fileMap->current++
	) {
		result *= 10;
		result += *fileMap->current - '0';
	}

	return result;
}

static float GetFloat(FileMap *fileMap) {
	BOOL isNegative = FALSE;
	if(*fileMap->current == '-') {
		isNegative = TRUE;
		fileMap->current++;
	}

	float result = GetInt(fileMap);
	if(*fileMap->current != '.') {
		return result;
	}

	fileMap->current++;
	float mul = 0.1;
	for(
		;
		IsIntChar(*fileMap->current) &&
		fileMap->current < fileMap->end;
		fileMap->current++
	) {
		result += (*fileMap->current - '0') * mul;
		mul *= 0.1;
	}

	return result * (isNegative ? -1 : 1);
}

static inline BOOL IsFloatChar(uint8_t input) {
	return (
		(input >= '0' && input <= '9') ||
		input == '.' ||
		input == '-'
	);
}

static Error AssignFloats(FileMap *fileMap, int size, ...) {
	va_list args = { 0 };
	va_start(args, size);
		for(
			int i = 0;
			i < size &&
			fileMap->current < fileMap->end;
			i++
		) {
			float *currentFloat = va_arg(args, float *);
			if(!IsFloatChar(*fileMap->current)) {
				goto errorEarlyOut;
			}

			*currentFloat = GetFloat(fileMap);
			SkipWhitespace(fileMap);
		}
	va_end(args);
	return 0;

	errorEarlyOut: {
		va_end(args);
		return -1;
	}
}

static Error AssignInts(FileMap *fileMap, int size, ...) {
	va_list args = { 0 };
	va_start(args, size);
		for(
			int i = 0;
			i < size &&
			fileMap->current < fileMap->end;
			i++
		) {
			int *currentInt = va_arg(args, int *);
			if(!IsIntChar(*fileMap->current)) {
				goto errorEarlyOut;
			}

			*currentInt = GetInt(fileMap);
			SkipWhitespace(fileMap);
		}
	va_end(args);
	return 0;

	errorEarlyOut: {
		va_end(args);
		return -1;
	}
}

Mesh LoadMeshFromText(char *fileName, ProgramState *state, int pointIndexOffset) {
	Mesh result = { 0 };
	FileMap fileMap = OpenFileMap(fileName);
	if(fileMap.map == NULL) {
		return (Mesh) { 0 };
	}
		result.triIndices = &((TriIndices *)(state->triIndices.memory))[
			state->triIndices.size
		];

		result.meshPoints = &((Vec *)(state->meshPoints.memory))[
			state->meshPoints.size
		];

		result.worldPoints = &state->worldPoints[
			state->meshPoints.size
		];

		int startPointsSize = state->meshPoints.size;
		SkipWhitespace(&fileMap);

		Pixel triColor = { 0 };

		while(fileMap.current < fileMap.end) {
			Literal startLiteral = GetLiteral(&fileMap);
			if(startLiteral.name == NULL) {
				goto earlyOut;
			}

			if(IsLiteralEqual(startLiteral, "point")) {
				Vec newPoint = { 0 };
				newPoint.w = 1;
				if(AssignFloats(
					&fileMap, 3,
					&newPoint.x,
					&newPoint.y,
					&newPoint.z
				) < 0) {
					goto earlyOut;
				}

				AppendToArena(&state->meshPoints, 1);
				result.pointsSize++;
				result.meshPoints[result.pointsSize - 1] = newPoint;
				result.worldPoints[result.pointsSize - 1] = newPoint;
				continue;
			}

			if(IsLiteralEqual(startLiteral, "tri")) {
				TriIndices newTriIndices = { 0 };
				newTriIndices.albedoColor = triColor;
				while(fileMap.current < fileMap.end) {
					uint8_t *origCurrent = fileMap.current;
					Literal paramLiteral = GetLiteral(&fileMap);
					if(IsLiteralEqual(paramLiteral, "points")) {
						if(AssignInts(
							&fileMap, 3,
							&newTriIndices.a,
							&newTriIndices.b,
							&newTriIndices.c
						) < 0) {
							goto earlyOut;
						}

						newTriIndices.a -= pointIndexOffset;
						newTriIndices.b -= pointIndexOffset;
						newTriIndices.c -= pointIndexOffset;

						newTriIndices.a += startPointsSize;
						newTriIndices.b += startPointsSize;
						newTriIndices.c += startPointsSize;
						continue;
					}

					if(IsLiteralEqual(paramLiteral, "color")) {
						int r = 0;
						int g = 0;
						int b = 0;
						if(AssignInts(
							&fileMap, 3,
							&r, &g, &b
						) < 0) {
							goto earlyOut;
						}

						triColor = PIXEL(r, g, b);
						newTriIndices.albedoColor = triColor;
						continue;
					}

					fileMap.current = origCurrent;
					break;
				}

				AppendToArena(&state->triIndices, 1);
				result.triIndicesSize++;
				result.triIndices[result.triIndicesSize - 1] = newTriIndices;
				continue;
			}

			SkipLine(&fileMap);
		}
	earlyOut: {
		CloseFileMap(&fileMap);
	}

	return result;
}

Mesh LoadMeshFromBinary(char *fileName, ProgramState *state) {
	Mesh result = { 0 };

	int startPointsSize = state->meshPoints.size;
	FileMap fileMap = OpenFileMap(fileName);
	if(fileMap.map == NULL) {
		return (Mesh) { 0 };
	}
		memcpy(&result.triIndicesSize, fileMap.current, sizeof(int));
		fileMap.current += sizeof(int);
		result.triIndices = &((TriIndices *)(state->triIndices.memory))[
			state->triIndices.size
		];

		AppendToArena(&state->triIndices, result.triIndicesSize);

		for(int i = 0; i < result.triIndicesSize; i++) {
			TriIndices triIndices = { 0 };
			memcpy(
				&triIndices,
				fileMap.current,
				sizeof(TriIndices)
			);

			fileMap.current += sizeof(TriIndices);

			triIndices.a += startPointsSize;
			triIndices.b += startPointsSize;
			triIndices.c += startPointsSize;

			triIndices.outputColor = PIXEL(0, 0, 0);
			result.triIndices[i] = triIndices;
		}

		memcpy(&result.pointsSize, fileMap.current, sizeof(int));
		fileMap.current += sizeof(int);
		result.meshPoints = &((Vec *)(state->meshPoints.memory))[
			state->meshPoints.size
		];

		result.worldPoints = &state->worldPoints[state->meshPoints.size];
		AppendToArena(&state->meshPoints, result.pointsSize);
		memcpy(
			result.meshPoints,
			fileMap.current,
			sizeof(Vec) * result.pointsSize
		);

		memcpy(
			result.worldPoints,
			fileMap.current,
			sizeof(Vec) * result.pointsSize
		);

		fileMap.current += sizeof(Vec) * result.pointsSize;
	earlyOut: {
		CloseFileMap(&fileMap);
	}

	return result;
}

static inline int minInt(int a, int b) {
	return a < b ? a : b;
}

void WriteBinaryFromMesh(Mesh *mesh, char *fileName) {
	int fileSize = (
		sizeof(int) + ( // tri indices size
			sizeof(TriIndices) * mesh->triIndicesSize
		) +

		sizeof(int) + ( // mesh points size
			sizeof(Vec) * mesh->pointsSize
		)
	);

	int decreaseTriIndicesBy = mesh->triIndices[0].a;
	for(int i = 0; i < mesh->triIndicesSize; i++) {
		decreaseTriIndicesBy = minInt(decreaseTriIndicesBy, mesh->triIndices[i].a);
		decreaseTriIndicesBy = minInt(decreaseTriIndicesBy, mesh->triIndices[i].b);
		decreaseTriIndicesBy = minInt(decreaseTriIndicesBy, mesh->triIndices[i].c);
	}

	FileMap fileMap = OpenWriteFileMap(fileName, fileSize);
	if(fileMap.map == NULL) {
		return;
	}
		memcpy(fileMap.current, &mesh->triIndicesSize, sizeof(int));
		fileMap.current += sizeof(int);
		memcpy(
			fileMap.current,
			mesh->triIndices,
			sizeof(TriIndices) * mesh->triIndicesSize
		);

		for(int i = 0; i < mesh->triIndicesSize; i++) {
			TriIndices triIndices = mesh->triIndices[i];
			triIndices.a -= decreaseTriIndicesBy;
			triIndices.b -= decreaseTriIndicesBy;
			triIndices.c -= decreaseTriIndicesBy;

			triIndices.outputColor = PIXEL(0, 0, 0);
			memcpy(
				fileMap.current,
				&triIndices,
				sizeof(TriIndices)
			);

			fileMap.current += sizeof(TriIndices);
		}

		memcpy(fileMap.current, &mesh->pointsSize, sizeof(int));
		fileMap.current += sizeof(int);
		memcpy(
			fileMap.current,
			mesh->meshPoints,
			sizeof(Vec) * mesh->pointsSize
		);

		fileMap.current += sizeof(Vec) * mesh->pointsSize;
	earlyOut: {
		CloseFileMap(&fileMap);
	}
}

void UpdateMesh(
	Mesh *mesh,
	Vec pos,
	Quaternion rot
) {
	IdentMat(&mesh->mat);

	mesh->pos = pos;
	Mat moveMat = { 0 };
	MoveMat(&moveMat, pos);
	MatXMat(&mesh->mat, &moveMat, &mesh->mat);

	mesh->rot = rot;
	Mat rotMat = { 0 };
	RotMat(&rotMat, rot);
	MatXMat(&mesh->mat, &rotMat, &mesh->mat);

	VecsXMat(
		mesh->meshPoints,
		mesh->worldPoints,
		mesh->pointsSize,
		&mesh->mat
	);
}

