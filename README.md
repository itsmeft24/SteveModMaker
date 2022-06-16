# SteveModMaker
A tool for generating Steve/Alex mods for Super Smash Bros. Ultimate.

# Credits
The code that handles NUTEXB and BNTX file IO was based off of the existing Rust libraries [nutexb](https://github.com/jam1garner/nutexb) and [bntx](https://github.com/jam1garner/bntx). Texture swizzling is handled by [tegra_swizzle](https://github.com/ScanMountGoat/tegra_swizzle)'s C FFI API. Renders and UI are generated using OpenCV perspective warping. Skin data is grabbed from the Minecraft/Mojang web APIs using curl and rapidjson.
