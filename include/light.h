#ifndef __LIGHT_H
#define __LIGHT_H
	#include <mesh.h>

	typedef enum {
		LIGHT_TYPE_DIRECTIONAL,
		LIGHT_TYPE_POINT,
		LIGHT_TYPE_AMBIENT
	} LightType;

	typedef struct PointLightStruct {
		Vec pos;
	} PointLight;

	typedef struct DirectionalLightStruct {
		Vec direction;
	} DirectionalLight;

	typedef struct LightStruct {
		LightType type;
		union {
			PointLight point;
			DirectionalLight directional;
			// ambient
		};

		float intensity;
	} Light;

	float ComputeLight(Light *, Vec, Vec);
	void ComputeLightsForTri(TriIndices *, ProgramState *);
#endif
