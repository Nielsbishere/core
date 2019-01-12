# Introduction
![OCore architecture](Osomi%20Core%20architecture.png)  
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
	
	void load(oi::String path) override;
	void save(oi::String path) override;

	void update(f32 dt) override;
	void initSceneSurface() override;

	void refreshPlanet(Planet planet);
	void readPlanets(bool fromResource = false);
	void writePlanets();
    
	void refreshPlanetMesh(bool fromResource);
  
};
```
The functions of the planet generator example are as follows:  
initScene is called before initSceneSurface and sets up all graphics objects that will be used, while initSceneSurface is called everytime the surface is resized.  
onInput is called for every key/button that is updated.  
load/save are mandatory functions that should serialize your interface to/from a file, so that the app can be restored if it was closed in the background (android requirement).  
update before every render. This is where you should update buffers and get render data ready. renderScene should only be used to make calls to the command list.  
onAspectChange could be used to initialize the cameras, this is called after the surface has been resized and only the cameras should be updated. This is not the same as a resize, because Android allows rotating without changing resolution (it just changes aspect).

# FAQ

## Where are my resources?

Q. I added resources, but they don't show up in the final build?

A. Resources (paths starting with "res/") are all packaged into the final executable; .exe, .apk, etc. This means that you won't be able to modify them when you built your exe/apk, meaning that you have one self-containing executable with all resources attached. This means you can easily distribute versions for all different architectures without having to worry about missing resources. It also prevents people from easily modifying the resources themselves, since they will have to modify the exe and apk to do that. If you sign your executable, it will ensure that it's harder for third parties to change your assets. If you don't want that, you can copy resources from the packaged build to the "out/" directory. 

### Won't that affect file size & RAM?

Q. If you package all resources into apk/exe, won't that increase file size? And what about RAM?

A. Yeah, but the entire executable isn't loaded into RAM at the same time. This means that only when the resource is read, it will be present in memory (which is handled by the FileManager). The increased file size just means that the resources aren't available by the file explorer, making your resources more secure/prone to modifications. You will still be downloading the assets anyways, so whether or not it is in one file or multiple is not important; since the OS will load/unload parts of the executable anyways if they are required, which will probably be faster than loading files manually anyways.

### What about mods & downloadable content?

Q. If those resources are packaged into an apk or exe, does that mean our content is invisible to others? And what about modding or expansion packs?

A. Like specified in "Where are my resources?" the content is present in the executable, so it doesn't mean the content is invisible; just harder to modify/copy. If people really want to use your assets, they will; even if they are baked in our formats (since this project is open source, and even if it wasn't; look at Nintendo and how their file formats got reversed). The files are accessed through FileManager (included in Osomi Window Core; owc), which allows both read and readwrite files. The read files are packaged into the executable ("res/") and the write files are stored on the device's storage. This means you can still download/load/extract files to the device if you want to allow modification or want to add content. 

## Platform dependent names?

Q. How does the framework handle case sensitivity (between Windows and Linux).

A. Using the same names for files/directories is highly discouraged if working on a Linux distro. This is because Windows is case-insensitive, so packaging resources into an exe gets rid of the casing. However, in other executable formats (like apk), the resources will keep the same naming convention.

## Directories vs files?

Q. How does the framework handle directories and files? Seeing as files and folders can both have extensions, and files can also not have extensions.

A. getFileInfo will return the information about the file; including whether or not it is a file or folder; you should never rely on the extension or the file name to determine if it is. However, this ties into "Platform dependent names" and means that you can't have a file and folder named the same thing (case insensitive), as that will break platform-compatibility.