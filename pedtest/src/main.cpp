#include "../../engine/gtaengine.h"
#include <GLFW/glfw3.h>
#include "pedtest.h"

using namespace std;

// needed by engine
Texture *refmap;


bool running;
Camera *cam;
Ped *playerped;


static int screen_width = 800, screen_height = 600;

static Clump *player[3][3];
static AnimPackage animlist[3][3];
static AnimPackage::iterator animit[3][3];
static AnimInterpolator *interp[3][3];

enum {
	III = 0,
	VC,
	SA
};

enum {
	PS2 = 0,
	PC,
	XBOX
};

enum {
	DFF = 0,
	TXD,
	IFP,
	ANIMNAME
};

const char *paths[3][3][4] = {
	{
		{ "/home/aap/gamedata/ps2/gta3/MODELS/gta3_archive/player.DFF",
		  "/home/aap/gamedata/ps2/gta3/MODELS/gta3_archive/PLAYER.TXD",
		  "/home/aap/gamedata/ps2/gta3/ANIM/PED.IFP",
		  "walk_player" },
		{ "/home/aap/gamedata/pc/gta3/models/gta3_archive/player.DFF",
		  "/home/aap/gamedata/pc/gta3/models/gta3_archive/PLAYER.TXD",
		  "/home/aap/gamedata/pc/gta3/anim/ped.ifp",
		  "walk_player" },
		{ "/home/aap/gamedata/xbox/gta3/models/gta3_archive/player.DFF",
		  "/home/aap/gamedata/xbox/gta3/models/gta3_archive/PLAYER.TXD",
		  "/home/aap/gamedata/xbox/gta3/anim/ped.ifp",
		  "walk_player" }
	},
	{
		{ "/home/aap/gamedata/ps2/gtavc/MODELS/gta3_archive/player.DFF",
		  "/home/aap/gamedata/ps2/gtavc/MODELS/gta3_archive/player.txd",
//		{ "/home/aap/gamedata/ps2/gtavc/MODELS/gta3_archive/CSplay.dff",
//		  "/home/aap/gamedata/ps2/gtavc/MODELS/gta3_archive/CSplay.txd",
		  "/home/aap/gamedata/ps2/gtavc/ANIM/PED.IFP",
		  "walk_player" },
		{ "/home/aap/gamedata/pc/gtavc/models/gta3_archive/player.dff",
		  "/home/aap/gamedata/pc/gtavc/models/gta3_archive/player.txd",
		  "/home/aap/gamedata/pc/gtavc/anim/ped.ifp",
		  "walk_player" },
		{ "/home/aap/gamedata/xbox/gtavc/models/gta3_archive/player.DFF",
		  "/home/aap/gamedata/xbox/gtavc/models/gta3_archive/player.txd",
		  "/home/aap/gamedata/xbox/gtavc/anim/ped.ifp",
		  "walk_player" }
	},
	{
		{ "/home/aap/gamedata/ps2/gtasa/models/gta3_archive/cesar.dff",
		  "/home/aap/gamedata/ps2/gtasa/models/gta3_archive/cesar.txd",
		  "/home/aap/gamedata/ps2/gtasa/anim/ped.ifp",
		  "walk_player" },
		{ "/home/aap/gamedata/pc/gtasa/models/gta3_archive/cesar.dff",
		  "/home/aap/gamedata/pc/gtasa/models/gta3_archive/cesar.txd",
		  "/home/aap/gamedata/pc/gtasa/anim/ped.ifp",
		  "walk_player" },
		{ "/home/aap/gamedata/xbox/gtasa/models/gta3_archive/cesar.dff",
		  "/home/aap/gamedata/xbox/gtasa/models/gta3_archive/cesar.txd",
		  "/home/aap/gamedata/xbox/gtasa/anim/ped.ifp",
		  "walk_player" }
	}
};

int testobjects[][2] = {
	{ VC, PS2 },
	{ III, PS2 },
	{ SA, PS2 },
	{ III, XBOX },
	{ VC, XBOX },
	{ SA, XBOX },
	{ III, PC },
	{ VC, PC },
	{ SA, PC },
};
int ntestobjects = 9;

void
animupdate(void)
{
	for(int i = 0; i < ntestobjects; i++){
		int g = testobjects[i][0], p = testobjects[i][1];
		RefFrame *f;
		FORALLCHILDREN(it, player[g][p]->getFrame()){
			f = *it;
			break;
		}
		applyIfpAnim(f, interp[g][p]);
	}
}

void
animload(void)
{
	ifstream ifp;

	for(int i = 0; i < ntestobjects; i++){
		int g = testobjects[i][0], p = testobjects[i][1];
		ifp.open(paths[g][p][IFP], ios::binary);
		animlist[g][p] = readIfp(ifp);
		ifp.close();

		for(animit[g][p] = animlist[g][p].begin();
		    animit[g][p] != animlist[g][p].end(); animit[g][p]++)
			if(animit[g][p]->first == paths[g][p][ANIMNAME])
				break;
		interp[g][p] = new AnimInterpolator();
		interp[g][p]->setAnimation(animit[g][p]->second);
		interp[g][p]->reset();
		interp[g][p]->loop = true;
	}

//	for(AnimPackage::iterator it = animlist[VC][PS2].begin(); it != animlist[VC][PS2].end(); it++)
//		cout << it->first << endl;

	animupdate();
}

void
movegeo(Clump *clump)
{
	Atomic *atm;
	NodeHierarchy *hier;
	FORALLATOMICS(it, clump){
		if((hier = (atm = *it)->getHierarchy()))
			break;
	}
	if(hier == 0)
		return;
//	cout << "found atomic:" SP atm->getFrame()->name << endl;
	RefFrame *f = clump->getFrame();
	atm->setFrame(f);
}

int
init(void)
{
	initEngine();

	defaultShader = new Shader;
	if(defaultShader->load("../shaders/gta.vert", "../shaders/gta.frag"))
		return 0;

	IfpInterpInfo *info = new IfpInterpInfo;
	info->doRegister();
	FrameAnimation::interpInfo = info;

	int g, p;
	for(int i = 0; i < ntestobjects; i++){
		g = testobjects[i][0];
		p = testobjects[i][1];

		player[g][p] = loadClumpAndTxd(paths[g][p][DFF], paths[g][p][TXD]);
		if(g == SA || p == XBOX)
			movegeo(player[g][p]);
		player[g][p]->getFrame()->getMatrix() = Mat4(1.0f);
		player[g][p]->getFrame()->getMatrix() *=Mat4::translation(Vec3(g*2.0f,-p*3.0f, 0.0f));
	}
	animload();

	g = VC; p = PS2;
	playerped = new Ped;
	playerped->setClump(loadClumpAndTxd(paths[g][p][DFF], paths[g][p][TXD]));
	playerped->animGroup = g == SA ? 1 : 0;
	PedState::tmp_anpk = &animlist[g][p];
	playerped->initController();

	cam = new Camera;
	state = new State;

	cam->setPosition(Vec3(0.0f, 10.0f, 0.0f));
	cam->setTarget(Vec3(2.0f, -3.0f, 0.0f));
	cam->setAspectRatio(1.0f*screen_width/screen_height);

	initState(state);
	defaultShader->use();
        state->updateLocs(*defaultShader);

	return 1;
}

void
render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cam->setTarget(Vec3(&playerped->getClump()->getFrame()->getMatrix(false).e[3][0]));
	cam->look();

	state->vec4(AMBCOL, true)->val = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
	Vec3 lightdir = Vec3(-1.0f, -1.0f, -1.0f).normalized();
	state->vec3(LIGHTDIR, true)->val = Vec3(state->mat4(MVMAT)->val * Vec4(lightdir));

	glVertexAttrib4f(IN_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
	glVertexAttrib3f(IN_NORMAL, 1.0f, 0.0f, 0.0f);
	axes->draw();

	for(int i = 0; i < ntestobjects; i++){
		int g = testobjects[i][0], p = testobjects[i][1];
		player[g][p]->render();
	}
	animupdate();

	playerped->render();
}

void
update(double time)
{
	for(int i = 0; i < ntestobjects; i++){
		int g = testobjects[i][0], p = testobjects[i][1];
		interp[g][p]->addTime(time);
	}
	playerped->addTime(time);
}

void
cleanup(void)
{
	delete playerped;

	delete cam;
	delete state;
	defaultShader->destroy();
	delete defaultShader;

	closeEngine();

/*
	cout << Clump::s__numInstances << endl;
	cout << Atomic::s__numInstances << endl;
	cout << Mesh::s__numInstances << endl;
	cout << RefFrame::s__numInstances << endl;
	cout << Material::s__numInstances << endl;
	cout << TexDict::s__numInstances << endl;
	cout << Texture::s__numInstances << endl;
*/
}



// Display

void
resize(GLFWwindow*, int width, int height)
{
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
	cam->setAspectRatio(1.0f*screen_width/screen_height);
}

void
closewindow(GLFWwindow*)
{
	running = false;
}

int
main(int, char *[])
{
	GLenum status;

	if(!glfwInit()){
		cerr << "Error: could not initialize GLFW\n";
		return 1;
	}
	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	GLFWwindow *window = glfwCreateWindow(640, 480, "Ped test", 0, 0);
	if(!window){
		cerr << "Error: could not create GLFW window\n";
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);

	status = glewInit();
	if(status != GLEW_OK){
		cerr << "Error: " << glewGetErrorString(status) << endl;;
		return 1;
	}

	if(!GLEW_VERSION_2_0){
		cerr << "Error: OpenGL 2.0 needed\n";
		return 1;
	}

        GLint depth;
        glGetIntegerv(GL_DEPTH_BITS, &depth);
        cout << "depth: " << depth << endl;

	Pad pad;
	void (*readPad)(Pad&) = 0;

	pad.id = -1;
	if(glfwJoystickPresent(GLFW_JOYSTICK_1)){
		pad.id = GLFW_JOYSTICK_1;
		string padname = glfwGetJoystickName(pad.id);
		if(padname == "Sony PLAYSTATION(R)3 Controller")
			readPad = readDS3Input;
	}

	if(init()){
		glfwSetMouseButtonCallback(window, mouseButton);
		glfwSetCursorPosCallback(window, mouseMotion);
		glfwSetScrollCallback(window, mouseWheel);
		glfwSetKeyCallback(window, keypress);

		glfwSetWindowSizeCallback(window, resize);
		glfwSetWindowCloseCallback(window, closewindow);

		running = true;
		double time, lastTime = glfwGetTime();
		while(running){
			time = glfwGetTime();
	                readKeyboard(window);
			if(readPad)
				readPad(pad);

			dispatchInput(pad);

			update(time-lastTime);
			render();

			lastTime = time;

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	cleanup();
	glfwTerminate();
	return 0;
}

