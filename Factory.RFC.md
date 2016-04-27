Проблемы:
 + Почти все formatter, sink, handler представляют собой PIMPL, дабы скрыть имплементацию, чтобы не ломалось ABI.
 + Никому, как правило, не нужны эти классы. Либа использует интерфейс.
 + Это правильно, что есть фабрика, которая возвращает дефолтную реализацию.
 - Сложно тестировать, т.к. нужен DI, и эти зависимости так или иначе должны фигурировать.
 + Registry слишком шаблонный, надо инкапсулировать, желательно все фабрики, чтобы потом можно было вводить новые категории.

Что если делать все formatter, sink и т.д. интерфейсами (возможно, incomplete type)? Тогда фабрика будет возвращать дефольную реализацию. Также можно инжектить зависимости.

class factory_t {
public:
    virtual auto type() const noexcept -> string_view = 0;    
};

template<typename T>
class factory<T> : public factory_t {
    virtual auto from(config_t& config) const -> std::unique_ptr<T> = 0;
};

template<>
class factory<sink::syslog_t> : public factory<sink_t> {
    typedef sink::syslog_t sink_type;

    virtual auto type() const noexcept -> string_view override {
        return "syslog";
    }

    virtual auto from(config_t& config) const -> std::unique_ptr<sink_t> override {
        // Implementation.
    }
};

^^^^^^^^^ Решает проблему кучи PIMPL и правильных фабрик.

Добавлять новые категории можно перегрузкой или полной специализацией:
template<T> void add(T t = T());
