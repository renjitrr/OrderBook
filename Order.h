#ifndef __ORDER__
#define __ORDER__

#include <iostream>
#include <thread>
#include <queue>
#include <deque>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <string>
#include <chrono>
#include <map>

namespace OrderMatching
{
  enum class Side
  {
	BUY,
	SELL,
	UNKNWON
  };

  class Order
  {
    long m_OrderId { 0 };
    Side m_side{ Side::UNKNWON };
    long m_qty { 0 };
    long m_matchedQty{ 0 };
    double m_price { 0.0 };
    bool m_matched { false };
    bool m_partialFill { false };
    std::vector<long> m_matchedOrderIds;
  public:
    Order( long orderId, Side side, long qty, double price ):
      m_OrderId( orderId ), m_side( side ), m_qty( qty ), m_price( price )
    {
    }
    long getOrderId() { return m_OrderId; }
    Side getSide() { return m_side; }
    long getQty() { return m_qty; }
    double getPrice() { return m_price; }
    bool isMatchedOrder() { return m_matched; }
    bool isPartialFill() { return m_partialFill; }
    void setMatched() { m_matched = true; }
    void setPartialFill() { m_partialFill = true; }
    long getMatchedQty() { return m_matchedQty; }
    void setMatchedQty( long qty ) { m_matchedQty = qty; }
    void setMatchingOrderId( long orderId ) { m_matchedOrderIds.emplace_back( orderId ); }
    std::string getMatchingOrderIds()
    {
        std::string orderIds;
        for( size_t i=0; i<m_matchedOrderIds.size(); ++i )
        {
            orderIds += "," + std::to_string( m_matchedOrderIds[ i ] );
        }
        return orderIds;
    }

    void updateQty( long qty ) { m_qty = qty; }

    Order() = delete;
    ~Order() = default;
    Order( const Order& ) = delete;
    Order& operator=( const Order& ) = delete;
    Order( Order&& ) = delete;
    Order& operator=( Order&& ) = delete;

 };

}

#endif // __ORDER__
