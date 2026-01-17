#include <base-renderer.h>

ScreenVec RectToScreenVecSize(RECT rect) {
	return (ScreenVec) {
		rect.right - rect.left,
		rect.bottom - rect.top
	};
}

ScreenVec GetWindowSize(HWND windowHandle) {
	RECT clientRect = { 0 };
	GetClientRect(windowHandle, &clientRect);
	return RectToScreenVecSize(clientRect);
}

void CursorGoTo(ScreenVec newPos) {

	// this is done 3 times to imitate a workaround
	// that SDL does when Windows ignores the mouse cursor
	SetCursorPos(newPos.x, newPos.y);
	SetCursorPos(newPos.x + 1, newPos.y);
	SetCursorPos(newPos.x, newPos.y);
}
