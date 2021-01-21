//
// Created by schrodinger on 1/21/21.
//

#include "grammar.h"

size_t MemoKeyHasher::operator()(const MemoKey &key) const {
    return key.first ^ key.second.hash_code();
}

bool Grammar::active() { return false; }

MemoKey PContext::key(const std::type_info &index) const {
    return { start_position, index };
}

PContext PContext::next() {
    return PContext {
            table,
            text,
            start_position + accumulator,
            0
    };
}

ParseTree::ParseTree(const PContext &context, size_t length, std::type_index index, std::vector<TreePtr> subtrees)
        : parsed_region(context.text.substr(context.start_position, length)), instance(index),
          subtrees(std::move(subtrees)) {}

GRAMMAR_MATCH(Start, {
    auto result =
            context.start_position == 0 ? MAKE_TREE(0) : nullptr;
    MEMOIZATION(result);
    return result;
})

GRAMMAR_MATCH(End, {
    auto result =
            context.start_position == context.text.size() ? MAKE_TREE(0) : nullptr;
    MEMOIZATION(result);
    return result;
})

GRAMMAR_MATCH(Nothing, {
    auto result = MAKE_TREE(0);
    MEMOIZATION(result);
    return result;
})
