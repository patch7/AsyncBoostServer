#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include <iostream>

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
boost::asio::io_service service;
boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), 2323);
boost::asio::ip::tcp::acceptor acc(service, ep);

std::string str(1024, 0);
uint32_t count = 0;

void _accept(socket_ptr sock, const boost::system::error_code &er);
void _read(socket_ptr sock, const boost::system::error_code &er, size_t len);
void _write(socket_ptr sock, const boost::system::error_code &er, size_t len);
void _run();

int main()
{
  std::vector<std::thread *> v;
  setlocale(LC_ALL, "Russian");
  socket_ptr sock(new boost::asio::ip::tcp::socket(service));
  acc.async_accept(*sock, boost::bind(_accept, sock, _1));

  for (uint16_t i = 0; i < 4; ++i)
    v.push_back(new std::thread(_run));

  for (auto& i : v)
    i->join();

  return 0;
}

void _run()
{
  service.run();
}

void _read(socket_ptr sock, const boost::system::error_code &er, size_t len)
{
  if (er)
  {
    if (sock->is_open())
    {
      sock->close();
      --count;
    }
    std::cout << count << " clients, " << er.message() << std::endl;
    return;
  }
  sock->async_write_some(boost::asio::buffer(str, len), boost::bind(_write, sock, _1, _2));
  sock->async_read_some(boost::asio::buffer(str), boost::bind(_read, sock, _1, _2));
  //std::cout << count << " clients, client has send: " << std::string(str.begin(), str.begin() + len) << std::endl;
}

void _accept(socket_ptr sock, const boost::system::error_code &er)
{
  if (er)
  {
    if (sock->is_open())
    {
      sock->close();
      --count;
    }
    std::cout << count << " clients, " << er.message() << std::endl;
    return;
  }
  socket_ptr new_sock(new boost::asio::ip::tcp::socket(service));
  acc.async_accept(*new_sock, boost::bind(_accept, new_sock, _1));
  sock->async_read_some(boost::asio::buffer(str), boost::bind(_read, sock, _1, _2));
  std::cout << ++count << " - client connected" << std::endl;
}

void _write(socket_ptr sock, const boost::system::error_code &er, size_t len)
{
  if (er)
  {
    if (sock->is_open())
    {
      sock->close();
      --count;
    }
    std::cout << count << " clients, " << er.message() << std::endl;
    return;
  }
}