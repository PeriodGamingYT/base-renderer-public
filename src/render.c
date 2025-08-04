#include <render.h>

void RenderFrame(ProgramState *state) {
	ClearScreen(state);
	CameraRot cameraRot = CAMERA_ROT(
		state->camera.rot.yaw + ((float)(state->mouseMove.x) * 0.001),
		state->camera.rot.targetY + ((float)(state->mouseMove.y) * 0.001)
	);

	if(cameraRot.yaw > 2 * M_PI) { cameraRot.yaw = 0; }
	if(cameraRot.yaw < 0) { cameraRot.yaw = 2 * M_PI; }

	Mat cameraRotMat = { 0 };
	CameraRotOnlyMat(&cameraRotMat, cameraRot);
	Vec forwardCameraPos = VEC(
		0,
		0,
		-IsKeyDown(state, 'S') + IsKeyDown(state, 'W')
	);

	forwardCameraPos = VecXMat(forwardCameraPos, &cameraRotMat);

	Vec rightCameraPos = VEC(
		-IsKeyDown(state, 'D') + IsKeyDown(state, 'A'),
		0,
		0
	);

	Mat rightYawMat = { 0 };
	RotYMat(&rightYawMat, cameraRot.yaw + M_PI);
	rightCameraPos = VecXMat(rightCameraPos, &rightYawMat);

	Vec cameraPos = VEC_ADD(
		forwardCameraPos,
		rightCameraPos
	);

	cameraPos.y += (
		-IsKeyDown(state, VK_SPACE) +
		 IsKeyDown(state, VK_SHIFT)
	);

	int speedBoost = state->keys[VK_CONTROL] ? 15 : 0;
	float speed = state->frameTime * (2 + speedBoost);
	cameraPos = VEC_MUL(cameraPos, VEC(speed, speed, speed));
	cameraPos = VEC_ADD(
		cameraPos,
		state->camera.pos
	);

	UpdateCamera(
		&state->camera,
		cameraPos,
		cameraRot
	);

	// START DEMO UPDATE CODE SECTION
		// Mesh *mesh = (Mesh *)(&state->meshes.memory[0]);
		// UpdateMesh(
		// 	mesh,
		// 	mesh->pos,
		// 	EulerToQuaternion(VEC(
		// 		state->totalFrameTime,
		// 		state->totalFrameTime + 30,
		// 		state->totalFrameTime + 50
		// 	))
		// );
	// END DEMO UPDATE CODE SECTION

	VecsXMat(
		state->worldPoints,
		state->cameraPoints,
		state->meshPoints.size,
		&state->camera.cameraMat
	);

	TriIndices *triIndices = (TriIndices *)(&state->triIndices.memory[0]);
	for(int i = 0; i < state->triIndices.size; i++) {
		ComputeLightsForTri(triIndices, state);
		DrawTri(triIndices, state);
		triIndices++;
	}
}
