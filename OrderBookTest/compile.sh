#!/bin/sh
echo "Starting $0..."
/usr/bin/g++ -std=c++1z -Wall -fPIC OrderBookTest.cpp -o order_book_test
echo "Ending $0..."
exit 0
