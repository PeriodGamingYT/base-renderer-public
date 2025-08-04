#include <light.h>

// this will probably be converted to simd at some point
// and have every single light be put into their own seperate arenas
float ComputeLight(Light *light, Vec pos, Vec normal) {
	switch(light->type) {
		case LIGHT_TYPE_DIRECTIONAL: {
			return (
				light->intensity * VecDot(
					normal,
					light->directional.direction
				)
			);
		}

		case LIGHT_TYPE_POINT: {
			float distance = VecLength(VEC_SUB(
				pos,
				light->point.pos
			));

			return (
				light->intensity * VecDot(
					normal,
					VecNormal(VEC_SUB(light->point.pos, pos))
				)
			) / distance;
		}

		case LIGHT_TYPE_AMBIENT: {
			return light->intensity;
		}

		// shouldn't happen
		default: { return 0; }
	}

	// can't happen
	return 0;
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

	float result = 0;
	Light *currentLight = (Light *)(&state->lights.memory[0]);

	// if there are enough lights then change this so something more efficient
	for(int i = 0; i < state->lights.size; i++) {
		result += max(0, ComputeLight(currentLight, pos, normal));
		currentLight++;
	}

	if(result > 1) { result = 1; }
	triIndices->outputColor = PIXEL(
		triIndices->albedoColor.r * result,
		triIndices->albedoColor.g * result,
		triIndices->albedoColor.b * result
	);
}
