#include "cmd_interpreter.h"
#include "log.h"

void CmdInterpreter::processCommand(Peer& peer)
{
	auto commandString = std::string_view{ peer.dataBuffer };
	if (commandString.find(' ') == std::string::npos)
	{
		if (commandString == Command::COM_PING)
			CmdInterpreter::ping(peer);
		else
			peer.writeBuffer = Response::BAD_COMMAND;
	}
	else
	{
		peer.writeBuffer = Response::BAD_COMMAND;
	}
	peer._sendPeerData();
}

void CmdInterpreter::ping(Peer& peer)
{
	if (peer.remoteEp.address().is_v4())
	{
		peer.writeBuffer += peer.peerEntry.Ev4->versionID + " ";
		peer.writeBuffer += peer.peerEntry.Ev4->ipAddress + " ";
		peer.writeBuffer += peer.peerEntry.Ev4->portNumber;
	}
	else
	{
		peer.writeBuffer += peer.peerEntry.Ev6->versionID + " ";
		peer.writeBuffer += peer.peerEntry.Ev6->ipAddress + " ";
		peer.writeBuffer += peer.peerEntry.Ev6->portNumber;
	}
}