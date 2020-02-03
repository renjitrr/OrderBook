#include "OrderBook.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <functional>

using namespace OrderMatching;

bool OrderBook::add( long orderId, long originalOrderId, std::string side, long qty, double price )
{
  Side s;
  Side sideToMatch;
  transform(side.begin(), side.end(), side.begin(), ::toupper);
  // reject if new order - orderId already exists in system
  if( m_OrderMap.find( orderId ) != end( m_OrderMap ) )
  {
    return false;
  }

  if( "BUY" == side )
  {
    s = Side::BUY;
    sideToMatch = Side::SELL;
  }
  else if( "SELL" == side )
  {
    s = Side::SELL;
    sideToMatch = Side::BUY;
  }
  std::shared_ptr<Order> newOrder = std::make_shared<Order>(originalOrderId, s, qty, price );
  long newOrderQty = newOrder->getQty();
  for( auto iter = begin( m_OrderMap ); iter != end( m_OrderMap ); ++iter )
  {
     long existingOrderQty{ 0 };
     if( !iter->second->isMatchedOrder() && iter->second->getSide() == sideToMatch )
     {
	    if( Side::BUY == sideToMatch && iter->second->getPrice() < newOrder->getPrice() )
	    {
	      continue;
	    }
	    if( Side::SELL == sideToMatch && iter->second->getPrice() > newOrder->getPrice() )
	    {
	      continue;
	    }
	    
	    existingOrderQty = iter->second->getQty();
	    if( newOrderQty > existingOrderQty ) 
	    {
		iter->second->setMatchedQty( existingOrderQty );
		newOrder->setMatchedQty( existingOrderQty );
                newOrder->setPartialFill();

	    }
	    else if( newOrderQty < existingOrderQty )
	    {
		newOrder->setMatchedQty( newOrderQty );
		iter->second->setMatchedQty( newOrderQty );
		iter->second->setPartialFill();
	    }
	    else
	    {
	        iter->second->setMatchedQty( newOrderQty );
	        newOrder->setMatchedQty( newOrderQty );
	    }
	    iter->second->setMatched();
	    iter->second->setMatchingOrderId( newOrder->getOrderId() );
	    newOrder->setMatched();
	    newOrder->setMatchingOrderId( iter->second->getOrderId() );
	    m_OrderMap.emplace( orderId, newOrder );
	    return true;
     }
   }
   m_OrderMap.emplace( orderId, newOrder );
   return true;
}

bool OrderBook::add( long originalOrderId, std::string side, long qty, double price )
{
  return add( originalOrderId, originalOrderId, side, qty, price );
}

bool OrderBook::cancel( long orderId )
{
  std::lock_guard<std::mutex> lockGuard( m_mutex );
  auto iter = m_OrderMap.find( orderId );
  if( iter != end( m_OrderMap ) && !iter->second->isMatchedOrder() )
  {
    m_OrderMap.erase( iter );
    return true;
  }
  return false;
}

bool OrderBook::amend( long orderId, long qty )
{
  std::lock_guard<std::mutex> lockGuard( m_mutex );
  auto iter = m_OrderMap.find( orderId );
  if( iter != end( m_OrderMap ) && !iter->second->isMatchedOrder() )
  {
    if( iter->second->getQty() > qty )
    {
	iter->second->updateQty( qty );
	return true;
    }
    auto last_element = end( m_OrderMap );
    last_element = prev( last_element );
    std::string side;
    if( iter->second->getSide() == Side::BUY )
    {
	side = "buy";
    }
    else if( iter->second->getSide() == Side::SELL )
    {
	side = "sell";
    }
    if( add( last_element->first +1, iter->second->getOrderId(), side, qty, iter->second->getPrice() ) )
    {
	m_OrderMap.erase( iter );
	return true;
    }
  }
  return false;
}

std::string OrderBook::getAskPrice( int level )
{
  std::map< double, std::shared_ptr<Order> > askPriceMap;
  std::lock_guard<std::mutex> lockGuard( m_mutex );
  for( auto iter = begin( m_OrderMap ); iter != end( m_OrderMap ); ++iter )
  {
    if( iter->second->getSide() == Side::BUY )
    {
	askPriceMap.emplace( iter->second->getPrice(), iter->second );
    }
  }
  if( level > (int)askPriceMap.size() )
  {
    auto iterLast = end( askPriceMap );
    iterLast = prev( iterLast );
    return std::to_string( level ) + "," + std::to_string( iterLast->second->getPrice() ) + "," + std::to_string( iterLast->second->getQty() );
  }
  auto iter = begin( askPriceMap );
  for( int i=0; i<level; ++i )
  {
    ++iter;
  }
  return std::to_string( level ) + "," + std::to_string( iter->second->getPrice() ) + "," + std::to_string( iter->second->getQty() );
}

std::string OrderBook::getBidPrice( int level )
{
  std::map< double, std::shared_ptr<Order> > bidPriceMap;
  std::lock_guard<std::mutex> lockGuard( m_mutex );
  for( auto iter = begin( m_OrderMap ); iter != end( m_OrderMap ); ++iter )
  {
    if( iter->second->getSide() == Side::SELL )
    {
	bidPriceMap.emplace( iter->second->getPrice(), iter->second );
    }
  }
  if( level > (int)bidPriceMap.size() )
  {
    auto iterFirst = begin( bidPriceMap );
    return std::to_string( level ) + "," + std::to_string( iterFirst->second->getPrice() ) + "," + std::to_string( iterFirst->second->getQty() );
  }
  auto iterLast = end( bidPriceMap );
  iterLast = prev( iterLast );
  for( int i=0; i<level; ++i )
  {
    --iterLast;
  }

  return std::to_string( level ) + "," + std::to_string( iterLast->second->getPrice() ) + "," + std::to_string( iterLast->second->getQty() );
}

std::string OrderBook::queryOrderStatus( long orderId )
{
  //Query may come for original order id ( prior to  amending it ) --- so need to iterate through orderMap
  std::string ret_str;
  std::lock_guard<std::mutex> lockGuard( m_mutex );
  for( auto iter = begin( m_OrderMap ); iter != end( m_OrderMap ); ++iter )
  {
    if( iter->second->getOrderId() == orderId )
    {
        std::string status{ "Open"};
	std::string fillStatus{ "" };
	std::string matchingOrderIds;
	if( iter->second->isMatchedOrder() )
	{
	    status = "Matched";
	    matchingOrderIds = iter->second->getMatchingOrderIds();
	    if( iter->second->isPartialFill() )
	    {
		fillStatus = "PartiallyFilled";
	    }
	    else
	    {
		fillStatus = "FullyFilled";
	    }
	    ret_str = "OrderId:" + std::to_string( iter->second->getOrderId() ) + ", Status:" + status + ", fillStatus:" + fillStatus +
		", Qty:" + std::to_string( iter->second->getQty() ) + ", Matched Qty:" + std::to_string( iter->second->getMatchedQty() ) + 
		", Price:" + std::to_string( iter->second->getPrice() ) + ", matchingOrderIds:" + matchingOrderIds;
	    return ret_str;
	}
	ret_str = "OrderId:" + std::to_string( iter->second->getOrderId() ) + ", Status:" + status + \
		", Qty:" + std::to_string( iter->second->getQty() ) + ", Price:" + std::to_string( iter->second->getPrice() );

	return ret_str;
    }
  }
  return "No entry in OrderBook with orderId:" + std::to_string( orderId );
}

void OrderBook::displayOrder()
{
  std::lock_guard<std::mutex> lockGuard( m_mutex );
  for( auto iter = begin( m_OrderMap ); iter != end( m_OrderMap ); ++iter )
  {
    std::cout << "OrderId:" << iter->second->getOrderId() << ", Side:" << (int)iter->second->getSide() << ", getQty:" << iter->second->getQty() <<\
	", Price:" << iter->second->getPrice() << std::endl;
  }
}

void OrderBook::listen_for_message_thread()
{
  std::string clientMessage;
  std::string reponseMessage;
  char str[1024] = { 0 }; 
  char buffer[1024] = { 0 }; 
  while ( !m_done )
  {
   
    memset( str, '\0', sizeof(str) );
    memset( buffer, '\0', sizeof(buffer) );
	
    int server_fd, new_socket;
    //int valread; 
    struct sockaddr_in address; 
    int addrlen = sizeof(address); 
    if( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0 ) 
    { 
      std::cout << "socket failed" << std::endl;
      stopThreads();
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( m_serverPort ); 
    if( bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 )
    { 
      std::cout << "bind failed" << std::endl;
      stopThreads();
    } 
    if (listen(server_fd, 3) < 0) 
    { 
      std::cout << "Error in listen" << std::endl;
      stopThreads();
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) 
    { 
      std::cout << "Error in accept" << std::endl;
      stopThreads();
    } 
    //valread = read( new_socket, str, sizeof(str) ); 
    read( new_socket, str, sizeof(str) ); 
    int l = strlen(str); 
    if( l > 0 )
    {
      clientMessage = str;
    }
    else
    {
      std::cout << "bytes read:" << l << std::endl;
    }
    std::cout << "Client Request:" << clientMessage << std::endl;
    executeCmd( clientMessage, reponseMessage );
    strcpy( buffer, reponseMessage.c_str() );
    send( new_socket, buffer, sizeof(buffer), 0 );
    //std::cout << "Response to the client:" << buffer << std::endl;
    close( server_fd );
    close( new_socket );
    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
  }
}

void OrderBook::run()
{
  std::cout << "Start listenint for requests..." << std::endl;
  m_threads.push_back( std::thread( &OrderBook::listen_for_message_thread, this ) );
}

void OrderBook::stopThreads()
{
        m_done.store( true );
}

OrderBook::~OrderBook()
{
  for_each( m_threads.begin(), m_threads.end(), std::mem_fn( &std::thread::join ) );
}

bool OrderBook::executeCmd( std::string request, std::string& response )
{
  std::string cmdType;
  std::string orderId;
  std::string orderQty;
  std::string ask_or_bid;
  std::string buy_or_sell;
  std::string orderPrice;
  char *rest = const_cast<char*>( request.c_str() );
  char* tok = strtok( rest , " ");
  if( tok != 0 )
  {
    cmdType = tok;
  }
  if( cmdType == "amend" )
  {
    if( ( tok = strtok(0, " ") ) )
    {
      orderId = tok;
    }
    if( ( tok = strtok(0, " ") ) )
    {
      orderQty = tok;
    }
    if( amend( std::stol( orderId ), std::stol( orderQty ) ) )
    {
      response = "Successfully amended the order:" + orderId; 
      return true;
    }
    response = "Failed to amended the order:" + orderId; 
    return false;
  }
  if( cmdType == "order" )
  {
    if( ( tok = strtok(0, " ") ) )
    {
      orderId = tok;
    }
    if( ( tok = strtok(0, " ") ) )
    {
      buy_or_sell = tok;
    }
    if( ( tok = strtok(0, " ") ) )
    {
      orderQty = tok;
    }
    if( ( tok = strtok(0, " ") ) )
    {
      orderPrice = tok;
    }
    if( add( std::stol( orderId ), buy_or_sell, std::stol( orderQty ), std::stod( orderPrice ) ) )
    {
      response = "Successfully added the order:" + orderId; 
      return true;
    }
    response = "Failed to add the order:" + orderId; 
    return false;
  }
  else if( cmdType == "cancel" )
  {
	
    if( ( tok = strtok(0, " ") ) )
    {
      orderId = tok;
    }
    if( cancel( std::stol( orderId ) ) )
    {
      response = "Successfully cancelled the order:" + orderId; 
      return true;
    }
    response = "Failed to cencel the order:" + orderId; 
    return false;
  }
  else if( cmdType == "q" )
  {
    std::string level;
    std::string subQuery;
	
    if( ( tok = strtok(0, " ") ) )
    {
      subQuery = tok;
    }
    if( subQuery == "level" )
    {
      if( ( tok = strtok(0, " ") ) )
      {
        ask_or_bid = tok;
      }
      if( ( tok = strtok(0, " ") ) )
      {
        level = tok;
      }
      if( ask_or_bid == "ask" )
      {
	 response = getAskPrice( std::stol( level  ) );
	 return true;
      }
      else if( ask_or_bid == "bid" )
      {
	 response = getBidPrice( std::stol( level  ) );
	 return true;
      }
    }
    else if( subQuery == "order" )
    {
      if( ( tok = strtok(0, " ") ) )
      {
        orderId = tok;
      }
      response = queryOrderStatus( std::stol( orderId ) );
      std::cout << "response:" << response << std::endl;

      return true;
    }

  }
  else if( cmdType == "display" )
  {
    displayOrder();
    response = "Orders displayed on server terminal";
    return true;
  }
  return false;
}



