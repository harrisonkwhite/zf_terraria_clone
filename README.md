# ZF Terraria Clone

<img src="https://github.com/user-attachments/assets/5bd76a70-30ed-4fe0-8b2e-2decf3a3511f" alt="Terraria Clone Screenshot" style="max-width: 100%; height: auto;" />

---

## About

This is a personal project I've been developing to test and demonstrate Zeta Framework, and also to further improve my programming skills. It began as an original game, but I eventually realised it would be more productive to just clone an existing one so I could focus solely on programming technique and not have to worry about game design. I chose to clone Terraria specifically because it is composed of so many different interesting systems (e.g. world generation, tilemap lighting, NPC AI, inventories).

---

## Building

Building and running this project has only been tested on Windows.

Clone the repository by running `git clone --recursive https://github.com/harrisonkwhite/terraria_clone.git`.

> **Note:** If the repository was cloned non-recursively before, just run `git submodule update --init --recursive` to clone the necessary submodules.

Then go into the repository root and build with CMake:

```
mkdir build
cd build
cmake ..
```
