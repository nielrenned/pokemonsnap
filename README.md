# Pokemon Snap Archipelago Decomp

Originally forked from [The Pokemon Snap Decomp](https://github.com/ethteck/pokemonsnap). This repository is used to create a patch for the Pokemon Snap ROM to make it compatible with the Pokemon Snap Archipelago Client.

The contents of this repository are **not** required to play the Pokemon Snap AP. It is only used for development.

Note: To use this repository, you must already have a ROM for the game. It is **NOT** provided for you as part of this repository.

## Setup

### Required tools
- Python 3
- [`uv`](https://docs.astral.sh/uv/)
- [`ninja`](https://ninja-build.org/)
- MIPS Cross-Compilation tools: differs by Linux distro. On Ubuntu, the packages are `binutils-mips-linux-gnu` and `gcc-mips-linux-gnu`.
  - On Windows, it is highly recommended to install [WSL2](https://ubuntu.com/wsl/docs/latest/howto/install-ubuntu-wsl2/) to compile this project.

### Initial setup (one-time only)
1. Clone the repository
2. Place your (legally acquired) US Pokemon Snap ROM (sha1: `edc7c49cc568c045fe48be0d18011c30f393cbaf`) into the root of the repository as "pokemonsnap.z64".
3. Run `uv sync` to download the dependencies
4. Run `uv run configure.py --setup` to setup the files for the `ninja` build
5. (Optional) Add a file named `.apworld_path` to the project root containing the path to the APWorld repository on your machine. If this file exists, the patch file and RAM map will be copied to the APWorld data directory automatically on every build.

### Building the ROM and patch file (every time a change is made)
1. Run extraction and disassembly: `uv run configure.py`
2. Rebuild the rom: `ninja`

## Pokemon Snap Archipelago AI Usage Disclosure
- Pokemon Snap Archipelago is **not** vibe-coded.
- Pokemon Snap Archipelago does **not** contain AI art.
- Conversations with LLMs have been used to gain insights about the repository and brainstorm ideas for improvements. All ideas and insights pitched by LLMs are read by a human before being implemented. Generally, all new code by me, [@nielrenned](https://github.com/nielrenned), is handwritten and hand-tested.
- The [repository that this was forked from](https://github.com/gerbiljames/pokemonsnap) **did** use LLMs to generate some code that is in the patch file. The amount of code that was LLM-generated vs. by a human is unknown. The generated code has been reviewed and tested by multiple humans. All that code is contained in the commit on `main` with short hash `83807f5`.

## Credit

Thanks to:
- [@GerbilJames](https://github.com/gerbiljames) for initial repository setup and doing 90% of the work to get this functional.
- [@SomeJakeGuy](https://github.com/SomeJakeGuy) for ideas and general guidance.
