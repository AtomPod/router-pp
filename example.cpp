#include <iostream>
#include "router.h"

int main()
{
    Router<int> a;

    a.NewRoute("atom", [](const Route::RouteMatch &m, int c) {
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

    a.Use([](Router<int>::Callback fn) -> Router<int>::Callback {
        return [fn](const Route::RouteMatch &m, int c) {
            std::cout << "hello" << '\n';
            fn(m, std::move(c));
        };
    });

    a.Match("GET" ,"/123/456/4654", 15);
    return 0;
}
