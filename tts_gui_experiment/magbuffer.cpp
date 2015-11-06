#include "magbuffer.h"

MagBuffer::MagBuffer()
{

}

magData MagBuffer::pop(){
    magData data = buffer.front();
    buffer.pop();
    return data;
}

void MagBuffer::push(magData data){

    buffer.push(data);
}



bool MagBuffer::isEmpty(){
    return buffer.empty();
}


int MagBuffer::size(){
    buffer.size();
}



