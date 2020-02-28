#ifndef ROUTE_H
#define ROUTE_H
#include <vector>
#include <string>
#include <map>
#include <set>

class Route
{
public:
    struct RouteConfig {
        RouteConfig() : strictMode(false) {}
        bool strictMode; //是否为严格模式，若是，那么将进行完整匹配，否则匹配前缀
    };
    struct RouteMatch {
        Route *route;   //匹配的Route
        std::map<std::string, std::string> vars; //Url匹配的参数，即{id}中的id将会作为key，匹配的信息作为值
    };
private:
    std::string m_prefix;
    std::string m_rawTpl;
    std::string m_regex;
    std::set<std::string> m_methods;
    std::vector<std::string> m_namedMapping;
protected:
    bool parseTemplate(const std::string &raw, const RouteConfig &conf);
public:
    Route();
    ~Route();
public:
    //Match 匹配path路径，若匹配成功，则返回true，并将匹配信息写入到routeMatch中
    //若匹配失败，那么返回false
    bool Match(const std::string &path, RouteMatch &routeMatch);
    //Path 设置当前路由的路径,支持{}参数写法，conf为解析配置信息
    //若解析完成，则返回this指针，否则返回nullptr
    //支持路径：
    //      /a/b/c  那么将匹配/a/b/c 前缀
    //      /a/{name}/c  那么将匹配/a/b/c 等前缀，并将b作为name为key的值
    //      支持自定义匹配表达式，格式{name:\\d+}即{键:表达式}
    //若成功返回this，否则返回nullptr
    Route *Path(const std::string &raw, const RouteConfig &conf = RouteConfig());
    //PathPrefix 添加前缀匹配，该前缀仅支持普通字符串
    //若成功返回this，否则返回nullptr
    Route *PathPrefix(const std::string &prefix);
    //Methods 设置匹配的method，用于ContaintMethod匹配使用
    //若成功返回this，否则返回nullptr
    Route *Methods(std::initializer_list<std::string> &&methods);
    //ContaintMethod 返回是否包含该method，若包含，返回true，否则返回false
    //注：若未调用Methods设置，那么该函数总是返回true
    bool ContaintMethod(const std::string &method);
};

#endif // ROUTE_H
