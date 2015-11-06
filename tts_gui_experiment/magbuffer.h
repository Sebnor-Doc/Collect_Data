#ifndef MAGBUFFER_H
#define MAGBUFFER_H

#include <array>
#include <vector>
#include <queue>

using std::array;
using std::vector;
using std::queue;


class MagBuffer
{

public:
    explicit MagBuffer();

    magData pop();
    void push(magData data);
    int size();
    bool isFull();
    bool isEmpty();

private:
    typedef array< array<int,3>, NUM_OF_SENSORS > magData;
    queue<magData> buffer;

};



#endif // MAGBUFFER_H
