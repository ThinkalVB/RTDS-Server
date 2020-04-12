#pragma once
#include <boost/asio.hpp>
#include "SPentry.h"
#include <list>

using namespace boost;
constexpr unsigned short RTDS_BUFF_SIZE = 300;

class Peer
{
	static std::list<Peer*> peerPtrContainer;	//!< All connected peers are kept in this container
	static std::mutex peerContainerLock;		//!< Lock Container before removing and adding elements(thread safety)

	asio::ip::tcp::socket* peerSocket;			//!< Socket handling the data from peer system
	asio::ip::tcp::endpoint remoteEp;			//!< Endpoint of the peerSocket with info on peer system
	SPentry peerEntry;							//!< Union DS that store the SourcePair entry(v4/v6) pointers

	char dataBuffer[RTDS_BUFF_SIZE];			//!< Buffer to which the commands are received
	std::string writeBuffer;					//!< Buffer from which the response will be send

	void _peerReceiveData();
	void _sendPeerData();

	void _processData(const boost::system::error_code&, std::size_t);
	void _sendData(const boost::system::error_code&, std::size_t);
public:
	Peer(asio::ip::tcp::socket*);
	static void _removeAllPeers();
	~Peer();

	friend class CmdInterpreter;
};