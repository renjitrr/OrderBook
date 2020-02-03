#!/bin/sh
echo "Starting $0..."
/usr/bin/g++ -std=c++1z -Wall -fPIC  -g Main.cpp OrderBook.cpp -o order_book -pthread
echo "Ending $0..."
exit 0
