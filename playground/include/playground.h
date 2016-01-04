#ifndef PLAYGROUND_H
#define PLAYGROUND_H

extern "C"
void halt();

extern "C"
void init_high();

extern "C"
int main();

extern "C"
void Default_Handler();

template<typename T, size_t N>
static inline constexpr
size_t SizeOfArray(const T(&)[N]) {
    return N;
}

#endif