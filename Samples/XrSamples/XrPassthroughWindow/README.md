# OpenXR Passthrough Window Sample

This sample demonstrates how to display passthrough as the background using the reconstruction layer from the `XrPassthrough` sample. It now includes a simple UI window built with TinyUI. The window's size and transparency can be changed at runtime using on-screen sliders.
The sample requires a headset with the `XR_FB_passthrough` extension (Quest runtime v53 or newer).

The Gradle wrapper JAR is intentionally omitted from version control. Running
`./gradlew` in the Android project directory will download the required wrapper files automatically.
