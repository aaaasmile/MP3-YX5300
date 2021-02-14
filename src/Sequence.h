#ifndef _SEQUENCE_H_
#define _SEQUENCE_H_

class Sequence
{
public:
    void CreateSeq(unsigned long seed, int num);
    int GetNext();
    int GetPrev();
    int GetCurrSongIx();

private:
    int _currPos;
    int _songs_count;
};

#endif