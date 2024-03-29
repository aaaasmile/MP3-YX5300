#include "Sequence.h"

#include <Arduino.h>

#include "MyDef.h"

const int _max_songs = 999;
int _seq[_max_songs] = {0x0};

void Sequence::CreateSeq(unsigned long seed, int num, int rndLevel) {
#ifdef DEBUG
    Console.print("Create sequence with seed ");
    Console.println(seed);
    Console.printf("Max %d, rndLevel %d\n", num, rndLevel);
#endif
    if (num >= _max_songs) {
        num = _max_songs;
    }
    for (int i = 0; i < _max_songs - 1; i++) {
        _seq[i] = i + 1;
    }
    if (rndLevel == 1) {
        randomSeed(seed);
        for (int8_t j = 0; j < 3; j++) {
            // Knuth-Fisher-Yates shuffle algorithm.
            //  While there remain elements to shuffle
            int m = num;
            while (m) {
                // Pick a remaining element…
                int i = random(m);  // intervall [0..m)
                m--;
                // And swap it with the current element.
                int t = _seq[m];
                _seq[m] = _seq[i];
                _seq[i] = t;
            }
        }
    }

#ifdef DEBUG
    Console.println("Sequence folder: ");
    for (int i = 0; i < num; i++) {
        if (i > 0) {
            Console.print(" ");
        }
        Console.print(_seq[i]);
    }
    Console.println(" -> the sequence");
#endif

    this->_currPos = -1;
    this->_songs_count = num;
}

int Sequence::GetNext() {
    this->_currPos++;
    if (this->_currPos >= this->_songs_count) {
        this->_currPos = 0;
    }
    return _seq[this->_currPos];
}

int Sequence::GetPrev() {
    this->_currPos--;
    if (this->_currPos < 0) {
        this->_currPos = this->_songs_count - 1;
    }
    return _seq[this->_currPos];
}

int Sequence::GetCurrSongIx() { return _seq[this->_currPos]; }