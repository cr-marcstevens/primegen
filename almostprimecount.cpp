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

#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <utility>
#include <vector>
#include <deque>
#include <map>
#include <list>

#include "primegen.hpp"
#include "program_options.hpp"

namespace pg = primegen;
namespace po = program_options;

namespace primegen 
{

// A k-almost prime counter for < 2^n
// Works like the sieve of Eratosthenes, except:
// - for every integer we keep a factor counter and a cumulative product
// - every prime and its powers 'walk' over the sieve and increase the counter
// - if we 'walk' every prime < sqrt(2^n) then there can be at most 1 prime factor >= sqrt(2^n)
//   to check this we compare the final cumulative product with the actual integer (equal <=> "no prime factor >= sqrt(2^n)")

class almost_prime_sieve
{
public:
    typedef std::uint64_t integer_t;
    typedef std::uint8_t count_t; // used to count number of prime factors k: 8 bits: 0 <= k < 256
    typedef std::size_t size_t;

#define segment_size (1ULL<<16)

private:
    size_t _maxbits, _maxval, _sqrtmaxval;
public:
    almost_prime_sieve(size_t maxbits)
        : _maxbits(maxbits)
    {
        _maxval = (1ULL << _maxbits);
        _sqrtmaxval = ceil_sqrt(_maxval);
        if (_maxval < segment_size)
            throw std::runtime_error("maxbits too small");
    }

private:
    std::vector<integer_t> _primecache;
public:
    void prepare_primecache()
    {
        _primecache.clear();
        std::cout << "Computing set of primes p < " << _sqrtmaxval << "..." << std::endl;
        prime_sieve ps;
        ps.genprimes(2, _sqrtmaxval, [&](size_t p){ _primecache.emplace_back(p); });
        std::cout << "Largest prime: " << _primecache.back() << std::endl;
    }

private:
    std::vector< std::vector<size_t> > interval_counts_odd;
    std::vector< std::vector<size_t> > interval_counts;
    std::vector< count_t > count;
    std::vector< integer_t > factor;

    struct prime_t 
    {
        prime_t(const prime_t&) = default;
        prime_t& operator= (const prime_t&) = default;
        prime_t (integer_t _p = 0, integer_t _n = 0)
            : p(_p), n(_n)
        {}
        integer_t p, n;
    };
    struct primepower_t
    {
        primepower_t(const primepower_t&) = default;
        primepower_t& operator= (const primepower_t&) = default;
        primepower_t(integer_t _p = 0, integer_t _q = 0, integer_t _n = 0)
            : p(_p), q(_q), n(_n)
        {}
        integer_t p, q, n;
    };
    
    inline void count_prime(size_t offset, prime_t& p)
    {
        if (p.n < offset || p.n >= offset+segment_size)
            throw std::runtime_error("count_prime: out of range");
        size_t i = (p.n - offset)/2;
        for (; i < segment_size/2; i += p.p)
        {
            ++count[i];
            factor[i] *= p.p;
        }
        p.n = 2*i+1 + offset;
    }

    inline void count_primepower(size_t offset, primepower_t& p)
    {
        if (p.n < offset || p.n >= offset+segment_size)
            throw std::runtime_error("count_prime: out of range");
        size_t i = (p.n - offset)/2;
        for (; i < segment_size/2; i += p.q)
        {
            ++count[i];
            factor[i] *= p.p;
        }
        p.n = 2*i+1 + offset;
    }

public:    
    void count_almostprimes(bool countodd =  true, bool countall = true)
    {
        interval_counts.clear();
        interval_counts.resize(_maxbits+1, std::vector<size_t>(_maxbits+1, 0));
        interval_counts_odd.clear();
        interval_counts_odd.resize(_maxbits+1, std::vector<size_t>(_maxbits+1, 0));

        count.resize(segment_size/2);
        factor.resize(segment_size/2);

        // start of first segment
        size_t offset = 0;

        // small prime (powers) < segmentsize
        std::vector<prime_t> smallprimes;
        std::vector<primepower_t> smallprimepowers;
        // prime (powers): [segmentsize, sqrtmaxbits)
        std::deque< std::vector< prime_t > > segmentprimes( (_sqrtmaxval / segment_size)*2+4 );
        std::deque< std::vector< primepower_t > > segmentprimepowers( (_sqrtmaxval / segment_size)*2+4 );
        // prime powers > sqrtmaxbits
        std::map< integer_t, std::list<primepower_t> > largeprimepowers;

        // distribute primes
        for (auto p : _primecache)
        {
            if (p == 2)
                continue;
            if (2*p < segment_size)
                smallprimes.emplace_back(p,p);
            else
                segmentprimes[p / segment_size].emplace_back(p,p);
            for (size_t q = p*p, oq = p; q < _maxval; q *= p)
            {
                if (q < oq) // overflow: real q > _maxval
                    break;
                oq = q;

                if (2*q < segment_size)
                {
                    smallprimepowers.emplace_back(p,q,q);
                } else {
                    if (q < _sqrtmaxval)
                        segmentprimepowers[q / segment_size].emplace_back(p,q,q);
                    else
                        largeprimepowers[q].emplace_back(p,q,q);
                }
            }
        }

        interval_counts[0][0] = 1;
        size_t k = 1, lb = 1ULL<<k, ub = 2ULL<<k;
        for (; offset < (1ULL<<_maxbits); offset += segment_size)
        {
            // reset count & factor
            std::fill(count.begin(), count.end(), 0);
            std::fill(factor.begin(), factor.end(), 1);

            // process small prime (powers) < segmentsize
            for (auto& p : smallprimes)
                count_prime(offset, p);
            for (auto& q : smallprimepowers)
                count_primepower(offset, q);

            // process large prime (powers) > segmentsize
            if (!segmentprimes.empty())
            {
                for (auto p : segmentprimes.front())
                {
                    count_prime(offset, p);
                    size_t i = (p.n - offset) / segment_size;
                    if (i == 0 || i >= segmentprimes.size())
                        throw std::runtime_error("segmentprimes insertion error");
                    segmentprimes[i].emplace_back(p);
                }
                segmentprimes.emplace_back( std::move( segmentprimes.front() ) );
                segmentprimes.pop_front();
                segmentprimes.back().clear();
            }
            if (!segmentprimepowers.empty())
            {
                for (auto q : segmentprimepowers.front())
                {
                    count_primepower(offset, q);
                    size_t i = (q.n - offset) / segment_size;
                    if (i == 0 || i >= segmentprimepowers.size())
                        throw std::runtime_error("segmentprimepowers insertion error");
                    segmentprimepowers[i].emplace_back(q);
                }
                segmentprimepowers.emplace_back( std::move( segmentprimepowers.front() ) );
                segmentprimepowers.pop_front();
                segmentprimepowers.back().clear();
            }
            
            // process very large prime powers >= _sqrtmaxval (of primes < _sqrtmaxval)
            auto it = largeprimepowers.begin();
            while (it != largeprimepowers.end() && it->first < offset+segment_size)
            {
                while (!it->second.empty())
                {
                    auto& q = it->second.front();
                    count_primepower(offset, q);
                    largeprimepowers[q.n].splice( largeprimepowers[q.n].begin(), it->second, it->second.begin() );
                }
                it = largeprimepowers.erase(it);
            }

            // integers that differ from their current factor product lack exactly one large prime >= _sqrtmaxval
            for (size_t i = 0, n = offset + 1; i < segment_size/2; ++i, n += 2)
            {
                if (factor[i] != n)
                    ++count[i];
            }

            // do all counts
            while (true)
            {
                if (lb < offset) throw;
                size_t segment_ub = std::min<size_t>(ub, offset+segment_size);

                for (integer_t i = (lb-offset)/2; i < (segment_ub-offset)/2; ++i)
                    ++interval_counts_odd[k][ count[i] ];

                if (segment_ub == ub)
                {
                    if (k == 1)
                    {
                        std::cout << "Output format: 'k: c(k,1) c(k,2) ....', where c(k,i) = #{ (odd) i-almostprimes in [2^k, 2^(k+1)) }." << std::endl;
                    }
                    interval_counts[k] = interval_counts_odd[k];
                    for (size_t i = 1; i <= k; ++i)
                        interval_counts[k][i] += interval_counts[k-1][i-1];
                    integer_t maxnum = 0;
                    for (auto v : interval_counts[k])
                        if (v > maxnum)
                            maxnum = v;
                    size_t printwidth = std::to_string(maxnum).size();
                    if (countodd)
                    {
                        std::cout << std::setw(2) << k << ":";
                        for (size_t c = 1; c <= k; ++c)
                            std::cout << " " << std::setw(printwidth) << interval_counts_odd[k][ c ];
                        std::cout << " (odd) " << std::endl;
                    }
                    if (countall)
                    {
                        std::cout << std::setw(2) << k << ":";
                        for (size_t c = 1; c <= k; ++c)
                            std::cout << " " << std::setw(printwidth) << interval_counts[k][ c ];
                        std::cout << " (all) " << std::endl;
                    }
                    // finished counting for [2^k, 2^(k+1) ), increase k
                    ++k;
                    lb = 1ULL<<k;
                    ub = 2ULL<<k;
                } else {
                    // this k is still unfinished
                    lb = segment_ub;
                    break;
                }
            }
        }
    }
};

}

int main(int argc, char** argv)
{
    // command line interface
    size_t k = 1;
    po::options_description opts("Command line options");
    opts.add_options()
        ("help,h", "Show options")
        ("k", po::value<size_t>(&k), "Output almost prime counts [2^i, 2^(i+1)) for i in [1,k). Must be 16 <= k < 64.")
        ("odd,o", "Print counts for odd almostprimes")
        ("all,a", "Print counts for all almostprimes")
        ;
    po::variables_map vm;
    bool allow_unregistered = false, allow_positional = true;
    po::store(po::parse_command_line(argc, argv, opts, allow_unregistered, allow_positional), vm);
    // if at least 1 positional argument is given then parse as: <k>
    if (vm.positional.size() >= 1)
    {
        k = vm.positional[0].as<size_t>();
    }

    // print help
    if (vm.count("help") || k < 16 || k > 63)
    {
        po::print_options_description({opts});
        return 0;
    }
    bool printodd = vm.count("odd") || vm.count("all")==0;
    bool printall = vm.count("all") || vm.count("odd")==0;

    // execute
    pg::almost_prime_sieve sieve( k);
    sieve.prepare_primecache();
    sieve.count_almostprimes(printodd, printall );

    return 0;
}
