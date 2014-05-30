ehttp
=====

ehttp (Embedded HTTP) is a modular, embedded HTTP server and a webapp microframework for C++ applications.

Out of the box, it provides a TCP server, a request parser, and a router. These components are loosely coupled, and can be used independently.

*ehttp is under active development, everything is subject to change*

Features
--------

* **Minimal dependencies**  
  All ehttp requires is a C++11-compatible compiler.  
  Interally, it uses [HTTP-Parser](https://github.com/joyent/http-parser), but it's compiled in.  
  The (optional) built-in server additionally depends on [asio](http://think-async.com/).
* **Independent**  
  Out of the box, ehttp provides a server, a request parser and a router.  
  These three together are everything you need to make an application, with no impact or demands on the surrounding program.
* **Modular**  
  The three parts can operate independently of each other.  
  If you already have a perfectly fine TCP server, you can use that to feed the request parser data.  
  If you don't like the router, you can just handle requests yourself.
* **Lightweight**  
  ehttp does not bloat your program, and has neither bells nor whistles.  
  It does one thing, and it does it well: to respond to HTTP requests.  
  Everything else is up to the application.
