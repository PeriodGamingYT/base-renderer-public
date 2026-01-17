#include <base-renderer.h>

// NOTE: 6.5.2.2.7 The ellipsis notation in a function prototype declarator causes
// argument type conversion to stop after the last declared parameter. The
// default argument promotions are performed on trailing arguments.
LightsSimd InitLightsSimd(float ambientLightAmount, ...) {
	LightsSimd result = {
		.ambientLightAmount = ambientLightAmount
	};

	int pointLightsSize = 0;
	int dirLightsSize = 0;

	// no initializer :(
	va_list args;
	va_start(args, ambientLightAmount); {
		while(TRUE) {
			LightType lightType = va_arg(args, LightType);
			if(lightType == LIGHT_TYPE_END) { break; }

			switch(lightType) {
				case LIGHT_TYPE_POINT: {
					pointLightsSize++;
					va_arg(args, double); // intensity
					va_arg(args, Vec); // pos
					break;
				}

				case LIGHT_TYPE_DIRECTIONAL: {
					dirLightsSize++;
					va_arg(args, double); // intensity
					va_arg(args, Vec); // dir
					break;
				}

				// unreachable
				default: break;
			}
		}
	} va_end(args);

	result.pointLightsSize = pointLightsSize;
	result.dirLightsSize = dirLightsSize;

	int pointLightsPropBytes = pointLightsSize * sizeof(float);
	result.pointLightsPosX = (float *)(InitPage(pointLightsPropBytes));
	result.pointLightsPosY = (float *)(InitPage(pointLightsPropBytes));
	result.pointLightsPosZ = (float *)(InitPage(pointLightsPropBytes));
	result.pointLightsIntensity = (float *)(InitPage(pointLightsPropBytes));

	int dirLightsPropBytes = dirLightsSize * sizeof(float);
	result.dirLightsDirX = (float *)(InitPage(dirLightsPropBytes));
	result.dirLightsDirY = (float *)(InitPage(dirLightsPropBytes));
	result.dirLightsDirZ = (float *)(InitPage(dirLightsPropBytes));
	result.dirLightsIntensity = (float *)(InitPage(dirLightsPropBytes));

	va_start(args, ambientLightAmount); {
		while(TRUE) {
			LightType lightType = va_arg(args, LightType);
			if(lightType == LIGHT_TYPE_END) { break; }

			int pointLightIndex = 0;
			int dirLightIndex = 0;
			switch(lightType) {
				case LIGHT_TYPE_POINT: {
					float intensity = (float)(va_arg(args, double));
					Vec pos = va_arg(args, Vec);

					result.pointLightsPosX[pointLightIndex] = pos.x;
					result.pointLightsPosY[pointLightIndex] = pos.y;
					result.pointLightsPosZ[pointLightIndex] = pos.z;
					result.pointLightsIntensity[pointLightIndex] = intensity;

					pointLightIndex++;
					break;
				}

				case LIGHT_TYPE_DIRECTIONAL: {
					float intensity = (float)(va_arg(args, double));
					Vec dir = va_arg(args, Vec);

					result.dirLightsDirX[dirLightIndex] = dir.x;
					result.dirLightsDirY[dirLightIndex] = dir.y;
					result.dirLightsDirZ[dirLightIndex] = dir.z;
					result.dirLightsIntensity[dirLightIndex] = intensity;

					dirLightIndex++;
					break;
				}

				// unreachable
				default: break;
			}
		}
	} va_end(args);

	return result;
}

void FreeLightsSimd(LightsSimd *lights) {
	lights->pointLightsSize = 0;
	FreePage((uint8_t **)(&lights->pointLightsPosX));
	FreePage((uint8_t **)(&lights->pointLightsPosY));
	FreePage((uint8_t **)(&lights->pointLightsPosZ));
	FreePage((uint8_t **)(&lights->pointLightsIntensity));

	lights->dirLightsSize = 0;
	FreePage((uint8_t **)(&lights->dirLightsDirX));
	FreePage((uint8_t **)(&lights->dirLightsDirY));
	FreePage((uint8_t **)(&lights->dirLightsDirZ));
	FreePage((uint8_t **)(&lights->dirLightsIntensity));

	lights->ambientLightAmount = 0;
}

float ComputeLights(
	LightsSimd *lights,
	Vec pos,
	Vec normal
) {
	float result = 0;

	__m256 posX = _mm256_set1_ps(pos.x);
	__m256 posY = _mm256_set1_ps(pos.y);
	__m256 posZ = _mm256_set1_ps(pos.z);

	__m256 normalX = _mm256_set1_ps(normal.x);
	__m256 normalY = _mm256_set1_ps(normal.y);
	__m256 normalZ = _mm256_set1_ps(normal.z);

	float *pointLightsPosXPointer = &lights->pointLightsPosX[0];
	float *pointLightsPosYPointer = &lights->pointLightsPosY[0];
	float *pointLightsPosZPointer = &lights->pointLightsPosZ[0];
	float *pointLightsIntensityPointer = (
		&lights->pointLightsIntensity[0]
	);

	// TODO: debug this, i suspect this might crash
	int i = 0;
	for(; i < lights->pointLightsSize - 8; i += 8) {
		__m256 pointLightPosX = _mm256_load_ps(pointLightsPosXPointer);
		__m256 pointLightPosY = _mm256_load_ps(pointLightsPosYPointer);
		__m256 pointLightPosZ = _mm256_load_ps(pointLightsPosZPointer);
		__m256 pointLightIntensity = _mm256_load_ps(
			pointLightsIntensityPointer
		);

		__m256 pointLightDiffX = _mm256_sub_ps(
			pointLightPosX, posX
		);

		__m256 pointLightDiffY = _mm256_sub_ps(
			pointLightPosY, posY
		);

		__m256 pointLightDiffZ = _mm256_sub_ps(
			pointLightPosZ, posZ
		);

		__m256 distanceRecip =_mm256_rcp_ps(
			_mm256_sqrt_ps(_mm256_add_ps(
				_mm256_add_ps(
					_mm256_mul_ps(pointLightDiffX, pointLightDiffX),
					_mm256_mul_ps(pointLightDiffY, pointLightDiffY)
				),

				_mm256_mul_ps(pointLightDiffZ, pointLightDiffZ)
			))
		);

		__m256 pointLight = _mm256_mul_ps(
			_mm256_mul_ps(
				pointLightIntensity,
				_mm256_add_ps(
					_mm256_add_ps(
						_mm256_mul_ps(
							normalX,
							_mm256_mul_ps(
								pointLightDiffX, distanceRecip
							)
						),

						_mm256_mul_ps(
							normalY,
							_mm256_mul_ps(
								pointLightDiffY, distanceRecip
							)
						)
					),

					_mm256_mul_ps(
						normalZ,
						_mm256_mul_ps(
							pointLightDiffZ, distanceRecip
						)
					)
				)
			),

			distanceRecip
		);

		__m128 pointLightSum = _mm_add_ps(
			_mm256_castps256_ps128(pointLight),
			_mm256_extractf128_ps(pointLight, 1)
		);

		pointLightSum = _mm_hadd_ps(pointLightSum, pointLightSum);
		pointLightSum = _mm_hadd_ps(pointLightSum, pointLightSum);

		result += _mm_cvtss_f32(pointLightSum);

		pointLightsPosXPointer += 8;
		pointLightsPosYPointer += 8;
		pointLightsPosZPointer += 8;
		pointLightsIntensityPointer += 8;
	}

	for(; i < lights->pointLightsSize; i++) {
		Vec pointLightPos = VEC(
			lights->pointLightsPosX[i],
			lights->pointLightsPosY[i],
			lights->pointLightsPosZ[i]
		);

		float distance = VecLength(VEC_SUB(
			pos, pointLightPos
		));

		result += (
			lights->pointLightsIntensity[i] * VecDot(
				normal,
				VecNormal(VEC_SUB(
					pointLightPos, pos
				))
			)
		) / distance;
	}


	float *dirLightsDirXPointer = &lights->dirLightsDirX[0];
	float *dirLightsDirYPointer = &lights->dirLightsDirY[0];
	float *dirLightsDirZPointer = &lights->dirLightsDirZ[0];
	float *dirLightsIntensityPointer = (
		&lights->dirLightsIntensity[0]
	);

	// TODO: debug this, i suspect this might crash
	i = 0;
	for(; i < lights->dirLightsSize - 8; i += 8) {
		__m256 dirLightDirX = _mm256_load_ps(dirLightsDirXPointer);
		__m256 dirLightDirY = _mm256_load_ps(dirLightsDirYPointer);
		__m256 dirLightDirZ = _mm256_load_ps(dirLightsDirZPointer);
		__m256 dirLightIntensity = _mm256_load_ps(
			dirLightsIntensityPointer
		);

		// _mm256_fmadd_ps isn't supported on my machine :(
		__m256 dirLight = _mm256_mul_ps(
			dirLightIntensity,
			_mm256_add_ps(
				_mm256_add_ps(
					_mm256_mul_ps(normalX, dirLightDirX),
					_mm256_mul_ps(normalY, dirLightDirY)
				),

				_mm256_mul_ps(normalZ, dirLightDirZ)
			)
		);

		__m128 dirLightSum = _mm_add_ps(
			_mm256_castps256_ps128(dirLight),
			_mm256_extractf128_ps(dirLight, 1)
		);

		dirLightSum = _mm_hadd_ps(dirLightSum, dirLightSum);
		dirLightSum = _mm_hadd_ps(dirLightSum, dirLightSum);

		result += _mm_cvtss_f32(dirLightSum);

		dirLightsDirXPointer += 8;
		dirLightsDirYPointer += 8;
		dirLightsDirZPointer += 8;
		dirLightsIntensityPointer += 8;
	}

	for(; i < lights->dirLightsSize; i++) {
		result += lights->dirLightsIntensity[i] * VecDot(
			normal,
			VEC(
				lights->dirLightsDirX[i],
				lights->dirLightsDirY[i],
				lights->dirLightsDirZ[i]
			)
		);
	}

	return min(
		max(0, result + lights->ambientLightAmount), 1
	);
}

void ComputeLightsForTri(TriIndices *triIndices, ProgramState *state) {
	Vec triPointA = state->worldPoints[triIndices->a];
	Vec triPointB = state->worldPoints[triIndices->b];
	Vec triPointC = state->worldPoints[triIndices->c];

	Vec pos = triPointA;
	Vec normal = VecNormal(VecCross(
		VEC_SUB(triPointB, triPointA),
		VEC_SUB(triPointC, triPointA)
	));

	float lightAmount = ComputeLights(&state->lights, pos, normal);
	triIndices->outputColor = PIXEL(
		triIndices->albedoColor.r * lightAmount,
		triIndices->albedoColor.g * lightAmount,
		triIndices->albedoColor.b * lightAmount
	);
}
