extern bool   running;
extern Shader *shader;
extern Camera *cam;
extern Mesh   *axes;

enum {
	III = 0x00,
	VC  = 0x01,
	SA  = 0x02,
	GameMask = 0x0F
};

enum {
	PS2  = 0x00,
	PC   = 0x10,
	XBOX = 0x20,
	PlatformMask = 0xF0
};

struct Pad
{
	enum {
		Button_O =    (1<<0),
		Button_X =    (1<<1),
		Button_SQ =   (1<<2),
		Button_TRI =  (1<<3),
		Button_L2 =   (1<<4),
		Button_R2 =   (1<<5),
		Button_L1 =   (1<<6),
		Button_R1 =   (1<<7),
		Button_SEL =  (1<<8),
		Button_L3 =   (1<<9),
		Button_R3 =   (1<<10),
		Button_START= (1<<11),
		Button_UP =   (1<<12),
		Button_RIGHT= (1<<13),
		Button_DOWN = (1<<14), 
		Button_LEFT = (1<<15)
	};
	int id;

	float axes[4];
	int prevButtons;
	int buttons;
};              

void readDS3Input(Pad &pad);

void readKeyboard(GLFWwindow *w);
void keypress(GLFWwindow *w, int key, int scancode, int action, int mods);
void mouseButton(GLFWwindow *w, int button, int action, int mods);
void mouseMotion(GLFWwindow *w, double x, double y);
void mouseWheel(GLFWwindow *w, double x, double y);
void dispatchInput(Pad &pad);

void nextanim(int g, int p);
