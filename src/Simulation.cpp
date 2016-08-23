/*
 * Copyright (c) 2002-2015 by Microwave and Wireless Systems Laboratory, by Andre Barreto and Calil Queiroz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Simulation.h"
#include "myexception.h"
#include "DataStatistics.h"
#include "Standard.h"

#include <iomanip>
#include <sstream>
#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// struct res_stats                                                           //
////////////////////////////////////////////////////////////////////////////////
struct res_stats {
	DataStatistics throughput;
	DataStatistics transfer_time;
	DataStatistics transfer_time_std;
	DataStatistics tx_time;
	DataStatistics tx_time_std;
	DataStatistics packet_loss_rate;
	DataStatistics overflow_rate;
	DataStatistics queue_length;
	DataStatistics average_power;

	res_stats() {};

	void add(res_struct res);
	res_struct mean();
	res_struct confidence_interval(double p);
	void reset();
};


void res_stats::add(res_struct res) {
	throughput.new_sample(res.throughput);
	if (res.transfer_time != HUGE_VAL)
		transfer_time.new_sample(res.transfer_time);
	if (res.transfer_time_std != HUGE_VAL)
		transfer_time_std.new_sample(res.transfer_time_std);
	if (res.tx_time != HUGE_VAL)
		tx_time.new_sample(res.tx_time);
	if (res.tx_time_std != HUGE_VAL)
		tx_time_std.new_sample(res.tx_time_std);
	if (res.packet_loss_rate != HUGE_VAL)
		packet_loss_rate.new_sample(res.packet_loss_rate);
	if (res.overflow_rate != HUGE_VAL)
		overflow_rate.new_sample(res.overflow_rate);
	if (res.queue_length != HUGE_VAL)
		queue_length.new_sample(res.queue_length);

	average_power.new_sample(res.average_power);
}

void res_stats::reset() {
	throughput.reset();
	transfer_time.reset();
	transfer_time_std.reset();
	tx_time.reset();
	tx_time_std.reset();
	packet_loss_rate.reset();
	overflow_rate.reset();
	queue_length.reset();
	average_power.reset();
}

res_struct res_stats::mean() {
	return res_struct(throughput.mean(), transfer_time.mean(),
			transfer_time_std.mean(), tx_time.mean(),
			tx_time_std.mean(), packet_loss_rate.mean(),
			overflow_rate.mean(), queue_length.mean(),
			average_power.mean());
}

res_struct res_stats::confidence_interval(double p) {
	return res_struct(throughput.confidence_interval(p),
			transfer_time.confidence_interval(p),
			transfer_time_std.confidence_interval(p),
			tx_time.confidence_interval(p),
			tx_time_std.confidence_interval(p),
			packet_loss_rate.confidence_interval(p),
			overflow_rate.confidence_interval(p),
			queue_length.confidence_interval(p),
			average_power.confidence_interval(p));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// class Simulation                                                           //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Simulation constructor                                                     //
////////////////////////////////////////////////////////////////////////////////
Simulation::Simulation(string dir, string par) : wdir(dir) {

	if (!sim_par.read_param(wdir)) throw (my_exception(CONFIG));

	log.open (wdir,sim_par.get_Log());

	string filename = wdir + '\\' + OUTPUT_FILE_NAME + OUTPUT_FILE_EXTENSION;
	out.open(filename.c_str());

	run();
}

////////////////////////////////////////////////////////////////////////////////
// Simulation::final_results                                                  //
//                                                                            //
// stops all simulation and output results                                    //
////////////////////////////////////////////////////////////////////////////////
void Simulation::final_results(){

	out << "\n\n%%%% Final results %%%%\n";

	int field_width = 10;
	int double_prec = 3;

	//////////////////////////////////////////
	// calculate mean results over all seeds
	unsigned n_seeds = 0;

	sim_par.reset_iterations();
	vector<res_struct>::iterator it = results.begin();

	vector<res_stats> stats;
	res_stats res;

	int it_number = 0;
	unsigned n_seeds_max = sim_par.get_number_of_Seeds();

	vector<string> param_string = sim_par.get_iter_pnames();

	int count = 0;

	for (vector<string>::iterator str_it = param_string.begin();
			str_it != param_string.end(); ++str_it) {
		if (str_it->size() < 15) str_it->append(15 - str_it->size(),' ');
		*str_it += "=";
	}

	do {
		res.add(*it++);

		if (++n_seeds >= n_seeds_max) {

			stats.push_back(res);
			res.reset();

			n_seeds = 0;

			vector<string> aux_vec = sim_par.get_param_str(field_width,double_prec);
			vector<string>::iterator str_it;
			vector<string>::const_iterator str_it2;
			for (str_it = param_string.begin(), str_it2  = aux_vec.begin();
					str_it != param_string.end() && str_it2 != aux_vec.end();
					++str_it, ++str_it2) {
				*str_it += *str_it2 + " ";
			}

		}

	} while (sim_par.new_iteration());


	///////////////////////////////////////
	// output mean results over all seeds
	vector<string> aux_vec = sim_par.get_iter_punits();
	vector<string>::iterator str_it;
	vector<string>::const_iterator str_it2;

	out << "\n\n";
	for (str_it = param_string.begin(), str_it2 = aux_vec.begin();
			str_it != param_string.end() && str_it2 != aux_vec.end();
			++str_it, ++str_it2) {
		*str_it += *str_it2 + "\n";
		out << *str_it;
	}

	out.setf(ios::right | ios::fixed);
	out.precision(double_prec);

	double conf = sim_par.get_Confidence();

	///////////////////////
	// output throughput
	out << "\n\nThroughput (Mbps)\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->throughput.mean() << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->throughput.confidence_interval(conf) << " ";
	}

	/////////////////////////
	// output transfer time
	out << "\n\nTransfer time (ms)\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->transfer_time.mean() * 1000 << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->transfer_time.confidence_interval(conf) * 1000 << " ";
	}

	///////////////////////////////////////////////
	// output standard deviation of transfer time
	out << "\n\nStandard deviation of transfer time (ms)\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->transfer_time_std.mean() * 1000 << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->transfer_time_std.confidence_interval(conf) * 1000 << " ";
	}


	/////////////////////////////
	// output transmission time
	out << "\n\nTransmission time (ms)\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->tx_time.mean() * 1000 << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->tx_time.confidence_interval(conf) * 1000 << " ";
	}

	//////////////////////////////////////////////////
	// output standard deviation of transmission time
	out << "\n\nStandard deviation of transmission time (ms)\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->tx_time_std.mean() * 1000 << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->tx_time_std.confidence_interval(conf) * 1000 << " ";
	}

	////////////////////////////
	// output packet loss rate
	out << "\n\nPacket loss rate\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->packet_loss_rate.mean() << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->packet_loss_rate.confidence_interval(conf) << " ";
	}

	////////////////////////////
	// output overflow rate
	out << "\n\nQueue overflow rate\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->overflow_rate.mean() << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->overflow_rate.confidence_interval(conf) << " ";
	}

	////////////////////////////
	// output queue length
	out << "\n\nAverage queue length\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->queue_length.mean() << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->queue_length.confidence_interval(conf) << " ";
	}

	////////////////////////////
	// output average power
	out << "\n\nAverage transmission power (mW)\nmean           = ";
	for (vector<res_stats>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		out << setw(field_width) << it->average_power.mean() << " ";

	if (n_seeds_max > 1) {
		out << "\nconf. interval = ";
		for (vector<res_stats>::const_iterator it = stats.begin();
				it != stats.end(); ++it)
			out << setw(field_width)
			<< it->average_power.confidence_interval(conf) << " ";
	}

	out.close();
}

////////////////////////////////////////////////////////////////////////////////
// Simulation::init_terminals                                                 //
//                                                                            //
// initializes terminals for a new iteration                                  //
////////////////////////////////////////////////////////////////////////////////
void Simulation::init_terminals(){

	adapt_struct adapt (sim_par.get_TxMode(), sim_par.get_AdaptMode(),
			sim_par.get_TxPowerMax(), sim_par.get_TxPowerMin(),
			sim_par.get_TxPowerStepUp(),sim_par.get_TxPowerStepDown(),
			sim_par.get_TargetPER(),sim_par.get_LAMaxSucceedCounter(),
			sim_par.get_LAFailLimit(), sim_par.get_UseRxMode());

	mac_struct mac(sim_par.get_RetryLimit(), sim_par.get_RTSThreshold(),
			sim_par.get_FragmentationThresh(), sim_par.get_QueueSize(),
			sim_par.get_set_BA_agg());

	PHY_struct phy(sim_par.get_NoiseVariance(), sim_par.get_CCASensitivity());

	traffic_struct tr_dl(sim_par.get_DataRateDL(), sim_par.get_PacketLength(),
			sim_par.get_ArrivalTime());

	traffic_struct tr_ul(sim_par.get_DataRateUL(), sim_par.get_PacketLength(),
			sim_par.get_ArrivalTime());

	timestamp tr_time = sim_par.get_TransientTime();

	for (unsigned i = 0; i < sim_par.get_NumberAPs(); i++) {
		AccessPoint* ap = new AccessPoint(sim_par.get_APPosition(i), &main_sch, ch,
				&randgent, &log, mac, phy, tr_time);
		term_vector.push_back(ap);

		if (log(log_type::setup))
			log << *ap << " created at position " << sim_par.get_APPosition(i)
			<< '\n' << endl;
	}


	double cell_radius = sim_par.get_Radius();

	// Array containing the amount of connections belonging to each AC
	unsigned ppArray[5] = {round(2*sim_par.get_ppAC_BK()*sim_par.get_NumberStas()),
					  	   round(2*sim_par.get_ppAC_BE()*sim_par.get_NumberStas()),
						   round(2*sim_par.get_ppAC_VI()*sim_par.get_NumberStas()),
						   round(2*sim_par.get_ppAC_VO()*sim_par.get_NumberStas()),
						   round(2*sim_par.get_ppLegacy()*sim_par.get_NumberStas())};
	unsigned sum = 0;
	for(int k = 0; k < 5; k++) {
		sum = sum + ppArray[k];
	}
	int diff = sum - 2*sim_par.get_NumberStas();
	if(diff > 0) {
		for(int k = 0; k < 5; k++) {
			while(ppArray[k] > 0 && diff > 0){
				ppArray[k]--;
				diff--;
			}
		}
	}
	if(diff < 0)	{
		ppArray[0] = (-1)*diff;
	}
	vector<int> noZe_ppArray;
	accCat MS_AC = AC_BK;
	accCat AP_AC = AC_BK;
	int idx = 0;

	for (unsigned i = 0; i < sim_par.get_NumberStas(); i++) {

		// Vector containing elements of ppArray that are non-zero
		noZe_ppArray.clear();
		for(int k = 0; k < 5; k++) {
			if(ppArray[k] != 0) noZe_ppArray.push_back(k);
		}
		// Choose one access category randomly
		if(noZe_ppArray.size() != 0) {
			idx = randgent.from_vec(noZe_ppArray);
			MS_AC = allACs[idx];
			ppArray[idx]--;
		}

		// if just one mobile station, then distance = cell radius
		Position pos(cell_radius,0);
		// else Stas are uniformly distributed
		if (sim_par.get_NumberStas() > 1) {
			do {
				pos = Position (randgent.uniform(-cell_radius,cell_radius),
						randgent.uniform(-cell_radius,cell_radius));
			} while(pos.distance() > cell_radius);
		}

		MobileStation* ms = new MobileStation(pos, &main_sch, ch, &randgent,
				&log, mac, phy, tr_time);
		term_vector.push_back(ms);

		double min_dist = HUGE_VAL;
		int min_index = -1;
		for (unsigned count = 0; count < sim_par.get_NumberAPs(); count++) {
			double dist = pos.distance(sim_par.get_APPosition(count));
			if (dist < min_dist) {
				min_dist = dist;
				min_index = count;
			}
		}

		// Vector containing elements of ppArray that are non-zero
		noZe_ppArray.clear();
		for(int k = 0; k < 5; k++) {
			if(ppArray[k] != 0) noZe_ppArray.push_back(k);
		}
		// Choose one access category randomly
		if(noZe_ppArray.size() != 0) {
			idx = randgent.from_vec(noZe_ppArray);
			AP_AC = allACs[idx];
			ppArray[idx]--;
		}

		// Connect mobile terminal to closest AP
		connect_two(term_vector[min_index], AP_AC, ms, MS_AC, ch, adapt, tr_dl, tr_ul);

		if (log(log_type::setup))
			log << *ms << " created at position " << pos << " with distance "
			<< min_dist << " m to " << *term_vector[min_index]
		    << "\nMobile station AC: "<< MS_AC << ". Access Point AC: "
			<< AP_AC << "." <<endl;
	}


	if (log(log_type::setup)) log_connections();
}

////////////////////////////////////////////////////////////////////////////////
// Simulation::log_connections                                                //
//                                                                            //
// saves all active communication links to log file                           //
////////////////////////////////////////////////////////////////////////////////
void Simulation::log_connections () {

	log << '\n';
	for (vector<Terminal*>::const_iterator it = term_vector.begin();
			it != term_vector.end(); ++it) {
		log << **it << " is connected to " << (*it)->get_connections() << endl;
	}
	log << endl;

}

////////////////////////////////////////////////////////////////////////////////
// Simulation::run                                                            //
//                                                                            //
// starts all simulations, iterates over all parameter combinations           //
////////////////////////////////////////////////////////////////////////////////
void Simulation::run() {
	int n_it = 0;

	do {
		n_it++;

		if(sim_par.get_partResults()) {
			cout << "\n\nIteration " << n_it << "\n    " << sim_par << "\n" << endl;
			out << "\n\nIteration " << n_it << "\n    " << sim_par << "\n" << endl;
		}

		if (log(log_type::setup))
			log << "\n\nIteration " << n_it << "\n\t" << sim_par << "\n"
			<< endl;

		/*
#ifdef _SAVE_RATE_ADAPT
    rate_adapt_file_ch << "\n\nIteration " << n_it << "\n\t" << sim_par << "\n"
                       << "Time      ,term 1,term 2,path loss ,"
                       << "fading(R) ,fading(I)\n";
    rate_adapt_file_ch.setf(ios::left);
    rate_adapt_file_rt << "\n\nIteration " << n_it << "\n\t" << sim_par << "\n"
                       << "Time      ,sender,target,data rate\n";
    rate_adapt_file_rt.setf(ios::left);
#endif
		 */
		main_sch.init();

		randgent.seed(sim_par.get_Seed());

		Standard::set_standard(sim_par.get_standard(),false);
		if(sim_par.get_TxMode() > Standard::get_maxMCS())
			throw (my_exception("MCS not supported by standard."));

		channel_struct ch_par(sim_par.get_LossExponent(),
				sim_par.get_RefLoss(),
				sim_par.get_DopplerSpread(),
				sim_par.get_NumberSinus());

		ch = new Channel(&main_sch, &randgent, ch_par, &log);

		init_terminals();

		start_sim();

		wrap_up();

		delete ch;
		for (vector<Terminal*>::iterator it = term_vector.begin();
				it != term_vector.end(); ++it) delete *it;
		term_vector.clear();

	} while (sim_par.new_iteration());

	final_results();
}


////////////////////////////////////////////////////////////////////////////////
// Simulation::start_sim                                                      //
//                                                                            //
// starts a new iteration                                                     //
////////////////////////////////////////////////////////////////////////////////
void Simulation::start_sim () {
	// schedule temporary outputs
	main_sch.schedule(Event(timestamp(sim_par.get_TempOutputInterval()),
			(void*)&wrapper_to_temp_output,(void*)this));

	// start scheduler
	main_sch.run(sim_par.get_MaxSimTime());
}

////////////////////////////////////////////////////////////////////////////////
// Simulation::temp_output                                                    //
//                                                                            //
// displays results in standard output during simulation                      //
////////////////////////////////////////////////////////////////////////////////
void Simulation::temp_output () {
	// schedule new temporary output
	main_sch.schedule(Event(main_sch.now() + sim_par.get_TempOutputInterval(),
			(void*)&wrapper_to_temp_output,(void*)this));

	cout << "Simulation time ellapsed = " << main_sch.now() << " sec. \n";

	double sim_time = double(main_sch.now());
	double tr_time = double(sim_par.get_TransientTime());
	if (sim_time > tr_time) {
		double total_tp = 0;
		for(vector<Terminal*>::const_iterator it = term_vector.begin();
				it != term_vector.end(); ++it) {
			total_tp += (*it)->get_n_bytes()*8.0/(sim_time-tr_time)/1e6;
		}
		cout << "\tTotal throughput = " << total_tp << endl;
	}
}

void Simulation::wrapper_to_temp_output (void* ptr2obj) {
	Simulation* which = (Simulation*) ptr2obj;
	which->temp_output();
}

////////////////////////////////////////////////////////////////////////////////
// Simulation::wrap_up                                                        //
//                                                                            //
// ends one iteration and collect performance results,                        //
// outputs them if required (if 'it_file_flag == true')                       //
////////////////////////////////////////////////////////////////////////////////
void Simulation::wrap_up () {

	res_stats res;

	if(main_sch.now() <= sim_par.get_TransientTime())
		throw (my_exception("transient time was longer than simulation time"));

	double ellapsed_time = double(main_sch.now() - sim_par.get_TransientTime());

	if(sim_par.get_partResults()) {
		out.setf(ios::right | ios::fixed);
		out << Standard::get_standard() << endl;
		out << "Term Position   dist.  AC     throughput transfer_t tx_time packets"
				<< " kbytes pack_loss overflow queue_l tx_rate(PHY) tx_power" << endl;
		out << "        m        m      Mbps         ms      ms           "
				<< "                                      Mbps        mW" << endl;
	}

	for (vector<Terminal*>::iterator it = term_vector.begin();
			it != term_vector.end(); ++it) {

		double tp = (*it)->get_n_bytes()*8.0/ellapsed_time/1e6;

		res.add (res_struct(tp,(*it)->get_transfer_delay(),
				(*it)->get_transfer_delay_std(),
				(*it)->get_transmission_delay(),
				(*it)->get_transmission_delay_std(),
				(*it)->get_packet_loss_rate(),
				(*it)->get_overflow_rate(),
				(*it)->get_queue_length(),
				(*it)->get_average_power()));
		if(sim_par.get_partResults()) {
			out << setw(4) << (*it)->get_id() << " ";
			out.precision(0);
			out.width(3);
			out << (*it)->get_pos();
			out.precision(1);
			out << setw(5) << ((*it)->get_pos()).distance();
			out <<  "  " << (*it)->get_term_ACs();
			out.precision(3);
			out << "  " << setw(8) << tp;
			out.precision(2);
			out << setw(12) << (*it)->get_transfer_delay() * 1000;
			out << setw(8) << (*it)->get_transmission_delay() * 1000;
			out << setw(8) << (*it)->get_n_packets();
			out << setw(8) << (*it)->get_n_bytes()/1000;
			out.precision(4);
			out << setw(8) << (*it)->get_packet_loss_rate();
			out << setw(9) << (*it)->get_overflow_rate();
			out.precision(1);
			out << setw(8) << (*it)->get_queue_length();
			out.precision(2);
			out << setw(10) << (*it)->get_tx_data_rate();
			out.precision(1);
			out << setw(9) << (*it)->get_average_power();

			out << endl;
		}
	}
	if(sim_par.get_partResults()) {
		out.unsetf(ios::right | ios::fixed);
		out.precision(6);
	}

	res_struct restotal(res.throughput.sum(), res.transfer_time.mean(),
			res.transfer_time_std.mean(), res.tx_time.mean(),
			res.tx_time_std.mean(), res.packet_loss_rate.mean(),
			res.overflow_rate.mean(), res.queue_length.mean(),
			res.average_power.mean());
	if(sim_par.get_partResults()) {
		out << "\n Total throughput = " << restotal.throughput << " Mbps\n";
		cout << "\nTotal throughput = " << restotal.throughput << " Mbps\n";

		out << " Average transfer time = " << restotal.transfer_time << "s\n";
		cout << "\nAverage transfer time = " << restotal.transfer_time << "s\n";

		out << " Average transmission time = " << restotal.tx_time << "s\n";
		cout << "\nAverage transmission time = " << restotal.tx_time << "s\n";

		out << " Packet loss rate = " << restotal.packet_loss_rate << "\n";
		cout << "\nPacket loss rate = " << restotal.packet_loss_rate << "\n";

		out << " Overflow rate = " << restotal.overflow_rate << "\n";
		cout << "\nOverflow rate = " << restotal.overflow_rate << "\n";
	}

	results.push_back(restotal);
}


