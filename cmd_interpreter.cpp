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
	if (commandString.find(' ') == std::string::npos)					
	{													
		/* Command is a single word command */
		if (commandString == COMM[Command::COM_PING])
		{
			if(peer.remoteEp.address().is_v4())
				CmdInterpreter::ping(peer, peer.peerEntry.Ev4);
			else
				CmdInterpreter::ping(peer, peer.peerEntry.Ev6);
		}
		else if (commandString == COMM[Command::COM_ADD])
		{
			if (peer.remoteEp.address().is_v4())
				CmdInterpreter::add(peer, peer.peerEntry.Ev4);
			else
				CmdInterpreter::add(peer, peer.peerEntry.Ev6);
		}
		else
		{
			peer.writeBuffer = RESP[Response::BAD_COMMAND];
		}
	}
	else
	{	
		/* Command is a multi word command */
		peer.writeBuffer = RESP[Response::BAD_COMMAND];
	}
	peer._sendPeerData();
}
