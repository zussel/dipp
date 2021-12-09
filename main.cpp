#include "di.hpp"

#include <iostream>

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
    explicit ivehicle(const std::string &name)
        : id_(++id_counter_)
        , name_(name)
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

class vehicle_module_builder : public matador::di::module_builder
{
public:
    void build(matador::di::module &module) override {
        module.bind<ivehicle>("bike")->to_transient<bike>();
        module.bind<ivehicle>("car")->to_transient<car>();
        module.bind<ivehicle>("truck")->to_transient<truck>();

        module.bind<igreeter>()->to_singleton<smart_greeter>();
    }
};

using namespace matador;

void print_vehicle(di::inject<ivehicle> &vec)
{
    std::cout << "vehicle <" << *vec << "> has " << vec->num_tyres() << " tyres with " << vec->seats() << " seats\n";
}

int main()
{
    di::install_module(std::make_unique<vehicle_module_builder>());

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

    auto copyied_vec3 = vec3;

    print_vehicle(copyied_vec3);

    return 0;
}