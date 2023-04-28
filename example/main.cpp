#include "di.hpp"

#include <thread>
#include <iostream>
#include <utility>

class ivehicle
{
public:
    virtual ~ivehicle() = default;

    long id() const { return id_; }
    std::string name() const { return name_; }
    virtual int num_tyres() const = 0;
    virtual int seats() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const ivehicle& vec);
protected:
    explicit ivehicle(std::string name)
        : id_(++id_counter_)
        , name_(std::move(name))
    {}

private:
    static long id_counter_;

private:
    const long id_ {};
    const std::string name_ {};
};

long ivehicle::id_counter_ = 0;

std::ostream& operator<<(std::ostream& os, const ivehicle& vec)
{
    os << vec.name_ << " (id: " << vec.id_ << ')';
    return os;
}

class bike : public ivehicle
{
public:
    bike() : ivehicle("bike"), num_tyres_(2), seats_(2) {}

    int num_tyres() const override { return num_tyres_; }
    int seats() const override { return seats_; }

private:
    int num_tyres_ {};
    int seats_ {};
};

class car : public ivehicle
{
public:
    car() : ivehicle("car"), num_tyres_(4), seats_(4) {}

    int num_tyres() const override { return num_tyres_; }
    int seats() const override { return seats_; }

private:
    int num_tyres_ {};
    int seats_ {};
};

class truck : public ivehicle
{
public:
    truck() : ivehicle("truck"), num_tyres_(6), seats_(2) {}

    int num_tyres() const override { return num_tyres_; }
    int seats() const override { return seats_; }

private:
    int num_tyres_ {};
    int seats_ {};
};

class igreeter
{
public:
  virtual ~igreeter() = default;

  virtual void greet() const = 0;
};

class smart_greeter : public igreeter
{
public:
  void greet() const override
  {
      std::cout << "hey dude\n";
  }
};

class iunknown
{};

class iperthread
{
public:
    virtual ~iperthread() = default;
    virtual void dump() = 0;
};

class perthreaddumper : public iperthread
{
public:
    explicit perthreaddumper( std::string name )
    : name_(std::move(name))
    {}
    void dump() override
    {
        std::cout << name_ << ": thread id " << std::this_thread::get_id() << "\n";
    }

private:
    std::string name_;
};
void print_vehicle(di::inject<ivehicle> &vec)
{
    std::cout << "vehicle <" << *vec << "> has " << vec->num_tyres() << " tyres with " << vec->seats() << " seats\n";
}

int main()
{
    di::install_module([](di::module &module) {
        module.bind<ivehicle>( "bike" )->to<bike>();
        module.bind<ivehicle>( "car" )->to<car>();
        module.bind<ivehicle>( "truck" )->to<truck>();
    });
    di::append_module([](di::module &module) {
      module.bind<igreeter>()->to_singleton<smart_greeter>();
    });
    di::append_module([](di::module &module) {
      module.bind<iperthread>()->to_singleton_per_thread<perthreaddumper>("otto");
    });

    di::inject<ivehicle> vec1("bike");
    di::inject<ivehicle> vec2("car");
    di::inject<ivehicle> vec3("truck");
    di::inject<ivehicle> vec4("truck");

    print_vehicle(vec1);
    print_vehicle(vec2);
    print_vehicle(vec3);
    print_vehicle(vec4);

    di::inject<igreeter> greeter;

    greeter->greet();

    auto copied_greeter = greeter;

    copied_greeter->greet();

    auto moved_greeter = std::move(copied_greeter);

    moved_greeter->greet();

    auto copied_vec3 = vec3;

    print_vehicle(copied_vec3);

    try {
      di::inject<iunknown> u;
      u.get();
    } catch (std::logic_error &ex) {
      std::cout << "caught exception: " << ex.what() << "\n";
    }

    di::inject<iperthread> ptd1;
    ptd1->dump();
    di::inject<iperthread> ptd2;
    ptd2->dump();

    auto t1 = std::thread([]() {
        di::inject<iperthread> ptd3;
        ptd3->dump();
        di::inject<iperthread> ptd4;
        ptd4->dump();
    });
    t1.join();
    return 0;
}