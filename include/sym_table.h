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

    /*!
     * The SymTable class. A symbol table with scoping support.
     * @tparam Value symbol type.
     */
    template<class Value>
    class SymTable {
        /*!
         * Symbols defined in the inner most scope, which can be popped away when the scope is escaped.
         */
        std::stack <std::vector<std::string>> local_defined{};
        /*!
         * Symbol table structure.
         */
        std::unordered_map <std::string, std::stack<SymDef<Value>>> table{};
        /*!
         * Scope depth.
         */
        size_t level{};
    public:
        /*!
         * Enter a new scope.
         */
        void enter() {
            local_defined.template emplace();
            level++;
        }

        /*!
         * Create a new symbol.
         * @tparam Args symbol constructor argument.
         * @param name symbol name.
         * @param args symbol arguments.
         * @return whether the new definition overwrites a previous symbol.
         */
        template<class ...Args>
        bool define(std::string name, Args &&... args) {
            local_defined.top().push_back(name);
            auto iter = table.find(name);
            if (iter == table.end()) {
                table.template insert({name, std::stack < SymDef<Value>>{}});
                table.at(name).push({level, Value(std::forward<Args>(args)...)});
            } else if (iter->second.top().first >= level) {
                return false;
            } else {
                iter->second.push({level, Value(std::forward<Args>(args)...)});
            }
            return true;
        }

        /*!
         * Update a value associated with the symbol in the inner most scope.
         * @tparam Args symbol constructor argument.
         * @param name symbol name
         * @param args symbol arguments.
         * @return whether the new definition overwrites a previous symbol.
         */
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

        /*!
         * Find a symbol.
         * @param name locate the symbol.
         * @return an optional structure contains the value.
         */
        std::optional <Value> operator()(std::string name) {
            auto iter = table.find(name);
            if (iter == table.end()) {
                return std::nullopt;
            } else {
                return iter->second.top().second;
            }
        }

        /*!
         * Escape the current scope.
         */
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

        template<class Collection>
        Collection local_definitions() {
            Collection collection{};
            for (auto &i : this->local_defined.top()) {
                collection.insert(std::make_pair(i, table.at(i).top().second));
            }
            return collection;
        }
    };
}


#endif //FRONTEND_SYM_TABLE_H
