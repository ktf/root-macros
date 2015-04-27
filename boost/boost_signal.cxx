#include <boost/signals2.hpp>
#include <vector>
#include <iostream>
#include <cstring>

using boost::signals2::signal;
using std::vector;

typedef signal<char* (unsigned)> callback_signal_t;

// Demonstrator for a callback function via boost signals
//
// based on boost signal2 tutorial
// http://www.boost.org/doc/libs/master/doc/html/signals2/tutorial.html
// and a nice description of lambda functions
// http://www.cprogramming.com/c++11/c++11-lambda-closures.html
//
// compilation:
// Note: because of the include file hierarchy of the boost libraries,
// BOOST_INCLUDE_DIR points to include directory containing the 'boost'
// include folder
// g++ -o boost_signal -I$BOOST_INCLUDE_DIR -std=c++11 boost_signal.cxx

struct HelloWorld 
{
  void operator()() const 
  { 
    std::cout << "Hello, World!" << std::endl;
  } 
};

class Worker
{
public:
  Worker() {}
  ~Worker() {}

  int convert() {
    const char* text="the white little shark";
    unsigned size=10;
    std::cout << "Worker: request Buffer of size " << size << std::endl;
    // return type of the signal is boost::optional<> which needs to be dereferenced
    char* buffer=*m_cbsignal(size);
    if (buffer) {
      strncpy(buffer, text, size-1);
    } else {
      std::cout << "Worker: no buffer provided by host" << std::endl;
    }

    size=17;
    std::cout << "Worker: request Buffer of size " << size << std::endl;
    buffer=*m_cbsignal(size);
    if (buffer) {
      strncpy(buffer, text, size-1);
    } else {
      std::cout << "Worker: no buffer provided by host" << std::endl;
    }
    return size;
  }

  int registerCallback(callback_signal_t::slot_function_type host) {
    m_cbsignal.disconnect_all_slots();
    m_cbsignal.connect(host);
  }

private:
  callback_signal_t m_cbsignal;

};

class Host
{
public:
  Host() : mBuffers() {};
  ~Host() {/* skip memory cleanup */};

  char* createBuffer(unsigned size) {
    std::cout << "Host " << this << ": create Buffer of size " << size << std::endl;
    mBuffers.push_back(new char[size]);
    memset(mBuffers.back(), 0, size);
    return mBuffers.back();
  }

  void print() {
    std::cout << "Host " << this << ": " << mBuffers.size() << " allocated buffer(s)" << std::endl;
    for (auto it : mBuffers) {
      std::cout << "       " << it << std::endl;
    }
  }

private:
  vector<char*> mBuffers;
};

int main ()
{
  // Signal with no arguments and a void return value
  signal<void ()> sig;

  // Connect a HelloWorld slot
  HelloWorld hello;
  sig.connect(hello);

  // Call all of the slots
  sig();


  // make one host and one worker instance and call wokers' convert function
  Host host1;
  Host host2;
  Worker worker;

  // callback to the function of the dedicated host instance given by lambda
  // expression
  worker.registerCallback([&host1](unsigned size) {return host1.createBuffer(size);} );
  worker.convert();
  Host *phost=&host2;
  worker.registerCallback([phost](unsigned size) {return phost->createBuffer(size);} );
  worker.convert();

  host1.print();
  host2.print();

  return 0;
}
