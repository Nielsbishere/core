# XR Changes

## Viewport rendering

Allow inheritance from a Viewport class, which sets up every frame. An example of these Viewport classes; VRViewportFt (subclass of XRViewportFt), ARViewportFt (subclass of XRViewportFt), WindowViewport (Physical window), RenderViewport (Virtual window). 

Every Viewport has a number of ViewportLayers, these layers represent an area or layer of the viewport. A VR layer would have 2 layers, one for each eye; which you can write to and obtain in the program. The app decides which layer the Window should use and depending on that, the WindowInterface should handle it.

You could inherit from the WindowViewport or RenderViewport if you want to add split screen for example. As long as you can specify what WindowLayers you want to create, it will allow you to.

A RenderViewport is meant for only having 1 frame in flight, it allows you to read from the result after the render target and allows you to store these frames. A WindowViewport is for a visual representation on screen. A XRViewportFt is not always available (like when disabled, or when the device doesn't support it), but it has subclasses which both support AR and VR.

### Rendering to a viewport

Your render function will get access to a `Viewport*`, which will provide you the layers you have access to. Every layer has a layer index and an area, and when one updates it will fire a callback, so you can update the aspect ratios and render targets.

The viewport doesn't have to support just one window, a VRViewportFt created with the mirror flag will allow rendering to a backing window viewport by maintaining a WindowViewport.

Every window has a viewport class, every viewport is responsible for updating the states (input, etc.). The Viewport has to call WindowInterface::update and WindowInterface::render whenever it is required.

## Input & haptics

OpenXR uses an abstracted system that doesn't work with the current system. Their system uses "paths" and every button is called an Action, while in owc it's implemented as a physical (keyboard / controller / mouse) action under the hood. InputManager could be mapped to work with Actions, instead, requiring InputHandler to be completely removed and revised. Through the InputManager you can suggest a list of bindings for an action name. The action name is then used to create the action internally. 

This system will require the use of "action states", which is a number of mappings that can be set by the action state. If you transition to a Menu for example, you could use a MenuState with mappings. These  mappings should follow the "snake_casing_naming_style".

When a state can't be determined, it will be set to "default".

### Types

`button [x] <name>` for a controller action (boolean).

`axis<n> [x] <name>` for n the dimensions of the action [1, 2, 3] (vector).

`click <name>` for a mouse action.

`key <name>` for a keyboard action.

`pose <name>` for a pose action (vec3).

`haptic_vibrate <name>` for a vibration haptic.

### Haptics

`getInputManager()->applyHaptic("<name>", VibrationHaptic(20, 0, 1));`

Will make the controller vibrate at full amplitude (undefined frequency) for 20 seconds.