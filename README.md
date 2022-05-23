# Overview

Welcome to this C++ roguelike tutorial! It is aimed to show you how to create a simple roguelike game from the ground up, using just a few libraries. The libraries used are fairly low-level, so that all the code you see and write is what makes the game tick and display on screen, so you won't find for example any opaque .display_game() library calls that handle things behind the scenes. *You* will be the one behind the scenes. So, if you have some knowledge of C++ and you want to know how to use that knowledge into developing a simple roguelike game, read on!

# What should you expect

There are lots of tutorials out there, with different aims, so it is only appropriate (and responsible) to communicate what you should or should not expect from this tutorial. The rationale behind this tutorial is to keep things simple and accessible for you to see (and change!) and this sometimes happens at the expense of doing things "properly" (highly subjective) or super-efficiently. For example, in C++ we can use templates to reduce code duplication and make some quite nice generic abstractions. In this tutorial there's minimal use of templates, as they can be complex and confusing to new learners, and the errors can be intimidating.

## What it is

* A tutorial on how to make a simple roguelike from scratch
* A demonstration of how we can use OpenGL to power the graphics of a roguelike game, even if it's displayed using ASCII fonts
* Stripping out much of the game, it can be an example framework for a 2D game using GLFW and Dear ImGui.

## What it is **not**

* A C++ tutorial. Although some of the frequently used concepts/constructs will be explained, and why they're used, there are more appropriate source for learning C++ out there, [here's one for modern C++](https://changkun.de/modern-cpp/). 
* An OpenGL tutorial. As above, there's lots of OpenGL code, so it will be explained why anything is used and why, but if you want to really learn OpenGL and GLSL, look at more focused sources like [this](https://learnopengl.com/).
* A library to use as-is for your bigger-scope roguelike. This tutorial is to give you an idea of how things could be done (especially when starting from the ground up), but for a larger-scope game you need to have a more powerful and robust backend, and in that case you should be considering using slightly higher level libraries where appropriate (e.g. for entity systems, GUI/graphics, overall media framework, etc). All the things that we're avoiding in terms of C++ (templates, shared pointers, more OOP patterns) and OpenGL (more complex shaders, framebuffers, more wrappers around low-level code) could be useful in a project with a bigger scope.
* The best or most optimal way of doing things. Besides the fact that it's hard to qualify such a statement, this tutorial makes some sacrifices in the sake of simplicity. Now the good part is that you, with your newfound knowledge and experience after a while, can tear things apart and improve what you see fit. There will be occasional cases where I might say "this could be done differently" to highlight such areas for improvement.

# Prerequisites and libraries

To build the project, you need a C++ compiler, [CMake](https://cmake.org/) and [vcpkg](https://vcpkg.io/en/index.html).
The project uses a number of libraries, which are handled/installed automatically via vcpkg:

* [**fmt**](https://github.com/fmtlib/fmt): String formatting library, essential until [C++20's equivalent](https://en.cppreference.com/w/cpp/utility/format)
* [**glm**](https://github.com/g-truc/glm): Simple math library, following style/conventions to GLSL
* [**GLEW**](http://glew.sourceforge.net/): OpenGL extension loading library
* [**GLFW**](https://www.glfw.org/): API for creating an OpenGL window and context that can receive input and events
* [**stb**](https://github.com/nothings/stb): Public domain single-file libraries, for functionality such as image read/write
* [**nlohmann json**](https://json.nlohmann.me/): JSON libarary for modern C++
* [**Dear ImGui**](https://github.com/ocornut/imgui): Developer GUI library using the Immediate Mode GUI paradigm
* [**magic enum**](https://github.com/Neargye/magic_enum): Static reflection for enums
* [**nano signal slot**](https://github.com/NoAvailableAlias/nano-signal-slot): [Signals and slots](https://en.wikipedia.org/wiki/Signals_and_slots) library for event handling

# Structure


