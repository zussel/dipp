# dipp

**D**ependency **I**njection for C **P**lus **P**lus

Goal of this project is to provide a simple C++ dependency injection or
service locator mechanism.

| Branches    | Linux-CI |
|-------------|----------|
| main      |[![Build Status](https://github.com/zussel/dipp/actions/workflows/linux.yml/badge.svg?branch=main)](https://github.com/zussel/dipp/actions/workflows/linux.yml?query=branch%3Amain)|[![Build status]

## Motivation

I wanted this mechanism as simple and intuitive as possible, so I
came up with the idea to 'inject' an object like a shared pointer.

```cpp
di::inject<igreeter> greeter;
greeter->greet()
```

Therefor the dependencies must be set up providing at least a
function setting up the services and install them in a global
service repository.

```cpp
di::install([](di::module &m) {
  m.bind<igreeter>()->to_singleton<simple_greeter>();
});
```

With this configuration it is possible to __inject__ our ```greeter```
everywhere.

To ensure this behaviour a singleton repository providing
all installed services is always accessible. Though the singleton
pattern meant to be an *anti-pattern* I decided to use it here
for practical reason.

Just ```include "di.hpp"``` and you're done.

### Service types

A service can be bound by now in four different ways:

- in transient scope (every inject leads to a new instance)
- in singleton scope (every inject leads to the same instance)
- as a given instance (every inject leads to the provided instance)
- in singleton per thread scope (every inject leads to the same instance with current thread)

### Named services

It is also possible to bind services with a name.

```cpp
di::install([](di::module &m) {
  m.bind<igreeter>("students")->to_singleton<students_greeter>();
  m.bind<igreeter>("teachers")->to_singleton<teacher_greeter>();
});
```

### Modules

To omit the use of the global (singleton) repository it is possible to
use a module as a local repository and pass this module to the
constructor of ```inject```.

```cpp
di::module m;
m.bind<igreeter>()->to_singleton<simple_greeter>();

inject<igreeter> greeter(m);
greeter->greet();
```

## API

The API consists of a handful straight forward interfaces.

### Binding

At first the user
must set up the service bindings within an instance of a ```module``` class.
To do that the class provides the following interface:

```cpp
// bind the interface of class type I
template < class I >
std::shared_ptr<proxy<I>> di::module::bind();
// bind the interface of class type I within the given name
template < class I >
std::shared_ptr<proxy<I>> di::module::bind(const std::string &name);
```

The call to ```bind()``` returns a proxy object providing methods to
register different strategies to provide an implementation for the interface.
The class declaration takes care that the interface class (```I```) is at least
the same or a derived class (```T```).

```cpp
// register the transient strategy (always a new instance)
template < class T, typename ...Args >
di::proxy<i>::to(Args &&...args);
// register the singleton strategy (always the same instance) 
template < class T, typename ...Args >
di::proxy<i>::to_singleton(Args &&...args);
// register the singleton per thread strategy (always the same instance) 
template < class T, typename ...Args >
di::proxy<i>::to_singleton_per_thread(Args &&...args);
// register the given instance
template < class T >
di::proxy<i>::to_instance(T &&obj);
```

Furthermore, arguments can be passed as construction arguments to the transient
and singleton strategy.

```cpp
di::install([](di::module &m) {
  m.bind<igreeter>().to_singleton<named_greeter>("hello everybody");
});

```
The user decides whether using a local module or using the global
repository.

```cpp
// using the global repository (clears the repository before)
di::install([](di::module &m) {
  // ...
})
// using the global repository (appends the given bindings)
di::append([](di::module &m) {
  // ...
})
```

### Injecting

```cpp
// injection from global repository
inject<igreeter> i;

// named injection from global repository
inject<igreeter> i("students");

// named injection from local repository
di::module m;
// setup ...
inject<igreeter> i(m, "students");
```
