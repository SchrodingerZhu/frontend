//
// Created by schrodinger on 1/21/21.
//

#ifndef FRONTEND_GRAMMAR_H
#define FRONTEND_GRAMMAR_H

#include <string_view>
#include <typeindex>
#include <vector>
#include <unordered_map>
#include <memory>

struct PContext;


using MemoKey = std::pair<size_t, std::type_index>;
using TreePtr = std::shared_ptr<struct ParseTree>;

struct MemoKeyHasher {
    size_t operator()(const MemoKey &key) const;
};

using MemoTable = std::unordered_map<MemoKey, TreePtr, MemoKeyHasher>;

class Grammar {
public:
    [[nodiscard]]  virtual TreePtr match(PContext context) const = 0;

    [[nodiscard]]  virtual bool active();
};


struct PContext {
    std::shared_ptr<MemoTable> table;

    std::string_view text;
    size_t start_position = 0;
    size_t accumulator = 0;

    [[nodiscard]] MemoKey key(const std::type_info &index) const;
    PContext next();
};

struct ParseTree {
    std::string_view parsed_region;
    std::type_index instance;
    std::vector<TreePtr> subtrees;

    ParseTree(const PContext &context, size_t length, std::type_index index, std::vector<TreePtr> subtrees);

};

#define GRAMMAR_MATCH(TYPE, BLOCK) \
    TreePtr TYPE::match(PContext context) const { \
        auto iter = context.table->find(context.key(typeid(*this)));                            \
        if (iter != context.table->end()) {                   \
            return iter->second;                        \
        } else {                    \
            BLOCK                            \
        } \
    }


#define GRAMMAR_DECLARE(TYPE, BASE, ...) \
struct TYPE : public BASE  { \
    TreePtr match(PContext context) const override; \
    __VA_ARGS__                                     \
};

#define MEMOIZATION(tree) \
    context.table->insert( { context.key(typeid(*this)), tree } ); \


#define MAKE_TREE(length, ...) \
    std::make_shared<ParseTree>(context, length, typeid(*this), std::vector<TreePtr> { __VA_ARGS__ })


GRAMMAR_DECLARE(Start, Grammar);


GRAMMAR_DECLARE(End, Grammar);

template<char C>
GRAMMAR_DECLARE(Char, Grammar);

template<char Begin, char End>
GRAMMAR_DECLARE(CharRange, Grammar);

#define TEMPLATE(TYPE, ...) TYPE<__VA_ARGS__>

template<typename Head, typename ...Tail>
GRAMMAR_DECLARE(Seq, Seq < Tail...>, virtual bool seq_match(PContext &context, std::vector<TreePtr> &) const override;);

template<typename Head>
GRAMMAR_DECLARE(Seq<Head>, Grammar, virtual bool seq_match(PContext &context, std::vector<TreePtr> &) const;);

#endif //FRONTEND_GRAMMAR_H
