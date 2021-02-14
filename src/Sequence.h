#ifndef _SEQUENCE_H_
#define _SEQUENCE_H_

class Sequence
{
public:
    void CreateSeq(unsigned long seed, int num);
    int GetNext();

private:
    int _next;
    int _songs_count;
};

#endif