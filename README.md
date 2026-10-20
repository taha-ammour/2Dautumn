# Autumn Harvest

A small 2D survival / defense game written in C with SDL2.

You are a little tree stump guarding the **Heart Seed**. During the day you gather
wood and stone and craft tools. At night, leaf spirits ride the wind into the
clearing and try to wither the seed. Survive as many nights as you can.

## What makes it different

- **The wind is part of the game.** It changes direction over time and you can
  read it on the wind vane (top right) and in the falling leaves. Leaf spirits are
  light, so the wind carries them: each wave enters from the side the wind is
  blowing *from*, and they move faster downwind. Your thrown embers drift with the
  wind too, so aim upwind.
- **The Heart Seed is your light.** At night the only light comes from the seed, from
  you and from your embers. Leaves inside the seed's glow are slowed and slowly burn.
  Feed the seed wood (`E`) to widen its glow and heal it, but that wood is no longer
  available for tools.
- **One resource, three uses.** Wood becomes tools, sticks become ember ammo, and the
  seed needs wood as fuel. You decide how to spend it each day.
- **Falling is not game over.** If you fall, the seed regrows you after a few seconds,
  but it pays for it with a fifth of its life. The game ends only when the seed withers.
- **The seed grows.** Every night you survive it levels up: more health, a wider glow,
  hotter burning.

## Controls

| Action | Input |
| --- | --- |
| Move | `WASD` / arrow keys |
| Mine a rock or tree / strike a leaf spirit | Left click |
| Throw an ember (costs 1 stick) | Right click |
| Crafting menu | `C` or `Tab` |
| Feed wood to the Heart Seed | `E` (stand next to it) |
| Select hotbar slot | `1`-`7` or mouse wheel |
| Pause | `Esc` / `P` |

## Crafting

| Item | Cost | Effect |
| --- | --- | --- |
| Stick x2 | 1 wood | ember ammo, tool handles |
| Wood Sword | 2 wood, 1 stick | 8 damage (fists do 4) |
| Wood Pick | 2 wood, 2 sticks | mines 2.5x faster |
| Stone Sword | 3 stone, 1 stick | 13 damage |
| Stone Pick | 3 stone, 2 sticks | mines 4x faster |

The item in the selected hotbar slot is the one you use.

## Leaf spirits

| Leaf | Appears from | Behaviour |
| --- | --- | --- |
| Orange | night 1 | the basic leaf |
| Red | night 2 | fast, fragile, hunts you from further away |
| Gold | night 3 | splits into two small leaves when it dies |
| Crimson | night 4 | slow and tough, ignores you and goes straight for the seed |

Leaf spirits drop sticks, and crimson ones also drop wood.

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

Sound effects are synthesized at startup, so no audio files are needed. Your best
score is saved to `highscore.txt`.

## Code layout

| File | Contents |
| --- | --- |
| `app.c` | main loop, game screens (title, pause, game over) |
| `world.c` | day/night cycle, wind, waves, Heart Seed, lighting, HUD |
| `combat.c` | leaf spirit pool, melee, embers, damage and drops |
| `entity.c` | player and enemy movement, camera, sprite rendering |
| `map.c` | terrain generation, rocks and trees, mining |
| `craft.c` | recipes and the crafting menu |
| `uielements.c` | hotbar, hearts, text rendering |
| `particle.c` | falling leaves, bursts, floating text |
| `sound.c` | synthesized sound effects |
