# Autumn Harvest

A small 2D exploration game written in C with SDL2.

You are a little tree stump carrying the **Heart Seed**. Autumn is ending, and the
seed needs a sunny home before winter arrives. Explore the land, gather and craft,
open the way into new areas, and plant the seed in the Sunlit Glade.

There are no waves and no base to defend. The leaf spirits that live here are
peaceful: they wander around, turn to look at you, and only fight back if you hit
them first.

## The journey

1. **Maple Meadow**: your camp. Chop trees for wood and craft a Wood Pick.
2. **Rocky Hills**: boulders block the pass east of camp. Break them with your
   pick, then mine stone and upgrade to a Stone Pick.
3. **Mirror Pond**: craft planks and build a bridge across the water to
   **Willow Isle**, where mossy stones grow.
4. Craft **Compost** from leaves (leaf piles and leaf spirits drop them) and moss.
5. **Sunlit Glade**: hidden in the hills behind a granite wall that only a Stone
   Pick can break. Plant the Heart Seed in the glowing soil.

The current goal is always shown under your hearts, and a golden arrow at the edge
of the screen points to it when it's far away. Signs along the way give hints.

You have 10 days of autumn (about 20 minutes). The last days bring the first snow.
If you get knocked out you simply wake up back at camp, keeping everything you carry.

## Controls

| Action | Input |
| --- | --- |
| Move | `WASD` / arrow keys |
| Swing at what you point at (hold to keep swinging) | Left mouse |
| Swing in the direction you face | `Space` |
| Place a plank on water (hold planks) | Right mouse |
| Plant the Heart Seed | `E` (standing at the glowing soil) |
| Crafting (pauses the game) | `C` or `Tab` |
| Map (pauses the game) | `M` |
| Select hotbar slot | `1`-`9`, `0` or mouse wheel |
| Pause | `Esc` / `P` |

The item in the selected hotbar slot is the one you use.

## Crafting

| Item | Cost | Use |
| --- | --- | --- |
| Stick x2 | 1 wood | tool handles |
| Plank x2 | 1 wood | each plank bridges one water tile |
| Wood Pick | 2 wood, 2 sticks | breaks rocks and boulders, chops faster |
| Wood Sword | 2 wood, 1 stick | 8 damage (bare hands do 4) |
| Stone Pick | Wood Pick + 3 stone | breaks granite, mines much faster |
| Stone Sword | Wood Sword + 3 stone | 13 damage |
| Compost | 4 leaves, 1 moss | the seed needs it to take root |

## Leaf spirits

| Leaf | Lives in | Notes |
| --- | --- | --- |
| Orange | Whispering Wood, Maple Meadow | the gentlest |
| Red | Rocky Hills | quicker and tougher |
| Gold | Mirror Pond, Sunlit Glade | drops extra leaves |
| Magenta | Willow Isle | the toughest |

Once hit, a leaf spirit gets angry (a red `!`) and chases you for a while; badly hurt
ones run away. They drop leaves, sometimes a stick, and come back later.

## Building

The game loads `res/` and `fonts/` relative to the working directory.

**Visual Studio**: open `2Dautumn.sln`. SDL2, SDL2_image, SDL2_ttf and SDL2_mixer must
be available (for example through vcpkg). Run from the `2Dautumn/` folder.

**CMake (Linux / macOS / MinGW)**:

```sh
# Debian / Ubuntu
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev

cd 2Dautumn
cmake -S . -B build
cmake --build build
cd build && ./2Dautumn
```

Sound effects are synthesized at startup, so no audio files are needed. Your fastest
planting day is saved to `highscore.txt`.

The UI font is [Pixelify Sans](https://github.com/eifetx/Pixelify-Sans), licensed under
the SIL Open Font License (`fonts/OFL-PixelifySans.txt`).

## Code layout

| File | Contents |
| --- | --- |
| `app.c` | main loop, depth-sorted drawing, screens (title, pause, endings) |
| `world.c` | season calendar, region discovery, goals, planting, signs, HUD |
| `combat.c` | player movement and swing, leaf spirit behaviour |
| `map.c` | world layout, terrain, breakable objects, bridges, collision, minimap |
| `entity.c` | player and creature setup, camera, sprite rendering |
| `craft.c` | recipes and the crafting menu |
| `uielements.c` | hotbar, hearts, text rendering |
| `particle.c` | falling leaves and snow, bursts, pickups, swing effect, floating text |
| `sound.c` | synthesized sound effects |
