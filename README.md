# Divvy - A lightweight Component framework [![Build Status](https://travis-ci.org/puradox/divvy.svg?branch=master)](https://travis-ci.org/puradox/divvy)
##### Current version: v0.5

Divvy is a lightweight component entity framework made in C++11. Licensed 
under the MIT license and designed to be extremely easy to integrate, it's 
purpose is to ease development where monolithic class structures would 
normally be the answer. See all of the details, benefits, and drawbacks of
the *Component pattern* [here](http://gameprogrammingpatterns.com/component.html).

## Features
  - Simple, no macros or unneccessary clutter
  - Extendable, Components are easy to make
  - Lightweight, no need to link libraries
  - Easy to integrate, all of Divvy is implemented in a single header file
  - Fast, Contiguous memory storage of Components allows for faster iterations
  - Type-safe, `std::enable_if` ensures that Components can't be mixed up

## Purpose

Divvy was made in mind for people that desire a simple, lightweight way to utilize the Component pattern in their program. Most other Component Entity libraries are large and hard-to-maintain that requires users to download, compile, then link against their library. 

Divvy was originally geared towards game development, but it can be used for
other areas of development as well.

**Still in development**
