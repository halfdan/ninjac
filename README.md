README
======

Table of Contents
-----------------

* Introduction
* Language
* Features
* Authors


Introduction
------------
This project has been developed at the University of Applied Sciences Gießen-Friedberg (Technical University Mittelhessen) between October 2010 and February 2011.

The goal was to implement a compiler for the object oriented language "Ninja" as part of the masters course "Compiler Engineering 2". You can find more details about the language and the course on the course [related website] or our german [FAQ] for the language.

Language
--------

Ninja is an object oriented language. It was invented by Professor Hellwig Geisse and is currently used in the bachelors course "Konzepte Systemnaher Programmierung".

**Note:** The language is still in development and there is no written language specification available. The here available implementation is just one interpretation of the language.

Features
--------

Our compiler implementation has the following features:

* Support for multiple files
* Standard library for some of the core classes (Object, Integer, Character, Boolean)
* Inline assembler using the special syntax:  asm {% pushc 2 %}
* Methods and field inside the same class can have the same name
* Compiler is being tested against ~100 test cases

Authors
-------
Fabian Müller
Fabian Becker


  [related website]: http://homepages.fh-giessen.de/~hg53/cb2-ws1011/index.html
  [FAQ]: http://faq.ninja-lang.de