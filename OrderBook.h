#ifndef __ORDER_BOOK__
#define __ORDER_BOOK__
#include "Order.h"
#include <map>
#include <thread>
#include <atomic>

namespace OrderMatching 
{
  class OrderBook
  {
    mutable std::mutex m_mutex;
    mutable std::mutex m_prntMutex;
    std::atomic<bool> m_done{false};
    std::vector<std::thread> m_threads;
    std::map<long, std::shared_ptr<Order> > m_OrderMap;
    // hardcoding ip and port
    std::string m_serverIp{ "127.0.0.1" };
    int m_serverPort{ 8000 };
    bool executeCmd( std::string request, std::string& response );
    void stopThreads();
    void listen_for_message_thread();
  public:
    bool add( long orderId, long originalOrderId, std::string side, long qty, double price );
    bool add( long originalOrderId, std::string side, long qty, double price );
    void run();
    bool cancel( long orderId );
    bool amend( long orderId, long qty );
    std::string getAskPrice( int level );
    std::string getBidPrice( int level );
    std::string queryOrderStatus( long orderId );
    void displayOrder();
    ~OrderBook();
  };

}

#endif // __ORDER_BOOK__
