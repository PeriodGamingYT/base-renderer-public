#include <base-renderer.h>

void FreePage(uint8_t **memory) {
	if(*memory == NULL) { return; }
	VirtualFree(*memory, 0, MEM_RELEASE);
	*memory = NULL;
}

uint8_t *InitPage(int bytes) {
	return VirtualAlloc(
		0,
		bytes,
		MEM_COMMIT,
		PAGE_READWRITE
	);
}

Arena InitArena(
	int bytes, int cap
) {
	return (Arena) {
		.memory = InitPage(cap * bytes),
		.bytes = bytes,
		.cap = cap
	};
}

uint8_t *AppendToArena(Arena *arena, int appendSize) {
	if(appendSize + arena->size >= arena->cap) {
		return NULL;
	}

	uint8_t *result = &arena->memory[arena->bytes * arena->size];
	arena->size += appendSize;
	return result;
}

void FreeArena(Arena *arena) {
	FreePage(&arena->memory);
	arena->size = 0;
	arena->cap = 0;
}

BOOL IsKeyDown(ProgramState *state, int keyCode) {
	return state->keys[keyCode];
}

