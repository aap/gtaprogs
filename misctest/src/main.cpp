#include "../../engine/gtaengine.h"
#include <sstream>
#include <GLFW/glfw3.h>
#include "misctest.h"

#include <json/json.h>

using namespace std;

bool running;
Shader *simpleShader;
Camera *cam;
Mesh *screenRect;
Material *screenMat;

GLuint Fbo;

FileDir *fileDir;
int game;


Json::Value config;


static int screen_width = 640, screen_height = 480;

static TexDict *vehicle;
static Clump *wheels;
static Car *car;
static MapObj *mapobj;


Vec4 envvector;


int
initIII(void)
{
	string dir = FileDir::correctPathCase("models/gta3.dir");
	string img = FileDir::correctPathCase("models/gta3.img");
	ifstream dirfile((basepath+dir).c_str(), ios::binary);
	if(dirfile.fail()){
		cerr << "can't open dir " << basepath+dir << endl;
		return 0;
	}
	fileDir->addFromFile(dirfile, img);
	dirfile.close();
	fileDir->addFile("particle.txd",
	                 FileDir::correctPathCase("models/particle.txd"), 0, 0);
	fileDir->addFile("misc.txd",
	                 FileDir::correctPathCase("models/misc.txd"), 0, 0);
	fileDir->addFile("wheels.dff",
		         FileDir::correctPathCase("models/generic/wheels.dff"),
	                 0, 0);

	// particle txd
	istream *in = fileDir->getHandle("particle.txd");
	if(in == 0){
		cerr << "can't open particle.txd\n";
		return 0;
	}
	rw::TextureDictionary *txd = new rw::TextureDictionary;
	txd->read(*in);
	fileDir->returnHandle(in);
	for(uint i = 0; i < txd->texList.size(); i++){
		if(txd->texList[i].platform == rw::PLATFORM_PS2)
			txd->texList[i].convertFromPS2(0x40);
		if(txd->texList[i].platform == rw::PLATFORM_XBOX)
			txd->texList[i].convertFromXbox();
		if(txd->texList[i].dxtCompression)
			txd->texList[i].decompressDxt();
		txd->texList[i].convertTo32Bit();
	}
	TexDict *mytxd = fromRwTxd(txd);
	INCREF(mytxd);
	delete txd;

	Texture *refmap = mytxd->getTex("reflection01");
	INCREF(refmap);
	DECREF(mytxd);

	car = new Car;
	istream *dffstrm = fileDir->getHandle("kuruma.dff");
	istream *txdstrm = fileDir->getHandle("kuruma.txd");
	if(dffstrm == 0 || txdstrm == 0){
		cerr << "couldn't load kuruma\n";
		return 0;
	}
	car->load(*dffstrm, *txdstrm);
	car->initHier("_hi");
	fileDir->returnHandle(dffstrm);
	fileDir->returnHandle(txdstrm);
	car->setRefmap(refmap);
	DECREF(refmap);
	car->setPrimColor(Vec4(58, 108, 96, 255)/255.0f);
	car->setSecColor(Vec4(245, 245, 245, 255)/255.0f);

	dffstrm = fileDir->getHandle("wheels.dff");
	txdstrm = fileDir->getHandle("misc.txd");
	wheels = loadClumpAndTxd(*dffstrm, *txdstrm);
	fileDir->returnHandle(dffstrm);
	fileDir->returnHandle(txdstrm);
	if(dffstrm == 0 || txdstrm == 0){
		cerr << "couldn't load wheels\n";
		return 0;
	}
	Atomic *wheelatm = wheels->getAtomic("wheel_saloon_l0");
	if(wheelatm)
		car->attachWheel(wheelatm, 0.7);

	mapobj = new MapObj;
//	dffstrm = fileDir->getHandle("lamppost2.dff");
//	txdstrm = fileDir->getHandle("dynsigns.txd");
//	dffstrm = fileDir->getHandle("building_fucked.dff");
//	txdstrm = fileDir->getHandle("italymisc.txd");
	dffstrm = fileDir->getHandle("trackshad08.dff");
	txdstrm = fileDir->getHandle("trackshad.txd");
	mapobj->load(*dffstrm, *txdstrm);
	fileDir->returnHandle(dffstrm);
	fileDir->returnHandle(txdstrm);
//	float dist[] = { 78, 250, 60 };
//	mapobj->initLevels(3, dist);
//	mapobj->setDamage(true);
	float dist[] = { 100 };
	mapobj->initLevels(1, dist);

	return 1;
}

int
initVC(void)
{
	string dir = FileDir::correctPathCase("models/gta3.dir");
	string img = FileDir::correctPathCase("models/gta3.img");
	ifstream dirfile((basepath+dir).c_str(), ios::binary);
	if(dirfile.fail()){
		cerr << "can't open dir " << basepath+dir << endl;
		return 0;
	}
	fileDir->addFromFile(dirfile, img);
	dirfile.close();
	fileDir->addFile("misc.txd",
	                 FileDir::correctPathCase("models/misc.txd"), 0, 0);
	fileDir->addFile("wheels.dff",
		         FileDir::correctPathCase("models/generic/wheels.dff"),
	                 0, 0);

	car = new Car;
	istream *dffstrm = fileDir->getHandle("admiral.dff");
	istream *txdstrm = fileDir->getHandle("admiral.txd");
	if(dffstrm == 0 || txdstrm == 0){
		cerr << "couldn't load admiral\n";
		return 0;
	}
	car->load(*dffstrm, *txdstrm);
	car->initHier("_hi");
	fileDir->returnHandle(dffstrm);
	fileDir->returnHandle(txdstrm);
	car->setPrimColor(Vec4(28, 55, 111, 255)/255.0f);
	car->setSecColor(Vec4(28, 55, 111, 255)/255.0f);

	dffstrm = fileDir->getHandle("wheels.dff");
	txdstrm = fileDir->getHandle("misc.txd");
	wheels = loadClumpAndTxd(*dffstrm, *txdstrm);
	fileDir->returnHandle(dffstrm);
	fileDir->returnHandle(txdstrm);
	if(dffstrm == 0 || txdstrm == 0){
		cerr << "couldn't load wheels\n";
		return 0;
	}
	Atomic *wheelatm = wheels->getAtomic("wheel_rim_l0");
	if(wheelatm)
		car->attachWheel(wheelatm, 0.7);

	mapobj = new MapObj;
	dffstrm = fileDir->getHandle("od_newscafe_dy.dff");
	txdstrm = fileDir->getHandle("od_hotels1.txd");
	mapobj->load(*dffstrm, *txdstrm);
	fileDir->returnHandle(dffstrm);
	fileDir->returnHandle(txdstrm);
	float dist[] = { 100 };
	mapobj->initLevels(1, dist);

	return 1;
}

int
initSA(void)
{
	envSpecShader = new Shader;
	if(envSpecShader->load("../shaders/sa_env_spec.vert", "../shaders/sa_env_spec.frag"))
		return 0;

	string img = FileDir::correctPathCase("models/gta3.img");
	ifstream imgfile((basepath+img).c_str(), ios::binary);
	if(imgfile.fail()){
		cerr << "can't open img " << basepath+img << endl;
		return 0;
	}
	fileDir->addFromFile(imgfile, img);
	imgfile.close();
	fileDir->addFile("vehicle.txd",
		         FileDir::correctPathCase("models/generic/vehicle.txd"),
	                 0, 0);

	istream *txdstrm = fileDir->getHandle("vehicle.txd");
        rw::TextureDictionary *txd = new rw::TextureDictionary;
        txd->read(*txdstrm);
        for(uint i = 0; i < txd->texList.size(); i++){
                if(txd->texList[i].platform == rw::PLATFORM_PS2)
                        txd->texList[i].convertFromPS2(0x40);
                if(txd->texList[i].platform == rw::PLATFORM_XBOX)
                        txd->texList[i].convertFromXbox();
                if(txd->texList[i].dxtCompression)
                        txd->texList[i].decompressDxt();
                txd->texList[i].convertTo32Bit();
        }
        vehicle = fromRwTxd(txd);
        INCREF(vehicle);
        delete txd;
	fileDir->returnHandle(txdstrm);

	car = new Car;
//	istream *dffstrm = fileDir->getHandle("bullet.dff");
//	txdstrm = fileDir->getHandle("bullet.txd");
	istream *dffstrm = fileDir->getHandle("tampa.dff");
	txdstrm = fileDir->getHandle("tampa.txd");
	if(dffstrm == 0 || txdstrm == 0){
		cerr << "couldn't load savanna\n";
		return 0;
	}
	car->load(*dffstrm, *txdstrm);
	car->initHier();
	fileDir->returnHandle(dffstrm);
	fileDir->returnHandle(txdstrm);
        FORALLATOMICS(it, car->getClump())
                (*it)->getMesh()->attachTexDict(vehicle);
// bullet
//	car->setPrimColor(Vec4(109, 40, 55, 255)/255.0f);
//	car->setSecColor(Vec4(164, 167, 165, 255)/255.0f);
// tampa
	car->setPrimColor(Vec4(96, 26, 35, 255)/255.0f);
	car->setSecColor(Vec4(109, 122, 136, 255)/255.0f);

	mapobj = new MapObj;
//	dffstrm = fileDir->getHandle("lae2_roads89.dff");
//	txdstrm = fileDir->getHandle("lae2roadshub.txd");
	dffstrm = fileDir->getHandle("hubst4alpha.dff");
	txdstrm = fileDir->getHandle("hub_alpha.txd");
	mapobj->load(*dffstrm, *txdstrm);
	fileDir->returnHandle(dffstrm);
	fileDir->returnHandle(txdstrm);
	float dist[] = { 100 };
	mapobj->initLevels(1, dist);

	return 1;
}

int
init(void)
{
	initEngine();
	glAlphaFunc(GL_GREATER, 0.0);

	simpleShader = new Shader;
	if(simpleShader->load("../shaders/simple.vert", "../shaders/simple.frag"))
		return 0;

	defaultShader = new Shader;
	if(defaultShader->load("../shaders/gta.vert", "../shaders/gta.frag"))
		return 0;

	envShader = new Shader;
	if(envShader->load("../shaders/vc_env.vert", "../shaders/vc_env.frag"))
		return 0;

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height,
	             0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint ztex;
	glGenTextures(1, &ztex);
	glBindTexture(GL_TEXTURE_2D, ztex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);


	GLfloat rect_verts[] = {
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};
	GLfloat rect_texcoords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
	};
	GLuint rect_indices[] = {
		0, 1, 2, 3
	};
	screenRect = new Mesh(Mesh::TriangleStrip,
	                      Mesh::Vertices | Mesh::TexCoords, 0, 4, 4);
	screenRect->incref();
	screenRect->gen();
	screenRect->setData(Mesh::Vertices, rect_verts);
	screenRect->setData(Mesh::TexCoords, rect_texcoords);
	screenRect->setIndices(rect_indices);
	screenMat = new Material(1);
	screenMat->shader = simpleShader;
	Texture *screenTex = new Texture("FBOTEX", tex);
	screenMat->setTexture(screenTex);
	screenRect->setMat(0, screenMat);


//	GLuint rbo;
//	glGenRenderbuffers(1, &rbo);
//	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
//	                      screen_width, screen_height);
//	GLint depth;
//	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
//	                             GL_RENDERBUFFER_DEPTH_SIZE, &depth);
//	cout << depth << endl;
//	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &Fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, Fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_2D, tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
	                       GL_TEXTURE_2D, ztex, 0);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
//	                          GL_RENDERBUFFER, rbo);

	int ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//	if(ret == GL_FRAMEBUFFER_COMPLETE)
//		cout << "framebuffer correct\n";
//	else
//		cout << "framebuffer error\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	fileDir = new FileDir;
	if((game&GameMask) == III){
		if(!initIII())
			return 0;
	}else if((game&GameMask) == VC){
		if(!initVC())
			return 0;
	}else if((game&GameMask) == SA){
		if(!initSA())
			return 0;
	}
	
	Mat4 &m = car->getClump()->getFrame()->getMatrix();
	m = Mat4::rotation(PI/2, Vec3(0,0,1)) * m;

	cam = new Camera;
	state = new State;

	cam->setAspectRatio(1.0f*screen_width/screen_height);
	cam->setNearFar(0.1f, 450.0f);
	cam->setTarget(Vec3(0.0f, 0.0f, 0.0f));
//	cam->setPosition(Vec3(0.0f, 10.0f, 0.0f));
//	cam->setPosition(Vec3(0.0f, -6.0f, 2.0f));
//	cam->setPosition(Vec3(8.12909, 3.38747, 4.35501));
	// tampa screen
	cam->setPosition(Vec3(-2.73501, 7.11621, 3.25182));

	envvector = Vec4(0.85496187, 0.0011143609, 1.0, 1.0);

	initState(state);
	defaultShader->use();
	state->updateLocs(*defaultShader);

	return 1;
}

void
render(void)
{
//	glBindFramebuffer(GL_FRAMEBUFFER, Fbo);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	defaultShader->use();
	state->updateLocs(*defaultShader);

	// 3d stuff
	cam->look();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.5f);
	glEnable(GL_BLEND);

//	state->vec4(AMBCOL, true)->val = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
//	state->vec4(AMBCOL, true)->val = Vec4(30, 30, 30, 255)/255.0f;
	state->vec4(AMBCOL, true)->val = Vec4(177, 177, 177, 255)/255.0f;
	state->vec4(ENVXFORM, true)->val = envvector;
//	Vec3 lightdir = Vec3(-1.0f, -1.0f, -1.0f).normalized();
	Vec3 lightdir = Vec3(0.5, 0.5, -sqrt(0.5));
	state->vec3(LIGHTDIR, true)->val = Vec3(state->mat4(MVMAT)->val * Vec4(lightdir));

	glVertexAttrib4f(IN_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
	glVertexAttrib3f(IN_NORMAL, 1.0f, 0.0f, 0.0f);
	axes->draw();

//	wheels->render();
//	mapobj->render(60);

	car->render();

	glAlphaFunc(GL_LESS, 0.5f);
	glDepthMask(GL_FALSE);
//	wheels->render();
//	mapobj->render(60);

	car->render();
	glDepthMask(GL_TRUE);
	glAlphaFunc(GL_GEQUAL, 0.5f);

	// 2d stuff

/*
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	state->mat4(PMAT,true)->val = Mat4::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
	state->mat4(MVMAT,true)->val = Mat4(1.0f);
	simpleShader->use();
	state->updateLocs(*simpleShader);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	glVertexAttrib4f(IN_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
	screenRect->draw();
*/
}

void
update(double)
{
}

void
cleanup(void)
{
	delete car;
	delete mapobj;
	DECREF(wheels);

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
readConfig(string filename, Json::Value &config)
{
	ifstream cf(filename.c_str());
	if(cf.fail())
		return 0;
	stringstream ss;
	ss << cf.rdbuf();
	cf.close();
	string json = ss.str();

	Json::Reader reader;
	bool success = reader.parse(json, config);
	return success;
}

int
main(int argc, char *argv[])
{
	GLenum status;

	if(argc < 2){
		cerr << "need a config as argument\n";
		return 1;
	}

	if(!readConfig(argv[1], config)){
		cerr << "Error: Could not get config\n";
		return 1;
	}
	game = 0;
	basepath = config["path"].asString();
	if(config["game"].asString() == "III")
		game |= III;
	else if(config["game"].asString() == "VC")
		game |= VC;
	else if(config["game"].asString() == "SA")
		game |= SA;
	if(config["platform"].asString() == "PS2")
		game |= PS2;
	else if(config["platform"].asString() == "PC")
		game |= PC;
	else if(config["platform"].asString() == "XBOX")
		game |= XBOX;

	if(!glfwInit()){
		cerr << "Error: could not initialize GLFW\n";
		return 1;
	}

// Canvas::createWindow()
	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	GLFWwindow *window = glfwCreateWindow(screen_width, screen_height, "Misc test", 0, 0);
	if(!window){
		cerr << "Error: could not create GLFW window\n";
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
//

// initGlew()

	status = glewInit();
	if(status != GLEW_OK){
		cerr << "Error: " << glewGetErrorString(status) << endl;
		return 1;
	}

	if(!GLEW_VERSION_2_0){
		cerr << "Error: OpenGL 2.0 needed\n";
		return 1;
	}
//

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

