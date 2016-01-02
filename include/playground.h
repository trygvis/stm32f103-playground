#ifndef PLAYGROUND_H
#define PLAYGROUND_H

extern "C" void halt();

template<typename T, size_t N>
static inline
size_t SizeOfArray(const T(&)[N]) {
    return N;
}

#endif
