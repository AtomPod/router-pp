#include "route.h"
#include <regex>
#include <iostream>

static bool splitBracket(const std::string &s, std::vector<size_t> &bs) {
    ssize_t leftBracket = -1;

    for (size_t i = 0; i < s.size(); ++i) {
        switch (s[i]) {
        case '{':
            if (leftBracket != -1) {
                return false;
            }
            leftBracket = i;
            break;
        case '}':
            if (leftBracket == -1) {
                return false;
            }
            bs.push_back(leftBracket);
            bs.push_back(i);
            leftBracket = -1;
            break;
        default:
            break;
        }
    }
    return true;
}

static std::pair<std::string,std::string> StringSplitKV(const std::string &raw, const std::string &s = " ") {
    size_t endIndex = raw.find_first_of(s);

    std::pair<std::string, std::string> kvPair;

    kvPair.first = raw.substr(0, endIndex);
    if (endIndex != std::string::npos) {
        kvPair.second = raw.substr(endIndex + 1);
    }
    return kvPair;
}


bool Route::parseTemplate(const std::string &raw, const RouteConfig &conf)
{
    std::vector<size_t> bracket;
    std::string rawTempl;
    size_t end = 0;
    std::string defaultPred = "[^/]+";
    std::string tpl = raw;
    std::vector<std::string> namedMapping;

    if (raw.size() == 0) {
        return false;
    }

    tpl.insert(tpl.begin() , '^');
    if (tpl.at(tpl.size() - 1) == '/') {
        tpl = tpl.substr(0, tpl.size() - 1);
    }

    if (conf.strictMode) {
        tpl.push_back('$');
    } else {
        tpl.append(".*");
    }

    if (!splitBracket(tpl, bracket)) {
        return false;
    }

    for (size_t i = 0; i < bracket.size(); i += 2) {
        std::string raw = tpl.substr(end, bracket[i] - end);
        std::string keyValue = tpl.substr(bracket[i] + 1, bracket[i+1] - bracket[i] - 1);
        rawTempl += raw;

        std::pair<std::string, std::string> kvPair = StringSplitKV(keyValue , ":");

        if (kvPair.first.empty()) {
            return false;
        }

        std::string pred = kvPair.second;
        if (pred.empty()) {
            pred = defaultPred;
        }

        rawTempl += "(" + pred + ")";
        namedMapping.push_back(kvPair.first);
        end = bracket[i+1] + 1;
    }

    if (end < tpl.size()) {
        rawTempl += tpl.substr(end);
    }

    m_namedMapping = std::move(namedMapping);
    m_regex = rawTempl;
    m_rawTpl = raw;
    return true;
}

Route::Route()
{

}

Route::~Route()
{

}

bool Route::Match(const std::string &path, RouteMatch &routeMatch)
{
    try {
        if (!m_prefix.empty()) {
            if (path.compare(0, m_prefix.size(), m_prefix) != 0) {
                return false;
            }
        }

        std::regex regex(m_regex);
        std::smatch matched;
        if (!std::regex_match(path.cbegin() + m_prefix.size(), path.cend(), matched, regex)) {
            return false;
        }

        if (matched.size() <= 1) {
            return false;
        }

        for (size_t i = 1; i < matched.size(); ++i) {
            routeMatch.vars[ m_namedMapping[i-1] ] = matched[i];
        }
        routeMatch.route = this;
    } catch (const std::regex_error &) {
        return false;
    } catch (...) {
        return false;
    }
    return true;
}

Route *Route::Path(const std::string &raw, const RouteConfig &conf)
{
    if (!parseTemplate(raw, conf)) {
        return nullptr;
    }
    return this;
}

Route *Route::PathPrefix(const std::string &prefix)
{
    m_prefix = prefix;
    return this;
}

Route *Route::Methods(std::initializer_list<std::string> &&methods)
{
    m_methods.insert(methods.begin(), methods.end());
    return this;
}

bool Route::ContaintMethod(const std::string &method)
{
    if (m_methods.size() == 0) {
        return true;
    }

    return m_methods.count(method) == 1;
}
