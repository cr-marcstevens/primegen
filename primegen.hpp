/*********************************************************************************\
*                                                                                 *
* https://github.com/cr-marcstevens/primegen                                      *
*                                                                                 *
* MIT License                                                                     *
*                                                                                 *
* Copyright (c) 2021 Marc Stevens                                                 *
*                                                                                 *
* Permission is hereby granted, free of charge, to any person obtaining a copy    *
* of this software and associated documentation files (the "Software"), to deal   *
* in the Software without restriction, including without limitation the rights    *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       *
* copies of the Software, and to permit persons to whom the Software is           *
* furnished to do so, subject to the following conditions:                        *
*                                                                                 *
* The above copyright notice and this permission notice shall be included in all  *
* copies or substantial portions of the Software.                                 *
*                                                                                 *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
* SOFTWARE.                                                                       *
*                                                                                 *
\*********************************************************************************/

#ifndef PRIMEGEN_HPP
#define PRIMEGEN_HPP

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace primegen
{

// we use __builtin_ctz functions that are present with gcc and clang
// for MSVC these are missing and we define them using _BitScanForward instead
#ifdef _MSC_VER
#ifndef __clang__
#include <intrin.h>
inline unsigned long __builtin_ctzll(unsigned long long x)
{
    unsigned long ret;
    _BitScanForward64(&index, x);
    return ret;
}
#endif
#endif

template<typename Int>
Int ceil_sqrt(Int x)
{
    // r := smallest i such that i*i >= x
    Int r = std::llround(std::sqrt(double(x)) - 1.0);
    while (r*r < x)
        ++r;
    if ((r-1)*(r-1) >= x)
        throw std::runtime_error("ceil_sqrt error");
    return r;
}

class prime_sieve
{
public:
    typedef uint64_t word_t;
    static const size_t wordbits = sizeof(word_t)*8;
    static const size_t wordnumbers = 2*wordbits;
    // use tmp buffer of 256KiB for small primes
    static const size_t tmpbufsize = (1<<18) * 8 / wordbits;
    
private:
    static inline unsigned _word_ctz(uint64_t x) { return __builtin_ctzll(x); }

    // MUST be the first k primes in order, for some chosen k
    const size_t _prefilterprimes[7] = { 2, 3, 5, 7, 11, 13, 17 };
    
    std::vector<word_t> _prefilter;
    std::vector<word_t> _sieve;
    std::vector<word_t> _tmpbuf;
    size_t _tmpbufend;
    

    inline void _markbit(std::vector<word_t>& sieve, size_t n) const
    {
        sieve[ n / wordnumbers ] |= word_t(1) << ((n%wordnumbers)/2);
    }

    void _make_prefilter()
    {
        if (!_prefilter.empty())
            return;

        size_t blockmodulus = size_t(wordbits);
        for (size_t p : _prefilterprimes)
            blockmodulus *= p;
        if (blockmodulus % wordnumbers != 0)
            throw;
        _prefilter.resize(blockmodulus/wordnumbers, 0);

        size_t end = 1;
        for (size_t p : _prefilterprimes)
        {
            // compacted sieve: no even numbers
            if (p == 2)
                continue;
            for (auto j = _prefilter.begin()+end; j != _prefilter.begin()+p*end; j += end)
            {
                for (auto it1 = _prefilter.begin(), it2=j; it1 != _prefilter.begin()+end; ++it1,++it2)
                    *it2 = *it1;
            }
            end *= p;
            for (size_t i = p; i < end*wordnumbers; i += 2*p)
                _markbit(_prefilter, i);
        }
    }
    
    void _markprime(size_t p, size_t ub)
    {
        size_t begin = ((ub/p)/wordnumbers)*wordnumbers + 1;
        size_t end = ((p/wordnumbers)*wordnumbers) + 1;
        for (size_t j = begin; ; j-=wordnumbers)
        {
            word_t x = ~_sieve[j/wordnumbers];
            while (x != 0)
            {
                size_t b = _word_ctz(x);
                x ^= word_t(1)<<b;
                size_t m = p * (j+b*2);
                if (m <= ub)
                    _markbit(_sieve, m);
            }
            if (j == end)
                break;
        }
    }

    inline void _markprimefast(size_t p, size_t ub)
    {
        if (_tmpbufend == 0)
        {
            // for sufficiently large primes scan the sieve for which multiples of p to mark
            _markprime(p, ub);
            return;
        }

        // for small primes p1, .., pi we use the same strategy as the prefilter:
        //   1. create word buffer of size wordbits*p1*..*pi < tmpbufsize
        //   2. mark all multiples of primes p1, ... , pi in buffer
        //   3. OR buffer into sieve and repeat buffer until end of sieve is reached
        if (_tmpbufend * p > _tmpbuf.size())
        {
            // process _tmpbuf
            for (auto it = _sieve.begin(); it < _sieve.end(); it += _tmpbufend)
            {
                auto itend = it + _tmpbufend; 
                if (itend > _sieve.end())
                    itend = _sieve.end();
                for (auto it1 = it, it2 = _tmpbuf.begin(); it1 != itend; ++it1,++it2)
                    *it1 |= *it2;
            }
            if (p < 192)
            {
                // reset _tmpbuf
                _tmpbufend = 1;
                _tmpbuf[0] = 0;
            } else
            {
                // stop using _tmpbuf and call _markprime instead
                _tmpbufend = 0;
                _markprime(p, ub);
                return;
            }
        }
        for (auto it = _tmpbuf.begin()+_tmpbufend; it != _tmpbuf.begin()+p*_tmpbufend; it += _tmpbufend)
        {
            auto itend = it + _tmpbufend;
            for (auto it1 = it, it2 = _tmpbuf.begin(); it1 != itend; ++it1,++it2)
                *it1 = *it2;
        }
        _tmpbufend *= p;
        for (size_t i = p; i < wordnumbers*_tmpbufend; i += 2*p)
            _markbit(_tmpbuf, i);
    }
            
public:


    // generate all primes p in range [lb,ub] and for each call callback(p)
    template<typename F>
    void genprimes(size_t lb, size_t ub, F&& callback)
    {
        // initialize prefilter
        _make_prefilter();

        // initialize sieve with prefilter
        size_t ub_block_factor = (ub + 2 + (_prefilter.size()*wordnumbers) - 1) / (_prefilter.size()*wordnumbers);
        _sieve.resize( ub_block_factor * _prefilter.size() );
        for (size_t i = 0; i < ub_block_factor; ++i)
            memcpy(&_sieve[i * _prefilter.size()], &_prefilter[0], _prefilter.size()*wordbits/8);
        _sieve[0] |= 1; // mark number 1 in sieve
        
        // handle small primes of prefilter
        for (auto p : _prefilterprimes)
        {
            if (p >= ub)
                return;
            if (p >= lb)
                callback(p);
        }

        // initialize tmp buffer        
        _tmpbuf.resize(tmpbufsize);
        _tmpbuf[0] = 0;
        _tmpbufend = 1;

        size_t maxp = ceil_sqrt(ub);

        // start sieving!
        for (size_t n = 1; n < ub; n += wordnumbers)
        {
            word_t x = ~_sieve[n/wordnumbers];
            while (x != 0)
            {
                size_t b = _word_ctz(x);
                x ^= word_t(1)<<b;
                size_t p = n+2*b;
                if (p >= ub)
                    return;
                if (p >= lb)
                    callback(p);
                if (p < maxp)
                    _markprimefast(p, ub);
            }
        }
    }
};

// print prime p
struct printprime
{
    size_t _printlen;
    size_t _printlast;
    char _printstr[32];
    
    printprime() : _printlast(~size_t(0)) {}
    
    void operator()(size_t p)
    {
        // first initialization (and upon any decrease to be generic)
        if (p < _printlast)
        {
            for (size_t i = 0; i < 30; ++i)
                _printstr[i] = '0';
            _printstr[30] = 0;
            _printlen = 1;
            _printlast = 0;
        }
        // update status
        size_t d = p - _printlast;
        _printlast = p;
        // update print string using d
        size_t i = 29;
        do
        {
            d += _printstr[i] - '0';
            _printstr[i] = '0' + (d%10);
            d /= 10;
            --i;
        } while (d != 0);
        // increase print length if needed
        if (i < 29 - _printlen)
            _printlen = 29-i;
        // print string
        puts(_printstr + 30 - _printlen);
    }
};

} // namespace

#endif
