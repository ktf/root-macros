#include <boost/signals2.hpp>
#include <vector>
#include <iostream>
#include <cstring>

using boost::signals2::signal;
using std::vector;

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

  int convert(signal<char* (unsigned)>::slot_function_type host) {
    signal<char* (unsigned)> cbSignal;

    cbSignal.connect(host);

    const char* text="the white little shark";
    unsigned size=10;
    std::cout << "Worker: request Buffer of size " << size << std::endl;
    // return type of the signal is boost::optional<> which needs to be dereferenced
    char* buffer=*cbSignal(size);
    strncpy(buffer, text, size-1);

    size=15;
    std::cout << "Worker: request Buffer of size " << size << std::endl;
    buffer=*cbSignal(size);
    strncpy(buffer, text, size-1);

    return size;
  }

private:

};

class Host
{
public:
  Host() : mBuffers() {};
  ~Host() {/* skip memory cleanup */};

  char* createBuffer(unsigned size) {
    std::cout << "Host: create Buffer of size " << size << std::endl;
    mBuffers.push_back(new char[size]);
    memset(mBuffers.back(), 0, size);
    return mBuffers.back();
  }

  void print() {
    std::cout << "Host: " << mBuffers.size() << " allocated buffer(s)" << std::endl;
    for (vector<char*>::const_iterator it=mBuffers.begin();
	 it!=mBuffers.end(); it++) {
      std::cout << "       " << *it << std::endl;
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
  Host host;
  Worker worker;

  // callback to the function of the dedicated host instance given by lambda
  // expression
  worker.convert([&host](unsigned size) {return host.createBuffer(size);} );

  host.print();

  return 0;
}
