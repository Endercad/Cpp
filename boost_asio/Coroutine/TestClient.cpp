#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;
const int MAX_LENGTH = 1024;

int main(){
    try{
        io_context io_context;
        tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 8848);
        tcp::socket socket(io_context);
        boost::system::error_code ec = boost::asio::error::host_not_found;
        socket.connect(endpoint, ec);
        if(ec){
            throw boost::system::system_error(ec);
        }
        char request[MAX_LENGTH];
        memset(request, 0, MAX_LENGTH);
        cout << "Enter message: ";
        std::cin.getline(request, MAX_LENGTH);
        size_t request_length = strlen(request);
        boost::asio::write(socket, boost::asio::buffer(request, request_length));
        char reply[MAX_LENGTH];
        memset(reply, 0, MAX_LENGTH);
        size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, request_length));
        cout << "Reply is: ";
        cout.write(reply, reply_length);
    }
    catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}