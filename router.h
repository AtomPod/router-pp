#ifndef ROUTER_H
#define ROUTER_H
#include <map>
#include <vector>
#include <list>
#include <memory>
#include <functional>
#include "route.h"

template<class ...Args>
class Router
{
public:
    typedef std::function<void(const Route::RouteMatch&, Args...)> Callback;
    typedef std::unique_ptr<Route> RoutePtr;
    typedef std::function<void(Route*)> WalkFunc;
    typedef std::function<Callback(Callback)> Middleware;
private:
    std::vector<RoutePtr> m_routes;
    std::map<std::string, Route*> m_namedRoutes;
    std::map<Route*, Callback> m_callbacks;
    std::vector<Middleware> m_middlewares;
    std::map< Route*, std::list<Middleware> > m_routeMiddlewares;
public:
    Router();
    virtual ~Router();
public:
    //NewRoute 创建一个Route，其中若name不为空，那么将name作为名称进行注册
    //cb作为回调函数，用于匹配响应，middlewares作为中间件集合，该middleware仅用于
    //当前创建的Route
    //若成功，返回一个新的Route，否则返回nullptr
    Route *NewRoute(const std::string &name, Callback cb, std::initializer_list<Middleware> &&middlwares = {});
    //Name 返回name作为名称的Route
    Route *Name(const std::string &name);
    //Match 进行method和path的匹配，method将匹配Route的Method
    //path作为匹配的路径，args为匹配成功后，传入回调函数的参数
    //若匹配成功，返回true，否则返回false
    bool  Match(const std::string &method ,const std::string &path, Args...args);
    //Walk 遍历所有Route信息，并传入到func中
    void  Walk(WalkFunc func) const;
    //Use  添加一个中间件到Router中
    //注：该函数注册的中间件为全局使用，均先于所有Route注册的中间件调用
    void  Use(Middleware m);
};

template<class ...Args>
Router<Args...>::Router()
{

}

template<class ...Args>
Router<Args...>::~Router()
{

}

template<class ...Args>
Route *Router<Args...>::NewRoute(const std::string &name, Router::Callback cb, std::initializer_list<Middleware> &&middlwares)
{
    if (name != "") {
        if (m_namedRoutes.count(name) != 0) {
            return nullptr;
        }
    }

    if (cb == nullptr) {
        return nullptr;
    }

    RoutePtr route(new Route());
    Route *routeRaw = route.get();

    m_routes.push_back(std::move(route));
    m_namedRoutes[name] = routeRaw;
    m_callbacks[routeRaw] = cb;

    if (middlwares.size() > 0) {
        m_routeMiddlewares[routeRaw] = std::list<Middleware>(middlwares.begin(), middlwares.end());
    }

    return routeRaw;
}

template<class ...Args>
Route *Router<Args...>::Name(const std::string &name)
{
    if (m_namedRoutes.count(name) != 0) {
        return m_namedRoutes[name];
    }
    return nullptr;
}

template<class ...Args>
bool Router<Args...>::Match(const std::string &method, const std::string &path, Args... args)
{
    Route::RouteMatch routeMatch;
    for (size_t i = 0; i < m_routes.size(); ++i) {
        RoutePtr &ptr = m_routes[i];
        routeMatch.vars.clear();
        routeMatch.route = nullptr;

        if (ptr->ContaintMethod(method) && ptr->Match(path, routeMatch)) {
            Callback callback = m_callbacks[ptr.get()];
            if (callback == nullptr) {
                continue;
            }

            if (m_routeMiddlewares.count(ptr.get()) != 0) {
                const std::list<Middleware> &midList = m_routeMiddlewares.at(ptr.get());
                for (auto rbeg = midList.crbegin(); rbeg != midList.crend(); ++rbeg) {
                    callback = (*rbeg)(callback);
                }
            }

            for (int i = m_middlewares.size() - 1; i >= 0; --i) {
                callback = m_middlewares[i](callback);
            }

            callback(routeMatch, std::forward<Args>(args)...);
            return true;
        }
    }
    return false;
}

template<class ...Args>
void Router<Args...>::Walk(Router::WalkFunc func) const
{
    for (size_t i = 0; i < m_routes.size(); ++i) {
        func(m_routes[i].get());
    }
}

template<class ...Args>
void Router<Args...>::Use(Router::Middleware m)
{
    m_middlewares.push_back(m);
}


#endif // ROUTER_H
