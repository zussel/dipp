#ifndef DI_HPP
#define DI_HPP

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>

namespace matador {
namespace di {
/**
 * Interface for the dependency injection
 * creation strategy.
 *
 * The injected service
 */
class strategy
{
public:
  /**
   * Destructor
   */
  virtual ~strategy() = default;

  /**
   * @brief Acquires an instance
   * 
   * Acquires an instance based on the internal strategy
   * and returns an anonymous pointer to the object
   * 
   * @return The current injected object depending of the strategy
   */
  virtual void *acquire() = 0;
};

/**
 * Implements the transient dependency injection
 * strategy. When ever injected a new instance
 * is created.
 *
 * @tparam T Type of the object to be injected
 */
template<class T>
class transient_strategy : public strategy
{
public:
  /**
   * @brief Construct a new transient strategy object
   * 
   * @tparam Args Type of arguments for constructor
   * @param args Arguments for constructor
   */
  template<typename ...Args>
  explicit transient_strategy(Args &&...args) {
    creator_ = [args..., this]() {
      instances_.push_back(std::make_unique<T>(args...));
      return instances_.back().get();
    };
  }

  void *acquire() override {
    return creator_();
  }

private:
  std::function<T *()> creator_{};
  std::vector<std::unique_ptr<T>> instances_;
};

/**
 * @brief Implements the singleton strategy 
 * 
 * The singleton strategy provides only one
 * instance of the type.
 * 
 * @tparam T Type of the object to be injected
 */
template<class T>
class singleton_strategy : public strategy
{
public:
  template<typename ...Args>
  explicit singleton_strategy(Args &&...args) {
    creator_ = [args..., this]() {
      if (!instance_) {
        instance_ = std::make_unique<T>(args...);
      }
      return instance_.get();
    };
  }

  void *acquire() override {
    return creator_();
  }

private:
  std::function<T *()> creator_{};
  std::unique_ptr<T> instance_;
};

/**
 * @brief Provides a specific instance on injection
 * 
 * @tparam T 
 */
template<class T>
class instance_strategy : public strategy
{
public:
  explicit instance_strategy(T &&obj)
    : instance_(obj) {}

  void *acquire() override {
    return &instance_;
  }

private:
  T &instance_;
};

class proxy_base
{
public:
  virtual ~proxy_base() = default;

  template<typename T>
  T &get() const {
    return *static_cast<T *>(strategy_->acquire());
  }

protected:
  void initialize_strategy(std::unique_ptr<strategy> &&strategy) {
    strategy_ = std::move(strategy);
  }

private:
  std::unique_ptr<strategy> strategy_;
};

template<typename I>
class proxy : public proxy_base
{
public:
  template<typename T, typename ...Args, typename std::enable_if<std::is_base_of<I, T>::value>::type * = nullptr>
  void to_transient(Args &&...args) {
    initialize_strategy(std::make_unique<transient_strategy<T>>(std::forward<Args &&>(args)...));
  }

  template<typename T>
  void to_instance(T &&obj) {
    initialize_strategy(std::make_unique<instance_strategy<T>>(obj));
  }

  template<typename T, typename ...Args, typename std::enable_if<std::is_base_of<I, T>::value>::type * = nullptr>
  void to_singleton(Args &&...args) {
    initialize_strategy(std::make_unique<singleton_strategy<T>>(std::forward<Args &&>(args)...));
  }
};

class module
{
private:
  using t_type_proxy_map = std::unordered_map<std::type_index, std::shared_ptr<proxy_base>>;

public:
  template<typename I>
  std::shared_ptr<proxy<I>> bind() {
    return bind<I>(default_map_);
  }

  template<typename I>
  std::shared_ptr<proxy<I>> bind(const std::string &name) {
    auto i = module_map_.find(name);
    if (i == module_map_.end()) {
        i = module_map_.insert(std::make_pair(name, t_type_proxy_map{})).first;
    }
    return bind<I>(i->second);
  }

  template<typename I>
  I& resolve() {
    return resolve<I>(default_map_);
  }

  template<typename I>
  I& resolve(const std::string &name) {
    auto i = module_map_.find(name);
    if (i == module_map_.end()) {
        throw std::logic_error("unkown name " + name);
    }
    return resolve<I>(i->second);
  }

private:
  template<typename I>
  std::shared_ptr<proxy<I>> bind(t_type_proxy_map &type_proxy_map) {
    auto di_proxy_ptr = std::make_shared<proxy<I>>();
    auto i = type_proxy_map.insert(std::make_pair(std::type_index(typeid(I)), di_proxy_ptr));
    return std::static_pointer_cast<proxy<I>>(i.first->second);
  }

  template<typename I>
  I& resolve(t_type_proxy_map &type_proxy_map) {
    auto i = type_proxy_map.find(std::type_index(typeid(I)));
    if (i == type_proxy_map.end()) {
      throw std::logic_error(std::string("couldn't find type ") + typeid(I).name());
    }
    return i->second->get<I>();
  }

private:
  t_type_proxy_map default_map_;

  std::unordered_map<std::string, t_type_proxy_map> module_map_ {};
};

class module_builder
{
public:
  virtual ~module_builder() = default;

  virtual void build(module &module) = 0;
};

template < typename T >
class singleton
{
public:
  typedef T value_type;

  static value_type& instance ()
  {
    static value_type instance_;
    return instance_;
  }
  virtual ~singleton() = default;

protected:
  singleton() = default;
};

class repository : public singleton<repository>
{
public:
  void install_module(std::unique_ptr<module_builder> &&builder) {
    builder->build(module_);
  }

  template<typename I>
  I &resolve() {
    return module_.resolve<I>();
  }

  template<typename I>
  I &resolve(const std::string &name) {
    return module_.resolve<I>(name);
  }

private:
  module module_;
};


template<class T>
class inject
{
public:
  inject()
    : obj(repository::instance().resolve<T>())
  {}

  explicit inject(const std::string &name)
    : obj(repository::instance().resolve<T>(name))
  {}

  inject(const inject &x)
    : obj(x.obj)
  {}
  
  inject& operator=(const inject &x)
  {
    obj = x.obj;
    return *this;
  }
  
  inject(inject &&x)
    : obj(x.obj)
  {}
  
  inject& operator=(inject &&x)
  {
    obj = x.obj;
    return *this;
  }
  
  T* operator->() const {
    return &obj;
  }

  T& operator*() const {
    return obj;
  }
private:
  T &obj;
};

void install_module(std::unique_ptr<module_builder> &&builder)
{
  repository::instance().install_module(std::move(builder));
}

}
}

#endif /* DI_HPP */