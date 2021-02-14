#include "Sequence.h"

const int _max_songs = 999;
int _seq[_max_songs] = {0x0};

void Sequence::CreateSeq(unsigned long seed, int num)
{
    if (num >= _max_songs)
    {
        num = _max_songs;
    }
    for (int i = 0; i < _max_songs - 1; i++)
    {
        _seq[i] = i + 1;
    }

    this->_next = -1;
    this->_songs_count = num;
}

int Sequence::GetNext()
{
    this->_next++;
    if (this->_next >= this->_songs_count)
    {
        this->_next = 0;
    }
    return _seq[this->_next];
}