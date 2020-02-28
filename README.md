# router-pp
基于C++的Router，用于注册路由，并自动匹配调用对应的路由，支持中间件注册

##### bool Match(const std::string &path, RouteMatch &routeMatch);
    Match 匹配path路径，若匹配成功，则返回true，并将匹配信息写入到routeMatch中，若匹配失败，那么返回false


##### Route *Path(const std::string &raw, const RouteConfig &conf = RouteConfig());
    Path 设置当前路由的路径,支持{}参数写法，conf为解析配置信息若解析完成，则返回this指针，否则返回nullptr  
    支持路径：  
          /a/b/c  那么将匹配/a/b/c 前缀  
          /a/{name}/c  那么将匹配/a/b/c 等前缀，并将b作为name为key的值  
          支持自定义匹配表达式，格式{name:\\d+}即{键:表达式}  
    若成功返回this，否则返回nullptr


##### Route *PathPrefix(const std::string &prefix);
    PathPrefix 添加前缀匹配，该前缀仅支持普通字符串，若成功返回this，否则返回nullptr

##### Route *Methods(std::initializer_list<std::string> &&methods);
    Methods 设置匹配的method，用于ContaintMethod匹配使用，若成功返回this，否则返回nullptr

##### bool ContaintMethod(const std::string &method);
    ContaintMethod 返回是否包含该method，若包含，返回true，否则返回false注：若未调用Methods设置，那么该函数总是返回true

##### 例子
    #include <iostream>
    #include "router.h"

    int main()
    {
        Router<int> a;

        a.NewRoute("atom", [](const Route::RouteMatch &m, int c)    {
            std::cout << "a=" << m.vars.at("a") << " b=" << m.vars.at("b") << " c=" << c << '\n';
        }, 
        {
         [](Router<int>::Callback fn) -> Router<int>::Callback {
             return [fn](const Route::RouteMatch &m, int c) {
                 std::cout << "beta" << '\n';
                  fn(m, std::move(c));
              };
          }
        })->Path("/{a:\\d+}/{b:\\d+}")->Methods({"GET"});
    
        a.Use([](Router<int>::Callback fn) ->           Router<int>::Callback {
            return [fn](const Route::RouteMatch &m, int c) {
                std::cout << "hello" << '\n';
                fn(m, std::move(c));
         };
        });

        a.Match("GET" ,"/123/456/4654", 15);
        return 0;
    }
