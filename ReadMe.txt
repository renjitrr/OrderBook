1) ./compile.sh ---> creates order_book executable
2) cd OrderBookTest; ./compile.sh ---> creates order_book_test executable
3) cd ..; ./order_book ---> it starts the server listening for messages ( local host and default port )
4) cd OrderBookTest; ./OrderBookTest ---> will give possible arguments to be passed.
5) watch both server terminal and OrderBookTest terminals to watch the messages go back and forth.

./order_book
---Start---
Start listenint for requests...
Client Request:order 1001 buy 100 12.7

./order_book_test "order 1001 buy 100 12.7"
*** Message from Server ***:Successfully added the order:1001

./order_book_test "order 1002 sell 70 12.6"
*** Message from Server ***:Successfully added the order:1002

./order_book_test "q order 1001"
*** Message from Server ***:OrderId:1001, Status:Matched, fillStatus:PartiallyFilled, Qty:100, Matched Qty:70, Price:12.700000, matchingOrderIds:,1002

./order_book_test "q order 1002"
*** Message from Server ***:OrderId:1002, Status:Matched, fillStatus:FullyFilled, Qty:70, Matched Qty:70, Price:12.600000, matchingOrderIds:,1001

./order_book_test "order 1003 sell 10 12.6"
*** Message from Server ***:Successfully added the order:1003

./order_book_test "q order 1003"
*** Message from Server ***:OrderId:1003, Status:Open, Qty:10, Price:12.600000

./order_book_test "amend 1003 20"
*** Message from Server ***:Successfully amended the order:1003

./order_book_test "q order 1003"
*** Message from Server ***:OrderId:1003, Status:Open, Qty:20, Price:12.600000

---- all other options can be tried out ----
./order_book_test
Usage : Please use either of following options...
./order_book_test "order <orderId> buy/sell <qty> <price>"
*ex:./order_book_test "order 1001 buy 100 12.5"
./order_book_test "cancel <orderId>"
*ex:./order_book_test "cancel 1001"
./order_book_test "amend <orderId> <qty>"
*ex:./order_book_test "amend 1001 2000"
./order_book_test "q level ask <level>"
*ex:./order_book_test "q level ask 0"
./order_book_test "q level bid <level>"
*ex:./order_book_test "q level bid 1"
./order_book_test "q order <orderId>"
*ex:./order_book_test "q order 1001"
./order_book_test "display orders"



