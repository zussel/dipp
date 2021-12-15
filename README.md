# dipp

**D**ependency **I**njection for C **P**lus **P**lus

Goal of this project is to provide a simple C++ dependency injection or
service locator mechanism.

## Introduction

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
  m.bind<igreeter>()->to_singleton<greeter>();
});
```

With this configuration it is possible to __inject__ our ```greeter```
everywhere.

To ensure this behaviour there is a singleton repository providing
all installed services. Though the singleton pattern meant to be an
*anti pattern* I decided to use it here for practical reason.

Just ```include "di.hpp"``` and you're done.

## API

### Service types
### Named services
### Modules