ehttp
=====

ehttp (Embedded HTTP) is a modular HTTP server and a webapp microframework, written in C++.

Out of the box, it provides a TCP server, an HTTP parser, and a request router.  
These components are loosely coupled, and can be used independently.

*ehttp is under active development, everything is subject to change*

Features
--------

* **Minimal dependencies**  
  All ehttp requires is a C++11-compatible compiler - GCC 4.8 or Visual Studio 2013.  
  The (optional) built-in server additionally requires [asio](http://think-async.com/) in the include path.
* **Independent**  
  Out of the box, ehttp provides a server, an HTTP parser and a request router.  
  They can work fully independently, or integrate easily into a parent application.
* **Modular**  
  The three parts operate independently of each other, and you're not forced to use any of them.  
  If you already have a perfectly fine TCP server, you can use that to feed the request parser data. If you don't like the router, you can just handle requests yourself.  
  Additionally, `http_server` provides an example of how to combine all three into a single class.
