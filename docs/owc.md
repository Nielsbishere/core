# Osomi Windows Core (owc)
## File io
Files are written though the FileManager; not std::ifstream or std::ofstream. This is because low-level, there might be different ways of reading/writing. This is especially true when using Android; because resources packed inside the APK are read-only and can't be read from using std::ifstream. FileManager has the read and write function that take a path and buffer or string.  
The FileManager can be acquired by using the FileManager::get() function (if set up correctly).
### File paths
Because of the resource model on Android, we use an indicator before our file path. "out/" is when you want the file to be written and you want to read what you've written; it is an actual file on Android in the program's directory. "res/" is when you just want to read a file and writing to it can only work on PC (it's prevented by default; so please use out/ instead).
### Checking directories/files
If you want a directory path to exist, you can use the 'mkdir' function in FileManager. The fileExists, dirExists and exists functions are used to determine if something is already on the disk.
## Osomi String List (.oiSL)
oiSL is a file format that stores strings in an efficient way; it stores a keyset next to names (if it isn't the default keyset). By default; there is no keyset and it uses the default keyset of " 0-9A-Za-z.", which results into 6 bits per character. This might not be a big deal, since it only saves 2 bits per character, but it also helps to obfuscate/encode strings and keep them safe from modification.
## Binding
A binding is a key on an InputDevice; it can be either a Key, a MouseButton, a MouseAxis, a ControllerButton or a ControllerAxis. These are implicitly cast if you need a binding (unless it's a controller, then you also need a controllerId). This binding class stores a binding as 32-bits (uint), allowing it to be sent quickly & stored efficiently. It can also be translated to a String and created from a String; which is very handy for allowing users to modify controls.
### Key
A Key refers to a key on the keyboard. This is even implemented on Android, so you can use keyboard input from any device that supports it. The keys are stored as an Osomi enum so can be found from String or just by value or compile-time. They are normally accessed as Key::...; ex. Key::Zero, Key::A, Key::Delete, Key::Volume_down, etc.
### MouseButton
Works the same way as any Osomi enum but has different values. It only supports Left, Middle, Right, Back, Forward buttons.
### MouseAxis
The XY cursor position are stored as MouseAxis::X and MouseAxis::Y. The Mouse_wheel axis is adjusted by the mouse wheel.
### ControllerAxis & ControllerButton
Every axis on the controller has a different binding; Lx, Ly, Rx, Ry, L2, R2. Other keys are treated like a button.
## InputDevice
An InputDevice is something that can get input to our framework, this can be a mouse, keyboard or controller. The InputDevice is not necessarily responsible for updating the buttons/axes itself, but it does store them and have the ability to. On Android; InputDevices don't update any buttons (the Window does) and on Windows; the controller does update itself, but others are updated by callbacks as well.  
By using isDown, isUp, isPressed and isReleased, you can determine the state of a button, however this can be handled by a callback (which is faster). You can also get an axis of the InputDevice.
## InputHandler
The InputHandler stores the devices and allows you to get controllers, the keyboard or the mouse. However, these are not guaranteed to return anything, so they can return nullptr on different devices. Using callbacks through a WindowInterface is recommended.
## InputManager
The InputManager uses the InputHandler to abstract it under handles. You can manually bind axes or states under a name, or you can load it from a JSON.
## TODO: Windows
