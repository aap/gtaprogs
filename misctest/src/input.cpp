#include "../../engine/gtaengine.h"
#include <GLFW/glfw3.h>
#include "misctest.h"

using namespace std;

// mouse
static int lastX, lastY;
static int clickX, clickY;

// keyboard
static Vec3 kbDir;
static bool isLDown, isMDown, isRDown;
static bool isShiftDown, isCtrlDown, isAltDown;

void
readDS3Input(Pad &pad)
{
	int naxes, nbuttons;
	const float *axes = glfwGetJoystickAxes(pad.id, &naxes);
	const unsigned char *buttons = glfwGetJoystickButtons(pad.id, &nbuttons);
	pad.prevButtons = pad.buttons;
	pad.axes[0] = axes[0];
	pad.axes[1] = -axes[1];
	pad.axes[2] = axes[2];
	pad.axes[3] = -axes[3];
	for(int i = 0; i < 4; i++)
		if(pad.axes[i] < 0.1f && pad.axes[i] > -0.1f)
			pad.axes[i] = 0.0f;
	int shift = 0;
	pad.buttons = 0;
	// O, X, SQ, TRI
	pad.buttons |= buttons[13] << shift++;
	pad.buttons |= buttons[14] << shift++;
	pad.buttons |= buttons[15] << shift++;
	pad.buttons |= buttons[12] << shift++;
	// L2, R2, L1, R1
	pad.buttons |= buttons[8] << shift++;
	pad.buttons |= buttons[9] << shift++;
	pad.buttons |= buttons[10] << shift++;
	pad.buttons |= buttons[11] << shift++;
	// SELECT, L3, R3, START
	pad.buttons |= buttons[0] << shift++;
	pad.buttons |= buttons[1] << shift++;
	pad.buttons |= buttons[2] << shift++;
	pad.buttons |= buttons[3] << shift++;
	// DPAD UP, RIGHT, DOWN, LEFT
	pad.buttons |= buttons[4] << shift++;
	pad.buttons |= buttons[5] << shift++;
	pad.buttons |= buttons[6] << shift++;
	pad.buttons |= buttons[7] << shift++;
}

void
readKeyboard(GLFWwindow *w)
{
	float dist = 0.5f;

	isShiftDown = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
	isCtrlDown = glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
	isAltDown = glfwGetKey(w, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;

	if(glfwGetKey(w, 'W')){
		if(isShiftDown)
			cam->dolly(dist);
		else
			cam->zoom(dist);
	}

	if(glfwGetKey(w, 'S')){
		if(isShiftDown)
			cam->dolly(-dist);
		else
			cam->zoom(-dist);
	}

        int u, r, d, l;
        u = glfwGetKey(w, GLFW_KEY_UP);
        r = glfwGetKey(w, GLFW_KEY_RIGHT);
        d = glfwGetKey(w, GLFW_KEY_DOWN);
        l = glfwGetKey(w, GLFW_KEY_LEFT);
        if(u || r || d || l){
                kbDir = Vec3(0.0f, 1.0f, 0.0f)*u +
                        Vec3(1.0f, 0.0f, 0.0f)*r +
                        Vec3(0.0f, -1.0f, 0.0f)*d +
                        Vec3(-1.0f, 0.0f, 0.0f)*l;
                float n = kbDir.norm();
                if(n > 0.1)
                        kbDir /= n;
                else
                        kbDir = Vec3(0.0f, 0.0f, 0.0f);
        }else{
                kbDir = Vec3(0.0f, 0.0f, 0.0f);
        }
}

void
keypress(GLFWwindow *w, int key, int scancode, int action, int mods)
{
	if(action != GLFW_PRESS)
		return;

	switch(key){
	case 'C':
		cout << cam->getPosition() SP cam->getTarget() << endl;
		break;
	case 'Q':
	case GLFW_KEY_ESCAPE:
		running = false;
		break;
	default:
		break;
	}
}

void
mouseButton(GLFWwindow *w, int button, int action, int mods)
{
	double x, y;
	glfwGetCursorPos(w, &x, &y);
	if(action == GLFW_PRESS){
		lastX = clickX = x;
		lastY = clickY = y;
		if(button == GLFW_MOUSE_BUTTON_LEFT)
			isLDown = true;
		if(button == GLFW_MOUSE_BUTTON_MIDDLE)
			isMDown = true;
		if(button == GLFW_MOUSE_BUTTON_RIGHT)
			isRDown = true;
	}else if(action == GLFW_RELEASE){
		if(button == GLFW_MOUSE_BUTTON_LEFT)
			isLDown = false;
		if(button == GLFW_MOUSE_BUTTON_MIDDLE)
			isMDown = false;
		if(button == GLFW_MOUSE_BUTTON_RIGHT)
			isRDown = false;
	}
}

void
mouseMotion(GLFWwindow *w, double x, double y)
{
	GLfloat dx, dy;
	static int xoff = 0, yoff = 0;
	static bool wrappedLast = false;
	int width, height;

	glfwGetWindowSize(w, &width, &height);

	dx = float(lastX - x) / float(width);
	dy = float(lastY - y) / float(height);
	/* Wrap the mouse if it goes over the window border.
	 * Unfortunately, after glfwSetMousePos is done, there can be old
	 * events with an old mouse position,
	 * hence the check if the pointer was wrapped the last time. */
	if((isLDown || isMDown || isRDown) &&
	    (x < 0 || y < 0 || x >= width || y >= height)){
		if(wrappedLast){
			dx = float(lastX-xoff - x) / float(width);
			dy = float(lastY-yoff - y) / float(height);
		}
		xoff = yoff = 0;
		while (x+xoff >= width) xoff -= width;
		while (y+yoff >= height) yoff -= height;
		while (x+xoff < 0) xoff += width;
		while (y+yoff < 0) yoff += height;
		glfwSetCursorPos(w, x+xoff, y+yoff);
		wrappedLast = true;
	}else{
		wrappedLast = false;
		xoff = yoff = 0;
	}
	lastX = x+xoff;
	lastY = y+yoff;
	if(isLDown){
		if(isShiftDown)
			cam->turn(dx*2.0f, dy*2.0f);
		else
			cam->orbit(dx*2.0f, -dy*2.0f);
	}
	if(isMDown){
		if(isShiftDown)
			;
		else
			cam->pan(dx*8.0f, -dy*8.0f);
	}
	if(isRDown){
		if(isShiftDown)
			;
		else
			cam->zoom(dx*12.0f);
	}
}

void
mouseWheel(GLFWwindow *w, double x, double y)
{
	const float dist = 2.0f;
	static double lastX = 0;

	double diff = x - lastX;
	lastX = x;

	if (isShiftDown)
		cam->dolly(dist*diff);
	else
		cam->zoom(-dist*diff);
}

void
dispatchInput(Pad &pad)
{
	if(pad.id >= 0){
		if(pad.buttons & Pad::Button_L2)
			cam->zoom(-0.1);
		if(pad.buttons & Pad::Button_R2)
			cam->zoom(0.1);
	}
}
