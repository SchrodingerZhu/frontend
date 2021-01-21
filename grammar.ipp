template<char C>
GRAMMAR_MATCH(Char<C>,
              {
                  auto result =
                          context.start_position != context.text.size()
                          && context.text[context.start_position] == C
                          ? MAKE_TREE(1) : nullptr;
                  MEMOIZATION(result);
                  return
                          result;
              })

template<char Begin, char End>
GRAMMAR_MATCH(TEMPLATE(CharRange, Begin, End),
              {
                  auto result =
                          context.start_position != context.text.size()
                          && context.text[context.start_position] >= Begin
                          && context.text[context.start_position] <= End
                          ? MAKE_TREE(1) : nullptr;
                  MEMOIZATION(result);
                  return
                          result;
              })

template<typename Head, typename ...Tail>
GRAMMAR_MATCH(TEMPLATE(Seq, Head, Tail...),

              {
                  std::vector<TreePtr> subtrees;
                  auto flag = seq_match(context, subtrees);
                  if (flag) {
                      auto tree = MAKE_TREE(context.accumulator, std::move(subtrees));
                      MEMOIZATION(tree);
                      return tree;
                  }
                  MEMOIZATION(nullptr);
                  return nullptr;
              })

template<typename Head, typename ...Tail>
bool Seq<Head, Tail...>::seq_match(PContext &context, std::vector<TreePtr> &subtrees) const {
    TreePtr tree = Head().match(context.next());
    if (tree) {
        context.accumulator += tree->parsed_region.size();
        subtrees.push_back(tree);
        return this->Seq<Tail...>::seq_match(context, subtrees);
    } else {
        return false;
    }
}

template<typename Head>
GRAMMAR_MATCH(TEMPLATE(Seq, Head),

              {
                  TreePtr tree = Head().match(context.next());
                  if (tree) {
                      auto seq = MAKE_TREE(tree->parsed_region.size(), tree);
                      MEMOIZATION(seq);
                      return seq;
                  }
                  MEMOIZATION(nullptr);
                  return nullptr;
              })

template<typename Last>
bool Seq<Last>::seq_match(PContext &context, std::vector<TreePtr> &subtrees) const {
    TreePtr tree = Last().match(context.next());
    if (tree) {
        context.accumulator += tree->parsed_region.size();
        subtrees.push_back(tree);
        return true;
    } else {
        return false;
    }
}


template<typename Head, typename ...Tail>
GRAMMAR_MATCH(TEMPLATE(Ord, Head, Tail...),
              {
                  auto tree = Head().match(context.next());
                  if (tree) {
                      auto ord = MAKE_TREE(tree->parsed_region.size(), tree);
                      MEMOIZATION(ord);
                      return ord;
                  } else {
                      return Ord<Tail...>::match(context);
                  }
              })

template<typename Head>
GRAMMAR_MATCH(TEMPLATE(Ord, Head),

              {
                  TreePtr tree = Head().match(context.next());
                  if (tree) {
                      auto ord = MAKE_TREE(tree->parsed_region.size(), tree);
                      MEMOIZATION(ord);
                      return ord;
                  }
                  MEMOIZATION(nullptr);
                  return nullptr;
              })

template<typename S>
GRAMMAR_MATCH(Optional<S>, {
    TreePtr tree = S().match(context.next());
    if (tree) {
        auto some = MAKE_TREE(tree->parsed_region.size(), tree);
        MEMOIZATION(some);
        return some;
    }
    auto none = MAKE_TREE(0);
    MEMOIZATION(none);
    return none;
})