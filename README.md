# ZF Terraria Clone

This is a project I've been developing to test and demonstrate [Zeta Framework](https://github.com/harrisonkwhite/zeta_framework), and to also further improve my programming skills. I have chosen to clone *Terraria* specifically because it is comprised of so many different interesting systems (e.g. world generation, tilemap lighting, NPC AI, inventories).

### [Click here to see a video of it in action.](https://www.youtube.com/watch?v=JJ2J6unt9w8)

---

## Notable Technical Features (Not Already Provided by Zeta Framework)

- A clear game "phase" structure for managing the transition between title screen and world
- A BFS-based tilemap lighting system
- Procedurally generated cloud textures
- A grid-based inventory system allowing items to be stacked and moved between slots
- Tilemap collision detection
- A smooth camera system supporting parallax for a layering effect (used with clouds)
- An options menu allowing toggling of fullscreen state as well as volume mix

---

## Screenshots and GIFs

<img width="1280" height="720" src="https://github.com/user-attachments/assets/ce2af94d-36be-448f-8327-fdc36ebdabde" />

<img width="1280" height="720" src="https://github.com/user-attachments/assets/1b4206a9-4535-4709-8a2f-e82f9f9bf367" />

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
