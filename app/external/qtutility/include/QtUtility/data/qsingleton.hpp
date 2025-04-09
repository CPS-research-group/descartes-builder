#pragma once

#include <memory>
#include <mutex>
#include <QObject>

template<typename T>
class QSingleton : public QObject
{
public:
    static T &instance()
    {
        static std::once_flag initFlag;
        static std::shared_ptr<T> instancePtr;
        std::call_once(initFlag, []() { instancePtr.reset(new T()); });
        return *instancePtr;
    }

protected:
    QSingleton() = default;
    virtual ~QSingleton() = default;

private:
    QSingleton(const QSingleton &) = delete;
    QSingleton &operator=(const QSingleton &) = delete;
    QSingleton(QSingleton &&) = delete;
    QSingleton &operator=(QSingleton &&) = delete;
};