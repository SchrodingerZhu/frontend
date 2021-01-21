//
// Created by schrodinger on 1/21/21.
//

#include "frontend.h"
#include <cxxabi.h>

size_t parser::MemoKeyHasher::operator()(const MemoKey &key) const {
    return key.first ^ key.second.hash_code();
}

parser::MemoKey parser::PContext::key(const std::type_info &index) const {
    return {start_position, index};
}

parser::PContext parser::PContext::next() {
    return PContext{
            table,
            text,
            start_position + accumulator,
            0
    };
}

parser::ParseTree::ParseTree(const PContext &context, size_t length, std::type_index index, std::vector<TreePtr> subtrees)
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

GRAMMAR_MATCH(Any, {
    if (context.text.size() > context.start_position) {
        MEMOIZATION(nullptr);
        return nullptr;
    }
    auto result = MAKE_TREE(1);
    MEMOIZATION(result);
    return result;
})

void parser::ParseTree::display(std::ostream &out, int count) {
    std::string indent(count * 4, ' ');
    static char buffer[512];
    int status;
    size_t length = 512;
    abi::__cxa_demangle(instance.name(), buffer, &length, &status);
    out << indent << "- " << buffer << ", parsed: \"";
    escaped_string(out, { parsed_region.begin(), parsed_region.end() });
    out << "\"" << std::endl;
    for (const auto &i : subtrees) {
        i->display(out, count + 1);
    }
}

parser::ParseTree::ParseTree(std::string_view parsed_region, std::type_index instance, std::vector<TreePtr> subtrees)
        : parsed_region(parsed_region), instance(instance), subtrees(std::move(subtrees)) {

}

std::ostream &parser::escaped_string(std::ostream &out, const std::string &s) {
    for (auto ch : s) {
        switch (ch) {
            case '\'':
                out << "\\'";
                break;

            case '\"':
                out << "\\\"";
                break;

            case '\?':
                out << "\\?";
                break;

            case '\\':
                out << "\\\\";
                break;

            case '\a':
                out << "\\a";
                break;

            case '\b':
                out << "\\b";
                break;

            case '\f':
                out << "\\f";
                break;

            case '\n':
                out << "\\n";
                break;

            case '\r':
                out << "\\r";
                break;

            case '\t':
                out << "\\t";
                break;

            case '\v':
                out << "\\v";
                break;

            default:
                out << ch;
        }
    }

    return out;
}
