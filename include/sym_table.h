//
// Created by schrodinger on 1/21/21.
//

#ifndef FRONTEND_SYM_TABLE_H
#define FRONTEND_SYM_TABLE_H

#include <unordered_map>
#include <vector>
#include <string>
#include <stack>

namespace symtable {
    template<class Value>
    using SymDef = std::pair<size_t, Value>;

    template<class Value>
    class SymTable {
        std::stack<std::vector<std::string>> local_defined{};
        std::unordered_map<std::string, std::stack<SymDef<Value>>> table{};
        size_t level{};
    public:
        void enter() {
            local_defined.template emplace();
            level++;
        }

        template<class ...Args>
        bool define(std::string name, Args &&... args) {
            local_defined.top().push_back(name);
            auto iter = table.find(name);
            if (iter == table.end()) {
                table.template insert( { name, std::stack<SymDef<Value>> {} } );
                table.at(name).push( {level, Value(std::forward<Args>(args)...)});
            } else if (iter->second.top().first >= level) {
                return false;
            } else {
                iter->second.push({level, Value(std::forward<Args>(args)...)});
            }
            return true;
        }

        template<class ...Args>
        bool update(std::string name, Args &&... args) {
            auto iter = table.find(name);
            if (iter == table.end()) {
                return false;
            } else {
                iter->second.top().second = Value(std::forward<Args>(args)...);
            }
            return true;
        }

        std::optional<Value> operator()(std::string name) {
            auto iter = table.find(name);
            if (iter == table.end()) {
                return std::nullopt;
            } else {
                return iter->second.top().second;
            }
        }

        void escape() {
            auto a = std::move(local_defined.top());
            local_defined.pop();
            for (const auto &i : a) {
                auto iter = table.find(i);
                if (iter == table.end()) continue;
                if (iter->second.size() <= 1) {
                    table.erase(iter);
                } else {
                    iter->second.pop();
                }
            }
            level--;
        }
    };
}


#endif //FRONTEND_SYM_TABLE_H
