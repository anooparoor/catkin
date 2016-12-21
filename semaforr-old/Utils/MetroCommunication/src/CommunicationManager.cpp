/* communication_manager.cpp
 *
 * Copyright 2013 HRTeam.
 * License:
 * Created: May 30, 2013
 * Original authors: Eric Schneider <nitsuga@pobox.com>
 */

#include "CommunicationManager.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <list>

#include <boost/ref.hpp>
#include <boost/thread.hpp>

#define COMMUNICATION_MANAGER_DEBUG false

CommunicationManager::CommunicationManager(std::string type_id,
                                           std::string name_id,
                                           std::string cs_host,
                                           uint32_t cs_port)
    : type_id(type_id),
    name_id(name_id),
    cs_host(cs_host),
    cs_port(cs_port),
    io_service(),
    socket(boost::ref(io_service)) {
        // Initialize robot capabilities.
        init_provides();

        // Initialize default state.
        init_state();
    }

CommunicationManager::~CommunicationManager() {
    disconnect();
}

// Update the state machine
void CommunicationManager::operator()() {

  while (current_state != STATE_QUIT) {
    update();

    if(boost::this_thread::interruption_requested() &&
       queues_are_empty() ) {
      return;
    }

    usleep(10000);  // 1ms sleep is sort of a convention round here
  }
}

bool CommunicationManager::is_comm_alive() const {
    return socket.is_open();
}

// Don't provide anything by default. A robot might provide e.g. "position2d"
void CommunicationManager::init_provides() {
}

bool CommunicationManager::connect() {
    // Prepend function signature to error messages.
    static const std::string signature = "CommunicationManager::connect()";

    // Make sure that the socket isn't already open.
    if (socket.is_open()) {
        disconnect();
    }

    boost::system::error_code ec;  // Used to check for errors.

    // Get the IP address of the host.
    boost::asio::ip::address addr = boost::asio::ip::address::from_string(cs_host, ec);
    if (ec) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failed to retrieve host's IP address\n", signature.c_str());
        return false;
    }

    // Connect to the host.
    boost::asio::ip::tcp::endpoint endpt(addr, cs_port);
    socket.connect(endpt, ec);
    if (ec) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failed to connect to host\n", signature.c_str());
        return false;
    }

    // Go back to the initial state.
    init_state();
    return true;
}

void CommunicationManager::disconnect() {
    // Prepend function signature to error messages.
    static const std::string signature = "CommunicationManager::Disconnect()";

    // Make sure that the socket is already open.
    if (!socket.is_open())
        return;

    boost::system::error_code ec;  // Used to check for errors.

    // Shutdown the connection.
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    if (ec) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failed to disconnect\n", signature.c_str());
    }

    // Close the socket.
    socket.close(ec);
    if (ec) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failed to close the socket\n", signature.c_str());
    }
}

void CommunicationManager::update() {
    // Prepend function signature to error messages.
    static const std::string signature = "CommunicationManager::update()";

    // Make sure that the socket is already open.
    if (!is_comm_alive()) {
        return;
    }

    // Maintain the state machine.
    switch (current_state) {
        case STATE_INIT:
            do_state_action_init();
            break;
        case STATE_ACK:
            do_state_action_ack();
            break;
        case STATE_LISTEN:
            do_state_action_listen();
            break;
        case STATE_PING_SEND:
            do_state_action_ping_send();
            break;
        case STATE_PONG_READ:
            do_state_action_pong_read();
            break;
        case STATE_PONG_SEND:
            do_state_action_pong_send();
            break;
        default:
            if ( COMMUNICATION_MANAGER_DEBUG ) {
                fprintf(stderr, "%s - unrecognized state\n", signature.c_str());
            }
            do_state_change( STATE_QUIT );
            break;
    }
}  //  end switch

void CommunicationManager::init_state() {
    current_state = STATE_INIT;
    session_id = -1;
}

bool CommunicationManager::message_waiting() const {
    // Prepend function signature to error messages.
    static const std::string signature = "CommunicationManager::messsage_waiting()";

    // Make sure that the socket is already open.
    if (!is_comm_alive()) {
        return false;
    }

    boost::system::error_code ec;  // Used to check for errors.

    // Check the socket.
    size_t payload = socket.available(ec);
    if (ec) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failed to peek at the socket\n", signature.c_str());
        return false;
    } else if (payload >= sizeof(cmd_len_t)) {
        return true;
    } else {
        return false;
    }
}

std::string CommunicationManager::read() {
    // Prepend function signature to error messages.
    static const std::string signature = "CommunicationManager::Read()";

    // Make sure that the socket is already open.
    if (!is_comm_alive()) {
        return "";
    }

    // Read the message.
    char *len = (char*) malloc(1);
    char *msg = (char*) malloc(255);

    int rc = recv(socket.native(), len, 1, 0);
    if(rc == -1) {
      fprintf(stderr, "%s: error in recv() [length byte]: %s\n", signature.c_str(), strerror (errno ));
      return "";
    }

    rc = recv(socket.native(), msg, *len, 0);
    if(rc == -1) {
      fprintf(stderr, "%s: error in recv() [message body]: %s\n", signature.c_str(), strerror (errno ));
      return "";
    }

    std::string str(msg);
    str.resize((unsigned char)*len);
    free(len);
    free(msg);
    return str;
}

// Edited on Feb. 17, 2014. created by Ofear
bool CommunicationManager::write(const std::stringstream& ss) {
    // Prepend function signature to error messages.
    static const std::string signature = "CommunicationManager::Write()";

    //int supposed,actual;
    int rc;
    std::string str = ss.str();

    // Make sure that the socket is already open.
    if (!is_comm_alive())
        return false;

    char *msg = (char*)malloc(256);
    if(str.size() > 255) return false; // Just quit for now if message is too long
    msg[0] = (unsigned char)str.size();

    strncpy((msg+1), str.c_str(), str.size());
    msg[str.size()+1] = '\0';
    rc = send(socket.native(), msg, strlen(msg), MSG_NOSIGNAL);
    if(rc == -1) {
      fprintf(stderr, "%s: error in send() [msg]: %s\n", signature.c_str(), strerror(errno));
      free(msg);
      return false;
    }

    free(msg);
    silence_timer.start();
    return true;
}

void CommunicationManager::do_state_change(int state) {

    if (state == STATE_INIT) {
        init_state();
        init_provides();
    }

    current_state = state;
}

void CommunicationManager::do_state_action_init() {
    // Prepend function signature to error messages.
    static const std::string signature =
        "CommunicationManager::do_state_action_init()";

    // Send the INIT command.
    std::stringstream ss;
    ss << CMD_INIT << " " << type_id << " " << name_id;

    ss << " " << provides_list.size();

    for (unsigned int i = 0; i < provides_list.size(); i++) {
        ss << " " << provides_list[i];
    }

    if (write(ss)) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - success; next state: STATE_ACK\n", signature.c_str());
        do_state_change (STATE_ACK);
    } else if (state_timer.elapsed() >= MAX_TIME_STATE) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - timeout; next state: STATE_QUIT\n", signature.c_str());
        do_state_change (STATE_QUIT);
    } else {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failure; next state: STATE_INIT\n", signature.c_str());
        do_state_change (STATE_INIT);
    }
}

void CommunicationManager::do_state_action_ack() {
    // Prepend function signature to error messages.
    static const std::string signature =
        "CommunicationManager::do_state_action_ack()";

    // Don't wait forever.
    if (state_timer.elapsed() >= MAX_TIME_STATE) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - timeout; next state: STATE_INIT\n", signature.c_str());
        do_state_change (STATE_INIT);
    }

    // Prepare to read the command.
    std::string cmd;
    uint64_t tmp_session_id;
    std::string read_string = this->read();
    std::stringstream ss(read_string);
    if (read_string != "" && (ss >> cmd >> tmp_session_id)
        && (cmd.find(CMD_ACK) != std::string::npos)) {
        if (tmp_session_id < 0) {
            session_id = -1;
            if (COMMUNICATION_MANAGER_DEBUG)
                fprintf(stderr, "%s - rejected; next state: STATE_QUIT\n", signature.c_str());
            do_state_change (STATE_QUIT);
        } else {
            session_id = tmp_session_id;
            if (COMMUNICATION_MANAGER_DEBUG)
                fprintf(stderr, "%s - accepted; next state: STATE_LISTEN\n", signature.c_str());
            do_state_change (STATE_LISTEN);
        }
    } else {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failure; next state: STATE_INIT\n", signature.c_str());
        do_state_change (STATE_INIT);
    }
}

void CommunicationManager::do_state_action_listen() {
    // Prepend function signature to error messages.
    static const std::string signature =
        "CommunicationManager::do_state_action_listen()";

    std::stringstream out_stream;

    while (message_waiting()) {
        boost::mutex::scoped_lock in_lock(in_queue_mutex);

        std::string message_string(read().c_str());
	if (COMMUNICATION_MANAGER_DEBUG)
	  fprintf(stderr, "FROM READ: %s\n", message_string.c_str());
        in_queue.push_back(message_string);
    }

    while (!out_queue.empty()) {
        boost::mutex::scoped_lock out_lock(out_queue_mutex);

        std::string out_message = out_queue.front();
        out_queue.pop_front();

        out_stream.str(out_message);
        write(out_stream);

        out_stream.str("");
    }

    if (silence_timer.elapsed() >= MAX_TIME_SILENCE) {
        // Don't let the connection die.
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - silence timeout; next state: STATE_PING_SEND\n",
                    signature.c_str());
        do_state_change (STATE_PING_SEND);
    }
}

void CommunicationManager::do_state_action_ping_send() {
    // Prepend function signature to error messages.
    static const std::string signature =
        "CommunicationManager::do_state_action_ping_send()";

    // Send the PING command.
    std::stringstream ss;
    ss << CMD_PING;
    if (write(ss)) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - success; next state: STATE_PONG_READ\n", signature.c_str());
        do_state_change (STATE_PONG_READ);
    } else if (state_timer.elapsed() >= MAX_TIME_STATE) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - timeout; next state: STATE_INIT\n", signature.c_str());
        do_state_change (STATE_INIT);
    } else {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failure; next state: STATE_PING_SEND\n", signature.c_str());
        do_state_change (STATE_PING_SEND);
    }
}

void CommunicationManager::do_state_action_pong_read() {
    // Prepend function signature to error messages.
    static const std::string signature =
        "CommunicationManager::do_state_action_pong_read()";

    // Don't wait forever.
    if (silence_timer.elapsed() >= MAX_TIME_SILENCE) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - silence timeout; next state: STATE_PING_SEND\n",
                    signature.c_str());
        do_state_change (STATE_LISTEN);
    }

    // Don't block while waiting for the command.
    if (! message_waiting())
        return;

    // Prepare to read the command.
    std::string read_string(this->read());
    std::stringstream ss(read_string);
    std::string cmd;
    if ( read_string != "" && (ss >> cmd) && (cmd.find(CMD_PONG) != std::string::npos) ) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - success; next state: STATE_IDLE\n", signature.c_str());
        do_state_change (STATE_LISTEN);
    } else {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failure; next state: STATE_PING_SEND\n", signature.c_str());
        do_state_change (STATE_PING_SEND);
    }
}

void CommunicationManager::do_state_action_pong_send() {
    // Prepend function signature to error messages.
    static const std::string signature =
        "CommunicationManager::do_state_action_pong_send()";

    // Send the PONG command.
    std::stringstream ss;
    ss << CMD_PONG;
    if (write(ss)) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - success; next state: STATE_IDLE\n", signature.c_str());
        do_state_change (STATE_LISTEN);
    } else if (state_timer.elapsed() >= MAX_TIME_STATE) {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - silence timeout; next state: STATE_PING_SEND\n",
                    signature.c_str());
        do_state_change (STATE_PING_SEND);
    } else {
        if (COMMUNICATION_MANAGER_DEBUG)
            fprintf(stderr, "%s - failure; next state: STATE_PONG_SEND\n", signature.c_str());
        do_state_change (STATE_PONG_SEND);
    }
}

std::string CommunicationManager::next_message() {

    // Lock the in queue
    boost::mutex::scoped_lock in_lock(this->in_queue_mutex);

    if (!this->in_queue.empty()) {
        std::string front(in_queue.front().c_str());
        this->in_queue.pop_front();
        return front;
    }

    return "";
}

uint64_t CommunicationManager::get_session_id() {
  return session_id;
}

void CommunicationManager::send_message(std::string msg) {

    // Lock the out queue
    boost::mutex::scoped_lock out_lock(out_queue_mutex);

    //  out_queue.push_front(msg);
    out_queue.push_back(msg);
}

bool CommunicationManager::queues_are_empty() {
    boost::mutex::scoped_lock in_lock(in_queue_mutex);
    boost::mutex::scoped_lock out_lock(out_queue_mutex);

    return (in_queue.empty() && out_queue.empty());
}

void CommunicationManager::print_time() {
    time_t rawtime;
    struct tm *tm;
    time ( &rawtime );
    tm = localtime ( &rawtime );
    // get only the milliseconds
    struct timeval tv;
    gettimeofday( &tv, NULL );
    char time_buffer[] = "00:00:00:000:000";
    sprintf( time_buffer, "%02d:%02d:%02d:%03d:%03d", tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec/1000, (int)tv.tv_usec%1000 );

    fprintf(stderr, "%s", time_buffer);
}