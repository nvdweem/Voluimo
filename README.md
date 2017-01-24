# Voluimo
Control the Windows volume with a Nuimo

## Usage

If the application is started it will automatically connect to your Nuimo (it must have been paired in Windows before). If the connection succeeds, the tray icon will become blue. If the connection can't be made the icon will turn red. Clicking it when it's red will trigger a reconnect.

As soon as the connection is established a logo will be shown on the Nuimo, followed by the current battery level. Rotating the ring will change the Windows volume.

Swiping left or right will allow picking an application to control the volume for that specific application. To go back to control the Windows volume it's possible to swipe up. \
If an application that is playing audio is focused you can swipe down to control that application without scrolling to it.

Clicking the button will show the current volume. If the Nuimo has been idle for a short while it won't send rotate events anymore, clicking the button will wake it back up.

To exit the application, right click the taskbar icon.

## Code

This project does not use the Nuimo SDK since it only works for Windows Universal apps and Windows API calls that are needed to control the volume of the separate sessions won't run in those apps.

## Building

This project has additional dependencies. It has the EarTrumpet interop DLL included and other than that it only relies on Windows libraries.
It does require a Visual Studio that includes the stdafx libraries (VS 2017 Community didn't seem to have these, VS 2015 does).

## Thanks

This project uses the interop dll from Eartrumpet. The Bluetooth LE code is largely based on a post from [ihateble on the MSDN](https://social.msdn.microsoft.com/Forums/en-US/bad452cb-4fc2-4a86-9b60-070b43577cc9/is-there-a-simple-example-desktop-programming-c-for-bluetooth-low-energy-devices?forum=wdk).

## License

This project uses the MIT License.
