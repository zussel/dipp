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
      return "hey dude";
  }
};

class hello_greeter : public igreeter
{
public:
  std::string greet() const override
  {
      return "hello";
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

TEST(dipp, inject)
{
  di::install_module([](di::module &module) {
    module.bind<igreeter>()->to_singleton<smart_greeter>();
  });

  di::inject<igreeter> g1;

  EXPECT_FALSE(g1.get() == nullptr);

  auto g2 = g1;

  EXPECT_FALSE(g2.get() == nullptr);
  EXPECT_EQ(g1.get(), g2.get());

  g2 = std::move(g1);

  EXPECT_TRUE(g1.get() == nullptr);
  EXPECT_FALSE(g2.get() == nullptr);
  EXPECT_NE(g1.get(), g2.get());

  auto g3(std::move(g2));

  EXPECT_TRUE(g2.get() == nullptr);
  EXPECT_FALSE(g3.get() == nullptr);
  EXPECT_NE(g3.get(), g2.get());

  di::module m;
  m.bind<igreeter>()->to_singleton<hello_greeter>();
  m.bind<igreeter>("smart")->to_singleton<smart_greeter>();

  di::inject<igreeter> g4(m, "smart");
  EXPECT_FALSE(g4.get() == nullptr);

  di::inject<igreeter> g5(m);
  EXPECT_FALSE(g5.get() == nullptr);

  EXPECT_NE(g4.get(), g5.get());
}

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

  di::clear_module();

  EXPECT_THROW(di::inject<igreeter>(), std::logic_error);
}

TEST(dipp, inject_named)
{
  di::install_module([](di::module &module) {
    module.bind<igreeter>("smart")->to_singleton<smart_greeter>();
  });

  EXPECT_THROW(di::inject<igreeter>("hello"), std::logic_error);

  di::append_module([](di::module &module) {
    module.bind<igreeter>("hello")->to_singleton<hello_greeter>();
  });


  di::inject<igreeter> g1("smart");
  di::inject<igreeter> g2("hello");

  EXPECT_FALSE(g1.get() == nullptr);
  EXPECT_FALSE(g2.get() == nullptr);

  EXPECT_NE(g1.get(), g2.get());
  EXPECT_EQ("hey dude", g1->greet());
  EXPECT_EQ("hello", g2->greet());
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