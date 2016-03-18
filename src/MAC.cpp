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

#include "MAC.h"
#include "PHY.h"
#include "Terminal.h"
#include "Profiler.h"

////////////////////////////////////////////////////////////////////////////////
// IEEE 802.11a constant parameters                                           //
////////////////////////////////////////////////////////////////////////////////
const timestamp aSlotTime = timestamp(9.0e-6);
const timestamp DIFS = timestamp(34.0e-6);
const timestamp SIFS = timestamp(16.0e-6);

// Duration of signaling packets
const timestamp cts_duration = (MPDU(CTS, 0, 0, 0, M6)).get_duration();
const timestamp rts_duration = (MPDU(RTS, 0, 0, 0, M6)).get_duration();

// timeout intervals
inline timestamp ACK_Timeout(transmission_mode m) {
	return SIFS + ack_duration(m) + 5;
}
inline timestamp ADDBA_rsps_Timeout(transmission_mode m) {
	return SIFS + addba_rsps_duration(m) + 5;
}
inline timestamp BA_Timeout(transmission_mode m) {
	return SIFS + ba_duration(m) + 5;
}
const timestamp CTS_Timeout = SIFS + cts_duration + 5;

////////////////////////////////////////////////////////////////////////////////
// class MAC                                                                  //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// MAC constructor                                                            //
////////////////////////////////////////////////////////////////////////////////
MAC::MAC(Terminal* t, Scheduler* s, random *r, log_file* l, mac_struct mac, bool baf){
	term = t;
	ptr2sch = s;
	randgen = r;

	mylog = l;
	logflag = (*mylog)(log_type::mac);

	retry_limit = mac.retry;
	RTS_threshold = mac.RTS_thresh;
	frag_thresh = mac.frag_thresh;
	max_queue_size = mac.queue_size;

	BAFlag = baf;
	BArqstFlag = false;
	BArspsFlag = false;

	for(int k = 0; k < 5; k++)	{
		accCat auxAC = allACs[k];
		deque<MSDU> auxQue;
		packet_queue[auxAC] = auxQue;
		BOC_ACs[auxAC] = 0;
		CW_ACs[auxAC] = 0;
		BOC_flag[auxAC] = true;
	}

	NAV = timestamp(0);
	nfrags = 1;
	current_frag = 0;

	n_att_frags = 0;
	tx_data_rate = 0;

	countdown_flag = false;

}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::set_myAC		                                                  //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::set_myAC(accCat AC) {

	myAC = AC;

	switch(myAC){
	case AC_BK:
		aCWmin = 31;
		aCWmax = 1023;
		AIFSN = 7;
		TXOPmax = timestamp(0);
		break;
	case AC_BE:
		aCWmin = 31;
		aCWmax = 1023;
		AIFSN = 3;
		TXOPmax = timestamp(0);
		break;
	case AC_VI:
		aCWmin = 15;
		aCWmax = 31;
		AIFSN = 2;
		TXOPmax = timestamp(3.008e-3);
		break;
	case AC_VO:
		aCWmin = 7;
		aCWmax = 15;
		AIFSN = 2;
		TXOPmax = timestamp(1.504e-3);
		break;
	case legacy:
		aCWmin = 15;
		aCWmax = 1023;
		AIFSN = 2;
		TXOPmax = timestamp(0);
		break;
	}

	AIFS = SIFS + timestamp(AIFSN)*aSlotTime;
	TXOPflag = false;
	TXOPend = ptr2sch->now();
	TXOPla_win = success;

}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::ack_timed_out                                                 //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::ack_timed_out () {
	BEGIN_PROF("MAC::ack_timed_out")

	if(BArqstFlag) {
		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": did not receive ACK for ADDBA request." << endl;
		BArqstFlag = false;
		end_TXOP();
		return;
	}
	if(BArspsFlag){
		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": did not receive ACK for ADDBA response." << endl;
		return;
	}

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
	<< ": ACK time out for packet " << pck.get_id() << endl;

	if (TXOPflag) { // If during TXOP
		TXOPla_win = ACKfail; // Indicate that LA failed
	} else {
		term->la_failed(msdu.get_target()); // link adaptation
	}

#ifdef _SAVE_RATE_ADAPT
	timestamp t_aux = ptr2sch->now() - pck.get_duration() - ACK_Timeout(pck.get_mode());
	if (pck.get_nbytes_mac() >= RTS_threshold) {
		t_aux = t_aux - rts_duration - cts_duration - 2*SIFS;
	}
	rate_adapt_file_rt << setw(10) << double(t_aux) << ','
			<< setw(6) << get_id() << ','
			<< setw(6) << (msdu.get_target())->get_id() << ','
			<< setw(6) << tx_mode_to_double(pck.get_mode()) << ','
			<< 0 << endl;
#endif

	if (retry_count++ >= retry_limit) {

		if (logflag) *mylog << "  " << *term << ": retry count = retry limit ("
				<< retry_limit << "), give up sending this packet"
				<< endl;

		term->macUnitdataMaxRetry(msdu);

		packet_queue[myAC].pop_front();
		// If there is a packet on the queue, transmit next msdu
		if (get_queue_size()) new_msdu();

	} else {

		if (CW_ACs[myAC] <= aCWmax/2) {
			CW_ACs[myAC] = CW_ACs[myAC] * 2;
		}

		if (logflag) *mylog << "  " << *term << ": retry count = " << retry_count
				<< ", CW = " << CW_ACs[myAC] << ", try again"
				<< endl;

		tx_attempt();
	}

	END_PROF("MAC::ack_timed_out")
}


////////////////////////////////////////////////////////////////////////////////
// MAC_private::begin_countdown                                               //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::begin_countdown() {
	BEGIN_PROF("MAC::begin_countdown")

	// if channel is free now, schedule transmission for time
	// AIFS + contention window
	// CHANGE HERE!
	//backoff_counter = randgen->discrete_uniform(0,CW_ACs[myAC]-1);
	time_to_send = ptr2sch->now() + AIFS + timestamp(BOC_ACs[myAC]) * aSlotTime;

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
		<< ": begin countdown, CW = " << CW_ACs[myAC]
		<< ", backoff counter = " << BOC_ACs[myAC]
		<< ", schedule function transmit at time "
		<< time_to_send << endl;

	ptr2sch->schedule(Event(time_to_send,(void*)&wrapper_to_start_TXOP,
			(void*)this));

	myphy->notify_busy_channel();

	countdown_flag = true;

	END_PROF("MAC::begin_countdown")
}

////////////////////////////////////////////////////////////////////////////////
// MAC::phyCCA_busy                                                           //
//                                                                            //
// MAC receives message (from PHY) that channel has become busy               //
////////////////////////////////////////////////////////////////////////////////
void MAC::phyCCA_busy() {
	BEGIN_PROF("MAC::phyCCA_busy")

	//////////////////////////////////////////
	// stop countdown and cancel transmission
	timestamp time_diff = time_to_send - ptr2sch->now();
	if (time_diff < timestamp(BOC_ACs[myAC]) * aSlotTime) {
		BOC_ACs[myAC] = time_diff / aSlotTime;
	}

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
			<< ": received channel-busy message"
			<< ", stop countdown, backoff counter frozen at "
			<< BOC_ACs[myAC] << endl;

	time_to_send = not_a_timestamp();
	ptr2sch->remove((void*)(&wrapper_to_start_TXOP), (void*)this);

	END_PROF("MAC::phyCCA_busy")
}

////////////////////////////////////////////////////////////////////////////////
// MAC::phyCCA_free                                                           //
//                                                                            //
// MAC receives message (from PHY) that channel has become free               //
////////////////////////////////////////////////////////////////////////////////
void MAC::phyCCA_free() {
	BEGIN_PROF("MAC::phyCCA_free")

	timestamp now = ptr2sch->now();

	// check NAV
	if (now <= NAV) {

		// if NAV is set, try again later
		ptr2sch->schedule(Event(NAV+1,(void*)&wrapper_to_end_nav, (void*)this));
		return;
	}

	if (countdown_flag) {
		// resume countdown

		time_to_send = now + AIFS + timestamp(BOC_ACs[myAC]) * aSlotTime;
		ptr2sch->schedule(Event(time_to_send, (void*)(&wrapper_to_start_TXOP),
				(void*)this));

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< " received free-channel message\n    "
				<< "resume countdown, new transmission scheduled to "
				<< time_to_send << endl;

		myphy->notify_busy_channel();
	} else {
		// start countdown
		begin_countdown();
	}

	END_PROF("MAC::phyCCA_free")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::check_nav                                                     //
//                                                                            //
// check if channel is free despite NAV (e.g, when RTS was detected)          //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::check_nav () {
	BEGIN_PROF("Terminal::check_nav")

			if (!myphy->carrier_sensing()) {

				if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
						<< " checks if NAV is valid"
						<< ", channel is free, reset NAV" << endl;

				NAV = timestamp(0);
			}

	END_PROF("MAC::check_nav")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::cts_timed_out
//
// Change contention window if CTS is not received.
////////////////////////////////////////////////////////////////////////////////
void MAC_private::cts_timed_out () {
	BEGIN_PROF("MAC::cts_timed_out")

#ifdef _SAVE_RATE_ADAPT
	timestamp t_aux = ptr2sch->now() - CTS_Timeout - rts_duration;
	rate_adapt_file_rt << setw(10) << double(t_aux) << ','
			<< setw(6) << get_id() << ','
			<< setw(6) << (msdu.get_target())->get_id() << ','
			<< setw(6) << tx_mode_to_double(pck.get_mode()) << ','
			<< -1 << endl;
#endif

	if(TXOPflag){
		TXOPla_win = CTSfail;
		end_TXOP(); // Finish TXOP before it starts
	} else {
		// Remember that RTS might fail during TXOP, so this LA still needs to be performed
		// afterwards
		term->la_rts_failed(msdu.get_target()); // link adaptation
	}

	if (retry_count++ >= retry_limit) {

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< " did not receive CTS for packet "
				<< pck.get_id() << ", retry count = retry limit ("
				<< retry_limit << "), give up sending this packet"
				<< endl;

		term->macUnitdataMaxRetry(msdu);

		packet_queue[myAC].pop_front();
		if (get_queue_size()) new_msdu();

	} else {

		if (CW_ACs[myAC] <= aCWmax/2) {
			CW_ACs[myAC] = CW_ACs[myAC] * 2;
		}

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< " did not receive CTS for packet " << pck.get_id()
				<< ", retry count = " << retry_count << ", CW = "
				<< CW_ACs[myAC] << ", try again" << endl;

		tx_attempt();
	}


	END_PROF("MAC::cts_timed_out")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::addba_rsps_timed_out										  //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::addba_rsps_timed_out() {
	BEGIN_PROF("MAC::addba_rsps_timed_out")

	if(TXOPflag){
		TXOPla_win = CTSfail;
		end_TXOP(); // Finish TXOP before it starts
	} else {
		term->la_rts_failed(msdu.get_target()); // link adaptation
	}

	if (retry_count++ >= retry_limit) {

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< " did not receive ADDBA response for packet "
				<< pck.get_id() << ", retry count = retry limit ("
				<< retry_limit << "), give up sending this packet"
				<< endl;

		term->macUnitdataMaxRetry(msdu);

		packet_queue[myAC].pop_front();
		if (get_queue_size()) new_msdu();

	} else {

		if (CW_ACs[myAC] <= aCWmax/2) {
			CW_ACs[myAC] = CW_ACs[myAC] * 2;
		}

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< " did not receive ADDBA response for packet " << pck.get_id()
				<< ", retry count = " << retry_count << ", CW = "
				<< CW_ACs[myAC] << ", try again" << endl;

		tx_attempt();
	}

	END_PROF("MAC::addba_rsps_timed_out")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::ba_timed_out										  //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::ba_timed_out() {
	BEGIN_PROF("MAC::ba_timed_out")

	if (retry_count++ >= retry_limit) {

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< " did not receive BA. "
				<< " retry count = retry limit (" << retry_limit
				<< "), give up sending this packet and requeue packets."
				<< endl;

		term->macUnitdataMaxRetry(msdu);

		packet_queue[myAC].pop_front();
		if (get_queue_size()) new_msdu();

	} else {
		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< " did not receive BA"
				<< ". retry count = " << retry_count
				<< ", requeue packets" << endl;
	}

	for(int k = BApckts.size(); k >= 0; k--) {
		packet_queue[myAC].push_front(BApckts[k]);
	}

	END_PROF("MAC::ba_timed_out")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::end_nav                                                       //
// channel released according to NAV                                          //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::end_nav() {
	BEGIN_PROF("MAC::end_nav")


	timestamp now = ptr2sch->now();

	// check if NAV was updated in the meantime
	if (now <= NAV) {

		// try again later
		ptr2sch->schedule(Event(NAV+1,(void*)&wrapper_to_end_nav, (void*)this));
		myphy->cancel_notify_free_channel();

		END_PROF("MAC::end_nav")
		return;
	}

	// check if channel is really free
	if (myphy->carrier_sensing()) {

		myphy->notify_free_channel();

		END_PROF("MAC::end_nav")
		return;
	}

	// resume countdown
	if (countdown_flag) {

		time_to_send = now + AIFS + timestamp(BOC_ACs[myAC]) * aSlotTime;
		ptr2sch->schedule(Event(time_to_send, (void*)(&wrapper_to_start_TXOP),
				(void*)this));

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ", channel released according to NAV"
				<< " and channel is free, resume countdown"
				<< ", new transmission scheduled to " << time_to_send
				<< endl;

		myphy->notify_busy_channel();

	} else { // or beging contdown againg

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ", channel released according to NAV"
				<< " and channel is free" << endl;

		begin_countdown();
	}
	END_PROF("MAC::end_nav")
};

////////////////////////////////////////////////////////////////////////////////
// MAC::phyRxEndInd
//
// Check if packet is for this station or not.
// If not, receive_bc (receive broadcast).
// If so, receive_this (receive for this station).
////////////////////////////////////////////////////////////////////////////////
void MAC::phyRxEndInd(MPDU p) {

	BEGIN_PROF("MAC::phyRxEndInd")

	if (p.get_target() == term) receive_this(p);
	else receive_bc(p);

	END_PROF("MAC::phyRxEndInd")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::new_msdu                                                      //
//                                                                            //
// transmits next MSDU from packet queue                                      //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::new_msdu() {

	timestamp t;
	if(BArqstFlag) t = ptr2sch->now() + SIFS + pck.get_duration();
	else t = ptr2sch->now()+SIFS;

	// Change BOC flag before switching AC
	BOC_flag[myAC] = true;

	// Recalculate myAC, if not during TXOP or TXOP is going to end or has ended
	if(!TXOPflag || TXOPend <= ptr2sch->now() + 1){
		internal_contention();
	}

	msdu = packet_queue[myAC].front();

	retry_count = 0;

	current_frag = 0;

	msdu.set_tx_time(ptr2sch->now());

	// If during TXOP and TXOPend is not next time increment
	if(TXOPflag && TXOPend > ptr2sch->now() + 1) {
		// Schedule tx_attempt to now + SIFS
		ptr2sch->schedule(Event(t,(void*)&wrapper_to_tx_attempt,
				(void*)this));
	} else {
		// If not, tx_attempt is scheduled for next time increment
		ptr2sch->schedule(Event(ptr2sch->now()+1,(void*)&wrapper_to_tx_attempt,
			(void*)this));
	}
}    

////////////////////////////////////////////////////////////////////////////////
// MAC_private::receive_bc                                                    //
//                                                                            //
// receive message targeted at other terminal, update NAV if needed           //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::receive_bc(MPDU p) {
	BEGIN_PROF("MAC::receive_bc")

		if (p.get_nav() > NAV) {
			NAV = p.get_nav();

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
					<< ": received packet " <<  p.get_id()
					<< " targeted at other terminal, with NAV = "
					<< p.get_nav() << endl;
		}

	if (p.get_type() == RTS) {

		timestamp t = ptr2sch->now() + 2*SIFS + cts_duration + 2*aSlotTime;
		ptr2sch->schedule(Event(t,(void*)&wrapper_to_check_nav,(void*)this));
	}

	END_PROF("MAC::receive_bc")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::receive_this                                                  //
//                                                                            //
// receive message targeted at this terminal                                  //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::receive_this(MPDU p) {
	BEGIN_PROF("MAC::receive_this")

	timestamp now = ptr2sch->now();

	switch (p.get_type()) {
	/////////////////////////////////////////////////////////////////////
	// ACK was received.
	// Schedule further fragments or inform upper layers if last fragment
	// was transmitted.
	case ACK : {
		ptr2sch->remove((void*)(&wrapper_to_ack_timed_out), (void*)this);

		if(BArqstFlag) {

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received ACK for ADDBA request." << endl;

			timestamp t = now + ADDBA_rsps_Timeout(p.get_mode());
			ptr2sch->schedule(Event(t,(void*)&wrapper_to_addba_rsps_timed_out,(void*)this));

			break;
		}
		if(BArspsFlag){

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received ACK for ADDBA response." << endl;

			break;
		}

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received ACK for packet "
				<< pck.get_id() << ", fragment " << current_frag
				<< " of " << nfrags << endl;

#ifdef _SAVE_RATE_ADAPT
		timestamp t_aux = ptr2sch->now() - pck.get_duration() - SIFS
				- ack_duration(pck.get_mode());
		if (pck.get_nbytes_mac() >= RTS_threshold) {
			t_aux = t_aux - rts_duration - cts_duration - 2*SIFS;
		}
		rate_adapt_file_rt << setw(10) << double(t_aux) << ','
				<< setw(6) << get_id() << ','
				<< setw(6) << (msdu.get_target())->get_id() << ','
				<< setw(6) << tx_mode_to_double(pck.get_mode()) << ','
				<< 1 << endl;
#endif

		// if last fragment was received
		if (current_frag == nfrags) {

			term->macUnitdataStatusInd(msdu, p.get_duration() + SIFS);

			// Indicate LA success if not during TXOP
			if(!TXOPflag) term->la_success(msdu.get_target(), true);

			packet_queue[myAC].pop_front();
			if (get_queue_size()) new_msdu();

			break;

		} else {

			// If it is not the last fragment, still perform link adaptation
			term->la_success(msdu.get_target(), false);

			// schedule transmission of next packet
			++current_frag;

			unsigned pl;
			timestamp newnav;

			if(TXOPflag) { // If during TXOP, NAV is simply TXOPend

				newnav = TXOPend;

			}else if (current_frag == nfrags) {
				pl = msdu.get_nbytes() % frag_thresh;
				if (!pl) pl = frag_thresh;

				newnav = now + 2*SIFS + pck.get_duration() + ack_duration(p.get_mode());
			}
			else {
				pl = frag_thresh;
				newnav = now + 4*SIFS + 2*pck.get_duration() + 2*ack_duration(p.get_mode());
			}

			if (logflag) *mylog << "    schedule transmission of fragment "
					<< current_frag << " with " << pl
					<< " bytes and NAV = " << newnav << " for "
					<< now + SIFS << endl;


			pck = DataMPDU(msdu, pl, current_frag, nfrags, power_dBm, p.get_mode(),
					newnav);

			ptr2sch->schedule(Event(now+SIFS, (void*)(&wrapper_to_send_data),
					(void*)this));
			break;
		}
	}

	///////////////////////////////////////
	// data packet received, transmit ACK
	case DATA : {

		rx_mode = p.get_mode();
		NAV = p.get_nav();

		if(BArspsFlag) {

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
					<< ": received " << p
					<< " during Block ACK session." << endl;

			pcktsToACK.push_back(p.get_id());

		} else {

			timestamp t_ack = now + SIFS;

			ptr2sch->schedule(Event(t_ack, (void*)(&wrapper_to_send_ack),
					(void*)this, p.get_source()));

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
					<< ": received " << p
					<< ". Schedule ACK transmission for " << t_ack
					<< endl;

		}
		// rate adaptation
		term->la_rx_success(p.get_source(), rx_mode);

		break;
	}

	///////////////////////////////////////////////////////
	// RTS received, if channel is not busy, transmit CTS
	case RTS : {

		set_myAC(term->get_connection_AC(p.get_source()));

		if (now <= NAV) break;

		timestamp t_cts = now + SIFS;

		// update NAV
		NAV_RTS = p.get_nav();
		NAV = NAV_RTS;

		ptr2sch->schedule(Event(t_cts, (void*)(&wrapper_to_send_cts),
				(void*)this, p.get_source()));

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received " << p << ". Channel is free"
				<< ", CTS transmission scheduled at "
				<< t_cts << endl;

		break;
	}

	////////////////////////////////////////
	// CTS received, transmit data packet
	case CTS : {
		ptr2sch->remove((void*)(&wrapper_to_cts_timed_out), (void*)this);

		timestamp t = now + SIFS;

		/* If the TXOP CTS is received, tx_attempt(), not send_data():
		 * tx_attempt() fragments packet, while send_data() simply sends next fragment.
		 */
		if(TXOPflag && !BAFlag) ptr2sch->schedule(Event(t, (void*)(&wrapper_to_tx_attempt),
				(void*)this));
		else if(TXOPflag) ptr2sch->schedule(Event(t, (void*)(&wrapper_to_send_addba_rqst),
				(void*)this,p.get_source()));
		else ptr2sch->schedule(Event(t, (void*)(&wrapper_to_send_data),(void*)this));

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received " << p
				<< ", schedule next transmission at " << t << endl;

		// Schedule the end of the TXOP
		if(!BAFlag && TXOPflag){
			ptr2sch->schedule(Event(TXOPend,(void*)&wrapper_to_end_TXOP,(void*)this));
		}

		break;
	}
	////////////////////////////////////////////////////////////////////
	// ADDBA request received, if channel is not busy, transmit response
	case ADDBArqst : {

		rx_mode = p.get_mode();

		timestamp t_ack = now + SIFS;
		timestamp t_addba_rsps = now + ack_duration(rx_mode) + timestamp(2)*SIFS;

		// update NAV
		NAV_ADDBA = NAV = p.get_nav();

		ptr2sch->schedule(Event(t_ack, (void*)(&wrapper_to_send_ack),
						(void*)this, p.get_source()));

		ptr2sch->schedule(Event(t_addba_rsps, (void*)(&wrapper_to_send_addba_rsps),
				(void*)this, p.get_source()));

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received " << p << ". ACK transmission scheduled at "
				<< t_ack << ", ADDBA response transmission scheduled at "
				<< t_addba_rsps << endl;

		break;

	}
	////////////////////////////////////////////////////////////////////
	// ADDBA response received, send ACK and Data
	case ADDBArsps : {
		ptr2sch->remove((void*)(&wrapper_to_addba_rsps_timed_out), (void*)this);

		ptr2sch->schedule(Event(time_to_send_BAR, (void*)&wrapper_to_send_bar,
							(void*)this, p.get_source()));

		if (now <= NAV) break;

		timestamp t_ack = now + SIFS;
		timestamp t_data = t_ack + ack_duration(tx_mode) + SIFS;

		// update NAV
		NAV_ADDBA = NAV = p.get_nav();

		ptr2sch->schedule(Event(t_ack, (void*)(&wrapper_to_send_ack),
				(void*)this, p.get_source()));

		ptr2sch->schedule(Event(t_data, (void*)(&wrapper_to_tx_attempt),(void*)this));

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received " << p << ". ACK transmission scheduled at "
				<< t_ack << ", Data transmission scheduled at "
				<< t_data << endl;

		if(TXOPflag) ptr2sch->schedule(Event(TXOPend + 1,(void*)&wrapper_to_end_TXOP,(void*)this));

		break;
	}
	////////////////////////////////////////////////////////////////////
	// BAR received, send BA
	case BAR : {

		BArspsFlag = false;

		rx_mode = p.get_mode();

		timestamp t_ba = now + SIFS;

		// update NAV
		NAV = p.get_nav();

		ptr2sch->schedule(Event(t_ba, (void*)&wrapper_to_send_ba,
				(void*)this, p.get_source()));

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received " << p << ". BA transmission scheduled at "
				<< t_ba << endl;

		break;
	}
	////////////////////////////////////////////////////////////////////
	// BA received, requeue unacknowledged packets
	case BA : {

		ptr2sch->remove((void*)(&wrapper_to_ba_timed_out),(void*)this);

		BArqstFlag = false;

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": received " << p << ". Requeue unacknowledged packets."
				<< endl;

		requeue_pcks(p);

		break;
	}

	}

	END_PROF("MAC::receive_this")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::send_ack                                                      //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::send_ack(Terminal *to) {
	BEGIN_PROF("MAC::send_ack")

	transmission_mode ack_mode = TXOPflag? tx_mode : rx_mode;

	// transmit ACK with data rate of received data packet
	bool send2all = (NAV > ptr2sch->now() + ack_duration(ack_mode))?
														  true : false;

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
			<< ": send ACK to " << *to << ", NAV = " << NAV << endl;

	myphy->phyTxStartReq(MPDU(ACK,term, to, term->get_power(to, frag_thresh),
			ack_mode, NAV), false);

	END_PROF("MAC::send_ack")
}


////////////////////////////////////////////////////////////////////////////////
// MAC_private::send_cts                                                      //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::send_cts(Terminal *to) {
	BEGIN_PROF("MAC::send_cts")

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
	<< ": send CTS to " << *to << ", NAV = " << NAV_RTS
	<< endl;

	// always send CTS at 6Mbps
	myphy->phyTxStartReq(MPDU(CTS, term, to, term->get_power(to, frag_thresh),
			M6, NAV_RTS), true);

	END_PROF("MAC::send_cts")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::send_rqst                                                      //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::send_addba_rsps(Terminal *to) {
	BEGIN_PROF("MAC::send_addba_rsps")

	BApckts.clear();
	pcktsToACK.clear();
	pcktsDur.clear();
	BArspsFlag = true;

	timestamp t = ptr2sch->now() + addba_rsps_duration(rx_mode) + ACK_Timeout(rx_mode);

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
	<< ": send ADDBA response to " << *to << ", NAV = " << NAV_ADDBA
	<<". ACK timeout scheduled to " << t << endl;


	myphy->phyTxStartReq(MPDU(ADDBArsps, term, to, term->get_power(to, frag_thresh),
			rx_mode, NAV_ADDBA), true);

	ptr2sch->schedule(Event(t,(void*)&wrapper_to_ack_timed_out,(void*)this));

	END_PROF("MAC::send_addba_rsps")
}

////////////////////////////////////////////////////////////////////////////////
//MAC_private::send_addba_rqst                                                      //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::send_addba_rqst(Terminal *to) {
	BEGIN_PROF("MAC::send_addba_rqst")

	unsigned pl;
	if (current_frag == nfrags) {
		pl = msdu.get_nbytes() % frag_thresh;
		if (!pl) pl = frag_thresh;
	}
	else {
		pl = frag_thresh;
	}

	tx_mode = term->get_current_mode(msdu.get_target(),pl);

	NAV = ptr2sch->now() + addba_rqst_duration(tx_mode);
	timestamp t = NAV + ACK_Timeout(tx_mode);

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
	<< ": send ADDBA request to " << *to << ", NAV = " << TXOPend
	<<". ACK timeout scheduled to " << t << endl;

	myphy->phyTxStartReq(MPDU(ADDBArqst, term, to, term->get_power(to, frag_thresh),
			tx_mode, TXOPend), true);

	ptr2sch->schedule(Event(t,(void*)&wrapper_to_ack_timed_out,(void*)this));

	BApckts.clear();
	pcktsToACK.clear();
	pcktsDur.clear();
	BArqstFlag = true;

	END_PROF("MAC::send_addba_rqst")
}

////////////////////////////////////////////////////////////////////////////////
//MAC_private::send_bar                                                      //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::send_bar(Terminal *to) {
	BEGIN_PROF("MAC::send_bar")

	NAV = ptr2sch->now() + bar_duration(tx_mode);
	timestamp t = NAV + BA_Timeout(tx_mode);

	MPDU bar = MPDU(BAR, term, to, term->get_power(to, frag_thresh),
			tx_mode, TXOPend);
	bar.set_pcks2ACK(pcktsToACK);
	myphy->phyTxStartReq(bar, true);

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
	<< ": send BAR to " << *to << ", NAV = " << TXOPend
	<<". BA timeout scheduled to " << t << endl;

	ptr2sch->schedule(Event(t,(void*)&wrapper_to_ba_timed_out,(void*)this));

	END_PROF("MAC::send_bar")
}

////////////////////////////////////////////////////////////////////////////////
//MAC_private::send_ba                                                      //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::send_ba(Terminal *to) {
	BEGIN_PROF("MAC::send_ba")

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
	<< ": send BA to " << *to << ", NAV = " << NAV << endl;

	MPDU ba = MPDU(BA, term, to, term->get_power(to, frag_thresh),
				rx_mode, NAV);
	ba.set_pcks2ACK(pcktsToACK);
	myphy->phyTxStartReq(ba, true);

	END_PROF("MAC::send_ba")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::send_data                                                     //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::send_data() {
	BEGIN_PROF("MAC::send_data")

	pcktsToACK.push_back(pck.get_id());
	pcktsDur.push_back(pck.get_duration());
	BApckts.push_back(msdu);

	NAV = ptr2sch->now() + pck.get_duration();
	if(BArqstFlag) {

		n_att_frags++;
		tx_data_rate += tx_mode_to_double(pck.get_mode());

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
						<< ": send " << pck << " during Block ACK session."
						<< endl;

		myphy->phyTxStartReq(pck,true);

		timestamp t = ptr2sch->now() + SIFS + pck.get_duration();

		if(t < time_to_send_BAR) {
			if(n_att_frags == nfrags)	new_msdu();
			else ptr2sch->schedule(Event(t,(void*)&wrapper_to_tx_attempt,
					(void*)this));
		}

	} else {
		timestamp t = NAV + ACK_Timeout(pck.get_mode());

		n_att_frags++;
		tx_data_rate += tx_mode_to_double(pck.get_mode());

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": send " << pck << ". ACK timeout scheduled for "
				<< t << endl;

		myphy->phyTxStartReq(pck,true);

		ptr2sch->schedule(Event(t,(void*)&wrapper_to_ack_timed_out,(void*)this));
	}

	END_PROF("MAC::send_data")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::startTXOP                                                     //
//                                                                            //
// Start TXOP time counting                                //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::start_TXOP() {
	BEGIN_PROF("MAC::start_TXOP")

		if(!TXOPflag && TXOPmax != 0){ // If not during TXOP and AC has a TXOP

			TXOPflag = true;

			myphy->cancel_notify_busy_channel();

			unsigned count = 0;
		    unsigned auxNfrags = 0;
			unsigned lastpl = 0;

			timestamp now = ptr2sch->now();
			timestamp auxTXOPend = now;

			transmission_mode which_mode = term->get_current_mode(msdu.get_target(),frag_thresh);

			// TXOPend must account for RTS/CTS exchanged in the beginning of TXOP
			TXOPend = now + rts_duration + cts_duration + SIFS;

			if(BAFlag){
				TXOPend = TXOPend + addba_rqst_duration(which_mode) + addba_rsps_duration(which_mode) +
						timestamp(2)*ack_duration(which_mode) + bar_duration(which_mode) +
						ba_duration(which_mode) + timestamp(5)*SIFS;
			}

			power_dBm = term->get_power(msdu.get_target(), frag_thresh);

			while(TXOPend < now + TXOPmax && count < packet_queue[myAC].size()){

				auxTXOPend = TXOPend;

				MSDU auxmsdu = (packet_queue[myAC])[count];

				// Determine number of fragments
				auxNfrags = auxmsdu.get_nbytes() / frag_thresh;
				if (auxmsdu.get_nbytes()%frag_thresh) ++auxNfrags;

				// determine packet and duration of last fragment
				lastpl = auxmsdu.get_nbytes() % frag_thresh;
				if (!lastpl) lastpl = frag_thresh;

				DataMPDU auxpck (frag_thresh, term, auxmsdu.get_target(),power_dBm,
						which_mode);
				DataMPDU auxpckLast(lastpl, term, auxmsdu.get_target(), power_dBm,
						which_mode);

				TXOPend = TXOPend + auxpckLast.get_duration();

				if(BAFlag) {
					TXOPend = TXOPend + timestamp(auxNfrags)*SIFS;
				} else {
					TXOPend = TXOPend + timestamp(auxNfrags)*ack_duration(which_mode) + timestamp(2*auxNfrags)*SIFS;
				}

				if(auxNfrags != 1){
					// Update TXOPend accordingly
					TXOPend = TXOPend + timestamp(auxNfrags-1)*auxpck.get_duration();
				}

				//If an RTS/CTS is needed:
				// For not the last packet
				if(auxNfrags != 1 && auxpck.get_nbytes_mac() >= RTS_threshold){
					TXOPend = TXOPend + timestamp(auxNfrags-1)*(rts_duration + cts_duration +
							SIFS + 1);
				}
				// For the last packet
				if(auxpckLast.get_nbytes_mac() >= RTS_threshold) {
					TXOPend = TXOPend + rts_duration + cts_duration + SIFS;
				}
				count++;
			}

			if(TXOPend > now + TXOPmax) {
				TXOPend = auxTXOPend;
			}

			TXOPend = TXOPend + 1;

			if (logflag) *mylog << "\n >> " << ptr2sch->now() << "sec., " << *term
					<< ", of Access Category " << myAC << " begins TXOP scheduled to end at "
					<< TXOPend << "sec." << "\nPackets in queue = " << count << ". TXOP duration = "
					<< TXOPend - now << " sec." << endl;

			myphy->phyTxStartReq(MPDU(RTS,term,msdu.get_target(),power_dBm,M6,TXOPend),
					true);

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
					<< ": send TXOP RTS to " << *msdu.get_target() << endl;

			NAV = ptr2sch->now() + rts_duration;
			timestamp t = NAV + CTS_Timeout;
			ptr2sch->schedule(Event(t,(void*)&wrapper_to_cts_timed_out,(void*)this));

			time_to_send_BAR = TXOPend - bar_duration(which_mode) - ba_duration(which_mode)
					- timestamp(1)*SIFS;

		} else { // If already during TXOP or station does not have TXOP
			transmit();
		}

	END_PROF("MAC::start_TXOP")
}
////////////////////////////////////////////////////////////////////////////////
// MAC_private::end_TXOP                                                      //
//                                                                            //
// Finishes counting the TXOP time			                                  //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::end_TXOP() {
	BEGIN_PROF("MAC::end_TXOP")

	TXOPflag = false;
	TXOPend = timestamp(0);

	if (logflag) *mylog << "\n >> " << ptr2sch->now() << "sec., " << *term
		<< ", of Access Category " << myAC << " ends TXOP." << endl;

	switch (TXOPla_win) {
	case success:
		/*
		 * If all ACKs were received correctly, by the end of TXOP all fragments will have
		 * been transmitted, thus the lastfrag is set to true in
		 * term->la_success(msdu.get_target(), true) below
		 */
		term->la_success(msdu.get_target(), true);
		break;
	case ACKfail:
		term->la_failed(msdu.get_target()); // link adaptation
		break;
	default:
		term->la_rts_failed(msdu.get_target());
		break;
	}

	TXOPla_win = success;

	END_PROF("MAC::end_TXOP")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::requeue_pcks                                                  //
//                                                                            //
// Requeues unacknowledged packets			                                  //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::requeue_pcks(MPDU ba) {
	BEGIN_PROF("MAC::requeue_pcks")

		timestamp auxDur = ba.get_duration() + SIFS;

		for(int k = pcktsToACK.size() - 1; k >= 0; k--) {
			if(find((ba.get_pcks2Ack()).begin(), (ba.get_pcks2Ack()).end(), pcktsToACK[k])
					== (ba.get_pcks2Ack()).end()) {
				if (logflag) *mylog << "\n >> " << ptr2sch->now() << "sec., " << *term
						<< ". Packet " << pcktsToACK[k] << " not acknowledged. Requeue packet."
						<< endl;
				packet_queue[myAC].push_front(BApckts[k]);
			} else {
				term->macUnitdataStatusInd(BApckts[k], auxDur);
			}
			auxDur = auxDur + pcktsDur[k] + SIFS;
		}

	new_msdu();

	END_PROF("MAC::requeue_pcks")
}


////////////////////////////////////////////////////////////////////////////////
// MAC_private::internal_contention                       	                  //
//                                                                            //
// resolves internal competition between ACs			                      //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::internal_contention()	{
BEGIN_PROF("MAC::internal_contention")

	timestamp TTT_ACs[5];
	timestamp minTTT = timestamp_max();
	int minTTT_idx = 0;

	// This log line is distributed in multiple code lines
	if(logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
			<< ": BOCs and TTTs for all ACs: \nAC      BOC  TTT";

	// Loop through all ACs, to define which AC will gain access to the WM
	for(int k = 0; k < 5; k++)	{
		accCat auxAC = allACs[k];

		if(packet_queue[auxAC].size()) { // If there are packets in that AC's queue

			// Change to temporary AC to calculate TTT
			set_myAC(auxAC);
			if(BOC_flag[auxAC]) { // If it is a new packet and the BOC has not yet been calculated
				// Then calculate it
				CW_ACs[auxAC] = aCWmin;
				BOC_ACs[auxAC] = randgen->discrete_uniform(0,CW_ACs[auxAC]-1);
				BOC_flag[auxAC] = false;
			}

			// Calculate Time To Transmit
			TTT_ACs[k] = ptr2sch->now() + AIFS + timestamp(BOC_ACs[auxAC]) * aSlotTime;

			if(logflag) *mylog << "\n" << auxAC << "   " << BOC_ACs[auxAC]
					<< "   " << TTT_ACs[k];

			// If calculated TTT is less than the smallest TTT
			if(TTT_ACs[k] <= minTTT) {
				minTTT_idx = k;
				minTTT = TTT_ACs[k];
			}
		}
	}

	if(logflag) *mylog << endl;

	// Loop to determine if two ACs gained simultaneous access
	for(int k = 0; k < 5; k++) {
		accCat auxAC = allACs[k];

		// If calculated TTT is equal to the smallest one
		if(packet_queue[auxAC].size() && TTT_ACs[k] == minTTT && k != minTTT_idx) {

			if(logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
							<< ": simultaneous access between " << allACs[minTTT_idx]
							<< " and "<< (accCat)auxAC << "." <<endl;

			// Change to temporary AC to calculate BOC
			set_myAC(auxAC);

			// Double contention window for that AC
			if (CW_ACs[auxAC] <= aCWmax/2) CW_ACs[auxAC] = CW_ACs[auxAC] * 2;

			BOC_flag[auxAC] = false;

			// Recalculate BOC
			BOC_ACs[auxAC] = randgen->discrete_uniform(0,CW_ACs[auxAC]-1);

			// Calculate Time To Transmit
			TTT_ACs[k] = ptr2sch->now() + AIFS + timestamp(BOC_ACs[auxAC]) * aSlotTime;

			if(logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
					<< "new AC " << (accCat)auxAC << "BOC and TTT: "
					<< BOC_ACs[auxAC] << "	" << TTT_ACs[k] << " sec." << endl;

			// If calculated TTT is less than the smallest TTT
			if(TTT_ACs[k] <= minTTT) {
				minTTT_idx = k;
				minTTT = TTT_ACs[k];
			}
		}
	}

	if(logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
			<< ": packet of Acess Category " << (accCat)(allACs[minTTT_idx])
			<< " will be transmitted." << endl;

	// Loop to redefine BOCs
	for(int k = 0; k < 5; k++) {
		accCat auxAC = allACs[k];
		if(packet_queue[auxAC].size() && k != minTTT_idx) {
			timestamp time_diff = TTT_ACs[k] - TTT_ACs[minTTT_idx];
			if (time_diff < timestamp(BOC_ACs[auxAC]) * aSlotTime) {
				BOC_ACs[auxAC] = time_diff / aSlotTime;
			}
		}
	}

	set_myAC(allACs[minTTT_idx]);

END_PROF("MAC::internal_contention")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::transmit                                                      //
//                                                                            //
// send data packet if basic DCF or begin RTS                                 //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::transmit() {
	BEGIN_PROF("MAC::transmit")

	// determine packet length
	unsigned pl;
	if (current_frag == nfrags) {
		pl = msdu.get_nbytes() % frag_thresh;
		if (!pl) pl = frag_thresh;
	}
	else {
		pl = frag_thresh;
	}

	transmission_mode which_mode = term->get_current_mode(msdu.get_target(),pl);
	power_dBm = term->get_power(msdu.get_target(), frag_thresh);

	myphy->cancel_notify_busy_channel();

	DataMPDU auxpck (pl, term, msdu.get_target(), power_dBm, which_mode);

	if (auxpck.get_nbytes_mac() < RTS_threshold) { // Basic DCF protocol, without RTS/CTS exchange

		timestamp auxnav;
		if(TXOPflag) auxnav = TXOPend;
		else auxnav = ptr2sch->now() + auxpck.get_duration() + SIFS + ack_duration(which_mode);

		pck = DataMPDU (msdu, pl, current_frag, nfrags, power_dBm, which_mode,
				auxnav);

		countdown_flag = false;

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": begin transmission, start with fragment "
				<< current_frag << " of " << nfrags
				<< " with " << pl << " bytes and rate " << which_mode
				<< ". Basic DCF protocol, transmit data packet "
				<< pck.get_id() << " now" << endl;

		send_data();

	} else {

		// RTS/CTS Protocol
		NAV = ptr2sch->now() + rts_duration;

		timestamp newnav;
		if(TXOPflag) {
			newnav = TXOPend;
		} else if (current_frag == nfrags) { // If this is the last fragment:
			newnav = ptr2sch->now() + rts_duration + cts_duration +
					auxpck.get_duration() + ack_duration(which_mode) + 3*SIFS + 1;
		} else { // If there are other fragments after this one
			newnav = ptr2sch->now() + rts_duration + cts_duration +
					2*auxpck.get_duration() + 2*ack_duration(which_mode) +
					5*SIFS + 1;
		}

		pck = DataMPDU (msdu, pl, current_frag, nfrags, power_dBm, which_mode,
				newnav);

		timestamp t = NAV + CTS_Timeout;

		if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
				<< ": begin transmission, start with fragment "
				<< current_frag << " of " << nfrags
				<< " with " << pl << " bytes and rate " << which_mode
				<< ". RTS/CTS protocol, transmit RTS for data packet "
				<< pck.get_id() << "\n    Packet takes " << rts_duration
				<< ", CTS timeout scheduled for " << t
				<< ", NAV set to " << NAV << endl;

		newnav = ptr2sch->now() + rts_duration + cts_duration +
				auxpck.get_duration() + ack_duration(which_mode) +
				3*SIFS + 1;

		myphy->phyTxStartReq(MPDU(RTS,term,msdu.get_target(),power_dBm,M6,newnav),
				true);

		countdown_flag = false;

		ptr2sch->schedule(Event(t,(void*)&wrapper_to_cts_timed_out,(void*)this));
	}

	END_PROF("MAC::transmit")
}


////////////////////////////////////////////////////////////////////////////////
// MAC::macUnitdataReq                                                        //
//                                                                            //
// attempt to transmit MSDU 'p', put it in queue                              //
////////////////////////////////////////////////////////////////////////////////
unsigned MAC::macUnitdataReq(MSDU p) {

	if (get_queue_size() >= max_queue_size) {
		term->macUnitdataQueueOverflow(p);
	} else {
		accCat auxAC = term->get_connection_AC(p.get_target());
		packet_queue[auxAC].push_back(p);

		if (get_queue_size() == 1) new_msdu();
	}

	return get_queue_size();
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::tx_attempt                                                    //
//                                                                            //
// begins contention for new MSDU or for new train of fragments               //
////////////////////////////////////////////////////////////////////////////////
void MAC_private::tx_attempt() {
	BEGIN_PROF("MAC_private::tx_attempt")

    // fragmentation
	if (current_frag == 0) {
			nfrags = msdu.get_nbytes() / frag_thresh;
		if (msdu.get_nbytes()%frag_thresh) ++nfrags;

	current_frag = 1;

	if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
		<< ": transmission attempt of new packet with "
		<< msdu.get_nbytes() << " bytes and " << nfrags
		<< " fragments of at most " << frag_thresh << " bytes"
		<< endl;
	}

	if(TXOPflag) {

		transmit();

	} else {

		// check NAV
		if (ptr2sch->now() <= NAV) {

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
					<< ": transmission attempt"
					<< ", Channel is busy according to NAV (" << NAV
					<< ")\n    reschedule tx attempt to " << NAV+1 << endl;

			ptr2sch->schedule(Event(NAV+1,(void*)&wrapper_to_tx_attempt,(void*)this));

			// verify if channel is busy
		} else if (myphy->carrier_sensing()) {

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
					<< ": transmission attempt, Channel is busy"
					<< ", ask PHY to notify when it is free" << endl;

			// if channel is busy, ask channel when it is free
			myphy->notify_free_channel();
		} else {

			if (logflag) *mylog << "\n" << ptr2sch->now() << "sec., " << *term
					<< ": transmission attempt, Channel is free" << endl;


			begin_countdown();
		}
	}
	END_PROF("MAC::tx_attempt")
}

////////////////////////////////////////////////////////////////////////////////
// MAC_private::get_queue_size												  //
//                                                                            //
// returns size of packet queue              							      //
////////////////////////////////////////////////////////////////////////////////
size_t MAC_private::get_queue_size()	{
	size_t queSize = 0;
	for(int k = 0; k < 5; k++){
		accCat auxAC = allACs[k];
		queSize = queSize + packet_queue[auxAC].size();
	}
	return queSize;
}

// Output operator << for accCat type
ostream& operator<<(std::ostream& os, const accCat& AC) {
   switch(AC){
   case AC_BK:
	   return os << "AC_BK";
   case AC_BE:
   	   return os << "AC_BE";
   case AC_VI:
   	   return os << "AC_VI";
   case AC_VO:
   	   return os << "AC_VO";
   case legacy:
   	   return os << "legacy";
   default:
	   return os << "UNDEFINED AC";
   }
}
