#include <iostream>
#include "grammar.h"

#include "grammar.ipp"



struct Test : public Seq<Char<'a'>, Char<'b'>, Char<'b'>> {

};
int main() {
    PContext context {
            std::make_shared<MemoTable>(),
            "abb",
            0,
            0
    } ;
    auto t =  Test().match(context);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
