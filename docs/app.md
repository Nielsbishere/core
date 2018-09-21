# Introduction
![OCore architecture](docs/Osomi%20Core%20architecture.png)  
As shown above, the platforms could be expanded by just adding an entrypoint and implementing the platform-dependent code in owc.  
The required libraries (ostlc; Osomi STandard Library Core and owc; Osomi Window Core) are required for window creation and things like logging, error handling, input, etc.  
The optional libraries use owc (and ostlc) to add functionality that could also be created by the end-user, but are convenient to have. This includes rendering (ogc; Osomi Graphics Core), besides future implementations, such as sockets.  
A standalone is where you don't use the entrypoints and use it as an executable (pc only) instead of an application. An example of this is oish_gen (OsomI SHader generator) and oirm_gen (OsomI Raw Model generator).  
Now, the app is the entrypoint that is accessed by all platform-dependent entrypoints. It is where you setup all important requirements for the application you're using. Normally, this is where you setup a window.
# Example code
```cpp
void Application::instantiate(WindowHandleExt *param){
	Random::seedRandom();
	FileManager fmanager(param);
	WindowManager wmanager;
	Window *w = wmanager.create(WindowInfo(__PROJECT_NAME__, 1, param));
	w->setInterface(new MainInterface());
	wmanager.waitAll();
}
```
The static function Application::instantiate takes a platform-dependent handle and turns it into a Window. This handle is also used to create the FileManager (required by Android). Before that, we seed the random and create the WindowManager. At the end of the scope, the file manager singleton will be unset and the window manager will destroy all windows.  
WindowManager::create takes a WindowInfo struct and creates a Window from it, though it only uses the parameters given through the constructor (String name, u32 versionId, WindowHandleExt *handle, bool inFocus = true).  
After creating the window, we set the interface (which has all callback functions) to our custom interface. This will handle all Window's tasks, such as updating, rendering, handling input, etc. We then request the window manager to wait for our window to close and update it.
## MainInterface
The main interface is usually where the important user-code is stored. The way the interface is structured depends on what it inherrits from. If you inherrit from WindowInterface, you have to handle things like screen resizing, initializing/destroying a surface and rendering. However, if you're using ogc, you only have to provide higher level code. This includes, initializing and rendering a scene, initializing a scene surface and handling aspect changes. This can be extended yourself by an interface that helps you with those tasks.
```cpp
class MainInterface : public oi::gc::GraphicsInterface {

public:

	~MainInterface();
	void initScene() override;
	void renderScene() override;
	void onInput(oi::wc::InputDevice *device, oi::wc::Binding b, bool down) override;
	
	void load(oi::String path) override {}
	void save(oi::String path) override {}

	void update(f32 dt) override;
	void initSceneSurface() override;
	void onAspectChange(float asp) override;

	void refreshPlanet(Planet planet);
	void readPlanets(bool fromResource = false);
	void writePlanets();
  
};
```
The functions of the planet generator example are as follows:  
initScene is called before initSceneSurface and sets up all graphics objects that will be used, while initSceneSurface is called everytime the surface is resized (so don't forget to destroy objects you recreate).  
onInput is called for every key/button that is updated.  
load/save are mandatory functions that should serialize your interface to/from a file, so that the app can be restored if it was closed in the background (android requirement).  
update before every render. This is where you should update buffers and get render data ready. renderScene should only be used to make calls to the command list.  
onAspectChange should be used to initialize the cameras, this is called after the surface has been resized and only the cameras should be updated. This is not the same as a resize, because Android allows rotating without changing resolution (it just changes aspect).
