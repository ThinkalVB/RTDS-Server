#include "cmd_interpreter.h"
#include "directory.h"
#include "log.h"

const std::string CmdInterpreter::RESP[] = 
{
	"redudant_data",
	"ok_success",
	"no_privilege",
	"bad_command",
	"bad_param",
	"no_exist",
	"wait_retry",
};

const std::string CmdInterpreter::COMM[] =
{
	"ping",
	"add",
	"search",
	"ttl",
	"charge",
	"update",
	"remove",
	"count",
	"mirror",
	"leave",
	"exit"
};

void CmdInterpreter::processCommand(Peer& peer)
{
	auto commandString = std::string_view{ peer.dataBuffer };
	auto commEnd = commandString.find(' ');
	if (commEnd == std::string::npos)										/* Command is a single word command */	
	{													
		if (commandString == COMM[(short)Command::PING])
			_ping(peer);
		else if (commandString == COMM[(short)Command::EXIT])
			peer.peerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);
		else if (commandString == COMM[(short)Command::MIRROR])
		{}	// To be implimented
		else if (commandString == COMM[(short)Command::COUNT])
		{}	// To be implimented
		else if (commandString == COMM[(short)Command::LEAVE])
		{}	// To be implimented
	}



	else
	{	
		/* Command is a multi word command */
		peer.writeBuffer = RESP[(short)Response::BAD_COMMAND];
	}
	peer._sendPeerData();
}

inline void CmdInterpreter::_ping(Peer& peer)
{
	if (peer.remoteEp.address().is_v4())
		__ping(peer, peer.peerEntry.Ev4);
	else
		__ping(peer, peer.peerEntry.Ev6);
}
