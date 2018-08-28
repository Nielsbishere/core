# Osomi Windows Core (owc)
## File io
Files are written though the FileManager; not std::ifstream or std::ofstream. This is because low-level, there might be different ways of reading/writing. This is especially true when using Android; because resources packed inside the APK are read-only and can't be read by std::ifstream. FileManager has the read and write function that take a path and buffer or string.  
The FileManager can be acquired by using the FileManager::get() function (if set up correctly).
### File paths
Because of the resource model on Android, we use an indicator before our file path. "out/" is when you want the file to be written and you want to read what you've written; it is an actual file on Android in the program's directory. "res/" is when you just want to read a file and writing to it can only work on PC (it's prevented by default; so please use out/ instead).
### Checking directories/files
If you want a directory path to exist, you can use the 'mkdir' function in FileManager. The fileExists, dirExists and exists functions are used to determine if something is already on the disk.
## Osomi String List (.oiSL)
oiSL is a file format that stores strings in an efficient way; it stores a keyset next to names (if it isn't the default keyset). By default; there is no keyset and it uses the default keyset of " 0-9A-Za-z.", which results into 6 bits per character. This might not be a big deal, since it only saves 2 bits per character, but it also helps to obfuscate/encode strings and keep them safe from modification. For information on implementation and file structures, go to docs/oiSL.md.
## Binding
A binding is a key on an InputDevice; it can be either a Key, a MouseButton, a MouseAxis, a ControllerButton or a ControllerAxis. These are implicitly cast if you need a binding (unless it's a controller, then you also need a controllerId). This binding class stores a binding as 32-bits (uint), allowing it to be sent quickly & stored efficiently. It can also be translated to a String and created from a String; which is very handy for allowing users to modify controls.
### Key
A Key refers to a key on the keyboard. This is even implemented on Android, so you can use keyboard input from any device that supports it. The keys are stored as an Osomi enum so can be found from String or just by value or compile-time. They are normally accessed as Key::...; ex. Key::Zero, Key::A, Key::Delete, Key::Volume_down, etc.  
As a Binding, the name is "x key"; so "Space key", "Delete key", "A key", "Zero key", etc.
### MouseButton
Works the same way as any Osomi enum but has different values. It only supports Left, Middle, Right, Back, Forward buttons.  
As a Binding, the name is "x mouse"; so "Left mouse", "Right mouse", etc.
### MouseAxis
The XY cursor position are stored as MouseAxis::X and MouseAxis::Y. The Mouse_wheel axis is adjusted by the mouse wheel.  
As a binding, the name is "x axis"; so "X axis", "Y axis", "Mouse_wheel axis"
### ControllerAxis & ControllerButton
Every axis on the controller has a different binding; Lx, Ly, Rx, Ry, L2, R2. Other keys are treated like a button.  
As a binding, the name is `x key/axis #<controllerId>`, so `Cross key #0` would be controller 0's cross key. `Lx axis #0`, etc.
## InputDevice
An InputDevice is something that can get input to our framework, this can be a mouse, keyboard or controller. The InputDevice is not necessarily responsible for updating the buttons/axes itself, but it does store them and have the ability to. On Android; InputDevices don't update any buttons (the Window does) and on Windows; the controller does update itself, but others are updated by callbacks as well.  
By using isDown, isUp, isPressed and isReleased, you can determine the state of a button, however this can be handled by a callback (which is faster). You can also get an axis of the InputDevice.
## InputHandler
The InputHandler stores the devices and allows you to get controllers, the keyboard or the mouse. However, these are not guaranteed to return anything, so they can return nullptr on different devices. Using callbacks through a WindowInterface is recommended.
## InputManager
The InputManager uses the InputHandler to abstract it under handles. You can manually bind axes or states under a name, or you can load it from a JSON.
## Windows
A Window is the simple wrapper around handling multi platform layers that can be drawn to. This doesn't setup a context or swapchain for you though, that is the responsibility for the user of this framework (or you could use ogc (Osomi Graphics Core) instead). The Window can't be sized by the API (for protections and cross platform reasons), but it is sized by the OS (Android or Windows). You're recommended to use only one Window, as multiple windows is supported on Windows, but nowhere else. Other events like focusing the app can't be done by the user either.
### WindowInterface
The most important part of a Window is the WindowInterface, which allows you to setup callbacks to Window events; such as update, render, input, resize, etc.  
The load and write function take a path that points to a file which should contain the intermediate data. This is useful, as on Android your app can be terminated because it's out of memory. Before your app is terminated, the save function is called and you should write essential data to the file path and load it when it's needed. These functions are required, even if you don't have any data to write; to encourage the implemention of these functions.  
The following functions can be overriden:
```cpp
void load(String path) override;    //required; load the data from disk
void save(String path) override;    //required; write the data to disk
void init() override;               //Called when the surface is first initialized
void initSurface() override;        //Called when the surface is initialized (resized or initialized)
void destroySurface() override;     //Called when the surface is destroyed
void onResize(Vec2u size) override; //Called when the surface is resized
void onMove(Vec2i pos) override;    //Called when the surface is moved
void onInput(InputDevice *device, Binding b, bool isDown) override;  //Called when a boolean binding has changed state
void onMouseMove(Vec2 pos);         //Called when the mouse moves (normalized coordinates [0, 1])
void onMouseWheel(f32 delta);       //Called when the mouse wheel moves
void onMouseDrag(Vec2 delta);       //Called when the left mouse (or touchscreen) is clicked and the mouse moves
void update(f32 delta);             //Called before rendering; with a delta time between frames
void render();                      //Called when a frame can be rendered (after render), only use this function for render-heavy functions, setup everything in update, not render.
void setFocus(bool isFocussed);     //Called when the focus is changed
```
Loop functions, such as update and render, are only called when the app is visible, otherwise they won't be called.
### WindowExt
In Osomi Core, Ext after a class name means that has different implementations. These implementations can be toggled by the build system. An example is WindowExt, which is WWindow on Windows and AWindow on Android. These structs have members that represent a window on the platform and static functions that handle callbacks for that platform.
### WindowManager
The WindowManager can be obtained by using the static 'get' function. This class handles the creation and storage of Windows. It is also how you create a Window.
### WindowAction
WindowAction is a request from our app to the OS to complete an action. These actions currently include 'IN_FOCUS' and 'FULL_SCREEN'; which change the focus to the app or make it full screen. These actions are just requests, the implementation on a different platform can just deny it. Android doesn't allow these actions, but Windows does. These actions can be toggled through WindowInfo.
### WindowInfo
The WindowInfo is for storing the dimensions and states of the actual window. You can request full screen and focus from here (though only supported on Windows).  
If you're rendering to a Window, know that the 'size' and 'position' of a Window can be in any space defined by the OS. On Windows, this includes window borders and on Android, the width and height of the window can be flipped (rotated device). If you want to acquire the space you can render to, it is (0, 0) to resolution (this can be acquired with 'getResolution' from WindowInfo). 
### Window
The Window class stores the manager it belongs to, the WindowInfo, WindowExt, WindowInterface, InputHandler, InputManager and a few timing/initializing variables.
## Creating a Window
```cpp
void Application::instantiate(WindowHandleExt *param){
	FileManager fmanager(param);
	WindowManager wmanager;
	Window *w = wmanager.create(WindowInfo("Test project", 1, param));
	w->setInterface(new MainInterface());
	wmanager.waitAll();
}
```
As seen above; you have to setup the FileManager using the WindowHandleExt, because in cases like Android, the app handle is required to access resources. The WindowManager will be available until the end of the scope, which will be until the program dies.  
The 'waitAll' function from WindowManager will wait until all windows have been closed and update them until they are. The window that we create has the title "Test project", version 1 and requires that window handle to function. We then set our interface, that inherrits from WindowInterface.
