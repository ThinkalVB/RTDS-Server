#include "peer.h"

std::string_view Peer::getCommandString() const
{
	return mDataBuffer.getStringView();
}

asio::mutable_buffer Peer::getReadBuffer()
{
	return mDataBuffer.getReadBuffer();
}

asio::mutable_buffer Peer::getSendBuffer()
{
	return mDataBuffer.getSendBuffer();
}

bool Peer::cookString(std::size_t noOfStrBytes)
{
	return mDataBuffer.cookString(noOfStrBytes);
}
