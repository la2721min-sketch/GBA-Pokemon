# Pokémon World – GBA prototype

A homebrew GBA project using devkitARM + libgba. It is a playable starter prototype, not the complete seven-region game yet.

## Build
1. Install devkitPro with devkitARM and libgba, or use the included GitHub Actions workflow.
2. Push the project to a GitHub repository on the `main` branch.
3. Open **Actions** → **Build Pokemon World GBA** → run workflow (or push a commit).
4. Download the `Pokemon-World-GBA` artifact and extract `pokemon_world.gba`.
5. Open the `.gba` file in a GBA emulator.

## Controls
- D-pad: move
- A: interact / start a wild encounter when facing tall grass
- In battle: Left/Right selects an action, A confirms, B runs away

## Current prototype contents
- 240×160 GBA Mode 3 display
- FireRed-inspired starter-town map
- Player movement and basic collision
- One follower placeholder
- Day/night tint cycle
- Tall-grass encounter
- Basic battle-screen prototype with attack/capture/escape

## Not implemented yet
The complete Kanto–Alola world, full Gen 1–7 Pokémon database and official sprites, complete original storylines, real trading, flying mounts, boats, Mega Evolution, Z-Moves, advanced trainer scaling, and full save/party/box/Pokédex systems are not included yet. These require substantial additional data, art, and implementation.

All names and references to Pokémon are used to describe a fan-made project. This project contains no official Pokémon assets.
