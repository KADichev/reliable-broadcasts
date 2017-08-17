//  Copyright (c) 2013 Thomas Heller
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/lcos/broadcast.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

#include <signal.h> 

const std::size_t N = 1024;
std::vector<double> buffer(N);
using std::chrono::milliseconds;

void vector_bcast(std::vector<double> bcast)
{
	hpx::util::high_resolution_timer t;
	buffer = bcast;
	double elapsed = t.elapsed();
}

HPX_PLAIN_ACTION(vector_bcast);

HPX_REGISTER_BROADCAST_ACTION_DECLARATION(vector_bcast_action)
HPX_REGISTER_BROADCAST_ACTION(vector_bcast_action)

void kill_me() {
	//hpx::this_thread::sleep_for(milliseconds(sleep_time));
	std::cout << "Killed ...\n" << std::flush;
	raise(SIGKILL);
}
HPX_PLAIN_ACTION(kill_me, kill_me_action);

int hpx_main()
{
    hpx::id_type here = hpx::find_here();
    hpx::id_type there = here;
    std::vector<hpx::id_type> localities = hpx::find_all_localities();
    std::vector<double> bcast_array(N);
    for (std::size_t i=0; i<N; i++)
	    bcast_array[i] = 3.14;

    //kill_me_action act;
    //hpx::apply(act, localities[20],3000);
    bool failure_detected = false;

    for (std::size_t i = 0; i<10; i++) {
	    if (failure_detected) {
		    std::vector<hpx::id_type> new_localities;
		    for (auto l : localities) {
			    if (get_locality_id_from_id(l) != 1)
				    new_localities.push_back(l);
		    }
		    auto f = hpx::lcos::broadcast<vector_bcast_action>(new_localities, bcast_array);
		    f.get();
		    std::cout << "Completed new broadcast without failed node\n";
	    }
	    else {
		    hpx::util::high_resolution_timer t;
		    if (i == 3) {
			    kill_me_action act;
			    hpx::apply(act, localities[1]);
		    }
		    auto f = hpx::lcos::broadcast<vector_bcast_action>(localities, bcast_array);
		    //f.wait();
		    if (f.wait_for(milliseconds(2000)) != hpx::lcos::future_status::ready) {
			    std::cout << "Loc: " << hpx::get_locality_id() << " Broadcast:" << i << " taking too long ...\n";
			    failure_detected = true;
		    }
		    else {
			    double elapsed = t.elapsed();
			    std::cout << "Loc: " << hpx::get_locality_id() << " Broadcast:" << i << " complete in: " << elapsed << "secs\n" << std::flush;
		    }
	    }
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // Initialize and run HPX
    HPX_TEST_EQ_MSG(hpx::init(argc, argv), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}

