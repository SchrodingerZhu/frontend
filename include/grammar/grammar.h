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
#include <string>
#include <ostream>
#include <cstring>

namespace parser {
    struct PContext;


    using MemoKey = std::pair<size_t, std::type_index>;
    using TreePtr = std::shared_ptr<struct ParseTree>;

    /*!
     * The MemoKeyHasher class. Used to generate the hash value of memo key.
     */
    struct MemoKeyHasher {
        size_t operator()(const MemoKey &key) const;
    };

    using MemoTable = std::unordered_map<MemoKey, TreePtr, MemoKeyHasher>;

    /*!
     * The Grammar class. Represents a grammar rule.
     */
    class Grammar {
    public:
        /*!
         * Parse current context.
         * @param context parser context.
         * @return tree node.
         */
        [[nodiscard]]  virtual TreePtr match(PContext context) const = 0;
    };

    /*!
     * The PContext class. Represents the current context of parser.
     */
    struct PContext {
        /*!
         * Memory table.
         */
        std::shared_ptr<MemoTable> table;
        /*!
         * Source input.
         */
        std::string_view text;
        /*!
         * Current parse start position.
         */
        size_t start_position = 0;
        /*!
         * Current parsed length from start position.
         */
        size_t accumulator = 0;

        /*!
         * Create a memoization key for a grammar rule from current context.
         * @param index grammar rule type info.
         * @return memoization
         */
        [[nodiscard]] MemoKey key(const std::type_info &index) const;

        /*!
         * Create a new parser context with current location as the new starting point.
         * @return new parser context.
         */
        PContext next();
    };

    /*!
     * The ParseTree class.
     */
    struct ParseTree {
        /*!
         * Source section of current tree node.
         */
        std::string_view parsed_region;
        /*!
         * Type info of the grammar rule.
         */
        std::type_index instance;
        /*!
         * Subtrees.
         */
        std::vector<TreePtr> subtrees;

        /*!
         * Create a new tree node based parsed region.
         * @param parsed_region source section of current tree node.
         * @param instance grammar instance type info.
         * @param subtrees subtree nodes.
         */
        ParseTree(std::string_view parsed_region, std::type_index instance, std::vector<TreePtr> subtrees);

        /*!
         * Create a new tree node based on current context.
         * @param context parser context.
         * @param length parsed length.
         * @param index grammar instance type info.
         * @param subtrees subtree nodes.
         */
        ParseTree(const PContext &context, size_t length, std::type_index index, std::vector<TreePtr> subtrees);

        /*!
         * Print out the parsed tree structure.
         * @param out output stream.
         * @param count indentation level.
         */
        void display(std::ostream &out, int count = 0);

        /*!
         * Collapse silent rules.
         * @tparam S rule selector mark those active rules.
         * @return A new ParseTree with silent rule nodes collapsed.
         */
        template<class S>
        std::vector<TreePtr> compress() const;
    };


#define GRAMMAR_MATCH(TYPE, BLOCK) \
    parser::TreePtr parser::TYPE::match(parser::PContext context) const { \
        auto iter = context.table->find(context.key(typeid(*this)));                            \
        if (iter != context.table->end()) {                   \
            return iter->second;                        \
        } else {                    \
            BLOCK                            \
        } \
    }


#define GRAMMAR_DECLARE(TYPE, BASE, ...) \
/*! PEG Grammar rule TYPE. */                                         \
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
    GRAMMAR_DECLARE(Seq, Seq < Tail...>,
                    virtual bool seq_match(PContext &context, std::vector<TreePtr> &) const override;);

    template<typename Head>
    GRAMMAR_DECLARE(Seq<Head>, Grammar, virtual bool seq_match(PContext &context, std::vector<TreePtr> &) const;);

    template<typename Head, typename ...Tail>
    GRAMMAR_DECLARE(Ord, Ord < Tail...>);

    template<typename Head>
    GRAMMAR_DECLARE(Ord<Head>, Grammar);

    template<typename S>
    GRAMMAR_DECLARE(Optional, Grammar);

    template<typename S>
    GRAMMAR_DECLARE(Plus, Grammar);

    template<typename S>
    GRAMMAR_DECLARE(Asterisk, Grammar);

    GRAMMAR_DECLARE(Nothing, Grammar);

    GRAMMAR_DECLARE(Any, Grammar);

    template<typename S>
    GRAMMAR_DECLARE(Not, Grammar);

#define RULE(NAME, ...)      \
struct NAME : public __VA_ARGS__ {   \
};

    /*!
     * Grammar selector.
     * @tparam Head active grammars.
     * @tparam Tail active grammar.
     */
    template<typename Head, typename... Tail>
    struct Selector : Selector<Tail...> {
        bool operator()(std::type_index index) override {
            if (index == typeid(Head)) {
                return true;
            }
            return Selector<Tail...>::operator()(index);
        }
    };

    template<typename Last>
    struct Selector<Last> {
        virtual bool operator()(std::type_index index) {
            if (index == typeid(Last)) {
                return true;
            }
            return false;
        }
    };

    /*!
     * ASCII whitespace.
     */
    RULE(Separator, Asterisk<Ord<Char<'\t'>, Char<' '>, Char<'\n'>, Char<'\r'>, Char<'\v'>>>);

    /*!
     * Builtin combinator to joint rules.
     * @tparam Sep separator.
     * @tparam Rules other grammar rules.
     */
    template<typename Sep, typename... Rules>
    struct Interleaved;

    template<typename Sep, typename Rule0, typename... RulesRest>
    struct Interleaved<Sep, Rule0, RulesRest...>
            : Seq<Rule0, Sep, Interleaved<Sep, RulesRest...>> {
    };

    template<typename Rule0>
    struct Interleaved<Rule0> : Rule0 {
    };

    template<typename... Rules>
    struct SpaceInterleaved : public Interleaved<Separator, Rules...> {
    };
    /*!
    * Builtin combinator to declare a keyword.
    * @tparam Chars keyword characters.
    */
    template<char ...Chars>
    struct Keyword : Seq<Char<Chars>...> {
    };

    /*!
     * Print escaped string for debug usage.
     * @param out output stream
     * @param s string data
     * @return the output stream with the data printed.
     */
    std::ostream &escaped_string(std::ostream &out, std::string const &s);

}

#endif //FRONTEND_GRAMMAR_H
