/*********************************************************************************\
*                                                                                 *
* https://github.com/cr-marcstevens/primegen                                      *
*                                                                                 *
* MIT License                                                                     *
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

#include "primegen.hpp"
#include "program_options.hpp"

namespace pg = primegen;
namespace po = program_options;

int main(int argc, char** argv)
{
    // command line interface
    size_t lb = 1, ub = 0;
    po::options_description opts("Command line options");
    opts.add_options()
        ("help,h", "Show options")
        ("first,f", po::value<size_t>(&lb)->default_value(1), "Output primes >= first")
        ("last,l", po::value<size_t>(&ub), "Output primes <= last")
        ("sum,s", "Print sum of all primes, instead of primes")
        ;
    po::variables_map vm;
    bool allow_unregistered = false, allow_positional = true;
    po::store(po::parse_command_line(argc, argv, opts, allow_unregistered, allow_positional), vm);
    // if two positional arguments are given then parse as: <lb> <ub>
    if (vm.positional.size() >= 2)
    {
        ub = vm.positional[1].as<size_t>();
        lb = vm.positional[0].as<size_t>();
    }
    // if only one positional argument is given then parse as: <ub>
    else if (vm.positional.size() == 1)
    {
        ub = vm.positional[0].as<size_t>();
    }

    // print help
    if (vm.count("help") || ub < lb)
    {
        po::print_options_description({opts});
        return 0;
    }

    // execute
    pg::prime_sieve ps;
    if (!vm.count("sum"))
    {
        ps.genprimes(lb, ub, pg::printprime());
    } else {
        size_t psum = 0, pcnt = 0;
        bool overflow = false;
        ps.genprimes(lb, ub, [&](size_t p){ ++pcnt; psum += p; if (psum < p) overflow = true; });
        if (overflow)
            std::cerr << "Warning: sum overflow in size_t" << std::endl;
        std::cout << "count=" << pcnt << " sum=" << psum << std::endl;
    }

    return 0;
}
