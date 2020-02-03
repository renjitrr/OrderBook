#include "Order.h"
#include "OrderBook.h"
#include <string.h>
#include <fstream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace OrderMatching;

int main( int argc, char* argv[] )
{
    std::cout << "---Start---" << std::endl;
    OrderBook orderBook;
    orderBook.run();
    return 0;
}

