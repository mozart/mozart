// bqueue.cc

// bounded queue template implementation

// first seemingly working version.
// loeckelt@ps.uni-sb.de

#include <stdlib.h>
#include "_generic.hh"

template <class t>
class b_queue :public generic {
private:
  t *q; // the queue
  int start;
  int end;
  int size;
  int maxsize;
public:
  b_queue(int sz) {
    if (!sz) {
      DEBUG(("bqueue: size 0, no way.\r\n"));
      exit(-1);
    }
    maxsize = sz;
    size = 0;
    q = new t[maxsize];
    start = end = 0;
  }
  virtual ~b_queue() {
    delete [] q;
  }

  inline void append(t e) {
#ifdef MY_DEBUG
    if (size == maxsize) {
      DEBUG(("b_queue: Overflow -- append ignored\r\n"));
      return;
    }
#endif
    if (!size) q[end] = e;
    else {
      end = (end++) % maxsize;
      q[end] = e;
    }
    size++;
    // cout << "appended at " << end << ", size now " << size << endl;
  }
  inline bool empty() {
    //cout << "Test: queue empty? size=" << size << endl;
    return (size == 0);
  }
  inline t pop() {
#ifdef MY_DEBUG
    if (empty()) {
      DEBUG(("pop err: queue empty\r\n"));
      exit(-1);
    }
#endif

    t retval = q[start];
    if (start != end) start=(start++) % maxsize;
    size--;
    return retval;
  }

  void write() const {
    DEBUG(("b_queue ["));
    int pos=start;
    if (size) DEBUG(("%d", q[start]));
    if (start != end) do {
      pos = (pos++) % maxsize;
      DEBUG((" %d", q[pos]));
    } while (pos != end);
    DEBUG(("] "));
    DEBUG(("start: %d, end: %d, curr_size: %d\r\n", start, end, size));
  }
};

// int main() {
//   cout << "b_queue test" << endl << "------------------------------" << endl;
//   b_queue<int> q(10);
//   q.append(1);
//   q.append(2);
//   q.append(3);
//   q.append(4);
//   int i=q.pop();
//   q.append(5);
//   q.append(6);
//   q.append(7);
//   q.append(8);
//   q.append(9);
//   q.append(10);
//   q.append(11);
//   i=q.pop();
//   i=q.pop();
//   q.append(12);
//   q.append(13);
//   cout << i << endl;
//   cout << q << endl;
// }
