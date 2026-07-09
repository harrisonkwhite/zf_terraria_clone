# ZF Terraria Clone

This is a project I've been developing to test and demonstrate [Zeta Framework](https://github.com/harrisonkwhite/zeta_framework), and to also further improve my programming skills. I have chosen to clone *Terraria* specifically because it is comprised of so many different interesting systems (e.g. world generation, tilemap lighting, NPC AI, inventories).

### [Click here to see a video of it in action.](https://www.youtube.com/watch?v=JJ2J6unt9w8)

---

## Notable Technical Features (Not Part of Zeta Framework)

- A clear game "phase" structure for managing the transition between title screen and world
- A BFS-based tilemap lighting system
- Procedurally generated cloud texture pools (in which each cloud is assigned a random texture from the generated pool)
- A grid-based inventory system allowing items to be stacked and moved between slots
- Tilemap collision detection
- A smooth camera system supporting parallax for a layering effect (used with clouds)
- An options menu allowing toggling of fullscreen state as well as volume mix

---

## Screenshots and GIFs

<img width="1280" height="720" src="https://github.com/user-attachments/assets/2ea9a2c6-70e3-4797-99da-5e358b977615" />

<img width="1280" height="720" src="https://github.com/user-attachments/assets/879e8a7d-d286-4af1-9a2c-68ba9a90740a" />

<img width="1280" height="720" src="https://github.com/user-attachments/assets/ad4acde5-5fb4-40d3-94c9-a5644b250fe5" />

---

## Building

Building and running this project has only been tested on Windows.

Clone the repository by running `git clone --recursive https://github.com/harrisonkwhite/zf_terraria_clone.git`.

> **Note:** If the repository was cloned non-recursively before, just run `git submodule update --init --recursive` to clone the necessary submodules.

Then go into the repository root and build with CMake:

```
mkdir build
cd build
cmake ..
```

---

## Controls

The controls are modelled as closely as possible after the original *Terraria*.

**Move Right:** D

**Move Left:** A

**Jump:** Space (Hold down for more height!)

**Interact or Use Item:** Left Mouse Button (Note that most items support holding this down.)

**Toggle Inventory Open/Close:** Escape

**Change Selected Inventory Hotbar Slot:** Number Keys or Mouse Scroll
