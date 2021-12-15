#include <gtest/gtest.h>

#include <string>

#include "di.hpp"

class igreeter
{
public:
  virtual ~igreeter() = default;

  virtual std::string greet() const = 0;
};

class smart_greeter : public igreeter
{
public:
  std::string greet() const override
  {
      return "hey dude\n";
  }
};

class icar
{
public:
  virtual ~icar() = default;

  virtual long id() const = 0;
};

class truck : public icar
{
public:
  truck() : id_(++id_counter_) {}

  long id() const override
  {
    return id_;
  }

private:
  static long id_counter_;
  long id_ {};
};

long truck::id_counter_ = 0;

TEST(dipp, inject_singleton)
{

  di::install_module([](di::module &module) {
    module.bind<igreeter>()->to_singleton<smart_greeter>();
  });

  di::inject<igreeter> g1;
  di::inject<igreeter> g2;

  EXPECT_FALSE(g1.get() == nullptr);
  EXPECT_FALSE(g2.get() == nullptr);

  EXPECT_EQ(g1.get(), g2.get());
}

TEST(dipp, inject_transient)
{
  di::install_module([](di::module &m) {
    m.bind<icar>()->to_transient<truck>();
  });

  di::inject<icar> t1;
  di::inject<icar> t2;

  EXPECT_FALSE(t1.get() == nullptr);
  EXPECT_FALSE(t2.get() == nullptr);

  EXPECT_NE(t1.get(), t2.get());
}

TEST(dipp, inject_instance)
{
  di::install_module([](di::module &m) {
    truck t;
    m.bind<icar>()->to_instance(t);
  });

  di::inject<icar> t1;
  di::inject<icar> t2;

  EXPECT_FALSE(t1.get() == nullptr);
  EXPECT_FALSE(t2.get() == nullptr);

  EXPECT_EQ(t1.get(), t2.get());
}