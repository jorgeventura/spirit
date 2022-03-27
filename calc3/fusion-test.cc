#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>

#include <iostream>
#include <string>

using namespace boost::fusion;

int main() {
   vector<int, char, std::string> stuff(1, 'x', "howdy");
   
   for_each (stuff,[](auto item) { std::cout << item << std::endl; });

   return 0;
}

