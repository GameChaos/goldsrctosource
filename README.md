# GoldSrcToSource
Tool that converts GoldSrc BSPs into Source 1 BSPs (and VMFs).

This is currently unfinished, I'm not sure when/if I'm going to finish this, so here it is for now.

Unfinished features:
1. Models (props?) don't get converted (neither do sprites).
2. Automatic entity conversion is unfinished and in a testing state (this is most needed for direct bsp to bsp conversion, disabled for VMFS right now).
3. Visibility doesn't work in converted BSPs.
4. Water doesn't work quite properly in converted BSPs.
5. Skybox textures don't get converted.

Some (mostly finished) features:
1. Automatic texture conversion (including transparent textures). Source 1 also seems to support goldsrc animated textures.
2. Lighting conversion from GoldSrc to Source 1 BSP. (1:1 lighting!)
3. Conversion to BSP and VMF (as mentioned before).
4. Loads of jank and bugs.

## Building

This is currently an x64 Windows only program, though I think porting to other platforms isn't too hard.

Unfortunately I've never used build tools like cmake and such, so you'll have to spelunk through my build*.bat scripts! In theory you should just be able to build win32_goldsrcconvert.cpp and link kernel32.lib and shell32.lib.

For compiling debug shaders, you have to get sokol-shdc.exe. Dear ImGui and sokol are only used for debug visualisations, you can build without them!

## Example usage

`-input "D:\Steam\steamapps\common\Half-Life\cstrike\maps\kz_man_everest.bsp" -outputvmf debug/out.vmf -outputbsp debug/out.bsp -mod cstrike -enginepath "D:\Steam\steamapps\common\Half-Life" -assetpath "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\csgo"`
