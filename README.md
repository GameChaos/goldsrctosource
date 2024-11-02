# GoldSrcToSource
Tool that converts GoldSrc BSPs into Source 1 BSPs (and VMFs).

This is currently unfinished, I'm not sure when/if I'm going to finish this, so here it is for now.

Unfinished features:
1. Models (props?) don't get converted (neither do sprites).
2. Automatic entity conversion is unfinished and in a testing state (this is most needed for direct bsp to bsp conversion, entities are converted as is for VMFs right now).
3. Visibility doesn't work in converted BSPs.
4. Water doesn't work quite properly in converted BSPs.
5. Command line output is a mess, mostly debug stuff right now.

Some (mostly finished) features:
1. Automatic texture conversion (including transparent textures and the skybox). Source 1 (CS:GO) also seems to support GoldSrc animated textures to some extent.
2. Lighting conversion from GoldSrc to Source 1 BSP. (1:1 lighting!)
3. Conversion to BSP and VMF (as mentioned before).
4. Loads of jank and bugs.

## Building

Install zig 0.14 to system path somewhere.

Open the terminal in the root directory and type `zig build`.

Boom, done.

To do a release build run `zig build -Doptimize=ReleaseFast`.

To cross-compile for linux add `-Dtarget=x86_64-linux-gnu`.

To cross-compile for windows add `-Dtarget=x86_64-windows-gnu`.

## Example usage

```
goldsrctosource.exe -input "D:\Steam\steamapps\common\Half-Life\cstrike\maps\kz_man_everest.bsp" \
-outputvmf debug/out.vmf \
-outputbsp debug/out.bsp \
-mod cstrike
-enginepath "D:\Steam\steamapps\common\Half-Life" \
-assetpath "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\csgo"```
